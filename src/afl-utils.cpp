#include <cstdio>
#include <format>
#include <fstream>
#include <iostream>

#include "afl/bffnt.h"
#include "afl/bfres.h"
#include "afl/bntx.h"
#include "afl/byml/reader.h"
#include "afl/byml/writer.h"
#include "afl/result.h"
#include "afl/sarc.h"
#include "afl/util.h"
#include "afl/yaz0.h"

enum Error : result_t {
	InvalidArgument = 0x1001,
};

std::string print_byml(const byml::Reader& node, s32 level = 0) {
	// std::string indent(level, '\t');
	std::string out;

	if (node.getType() == byml::NodeType::Array) {
		out += "[";
		for (u32 i = 0; i < node.getSize(); i++) {
			byml::NodeType childType;
			node.getTypeByIdx(&childType, i);
			if (childType == byml::NodeType::Hash) {
				byml::Reader container;
				node.getContainerByIdx(&container, i);
				out += print_byml(container, level + 1);
			} else if (childType == byml::NodeType::String) {
				std::string str;
				node.getStringByIdx(&str, i);
				out += std::format("\"{}\"", str);
			} else if (childType == byml::NodeType::Bool) {
				bool value;
				node.getBoolByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::S32) {
				s32 value;
				node.getS32ByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::U32) {
				u32 value;
				node.getU32ByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::F32) {
				f32 value;
				node.getF32ByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::S64) {
				s64 value;
				node.getS64ByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::U64) {
				u64 value;
				node.getU64ByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::F64) {
				f64 value;
				node.getF64ByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::Bool) {
				out += "null";
			} else {
				out += std::format("{:x}", (u8)childType);
			}
			if (i != node.getSize() - 1) out += ", ";
		}
		out += "]";
	} else if (node.getType() == byml::NodeType::Hash) {
		out += "{";
		for (u32 i = 0; i < node.getSize(); i++) {
			byml::NodeType childType;
			node.getTypeByIdx(&childType, i);
			u32 keyIdx;
			node.getKeyByIdx(&keyIdx, i);
			out += std::format("\"{}\": ", node.getHashString(keyIdx));
			if (childType == byml::NodeType::Hash) {
				byml::Reader container;
				node.getContainerByIdx(&container, i);
				out += print_byml(container, level + 1);
			} else if (childType == byml::NodeType::String) {
				std::string str;
				node.getStringByIdx(&str, i);
				out += std::format("\"{}\"", str);
			} else if (childType == byml::NodeType::Bool) {
				bool value;
				node.getBoolByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::S32) {
				s32 value;
				node.getS32ByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::U32) {
				u32 value;
				node.getU32ByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::F32) {
				f32 value;
				node.getF32ByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::S64) {
				s64 value;
				node.getS64ByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::U64) {
				u64 value;
				node.getU64ByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::F64) {
				f64 value;
				node.getF64ByIdx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::Bool) {
				out += "null";
			} else {
				out += std::format("{:x}", (u8)childType);
			}
			if (i != node.getSize() - 1) out += ", ";
		}
		out += "}";
	}

	return out;
}

result_t handle_yaz0(s32 argc, char* argv[]) {
	result_t r;

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 5) {
			fprintf(stderr, "usage: %s yaz0 r <compressed file> <decompressed file>\n", argv[0]);
			return Error::InvalidArgument;
		}

		std::vector<u8> fileContents;
		r = util::readFile(fileContents, argv[3]);
		if (r) return r;

		std::vector<u8> outputBuffer;
		r = yaz0::decompress(outputBuffer, fileContents);
		if (r) return r;

		std::ofstream outfile(argv[4], std::ios::out | std::ios::binary);
		outfile.write(reinterpret_cast<const char*>(outputBuffer.data()), outputBuffer.size());
	} else if (util::isEqual(argv[2], "write") || util::isEqual(argv[2], "w")) {
		if (argc < 5) {
			fprintf(
				stderr, "usage: %s yaz0 w <decompressed file> <compressed file> [alignment]\n",
				argv[0]
			);
			return Error::InvalidArgument;
		}

		u32 alignment = argc > 5 ? atoi(argv[5]) : 0x80;

		std::vector<u8> fileContents;
		r = util::readFile(fileContents, argv[3]);
		if (r) return r;

		std::vector<u8> outputBuffer;
		yaz0::compress(outputBuffer, fileContents, alignment);

		util::writeFile(argv[4], outputBuffer);
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return Error::InvalidArgument;
	}

	return 0;
}

