#pragma once

#include <filesystem>

#ifdef _WIN32
# include <io.h>

bool isatty_win() {
	return _isatty(_fileno(stdout));
}
#endif

namespace fs = std::filesystem;

const fs::path getConfigPath();

void generateDefaultConfig();
