#include <array>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "afl/byml/reader.h"
#include "afl/result.h"
#include "afl/sarc.h"
#include "afl/types.h"
#include "afl/util.h"
#include "afl/vector.h"
#include "afl/yaz0.h"
#include "clipp/clipp.h"
#include "config.h"
#include "mini/ini.h"

namespace fs = std::filesystem;

enum class Game {
	SMO,
	SM3DW,
};

struct Query {
	const std::string name;
	const bool isRecurse = false;
};

struct Result {
	const std::string stageName;
	mutable std::array<bool, 15> scenarioFlag;
	const u32 scenarioIdx;
	const std::string itemList;

	const std::string unitConfigName;
	const std::string modelName;
	const std::string paramConfigName;
	const std::string objId;
	const Vector3f trans;
	const Vector3f rotate;
	const Vector3f scale;

	bool operator==(const Result& other) const;
};

template <>
struct std::hash<Result> {
	std::size_t operator()(const Result& res) const noexcept {
		size_t out = 0;
		util::hashCombine(out, res.stageName);
		util::hashCombine(out, res.unitConfigName);
		util::hashCombine(out, res.modelName);
		util::hashCombine(out, res.paramConfigName);
		util::hashCombine(out, res.objId);
		util::hashCombine(out, res.trans);
		util::hashCombine(out, res.rotate);
		util::hashCombine(out, res.scale);
		return out;
	}
};

bool Result::operator==(const Result& other) const {
	// return std::hash<Result>{}(*this) == std::hash<Result>{}(other);
	return util::isEqual(stageName, other.stageName) &&
	       util::isEqual(unitConfigName, other.unitConfigName) &&
	       util::isEqual(paramConfigName, other.paramConfigName) &&
	       util::isEqual(modelName, other.modelName) && util::isEqual(objId, other.objId) &&
	       trans.isEqual(other.trans) && scale.isEqual(other.scale) && rotate.isEqual(other.rotate);
}

bool endsWith(const std::string& fullString, const std::string& ending) {
	if (fullString.length() >= ending.length())
		return 0 ==
		       fullString.compare(fullString.length() - ending.length(), ending.length(), ending);
	else
		return false;
}

struct SearchEngine {
	SearchEngine(const Game& game, const Query& query) : mGame(game), mQuery(query) {}

	result_t searchAllStages(const fs::path& romfsPath);
	result_t searchBYML(const std::vector<u8> bymlContents);
	result_t searchStage(const fs::path& stagePath);
	result_t searchScenario(const byml::Reader& scenario);
	result_t saveResults(const fs::path& outPath) const;

	const Game mGame;
	const Query mQuery;
	std::vector<Result> mResults;
	std::string mCurStageName;
	u32 mCurScenarioIdx;
	std::string mCurItemList;
};

result_t readVec3f(Vector3f* out, const byml::Reader& reader, const std::string& name) {
	result_t r;

	byml::Reader vec;
	r = reader.getContainerByKey(&vec, name);
	if (r) return r;

	r = vec.getF32ByKey(&out->x, "X");
	if (r) return r;

	r = vec.getF32ByKey(&out->y, "Y");
	if (r) return r;

	r = vec.getF32ByKey(&out->z, "Z");
	if (r) return r;

	return 0;
}

result_t SearchEngine::searchScenario(const byml::Reader& scenario) {
	result_t r;

	for (u32 listIdx = 0; listIdx < scenario.getSize(); listIdx++) {
		u32 keyIdx;
		scenario.getKeyByIdx(&keyIdx, listIdx);
		std::string listName = scenario.getHashString(keyIdx);
		mCurItemList = listName;

		if (util::isEqual(listName, "FilePath") || util::isEqual(listName, "Objs")) continue;

		byml::Reader itemList;
		scenario.getContainerByIdx(&itemList, listIdx);

		for (u32 itemIdx = 0; itemIdx < itemList.getSize(); itemIdx++) {
			byml::Reader item;
			itemList.getContainerByIdx(&item, itemIdx);

			std::string unitConfigName;
			r = item.getStringByKey(&unitConfigName, "UnitConfigName");
			if (r) return r;

			byml::Reader unitConfig;
			r = item.getContainerByKey(&unitConfig, "UnitConfig");
			if (r) return r;

			std::string paramConfigName;
			r = unitConfig.getStringByKey(&paramConfigName, "ParameterConfigName");
			if (r) return r;

			std::string modelName;
			bool hasModelName = item.tryGetStringByKey(&modelName, "ModelName");

			if (util::isEqual(unitConfigName, mQuery.name) ||
			    util::isEqual(paramConfigName, mQuery.name) ||
			    (hasModelName && util::isEqual(modelName, mQuery.name))) {
				Vector3f trans;
				readVec3f(&trans, item, "Translate");
				Vector3f rotate;
				readVec3f(&rotate, item, "Rotate");
				Vector3f scale;
				readVec3f(&scale, item, "Scale");

				std::string objId;
				r = item.getStringByKey(&objId, "Id");
				if (r) return r;

				std::string optModelName = hasModelName ? modelName : "";
				std::array<bool, 15> scenarioFlag = { false };

				if (mGame == Game::SMO) scenarioFlag[mCurScenarioIdx] = true;

				Result result = { mCurStageName,   scenarioFlag,   mCurScenarioIdx,
					              mCurItemList,    unitConfigName, optModelName,
					              paramConfigName, objId,          trans,
					              rotate,          scale };
				mResults.push_back(result);
			}
		}
	}

	return 0;
}

