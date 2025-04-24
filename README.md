# afl-utils

this repository contains various scripts for interacting with some ActionLibrary games (e.g. SMO, SM3DW).

## Building

There is a workflow that uploads automatic builds [here](https://github.com/tetraxile/afl-utils/actions/workflows/build.yml) after every commit.

### Linux

* `git submodule update --init --recursive`
* `mkdir build`
* `cd build`
* `cmake ..`
* `make`

### Windows

* `git submodule update --init --recursive`
* `mkdir build`
* `cd build`
* `cmake .. -G "Visual Studio 17 2022" -A x64 && cmake --build . --config Release`

## Usage

### afl-search

`usage: ./afl-search <game> <romfs path> <object name> [output file]`

this script searches through all of a game's stages for an object that matches the search criteria.

`game` can currently only be `smo` and `3dw`, for Odyssey and 3D World respectively.

`object name` matches a `UnitConfigName`, `ModelName`, or `ParameterConfigName`.

### afl-copy

`usage: ./afl-copy <input file> <output file>`

this script reads a BYML file and writes it again. not particularly useful unless your name rhymes with schmetraxile.

### afl-utils

```
usage: ./afl-utils <format> <option>
        formats: yaz0, sarc, szs, bffnt, bntx, byml, bfres
        options: read, r, write, w
```

various readers/writers for different file formats. some of these don't do much

### afl-config

```
usage: ./afl-config <subcommand> <args...>

./afl-config romfs <game> <romfs path>
```

set config options for the other scripts to use

## License

The licenses found in the [LICENSE](LICENSE) file apply only to the source files in the [src/](src) directory.

## Credits

* [mINI](https://github.com/metayeti/mINI) for .ini file parsing
* [CLI11](https://github.com/CLIUtils/CLI11) for command line argument parsing
