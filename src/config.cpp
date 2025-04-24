#include "config.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "CLI11/CLI11.hpp"
#include "afl/types.h"
#include "afl/util.h"
#include "mini/ini.h"

s32 main(s32 argc, char** argv) {
	CLI::App app { "app description" };
	argv = app.ensure_utf8(argv);
	app.require_subcommand(1);

	std::string gameName;
	std::string romfsPath;

	CLI::App* romfs = app.add_subcommand("romfs", "set ROMFS path of a game");
	{
		romfs->add_option("game", gameName, "one of \"smo\" or \"3dw\"")
			->required()
			->check([](const std::string& input) {
				if (!util::isEqual(input, "smo") && !util::isEqual(input, "3dw"))
					throw CLI::ValidationError("must be one of \"smo\" or \"3dw\"");
				return "";
			});

		romfs->add_option("romfs_path", romfsPath, "path to game's ROMFS directory")
			->required()
			->check(CLI::ExistingDirectory);
	}

	CLI11_PARSE(app, argc, argv);

	generateDefaultConfig();

	mINI::INIFile iniFile(getConfigPath());
	mINI::INIStructure ini;
	bool readSuccess = iniFile.read(ini);

	if (!readSuccess) {
		fprintf(stderr, "error: couldn't read afl config file");
		return 1;
	}

	if (romfs->parsed()) {
		printf("setting ROMFS path to %s\n", romfsPath.c_str());
		ini["romfs"][gameName] = romfsPath;
	}

	iniFile.write(ini, true);
}
