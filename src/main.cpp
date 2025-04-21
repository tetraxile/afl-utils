#include <cstdio>
#include <format>
#include <fstream>

#include <afl/bffnt.h>
#include <afl/bfres.h>
#include <afl/bntx.h>
#include <afl/byml/reader.h>
#include <afl/byml/writer.h>
#include <afl/result.h>
#include <afl/sarc.h>
#include <afl/util.h>
#include <afl/yaz0.h>

enum class Format {
	Yaz0,
	SARC,
	SZS,
	BFFNT,
	BNTX,
	BYML,
	BFRES,
};

enum class Option {
	Read,
	Write,
};

std::string print_byml(const byml::Reader& node, s32 level = 0) {
	// std::string indent(level, '\t');
	std::string out;

	if (node.get_type() == byml::NodeType::Array) {
		out += "[";
		for (u32 i = 0; i < node.get_size(); i++) {
			byml::NodeType childType;
			node.get_type_by_idx(&childType, i);
			if (childType == byml::NodeType::Hash) {
				byml::Reader container;
				node.get_container_by_idx(&container, i);
				out += print_byml(container, level + 1);
			} else if (childType == byml::NodeType::String) {
				std::string str;
				node.get_string_by_idx(&str, i);
				out += std::format("\"{}\"", str);
			} else if (childType == byml::NodeType::Bool) {
				bool value;
				node.get_bool_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::S32) {
				s32 value;
				node.get_s32_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::U32) {
				u32 value;
				node.get_u32_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::F32) {
				f32 value;
				node.get_f32_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::S64) {
				s64 value;
				node.get_s64_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::U64) {
				u64 value;
				node.get_u64_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::F64) {
				f64 value;
				node.get_f64_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::Bool) {
				out += "null";
			} else {
				out += std::format("{:x}", (u8)childType);
			}
			if (i != node.get_size() - 1) out += ", ";
		}
		out += "]";
	} else if (node.get_type() == byml::NodeType::Hash) {
		out += "{";
		for (u32 i = 0; i < node.get_size(); i++) {
			byml::NodeType childType;
			node.get_type_by_idx(&childType, i);
			u32 keyIdx;
			node.get_key_by_idx(&keyIdx, i);
			out += std::format("\"{}\": ", node.get_hash_string(keyIdx));
			if (childType == byml::NodeType::Hash) {
				byml::Reader container;
				node.get_container_by_idx(&container, i);
				out += print_byml(container, level + 1);
			} else if (childType == byml::NodeType::String) {
				std::string str;
				node.get_string_by_idx(&str, i);
				out += std::format("\"{}\"", str);
			} else if (childType == byml::NodeType::Bool) {
				bool value;
				node.get_bool_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::S32) {
				s32 value;
				node.get_s32_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::U32) {
				u32 value;
				node.get_u32_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::F32) {
				f32 value;
				node.get_f32_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::S64) {
				s64 value;
				node.get_s64_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::U64) {
				u64 value;
				node.get_u64_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::F64) {
				f64 value;
				node.get_f64_by_idx(&value, i);
				out += std::format("{}", value);
			} else if (childType == byml::NodeType::Bool) {
				out += "null";
			} else {
				out += std::format("{:x}", (u8)childType);
			}
			if (i != node.get_size() - 1) out += ", ";
		}
		out += "}";
	}

	return out;
}

