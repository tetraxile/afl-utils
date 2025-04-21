#include <cstdio>
#include <vector>

#include <afl/byml/common.h>
#include <afl/byml/reader.h>
#include <afl/byml/writer.h>
#include <afl/types.h>
#include <afl/util.h>

void copy_byml_r(byml::Writer& writer, const byml::Reader& node) {
	if (node.get_type() == byml::NodeType::Array) {
		for (u32 i = 0; i < node.get_size(); i++) {
			byml::NodeType childType;
			node.get_type_by_idx(&childType, i);
			if (childType == byml::NodeType::Hash) {
				byml::Reader container;
				node.get_container_by_idx(&container, i);
				writer.push_hash();
				copy_byml_r(writer, container);
				writer.pop();
			} else if (childType == byml::NodeType::Array) {
				byml::Reader container;
				node.get_container_by_idx(&container, i);
				writer.push_array();
				copy_byml_r(writer, container);
				writer.pop();
			} else if (childType == byml::NodeType::String) {
				std::string value;
				node.get_string_by_idx(&value, i);
				writer.add_string(value);
			} else if (childType == byml::NodeType::Bool) {
				bool value;
				node.get_bool_by_idx(&value, i);
				writer.add_bool(value);
			} else if (childType == byml::NodeType::S32) {
				s32 value;
				node.get_s32_by_idx(&value, i);
				writer.add_s32(value);
			} else if (childType == byml::NodeType::U32) {
				u32 value;
				node.get_u32_by_idx(&value, i);
				writer.add_u32(value);
			} else if (childType == byml::NodeType::F32) {
				f32 value;
				node.get_f32_by_idx(&value, i);
				writer.add_f32(value);
			} else if (childType == byml::NodeType::S64) {
				s64 value;
				node.get_s64_by_idx(&value, i);
				writer.add_s64(value);
			} else if (childType == byml::NodeType::U64) {
				u64 value;
				node.get_u64_by_idx(&value, i);
				writer.add_u64(value);
			} else if (childType == byml::NodeType::F64) {
				f64 value;
				node.get_f64_by_idx(&value, i);
				writer.add_f64(value);
			} else if (childType == byml::NodeType::Null) {
				writer.add_null();
			}
		}
	} else if (node.get_type() == byml::NodeType::Hash) {
		for (u32 i = 0; i < node.get_size(); i++) {
			byml::NodeType childType;
			node.get_type_by_idx(&childType, i);
			u32 keyIdx;
			node.get_key_by_idx(&keyIdx, i);
			std::string key = node.get_hash_string(keyIdx);
			if (childType == byml::NodeType::Hash) {
				byml::Reader container;
				node.get_container_by_idx(&container, i);
				writer.push_hash(key);
				copy_byml_r(writer, container);
				writer.pop();
			} else if (childType == byml::NodeType::Array) {
				byml::Reader container;
				node.get_container_by_idx(&container, i);
				writer.push_array(key);
				copy_byml_r(writer, container);
				writer.pop();
			} else if (childType == byml::NodeType::String) {
				std::string value;
				node.get_string_by_idx(&value, i);
				writer.add_string(key, value);
			} else if (childType == byml::NodeType::Bool) {
				bool value;
				node.get_bool_by_idx(&value, i);
				writer.add_bool(key, value);
			} else if (childType == byml::NodeType::S32) {
				s32 value;
				node.get_s32_by_idx(&value, i);
				writer.add_s32(key, value);
			} else if (childType == byml::NodeType::U32) {
				u32 value;
				node.get_u32_by_idx(&value, i);
				writer.add_u32(key, value);
			} else if (childType == byml::NodeType::F32) {
				f32 value;
				node.get_f32_by_idx(&value, i);
				writer.add_f32(key, value);
			} else if (childType == byml::NodeType::S64) {
				s64 value;
				node.get_s64_by_idx(&value, i);
				writer.add_s64(key, value);
			} else if (childType == byml::NodeType::U64) {
				u64 value;
				node.get_u64_by_idx(&value, i);
				writer.add_u64(key, value);
			} else if (childType == byml::NodeType::F64) {
				f64 value;
				node.get_f64_by_idx(&value, i);
				writer.add_f64(key, value);
			} else if (childType == byml::NodeType::Null) {
				writer.add_null(key);
			}
		}
	}
}

void copy_byml(byml::Writer& writer, const byml::Reader& node) {
	if (node.get_type() == byml::NodeType::Array)
		writer.push_array();
	else if (node.get_type() == byml::NodeType::Hash)
		writer.push_hash();

	copy_byml_r(writer, node);

	writer.pop();
}

std::string print_byml(const byml::Reader& node, s32 level = 0) {
	std::string out;

	if (node.get_type() == byml::NodeType::Array) {
		out += "[\n";
		for (u32 i = 0; i < node.get_size(); i++) {
			for (s32 j = 0; j < level; j++) out += "\t";
			byml::NodeType childType;
			node.get_type_by_idx(&childType, i);
			if (childType == byml::NodeType::Hash) {
				byml::Reader container;
				node.get_container_by_idx(&container, i);
				out += print_byml(container, level + 1);
			} else if (childType == byml::NodeType::Array) {
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
				bool value;
				node.get_bool_by_idx(&value, i);
				out += value ? "true" : "false";
			} else if (childType == byml::NodeType::Null) {
				out += "null";
			} else {
				out += std::format("{:x}", (u8)childType);
			}
			if (i != node.get_size() - 1) out += ", ";
			out += "\n";
		}
		for (s32 j = 0; j < level - 1; j++) out += "\t";
		out += "]";
	} else if (node.get_type() == byml::NodeType::Hash) {
		out += "{\n";
		for (u32 i = 0; i < node.get_size(); i++) {
			for (s32 j = 0; j < level; j++) out += "\t";
			byml::NodeType childType;
			node.get_type_by_idx(&childType, i);
			u32 keyIdx;
			node.get_key_by_idx(&keyIdx, i);
			out += std::format("\"{}\": ", node.get_hash_string(keyIdx));
			if (childType == byml::NodeType::Hash) {
				byml::Reader container;
				node.get_container_by_idx(&container, i);
				out += print_byml(container, level + 1);
			} else if (childType == byml::NodeType::Array) {
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
				bool value;
				node.get_bool_by_idx(&value, i);
				out += value ? "true" : "false";
			} else if (childType == byml::NodeType::Null) {
				out += "null";
			} else {
				out += std::format("{:x}", (u8)childType);
			}
			if (i != node.get_size() - 1) out += ", ";
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
	r = util::read_file(fileContents, argv[1]);
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