result_t handle_sarc(s32 argc, char* argv[]) {
	result_t r;

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 5) {
			fprintf(stderr, "usage: %s sarc r <archive> <output dir>\n", argv[0]);
			return Error::InvalidArgument;
		}

		std::vector<u8> fileContents;
		r = util::readFile(fileContents, argv[3]);
		if (r) return r;

		SARC sarc(fileContents);
		r = sarc.read();
		if (r) return r;

		r = sarc.saveAll(argv[4]);
		if (r) return r;
	} else if (util::isEqual(argv[2], "write") || util::isEqual(argv[2], "w")) {
		if (argc < 5) {
			fprintf(stderr, "usage: %s sarc w <input dir> <archive>\n", argv[0]);
			return Error::InvalidArgument;
		}
	}

	return 0;
}

result_t handle_szs(s32 argc, char* argv[]) {
	result_t r;

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 5) {
			fprintf(stderr, "usage: %s szs r <archive> <output dir>\n", argv[0]);
			return Error::InvalidArgument;
		}

		std::vector<u8> fileContents;
		r = util::readFile(fileContents, argv[3]);
		if (r) return r;

		std::vector<u8> decompressed;
		r = yaz0::decompress(decompressed, fileContents);
		if (r) return r;

		SARC sarc(decompressed);
		r = sarc.read();
		if (r) return r;

		r = sarc.saveAll(argv[4]);
		if (r) return r;
	}

	return 0;
}

result_t handle_bffnt(s32 argc, char* argv[]) {
	result_t r;

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 4) {
			fprintf(stderr, "usage: %s bffnt r <font file>\n", argv[0]);
			return Error::InvalidArgument;
		}

		std::vector<u8> fileContents;
		r = util::readFile(fileContents, argv[3]);
		if (r) return r;

		BFFNT bffnt(fileContents);
		r = bffnt.read();
		if (r) return r;
	}

	return 0;
}

result_t handle_bntx(s32 argc, char* argv[]) {
	result_t r;

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 4) {
			fprintf(stderr, "usage: %s bntx r <texture file>\n", argv[0]);
			return Error::InvalidArgument;
		}

		std::vector<u8> fileContents;
		r = util::readFile(fileContents, argv[3]);
		if (r) return r;

		BNTX bntx(fileContents);
		r = bntx.read();
		if (r) return r;
	}

	return 0;
}

result_t handle_byml(s32 argc, char* argv[]) {
	result_t r;

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 4) {
			fprintf(stderr, "usage: %s byml r <input file>\n", argv[0]);
			return Error::InvalidArgument;
		}

		std::vector<u8> fileContents;
		r = util::readFile(fileContents, argv[3]);
		if (r) return r;

		byml::Reader byml;
		r = byml.init(&fileContents[0]);
		if (r) return r;

		std::string out = print_byml(byml);
		printf("%s\n", out.c_str());
	} else if (util::isEqual(argv[2], "write") || util::isEqual(argv[2], "w")) {
		if (argc < 4) {
			fprintf(stderr, "usage: %s byml w <output file>\n", argv[0]);
			return Error::InvalidArgument;
		}
	}

	return 0;
}

result_t handle_bfres(s32 argc, char* argv[]) {
	result_t r;

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 4) {
			fprintf(stderr, "usage: %s bfres r <input file>\n", argv[0]);
			return Error::InvalidArgument;
		}

		std::vector<u8> fileContents;
		r = util::readFile(fileContents, argv[3]);
		if (r) return r;

		BFRES bfres(fileContents);
		r = bfres.read();
		if (r) return r;

	} else if (util::isEqual(argv[2], "write") || util::isEqual(argv[2], "w")) {
		if (argc < 4) {
			fprintf(stderr, "usage: %s bfres w <input file>\n", argv[0]);
			return Error::InvalidArgument;
		}
	}

	return 0;
}

s32 main(s32 argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "usage: %s <format> <options...>\n", argv[0]);
		fprintf(stderr, "\tformats: yaz0, sarc, szs, bffnt, bntx, byml, bfres\n");
		return 1;
	}

	result_t r;

	if (util::isEqual(argv[1], "yaz0"))
		r = handle_yaz0(argc, argv);
	else if (util::isEqual(argv[1], "sarc"))
		r = handle_sarc(argc, argv);
	else if (util::isEqual(argv[1], "szs"))
		r = handle_szs(argc, argv);
	else if (util::isEqual(argv[1], "bffnt"))
		r = handle_bffnt(argc, argv);
	else if (util::isEqual(argv[1], "bntx"))
		r = handle_bntx(argc, argv);
	else if (util::isEqual(argv[1], "byml"))
		r = handle_byml(argc, argv);
	else if (util::isEqual(argv[1], "bfres"))
		r = handle_bfres(argc, argv);
	else {
		fprintf(stderr, "error: unrecognized format '%s'\n\n", argv[1]);
		fprintf(stderr, "usage: %s <format> <options...>\n", argv[0]);
		fprintf(stderr, "\tformats: yaz0, sarc, szs, bffnt, bntx, byml, bfres\n");
		return 1;
	}

	if (r == Error::InvalidArgument) return r;

	if (r) fprintf(stderr, "error %x: %s\n", r, resultToString(r));

	return 0;
}
