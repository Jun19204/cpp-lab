#ifndef PARSER_H
#define PARSER_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Parser {
  std::vector<uint32_t> parseBinary(const std::filesystem::path& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      throw std::runtime_error("파일 열기 실패: " + filepath.string());
    }
    std::streamsize raw_size = file.tellg();
    if (raw_size < 0) {
      throw std::runtime_error("파일 크기 읽어오기 실패.");
    }
    auto file_size = static_cast<size_t>(raw_size);
    if (file_size == 0) {
      throw std::runtime_error("파일이 비어있습니다(0 bytes)");
    }
    if (file_size % sizeof(uint32_t) != 0) {
      throw std::runtime_error("잘못된 바이너리 파일(" \
          + std::to_string(file_size) + " bytes)");
    }
    file.seekg(0, std::ios::beg);
    
    size_t num_instructions = file_size / sizeof(uint32_t);
    std::vector<uint32_t> instructions;
    instructions.resize(num_instructions);
    
    if (!file.read(reinterpret_cast<char*>(instructions.data()), raw_size)) {
      throw std::runtime_error("파일 읽기 I/O 에러");
    }
    return instructions;
  }
};

#endif