result_t SearchEngine::searchBYML(const std::vector<u8> bymlContents) {
	result_t r;

	byml::Reader reader;
	r = reader.init(&bymlContents[0]);
	if (r) return r;

	if (!reader.isExistStringValue(mQuery.name)) return 0;

	if (mGame == Game::SMO) {
		for (u32 scenarioIdx = 0; scenarioIdx < reader.getSize(); scenarioIdx++) {
			mCurScenarioIdx = scenarioIdx;

			byml::Reader scenario;
			reader.getContainerByIdx(&scenario, scenarioIdx);

			r = searchScenario(scenario);
			if (r) return r;
		}
	} else if (mGame == Game::SM3DW) {
		searchScenario(reader);
	}

	return 0;
}

result_t SearchEngine::searchStage(const fs::path& stagePath) {
	result_t r;

	std::string stageName = stagePath.filename().stem().string();

	if (mGame == Game::SMO) {
		std::vector<u8> szsContents;
		r = util::readFile(szsContents, stagePath);
		if (r) return r;

		std::vector<u8> sarcContents;
		r = yaz0::decompress(sarcContents, szsContents);
		if (r) return r;

		SARC sarc(sarcContents);
		r = sarc.read();
		if (r) return r;

		std::vector<u8> bymlContents;
		r = sarc.getFileData(bymlContents, stageName + ".byml");
		if (r) return r;

		searchBYML(bymlContents);
	} else if (mGame == Game::SM3DW) {
		std::vector<u8> szsContents;
		r = util::readFile(szsContents, stagePath);
		if (r) return r;

		std::vector<u8> sarcContents;
		r = yaz0::decompress(sarcContents, szsContents);
		if (r) return r;

		SARC sarc(sarcContents);
		r = sarc.read();
		if (r) return r;

		const std::array<std::string, 3> suffixes = { "Map", "Design", "Sound" };
		const auto& filenames = sarc.getFilenames();

		for (const auto& suffix : suffixes) {
			const std::string bymlName = stageName + suffix + ".byml";
			if (!filenames.contains(bymlName)) continue;

			std::vector<u8> bymlContents;
			r = sarc.getFileData(bymlContents, bymlName);
			if (r) return r;

			searchBYML(bymlContents);
		}
	}

	return 0;
}

result_t SearchEngine::searchAllStages(const fs::path& romfsPath) {
	result_t r;

	const fs::path stageDataPath = romfsPath / "StageData";
	if (!fs::is_directory(stageDataPath)) return util::Error::DirNotFound;

	printf("searching...\n");

	// sort stage paths
	std::set<fs::path> stagePaths;
	for (const auto& entry : fs::directory_iterator(stageDataPath))
		stagePaths.insert(entry.path());

	for (const auto& stagePath : stagePaths) {
		std::string stageName = stagePath.filename().stem().string();
		mCurStageName = stageName;

		r = searchStage(stagePath);
		if (r) return r;
	}

	return 0;
}

