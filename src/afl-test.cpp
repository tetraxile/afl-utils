#include <cstdio>
#include <filesystem>
#include <hk/diag/diag.h>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "afl/byml/reader.h"
#include "afl/results.h"
#include "afl/sarc/reader.h"
#include "afl/util.h"
#include "afl/yaz0.h"
#include "clipp/clipp.h"
#include "config.h"
#include "mini/ini.h"

namespace fs = std::filesystem;

bool endsWith(const std::string& fullString, const std::string& ending) {
	if (fullString.length() >= ending.length())
		return 0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending);
	else
		return false;
}

struct Result {
	Result(const std::string& name) : name(name) {}

	const std::string name;
	std::string objId;
	std::string suffix;
};

struct SearchEngine {
	SearchEngine() {}

	hk::Result searchAllStages(const fs::path& romfsPath);
	hk::Result searchBYML(const std::vector<u8> bymlContents);
	hk::Result searchStage(const fs::path& stagePath);
	hk::Result searchScenario(const byml::Reader& scenario);

	std::string mCurStageName;
	u32 mCurScenarioIdx;
	std::string mCurItemList;
	std::vector<Result> mResults;
};

hk::Result readVec3f(hk::util::Vector3f* out, const byml::Reader& reader, const std::string& name) {
	byml::Reader vec;
	HK_TRY(reader.getContainerByKey(&vec, name));

	HK_TRY(vec.getF32ByKey(&out->x, "X"));
	HK_TRY(vec.getF32ByKey(&out->y, "Y"));
	HK_TRY(vec.getF32ByKey(&out->z, "Z"));

	return hk::ResultSuccess();
}

hk::Result SearchEngine::searchBYML(const std::vector<u8> bymlContents) {
	byml::Reader reader;
	HK_TRY(reader.init(&bymlContents[0]));

	byml::Reader defaultTickets, startTickets, tickets;

	std::vector<byml::Reader*> ticketArr;

	HK_TRY(reader.getContainerByKey(&defaultTickets, "DefaultTickets"));
	printf("\tdefault tickets: %d\n", defaultTickets.getSize());
	for (u32 i = 0; i < defaultTickets.getSize(); i++) {
		byml::Reader* ticket = new byml::Reader;
		defaultTickets.getContainerByIdx(ticket, i);
		ticketArr.push_back(ticket);
	}

	bool hasStartTickets = reader.tryGetContainerByKey(&startTickets, "StartTickets");
	if (hasStartTickets) {
		printf("\tstart tickets: %d\n", startTickets.getSize());
		for (u32 i = 0; i < startTickets.getSize(); i++) {
			byml::Reader* ticket = new byml::Reader;
			startTickets.getContainerByIdx(ticket, i);
			ticketArr.push_back(ticket);
		}
	}

	bool hasTickets = reader.tryGetContainerByKey(&tickets, "Tickets");
	if (hasTickets) {
		printf("\ttickets: %d\n", tickets.getSize());
		for (u32 i = 0; i < tickets.getSize(); i++) {
			byml::Reader* ticket = new byml::Reader;
			tickets.getContainerByIdx(ticket, i);
			ticketArr.push_back(ticket);
		}
	}

	for (const byml::Reader* ticket : ticketArr) {
		byml::Reader id, class_, param;
		HK_TRY(ticket->getContainerByKey(&id, "Id"));
		HK_TRY(ticket->getContainerByKey(&class_, "Class"));
		bool hasParam = ticket->tryGetContainerByKey(&param, "Param");

		std::string name;
		HK_TRY(class_.getStringByKey(&name, "Name"));

		std::string objId;
		id.tryGetStringByKey(&objId, "ObjId");
		std::string suffix;
		id.tryGetStringByKey(&suffix, "Suffix");
	}

	for (byml::Reader* ticket : ticketArr) {
		delete ticket;
	}

	return hk::ResultSuccess();
}

hk::Result SearchEngine::searchStage(const fs::path& stagePath) {
	std::string stageName = stagePath.filename().stem().string();

	printf("searching %s\n", stageName.c_str());

	std::vector<u8> szsContents;
	HK_TRY(util::readFile(szsContents, stagePath));

	std::vector<u8> sarcContents;
	HK_TRY(yaz0::decompress(sarcContents, szsContents));

	sarc::Reader sarc(sarcContents);
	HK_TRY(sarc.init());

	if (!sarc.getFilenames().contains("CameraParam.byml")) return hk::ResultSuccess();

	std::vector<u8> bymlContents;
	HK_TRY(sarc.getFileData(bymlContents, "CameraParam.byml"));

	HK_TRY(searchBYML(bymlContents));

	return hk::ResultSuccess();
}

hk::Result SearchEngine::searchAllStages(const fs::path& romfsPath) {
	const fs::path stageDataPath = romfsPath / "StageData";
	if (!fs::is_directory(stageDataPath)) return ResultDirNotFound();

	printf("searching...\n");

	// sort stage paths
	std::set<fs::path> stagePaths;
	for (const auto& entry : fs::directory_iterator(stageDataPath))
		stagePaths.insert(entry.path());

	for (const auto& stagePath : stagePaths) {
		std::string stageName = stagePath.filename().stem().string();
		mCurStageName = stageName;

		if (!stageName.ends_with("Map")) continue;

		HK_TRY(searchStage(stagePath));
	}

	return hk::ResultSuccess();
}

s32 main(s32 argc, char** argv) {
	using namespace clipp;

	mINI::INIFile configFile(getConfigPath());
	mINI::INIStructure ini;
	bool readSuccess = configFile.read(ini);
	if (!readSuccess) generateDefaultConfig();

	std::string romfsPath;
	std::string objectName;
	std::string outPath = "results.txt";

	// clang-format off

	bool isShowHelp = false;
	auto cli = (
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

	std::string gameName = "smo";

	if (romfsPath.empty()) romfsPath = ini["romfs"][gameName];
	if (romfsPath.empty()) {
		fprintf(stderr, "error: romfs path for game '%s' not set in config\n", gameName.c_str());
		return 1;
	}

	SearchEngine engine;

	hk::Result r = engine.searchAllStages(romfsPath);

	if (r.failed()) fprintf(stderr, "error: %s\n", hk::diag::getResultName(r));

#ifdef _WIN32
	if (!isatty_win()) system("pause");
#endif

	if (r.failed()) return 1;
}
