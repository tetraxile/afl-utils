#include <cstdio>
#include <format>
#include <vector>

#include <afl/byml/common.h>
#include <afl/byml/reader.h>
#include <afl/byml/writer.h>
#include <afl/types.h>
#include <afl/util.h>

void copy_byml_r(byml::Writer& writer, const byml::Reader& node) {
	if (node.getType() == byml::NodeType::Array) {
		for (u32 i = 0; i < node.getSize(); i++) {
			byml::NodeType childType;
			node.getTypeByIdx(&childType, i);
			if (childType == byml::NodeType::Hash) {
				byml::Reader container;
				node.getContainerByIdx(&container, i);
				writer.pushHash();
				copy_byml_r(writer, container);
				writer.pop();
			} else if (childType == byml::NodeType::Array) {
				byml::Reader container;
				node.getContainerByIdx(&container, i);
				writer.pushArray();
				copy_byml_r(writer, container);
				writer.pop();
			} else if (childType == byml::NodeType::String) {
				std::string value;
				node.getStringByIdx(&value, i);
				writer.addString(value);
			} else if (childType == byml::NodeType::Bool) {
				bool value;
				node.getBoolByIdx(&value, i);
				writer.addBool(value);
			} else if (childType == byml::NodeType::S32) {
				s32 value;
				node.getS32ByIdx(&value, i);
				writer.addS32(value);
			} else if (childType == byml::NodeType::U32) {
				u32 value;
				node.getU32ByIdx(&value, i);
				writer.addU32(value);
			} else if (childType == byml::NodeType::F32) {
				f32 value;
				node.getF32ByIdx(&value, i);
				writer.addF32(value);
			} else if (childType == byml::NodeType::S64) {
				s64 value;
				node.getS64ByIdx(&value, i);
				writer.addS64(value);
			} else if (childType == byml::NodeType::U64) {
				u64 value;
				node.getU64ByIdx(&value, i);
				writer.addU64(value);
			} else if (childType == byml::NodeType::F64) {
				f64 value;
				node.getF64ByIdx(&value, i);
				writer.addF64(value);
			} else if (childType == byml::NodeType::Null) {
				writer.addNull();
			}
		}
	} else if (node.getType() == byml::NodeType::Hash) {
		for (u32 i = 0; i < node.getSize(); i++) {
			byml::NodeType childType;
			node.getTypeByIdx(&childType, i);
			u32 keyIdx;
			node.getKeyByIdx(&keyIdx, i);
			std::string key = node.getHashString(keyIdx);
			if (childType == byml::NodeType::Hash) {
				byml::Reader container;
				node.getContainerByIdx(&container, i);
				writer.pushHash(key);
				copy_byml_r(writer, container);
				writer.pop();
			} else if (childType == byml::NodeType::Array) {
				byml::Reader container;
				node.getContainerByIdx(&container, i);
				writer.pushArray(key);
				copy_byml_r(writer, container);
				writer.pop();
			} else if (childType == byml::NodeType::String) {
				std::string value;
				node.getStringByIdx(&value, i);
				writer.addString(key, value);
			} else if (childType == byml::NodeType::Bool) {
				bool value;
				node.getBoolByIdx(&value, i);
				writer.addBool(key, value);
			} else if (childType == byml::NodeType::S32) {
				s32 value;
				node.getS32ByIdx(&value, i);
				writer.addS32(key, value);
			} else if (childType == byml::NodeType::U32) {
				u32 value;
				node.getU32ByIdx(&value, i);
				writer.addU32(key, value);
			} else if (childType == byml::NodeType::F32) {
				f32 value;
				node.getF32ByIdx(&value, i);
				writer.addF32(key, value);
			} else if (childType == byml::NodeType::S64) {
				s64 value;
				node.getS64ByIdx(&value, i);
				writer.addS64(key, value);
			} else if (childType == byml::NodeType::U64) {
				u64 value;
				node.getU64ByIdx(&value, i);
				writer.addU64(key, value);
			} else if (childType == byml::NodeType::F64) {
				f64 value;
				node.getF64ByIdx(&value, i);
				writer.addF64(key, value);
			} else if (childType == byml::NodeType::Null) {
				writer.addNull(key);
			}
		}
	}
}

void copy_byml(byml::Writer& writer, const byml::Reader& node) {
	if (node.getType() == byml::NodeType::Array)
		writer.pushArray();
	else if (node.getType() == byml::NodeType::Hash)
		writer.pushHash();

	copy_byml_r(writer, node);

	writer.pop();
}

std::string print_byml(const byml::Reader& node, s32 level = 0) {
	std::string out;

	if (node.getType() == byml::NodeType::Array) {
		out += "[\n";
		for (u32 i = 0; i < node.getSize(); i++) {
			for (s32 j = 0; j < level; j++) out += "\t";
			byml::NodeType childType;
			node.getTypeByIdx(&childType, i);
			if (childType == byml::NodeType::Hash) {
				byml::Reader container;
				node.getContainerByIdx(&container, i);
				out += print_byml(container, level + 1);
			} else if (childType == byml::NodeType::Array) {
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
				bool value;
				node.getBoolByIdx(&value, i);
				out += value ? "true" : "false";
			} else if (childType == byml::NodeType::Null) {
				out += "null";
			} else {
				out += std::format("{:x}", (u8)childType);
			}
			if (i != node.getSize() - 1) out += ", ";
			out += "\n";
		}
		for (s32 j = 0; j < level - 1; j++) out += "\t";
		out += "]";
	} else if (node.getType() == byml::NodeType::Hash) {
		out += "{\n";
		for (u32 i = 0; i < node.getSize(); i++) {
			for (s32 j = 0; j < level; j++) out += "\t";
			byml::NodeType childType;
			node.getTypeByIdx(&childType, i);
			u32 keyIdx;
			node.getKeyByIdx(&keyIdx, i);
			out += std::format("\"{}\": ", node.getHashString(keyIdx));
			if (childType == byml::NodeType::Hash) {
				byml::Reader container;
				node.getContainerByIdx(&container, i);
				out += print_byml(container, level + 1);
			} else if (childType == byml::NodeType::Array) {
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
				bool value;
				node.getBoolByIdx(&value, i);
				out += value ? "true" : "false";
			} else if (childType == byml::NodeType::Null) {
				out += "null";
			} else {
				out += std::format("{:x}", (u8)childType);
			}
			if (i != node.getSize() - 1) out += ", ";
			out += "\n";
		}
		for (s32 j = 0; j < level - 1; j++) out += "\t";
		out += "}";
	}

	return out;
}

s32 main(s32 argc, char** argv) {
	// if (argc < 3) {
	// 	fprintf(stderr, "usage: %s <input file> <output file>\n", argv[0]);
	// 	return 1;
	// }

	if (argc < 3) {
		fprintf(stderr, "usage: %s <input file> <output file>\n", argv[0]);
		return 1;
	}

	result_t r = 0;

	std::vector<u8> fileContents;
	r = util::readFile(fileContents, argv[1]);
	if (r) return r;

	byml::Reader reader;
	r = reader.init(&fileContents[0]);
	if (r) return r;

	std::string out = print_byml(reader);
	// printf("%s\n", out.c_str());

	byml::Writer writer(3);

	copy_byml(writer, reader);

	writer.save(argv[2], util::ByteOrder::Little);
}
