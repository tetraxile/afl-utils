#pragma once

#include <filesystem>

namespace fs = std::filesystem;

const fs::path getConfigPath();

void generateDefaultConfig();
