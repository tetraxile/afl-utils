#include <cstdio>
#include <filesystem>
#include <set>
#include <vector>

#include <afl/byml/reader.h>
#include <afl/result.h>
#include <afl/sarc.h>
#include <afl/types.h>
#include <afl/util.h>
#include <afl/yaz0.h>

namespace fs = std::filesystem;

enum class Game {
    SMO,
    SM3DW,
};

struct Query {
    std::string name;
    bool isRecurse = false;
};

struct Result {
    std::string stageName;
    u32 scenarioIdx;
    std::string itemList;
    std::string objId;
};

bool endsWith(const std::string& fullString, const std::string& ending) {
    if (fullString.length() >= ending.length())
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    else
        return false;
}

struct SearchEngine {
    SearchEngine(const Game& game, const Query& query) : mGame(game), mQuery(query) {}

    result_t searchAllStages(const fs::path& romfsPath);
    result_t searchStage(const fs::path& stagePath);
    result_t searchScenario(const byml::Reader& scenario);
    result_t loadStage(std::vector<u8>& bymlContents, const fs::path& stagePath);
    void saveResults(const fs::path& outPath) const;

    const Game mGame;
    const Query mQuery;
    std::vector<Result> mResults;
    std::string mCurStageName;
    u32 mCurScenarioIdx;
    std::string mCurItemList;
};

result_t SearchEngine::loadStage(std::vector<u8>& bymlContents, const fs::path& stagePath) {
    result_t r;

    std::string stageName = stagePath.filename().replace_extension();

    std::vector<u8> szsContents;
    r = util::read_file(szsContents, stagePath);
    if (r) return r;

    std::vector<u8> decompressed;
    r = yaz0::decompress(decompressed, szsContents);
    if (r) return r;

    SARC sarc(decompressed);
    r = sarc.read();
    if (r) return r;

    std::string suffix;
    if (mGame == Game::SMO)
        suffix = ".byml";
    else if (mGame == Game::SM3DW)
        suffix = "Map.byml";
    else
        return util::Error::FileNotFound;

    r = sarc.get_file_data(bymlContents, stageName + suffix);
    if (r) return r;

    return 0;
}

result_t SearchEngine::searchScenario(const byml::Reader& scenario) {
    result_t r;

    for (u32 listIdx = 0; listIdx < scenario.get_size(); listIdx++) {
        u32 keyIdx;
        scenario.get_key_by_idx(&keyIdx, listIdx);
        std::string listName = scenario.get_hash_string(keyIdx);
        mCurItemList = listName;

        if (util::is_equal(listName, "FilePath")) continue;

        byml::Reader itemList;
        scenario.get_container_by_idx(&itemList, listIdx);

        for (u32 itemIdx = 0; itemIdx < itemList.get_size(); itemIdx++) {
            byml::Reader item;
            itemList.get_container_by_idx(&item, itemIdx);

            std::string itemName;
            r = item.get_string_by_key(&itemName, "UnitConfigName");
            if (r) return r;

            if (util::is_equal(itemName, mQuery.name)) {
                std::string objId;
                r = item.get_string_by_key(&objId, "Id");
                if (r) return r;

                Result result = { mCurStageName, mCurScenarioIdx, mCurItemList, objId };
                mResults.push_back(result);
            }
        }
    }

    return 0;
}

result_t SearchEngine::searchStage(const fs::path& stagePath) {
    result_t r;

    std::string stageName = stagePath.filename().replace_extension();

    std::vector<u8> bymlContents;
    loadStage(bymlContents, stagePath);

    byml::Reader stageReader;
    r = stageReader.init(&bymlContents[0]);
    if (r) return r;

    if (!stageReader.is_exist_string_value(mQuery.name)) return 0;

    if (mGame == Game::SMO) {
        for (u32 scenarioIdx = 0; scenarioIdx < stageReader.get_size(); scenarioIdx++) {
            mCurScenarioIdx = scenarioIdx;

            byml::Reader scenario;
            stageReader.get_container_by_idx(&scenario, scenarioIdx);

            searchScenario(scenario);
        }
    } else if (mGame == Game::SM3DW) {
        searchScenario(stageReader);
    }

    return 0;
}

result_t SearchEngine::searchAllStages(const fs::path& romfsPath) {
    result_t r;

    const fs::path stageDataPath = romfsPath / "StageData";
    if (!fs::is_directory(stageDataPath)) return util::Error::DirNotFound;

    std::string stageSuffix;
    if (mGame == Game::SMO)
        stageSuffix = "Map.szs";
    else if (mGame == Game::SM3DW)
        stageSuffix = ".szs";

    // sort stage paths
    std::set<fs::path> stagePaths;
    for (const auto& entry : fs::directory_iterator(stageDataPath)) {
        if (!endsWith(entry.path().filename(), stageSuffix)) continue;
        stagePaths.insert(entry.path());
    }

    for (const auto& stagePath : stagePaths) {
        std::string stageName = stagePath.filename().string();
        stageName.erase(stageName.find(stageSuffix), stageSuffix.length());
        mCurStageName = stageName;

        // if (!util::is_equal(stageName, "TitleDemo00Stage")) continue;

        r = searchStage(stagePath);
        if (r) return r;
    }

    printf("found %zu matches\n", mResults.size());



    return 0;
}

void SearchEngine::saveResults(const fs::path& outPath) const {
    FILE* f = fopen(outPath.c_str(), "w");

    for (const Result& result : mResults) {
        if (mGame == Game::SMO)
            fprintf(f, "%s %d %s %s\n", result.stageName.c_str(), result.scenarioIdx, result.itemList.c_str(), result.objId.c_str());
        else if (mGame == Game::SM3DW)
            fprintf(f, "%s\t%s\t%s\n", result.stageName.c_str(), result.itemList.c_str(), result.objId.c_str());
    }

    fclose(f);
}

s32 main(s32 argc, char** argv) {
	if (argc < 4) {
		fprintf(stderr, "usage: %s <game> <romfs path> <object name> [output file]\n", argv[0]);
		return 1;
	}

    std::string gameName = argv[1];
    std::string romfsPath = argv[2];
    std::string objectName = argv[3];
    std::string outPath = argc < 5 ? "results.txt" : argv[4];
    // std::string outPath = "./result.txt";

    Game game;
    if (util::is_equal(gameName, "smo"))
        game = Game::SMO;
    else if (util::is_equal(gameName, "3dw"))
        game = Game::SM3DW;
    else {
        fprintf(stderr, "error: invalid game name (expected \"smo\" or \"3dw\")");
        return 1;
    }

    Query query = {
        .name = objectName,
        .isRecurse = false
    };

    SearchEngine engine(game, query);

	result_t r = engine.searchAllStages(romfsPath);
    engine.saveResults(outPath);

	if (r) fprintf(stderr, "error %x: %s\n", r, result_to_string(r));
}
