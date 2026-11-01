#ifndef TEST_PARSER_H
#define TEST_PARSER_H

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <vector>

namespace Parser {

inline std::vector<uint32_t>
parseBinary(const std::filesystem::path &filepath) {
  if (!std::filesystem::exists(filepath)) {
    throw std::runtime_error("파일이 존재하지 않습니다: " + filepath.string());
  }

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
    throw std::runtime_error("파일이 비어있습니다 (0 bytes)");
  }

  if (file_size % sizeof(uint16_t) != 0) {
    throw std::runtime_error(
        "잘못된 바이너리 파일 크기 (2바이트 배수가 아님): " +
        std::to_string(file_size) + " bytes");
  }

  file.seekg(0, std::ios::beg);
  std::vector<uint8_t> buffer(file_size);
  if (!file.read(reinterpret_cast<char*>(buffer.data()), 
                 static_cast<std::streamsize>(file_size))) {
    throw std::runtime_error("파일 읽기 I/O 에러");
  }

  std::vector<uint32_t> instructions;
  size_t pos = 0;

  while (pos < file_size) {
    if (pos + sizeof(uint16_t) > file_size) {
      throw std::runtime_error("파일이 비정상적으로 잘렸습니다.");
    }
    
    uint16_t half = 0;
    std::memcpy(&half, &buffer[pos], sizeof(uint16_t));
    
    if ((half & 0x3) != 0x3) {
      // 16비트 압축 명령어
      instructions.push_back(static_cast<uint32_t>(half));
      pos += 2;
    } else {
      // 32비트 표준 명령어
      if (pos + sizeof(uint32_t) > file_size) {
        throw std::runtime_error("32비트 명령어 크기가 부족합니다.");
      }
      uint32_t full = 0;
      std::memcpy(&full, &buffer[pos], sizeof(uint32_t));
      instructions.push_back(full);
      pos += 4;
    }
  }

  return instructions;
}

}

#endif