s32 main(s32 argc, char* argv[]) {
	if (argc < 3) {
		fprintf(stderr, "usage: %s <format> <option>\n", argv[0]);
		fprintf(stderr, "\tformats: yaz0, sarc, szs, bffnt, bntx, byml, bfres\n");
		fprintf(stderr, "\toptions: read, r, write, w\n");
		return 1;
	}

	Format format;
	if (util::is_equal(argv[1], "yaz0"))
		format = Format::Yaz0;
	else if (util::is_equal(argv[1], "sarc"))
		format = Format::SARC;
	else if (util::is_equal(argv[1], "szs"))
		format = Format::SZS;
	else if (util::is_equal(argv[1], "bffnt"))
		format = Format::BFFNT;
	else if (util::is_equal(argv[1], "bntx"))
		format = Format::BNTX;
	else if (util::is_equal(argv[1], "byml"))
		format = Format::BYML;
	else if (util::is_equal(argv[1], "bfres"))
		format = Format::BFRES;
	else {
		fprintf(stderr, "error: unrecognized format '%s'\n", argv[1]);
		return 1;
	}

	Option option;
	if (util::is_equal(argv[2], "read") || util::is_equal(argv[2], "r"))
		option = Option::Read;
	else if (util::is_equal(argv[2], "write") || util::is_equal(argv[2], "w"))
		option = Option::Write;
	else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return 1;
	}

	result_t r = 0;

	switch (format) {
	case Format::Yaz0:
		if (option == Option::Read) {
			if (argc < 5) {
				fprintf(
					stderr, "usage: %s yaz0 r <compressed file> <decompressed file>\n", argv[0]
				);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(fileContents, argv[3]);
			if (r) break;

			std::vector<u8> outputBuffer;
			r = yaz0::decompress(outputBuffer, fileContents);
			if (r) break;

			std::ofstream outfile(argv[4], std::ios::out | std::ios::binary);
			outfile.write(reinterpret_cast<const char*>(outputBuffer.data()), outputBuffer.size());
		} else {
			if (argc < 5) {
				fprintf(
					stderr,
					"usage: %s yaz0 w <decompressed file> <compressed file>"
					"[alignment]\n",
					argv[0]
				);
				return 1;
			}

			u32 alignment = argc > 5 ? atoi(argv[5]) : 0x80;

			std::vector<u8> fileContents;
			r = util::read_file(fileContents, argv[3]);
			if (r) break;

			std::vector<u8> outputBuffer;
			yaz0::compress(outputBuffer, fileContents, alignment);

			util::write_file(argv[4], outputBuffer);
		}

		break;
	case Format::SARC:
		if (option == Option::Read) {
			if (argc < 5) {
				fprintf(stderr, "usage: %s sarc r <archive> <output dir>\n", argv[0]);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(fileContents, argv[3]);
			if (r) break;

			SARC sarc(fileContents);
			r = sarc.read();
			if (r) break;

			r = sarc.save_all(argv[4]);
			if (r) break;
		} else {
			if (argc < 5) {
				fprintf(stderr, "usage: %s sarc w <input dir> <archive>\n", argv[0]);
				return 1;
			}
		}

		break;
	case Format::SZS:
		if (option == Option::Read) {
			if (argc < 5) {
				fprintf(stderr, "usage: %s szs r <archive> <output dir>\n", argv[0]);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(fileContents, argv[3]);
			if (r) break;

			std::vector<u8> decompressed;
			r = yaz0::decompress(decompressed, fileContents);
			if (r) break;

			SARC sarc(decompressed);
			r = sarc.read();
			if (r) break;

			r = sarc.save_all(argv[4]);
			if (r) break;
		}

		break;
	case Format::BFFNT:
		if (option == Option::Read) {
			if (argc < 4) {
				fprintf(stderr, "usage: %s bffnt r <font file>\n", argv[0]);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(fileContents, argv[3]);
			if (r) break;

			BFFNT bffnt(fileContents);
			r = bffnt.read();
			if (r) break;
		}

		break;
	case Format::BNTX:
		if (option == Option::Read) {
			if (argc < 4) {
				fprintf(stderr, "usage: %s bntx r <texture file>\n", argv[0]);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(fileContents, argv[3]);
			if (r) break;

			BNTX bntx(fileContents);
			r = bntx.read();
			if (r) break;
		}

		break;
	case Format::BYML:
		if (option == Option::Read) {
			if (argc < 4) {
				fprintf(stderr, "usage: %s byml r <input file>\n", argv[0]);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(fileContents, argv[3]);
			if (r) break;

			byml::Reader byml;
			r = byml.init(&fileContents[0]);
			if (r) break;

			std::string out = print_byml(byml);
			printf("%s\n", out.c_str());
		} else {
			if (argc < 4) {
				fprintf(stderr, "usage: %s byml w <output file>\n", argv[0]);
				return 1;
			}
		}

		break;
	case Format::BFRES:
		if (option == Option::Read) {
			if (argc < 4) {
				fprintf(stderr, "usage: %s bfres r <input file>\n", argv[0]);
				return 1;
			}

			std::vector<u8> fileContents;
			r = util::read_file(fileContents, argv[3]);
			if (r) break;

			BFRES bfres(fileContents);
			r = bfres.read();
			if (r) break;

		} else {
			if (argc < 4) {
				fprintf(stderr, "usage: %s bfres w <input file>\n", argv[0]);
				return 1;
			}
		}

		break;
	}

	if (r) fprintf(stderr, "error %x: %s\n", r, result_to_string(r));

	return 0;
}