result_t SearchEngine::saveResults(const fs::path& outPath) const {
	if (mResults.size() == 0) {
		printf("found no matches\n");
		return 0;
	}

	FILE* f = fopen(outPath.string().c_str(), "w");

	if (!f) {
		fprintf(stderr, "error: could not create file %s\n", outPath.string().c_str());
		return util::Error::FileError;
	}

	if (mGame == Game::SMO) {
		std::unordered_set<Result> collapsedResults;
		for (const auto& result : mResults) {
			if (collapsedResults.contains(result))
				collapsedResults.find(result)->scenarioFlag[result.scenarioIdx] = true;
			else
				collapsedResults.insert(result);
		}

		printf("found %zu matches\n", collapsedResults.size());

		std::map<std::string, std::vector<Result>> stages;
		for (const auto& result : collapsedResults) {
			if (stages.contains(result.stageName))
				stages[result.stageName].push_back(result);
			else
				stages[result.stageName] = std::vector<Result> { result };
		}

		for (const auto& [stageName, results] : stages) {
			fprintf(f, "%s:\n", stageName.c_str());
			for (const auto& result : results) {
				fprintf(f, "\tUnitConfigName: %s\n", result.unitConfigName.c_str());
				if (!util::isEqual(result.unitConfigName, result.modelName) &&
				    !result.modelName.empty())
					fprintf(f, "\tModelName: %s\n", result.modelName.c_str());
				if (!util::isEqual(result.unitConfigName, result.paramConfigName) &&
				    !result.paramConfigName.empty())
					fprintf(f, "\tParameterConfigName: %s\n", result.paramConfigName.c_str());
				fprintf(
					f, "\tTranslate: (%.3f, %.3f, %.3f)\n", result.trans.x, result.trans.y,
					result.trans.z
				);
				fprintf(f, "\tId: %s\n", result.objId.c_str());
				fprintf(f, "\titem list: %s\n", result.itemList.c_str());
				fprintf(f, "\tscenarios: ");
				for (u32 i = 0; i < result.scenarioFlag.size(); i++)
					if (result.scenarioFlag[i]) fprintf(f, "%d, ", i + 1);
				fprintf(f, "\n\n");
			}
		}
	} else if (mGame == Game::SM3DW) {
		printf("found %zu matches\n", mResults.size());

		std::map<std::string, std::vector<Result>> stages;
		for (const auto& result : mResults) {
			if (stages.contains(result.stageName))
				stages[result.stageName].push_back(result);
			else
				stages[result.stageName] = std::vector<Result> { result };
		}

		for (const auto& [stageName, results] : stages) {
			fprintf(f, "%s:\n", stageName.c_str());
			for (const auto& result : results) {
				fprintf(f, "\tUnitConfigName: %s\n", result.unitConfigName.c_str());
				if (!util::isEqual(result.unitConfigName, result.modelName) &&
				    !result.modelName.empty())
					fprintf(f, "\tModelName: %s\n", result.modelName.c_str());
				if (!util::isEqual(result.unitConfigName, result.paramConfigName) &&
				    !result.paramConfigName.empty())
					fprintf(f, "\tParameterConfigName: %s\n", result.paramConfigName.c_str());
				fprintf(
					f, "\tTranslate: (%.3f, %.3f, %.3f)\n", result.trans.x, result.trans.y,
					result.trans.z
				);
				fprintf(f, "\tId: %s\n", result.objId.c_str());
				fprintf(f, "\titem list: %s\n", result.itemList.c_str());
				fprintf(f, "\n");
			}
		}
	}

	fclose(f);

	printf("saved results to %s\n", outPath.string().c_str());

	return 0;
}

s32 main(s32 argc, char** argv) {
	using namespace clipp;

	mINI::INIFile configFile(getConfigPath());
	mINI::INIStructure ini;
	bool readSuccess = configFile.read(ini);
	if (!readSuccess) generateDefaultConfig();

	std::string gameName;
	std::string romfsPath;
	std::string objectName;
	std::string outPath = "results.txt";

	// clang-format off

	bool isShowHelp = false;
	auto cli = (
		opt_value("game", gameName).doc("one of \"smo\" or \"3dw\""),
		option("-r", "--romfs").doc("path to game's romfs") & value("romfs path", romfsPath),
		option("-n", "--name").doc("name of object to search for") & value("name", objectName),
	    option("-o", "--output").doc("path to output file (default: results.txt)") & value("outfile", outPath),
	    option("-h", "--help").set(isShowHelp).doc("show this screen")
	);

	// clang-format on

	if (!parse(argc, argv, cli) || isShowHelp) {
		auto fmt = doc_formatting {}.first_column(2).doc_column(20);

		std::string programName = "./" + fs::path(argv[0]).filename().string();

		std::cout << "usage:\n"
				  << usage_lines(cli, programName, fmt) << "\n\noptions:\n"
				  << documentation(cli, fmt) << std::endl;
		return 1;
	}

	if (gameName.empty()) gameName = ini["default"]["game"];
	if (gameName.empty()) {
		fprintf(stderr, "error: default game not set in config\n");
		return 1;
	}

	Game game;
	if (util::isEqual(gameName, "smo"))
		game = Game::SMO;
	else if (util::isEqual(gameName, "3dw"))
		game = Game::SM3DW;
	else {
		fprintf(stderr, "error: invalid game name (expected \"smo\" or \"3dw\")");
		return 1;
	}

	if (romfsPath.empty()) romfsPath = ini["romfs"][gameName];
	if (romfsPath.empty()) {
		fprintf(stderr, "error: romfs path for game '%s' not set in config\n", gameName.c_str());
		return 1;
	}

	if (objectName.empty()) {
		printf("object name: ");
		std::getline(std::cin, objectName);
	}

	Query query = { .name = objectName, .isRecurse = false };

	SearchEngine engine(game, query);

	result_t r = engine.searchAllStages(romfsPath);

	if (r) fprintf(stderr, "error %x: %s\n", r, resultToString(r));

	r = engine.saveResults(outPath);

#ifdef _WIN32
	system("pause");
#endif

	if (r == util::Error::FileError) return 1;
}
