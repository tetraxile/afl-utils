#include "config.h"

#include <filesystem>

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
# include <stdlib.h>

const fs::path getConfigPath() {
	char* buf = nullptr;
	size_t sz = 0;
	fs::path out = "";
	if (_dupenv_s(&buf, &sz, "APPDATA") == 0 && buf != nullptr) {
		out = fs::path(buf) / "afl-utils" / "config.ini";
		free(buf);
	}

	return out;
}

bool isatty_win() {
	return false;
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
