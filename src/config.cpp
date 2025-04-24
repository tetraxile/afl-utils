#include "config.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "CLI11/CLI11.hpp"
#include "afl/types.h"
#include "afl/util.h"
#include "mini/ini.h"

#ifdef __linux__
# include <pwd.h>
# include <unistd.h>

const fs::path getConfigPath() {
	const char* xdgConfDir = getenv("XDG_CONFIG_HOME");
	if (xdgConfDir) return fs::path(xdgConfDir) / "afl-utils" / "config.ini";

	const char* homeDir;
	if (!(homeDir = getenv("HOME"))) homeDir = getpwuid(getuid())->pw_dir;

	return fs::path(homeDir) / ".config" / "afl-utils" / "config.ini";
}
#elif _WIN32
const fs::path getConfigPath() {
	return fs::path(getenv("APPDATA")) / "afl-utils" / "config.ini";
}
#endif

void generateDefaultConfig() {
	const fs::path configPath = getConfigPath();
	if (fs::exists(configPath)) return;

	printf("creating config file... (%s)\n", configPath.string().c_str());

	fs::path parentPath = configPath.parent_path();
	if (!fs::is_directory(parentPath)) fs::create_directory(parentPath);

	mINI::INIFile file(configPath);
	mINI::INIStructure ini;
	bool success = file.generate(ini, true);
}

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
