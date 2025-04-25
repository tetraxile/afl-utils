#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "afl/types.h"
#include "afl/util.h"
#include "clipp/clipp.h"
#include "config.h"
#include "mini/ini.h"

s32 main(s32 argc, char** argv) {
	using namespace clipp;

	std::string gameName;
	std::string romfsPath;

	// clang-format off

	enum class Mode { none, help, romfs, defaultGame };
	Mode mode = Mode::none;

	auto isGameName = [](const std::string& arg) { return util::isEqual(arg, "smo") || util::isEqual(arg, "3dw"); };

	auto romfsMode = (
		command("romfs").set(mode, Mode::romfs),
		value(isGameName, "game", gameName).doc("one of \"smo\" or \"3dw\""),
		value("romfs path", romfsPath).doc("path to game's romfs directory")
	);

	auto defaultMode = (
		command("default_game").set(mode, Mode::defaultGame),
		value(isGameName, "game", gameName).doc("one of \"smo\" or \"3dw\"")
	);

	auto cli = (
		(romfsMode | defaultMode),
	    option("-h", "--help").set(mode, Mode::help).doc("show this screen")
	);

	// clang-format on

	if (!parse(argc, argv, cli) || (mode == Mode::help)) {
		auto fmt = doc_formatting {}.first_column(2).doc_column(20);

		std::cout << "usage:\n"
				  << usage_lines(cli, argv[0], fmt) << "\n\noptions:\n"
				  << documentation(cli, fmt) << std::endl;
		return 1;
	}

	if (mode == Mode::romfs && !fs::is_directory(romfsPath)) {
		fprintf(stderr, "error: romfs path not found\n");
		return 1;
	}

	generateDefaultConfig();

	mINI::INIFile iniFile(getConfigPath());
	mINI::INIStructure ini;
	bool readSuccess = iniFile.read(ini);

	if (!readSuccess) {
		fprintf(stderr, "error: couldn't read afl config file");
		return 1;
	}

	if (mode == Mode::romfs) {
		printf("setting romfs path to %s\n", romfsPath.c_str());
		ini["romfs"][gameName] = romfsPath;
	} else if (mode == Mode::defaultGame) {
		printf("setting default game to %s\n", gameName.c_str());
		ini["default"]["game"] = gameName;
	}

	iniFile.write(ini, true);
}
