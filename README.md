# afl-utils

this repository contains various scripts for interacting with some ActionLibrary games (e.g. SMO, SM3DW).

## Building

* `git submodule update --init --recursive`
* `mkdir build`
* `cd build`
* `cmake ..`
    * `make afl-search`
    * `make afl-copy`
    * `make afl-utils`

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
