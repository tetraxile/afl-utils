#include <array>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <format>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "afl/byml/reader.h"
#include "afl/result.h"
#include "afl/sarc/reader.h"
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
	const std::string keyQueryName;
};

struct Value {
	byml::NodeType type;

	union {
		std::string_view val_string;
		bool val_bool;
		u32 val_u32;
		s32 val_s32;
		f32 val_f32;
		u64 val_u64;
		s64 val_s64;
		f64 val_f64;
	};

	Value() { setNull(); }

	void setString(const std::string& val) {
		type = byml::NodeType::String;
		val_string = val;
	}

	void setBool(bool val) {
		type = byml::NodeType::Bool;
		val_bool = val;
	}

	void setU32(u32 val) {
		type = byml::NodeType::U32;
		val_u32 = val;
	}

	void setS32(s32 val) {
		type = byml::NodeType::S32;
		val_s32 = val;
	}

	void setF32(f32 val) {
		type = byml::NodeType::F32;
		val_f32 = val;
	}

	void setU64(u64 val) {
		type = byml::NodeType::U64;
		val_u64 = val;
	}

	void setS64(s64 val) {
		type = byml::NodeType::S64;
		val_s64 = val;
	}

	void setF64(f64 val) {
		type = byml::NodeType::F64;
		val_f64 = val;
	}

	void setNull() {
		type = byml::NodeType::Null;
		val_u32 = 0;
	}

	void setByKey(const byml::Reader& container, const std::string& key) {
		byml::NodeType queryType;
		if (container.getTypeByKey(&queryType, key)) {
			std::string valString;
			bool valBool;
			u32 valU32;
			s32 valS32;
			f32 valF32;
			u64 valU64;
			s64 valS64;
			f64 valF64;

			switch (queryType) {
			case byml::NodeType::String:
				container.getStringByKey(&valString, key);
				setString(valString);
				break;
			case byml::NodeType::Bool:
				container.getBoolByKey(&valBool, key);
				setBool(valBool);
				break;
			case byml::NodeType::U32:
				container.getU32ByKey(&valU32, key);
				setU32(valU32);
				break;
			case byml::NodeType::S32:
				container.getS32ByKey(&valS32, key);
				setS32(valS32);
				break;
			case byml::NodeType::F32:
				container.getF32ByKey(&valF32, key);
				setF32(valF32);
				break;
			case byml::NodeType::U64:
				container.getU64ByKey(&valU64, key);
				setU64(valU64);
				break;
			case byml::NodeType::S64:
				container.getS64ByKey(&valS64, key);
				setS64(valS64);
				break;
			case byml::NodeType::F64:
				container.getF64ByKey(&valF64, key);
				setF64(valF64);
				break;

			case byml::NodeType::Array:
			case byml::NodeType::Hash:
			case byml::NodeType::StringTable:
			case byml::NodeType::Null: setNull(); break;
			}
		}
	}

	std::string toString() const {
		if (type == byml::NodeType::String)
			return std::format("\"{}\"", val_string);
		else if (type == byml::NodeType::S32)
			return std::format("{}", val_s32);
		else if (type == byml::NodeType::U32)
			return std::format("{}", val_u32);
		else if (type == byml::NodeType::F32)
			return std::format("{}", val_f32);
		else if (type == byml::NodeType::S64)
			return std::format("{}", val_s64);
		else if (type == byml::NodeType::U64)
			return std::format("{}", val_u64);
		else if (type == byml::NodeType::F64)
			return std::format("{}", val_f64);
		else if (type == byml::NodeType::Bool)
			return val_bool ? "true" : "false";
		else
			return "null";
	}
};

struct Result {
	const std::string stageName;
	mutable std::array<bool, 15> scenarioFlag;
	const u32 scenarioIdx;
	const std::string itemList;
	const std::string baseName;

	const std::string unitConfigName;
	const std::string modelName;
	const std::string paramConfigName;
	const std::string objId;
	const Vector3f trans;
	const Vector3f rotate;
	const Vector3f scale;

	const Value queryValue;

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
	return util::isEqual(stageName, other.stageName) && util::isEqual(unitConfigName, other.unitConfigName) &&
	       util::isEqual(paramConfigName, other.paramConfigName) && util::isEqual(modelName, other.modelName) &&
	       util::isEqual(objId, other.objId) && trans.isEqual(other.trans) && scale.isEqual(other.scale) &&
	       rotate.isEqual(other.rotate);
}

bool endsWith(const std::string& fullString, const std::string& ending) {
	if (fullString.length() >= ending.length())
		return 0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending);
	else
		return false;
}

struct SearchEngine {
	SearchEngine(const Game& game, const Query& query, bool isVerbose = false) :
		mGame(game), mQuery(query), mIsVerbose(isVerbose) {}

	result_t searchAllStages(const fs::path& romfsPath);
	result_t searchBYML(const std::vector<u8> bymlContents);
	result_t searchStage(const fs::path& stagePath);
	result_t searchScenario(const byml::Reader& scenario);
	result_t searchItem(const byml::Reader& item, std::string_view baseName = "", u32 level = 0);
	result_t saveResults(const fs::path& outPath) const;

	const Game mGame;
	const Query mQuery;
	std::vector<Result> mResults;
	std::string mCurStageName;
	u32 mCurScenarioIdx;
	std::string mCurItemList;
	bool mIsVerbose;
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

result_t SearchEngine::searchItem(const byml::Reader& item, std::string_view baseName, u32 level) {
	result_t r;

	std::string unitConfigName;
	r = item.getStringByKey(&unitConfigName, "UnitConfigName");
	if (r) return r;

	if (level == 0) baseName = unitConfigName;

	byml::Reader unitConfig;
	r = item.getContainerByKey(&unitConfig, "UnitConfig");
	if (r) return r;

	std::string paramConfigName;
	r = unitConfig.getStringByKey(&paramConfigName, "ParameterConfigName");
	if (r) return r;

	std::string modelName;
	bool hasModelName = item.tryGetStringByKey(&modelName, "ModelName");

	if (util::isEqual(unitConfigName, mQuery.name) || util::isEqual(paramConfigName, mQuery.name) ||
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

		if (level == 0) baseName = "";

		Value queryValue;
		if (!mQuery.keyQueryName.empty()) {
			queryValue.setByKey(item, mQuery.keyQueryName);
		};

		Result result = { .stageName = mCurStageName,
			              .scenarioFlag = scenarioFlag,
			              .scenarioIdx = mCurScenarioIdx,
			              .itemList = mCurItemList,
			              .baseName = baseName.data(),
			              .unitConfigName = unitConfigName,
			              .modelName = optModelName,
			              .paramConfigName = paramConfigName,
			              .objId = objId,
			              .trans = trans,
			              .rotate = rotate,
			              .scale = scale,
			              .queryValue = queryValue };
		mResults.push_back(result);

		return 0;
	}

	if (mQuery.isRecurse) {
		byml::Reader linkGroups;
		r = item.getContainerByKey(&linkGroups, "Links");
		if (r) return r;

		for (u32 groupIdx = 0; groupIdx < linkGroups.getSize(); groupIdx++) {
			byml::Reader group;
			r = linkGroups.getContainerByIdx(&group, groupIdx);
			if (r) return r;

			for (u32 linkIdx = 0; linkIdx < group.getSize(); linkIdx++) {
				byml::Reader item;
				r = group.getContainerByIdx(&item, linkIdx);
				if (r) return r;

				r = searchItem(item, baseName, level + 1);
				if (r) return r;
			}
		}
	}

	return 0;
}

result_t SearchEngine::searchScenario(const byml::Reader& scenario) {
	result_t r;

	for (u32 listIdx = 0; listIdx < scenario.getSize(); listIdx++) {
		std::string listName = scenario.getKeyByIdx(listIdx);
		mCurItemList = listName;

		if (util::isEqual(listName, "FilePath") || util::isEqual(listName, "Objs")) continue;

		byml::Reader itemList;
		scenario.getContainerByIdx(&itemList, listIdx);

		for (u32 itemIdx = 0; itemIdx < itemList.getSize(); itemIdx++) {
			byml::Reader item;
			itemList.getContainerByIdx(&item, itemIdx);

			r = searchItem(item);
			if (r) return r;
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

	if (mIsVerbose) {
		printf("%s - found string\n", mCurStageName.c_str());
	}

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

		sarc::Reader sarc(sarcContents);
		r = sarc.init();
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

		sarc::Reader sarc(sarcContents);
		r = sarc.init();
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

		fprintf(f, "query:\n");
		fprintf(f, "\tname: %s\n", mQuery.name.c_str());
		fprintf(f, "\tsearch links?: %s\n", mQuery.isRecurse ? "true" : "false");
		fprintf(f, "\t# matches: %zu\n", collapsedResults.size());
		if (!mQuery.keyQueryName.empty()) {
			fprintf(f, "\tquery key: %s\n", mQuery.keyQueryName.c_str());
		}
		fprintf(f, "\n");

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
				if (!util::isEqual(result.unitConfigName, result.modelName) && !result.modelName.empty()) {
					fprintf(f, "\tModelName: %s\n", result.modelName.c_str());
				}
				if (!util::isEqual(result.unitConfigName, result.paramConfigName) && !result.paramConfigName.empty()) {
					fprintf(f, "\tParameterConfigName: %s\n", result.paramConfigName.c_str());
				}
				if (!result.baseName.empty()) {
					fprintf(f, "\tbase object UnitConfigName: %s\n", result.baseName.c_str());
				}
				fprintf(f, "\tTranslate: (%.3f, %.3f, %.3f)\n", result.trans.x, result.trans.y, result.trans.z);
				fprintf(f, "\tId: %s\n", result.objId.c_str());
				if (!mQuery.keyQueryName.empty()) {
					fprintf(f, "\t%s: %s\n", mQuery.keyQueryName.c_str(), result.queryValue.toString().c_str());
				}
				fprintf(f, "\titem list: %s\n", result.itemList.c_str());
				fprintf(f, "\tscenarios: ");
				for (u32 i = 0; i < result.scenarioFlag.size(); i++) {
					if (result.scenarioFlag[i]) fprintf(f, "%d ", i + 1);
				}
				fprintf(f, "\n\n");
			}
		}
	} else if (mGame == Game::SM3DW) {
		printf("found %zu matches\n", mResults.size());

		fprintf(f, "query:\n");
		fprintf(f, "\tname: %s\n", mQuery.name.c_str());
		fprintf(f, "\tsearch links?: %s\n", mQuery.isRecurse ? "true" : "false");
		fprintf(f, "\t# matches: %zu\n", mResults.size());
		fprintf(f, "\n");

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
				if (!util::isEqual(result.unitConfigName, result.modelName) && !result.modelName.empty())
					fprintf(f, "\tModelName: %s\n", result.modelName.c_str());
				if (!util::isEqual(result.unitConfigName, result.paramConfigName) && !result.paramConfigName.empty())
					fprintf(f, "\tParameterConfigName: %s\n", result.paramConfigName.c_str());
				fprintf(f, "\tTranslate: (%.3f, %.3f, %.3f)\n", result.trans.x, result.trans.y, result.trans.z);
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
	std::string keyQueryName;
	std::string outPath = "results.txt";

	// clang-format off

	bool isShowHelp = false;
	bool isVerbose = false;
	auto cli = (
		opt_value("game", gameName).doc("one of \"smo\" or \"3dw\""),
		option("-r", "--romfs").doc("path to game's romfs") & value("romfs path", romfsPath),
		option("-n", "--name").doc("name of object to search for") & value("name", objectName),
	    option("-o", "--output").doc("path to output file (default: results.txt)") & value("outfile", outPath),
	    option("-v", "--verbose").set(isVerbose).doc("print more detailed output"),
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

	if (keyQueryName.empty()) {
		printf("query key?: ");
		std::getline(std::cin, keyQueryName);
	}

	Query query = { .name = objectName, .isRecurse = true, .keyQueryName = keyQueryName };

	SearchEngine engine(game, query, isVerbose);

	result_t r = engine.searchAllStages(romfsPath);

	if (r) fprintf(stderr, "error %x: %s\n", r, resultToString(r));

	r = engine.saveResults(outPath);

#ifdef _WIN32
	if (!isatty_win()) system("pause");
#endif

	if (r == util::Error::FileError) return 1;
}
