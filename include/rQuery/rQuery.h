#pragma once

#include <cstddef>
#include <fstream>
#include <set>
#include <map>
#include <sstream>
#include <vector>
#include <string>

class QueryResult;

// file을 읽고 map을 생성하는 클래스
class TextQuery {
public:
  using LineNo = std::vector<std::string>::size_type;

  explicit TextQuery(std::ifstream& file);
  explicit TextQuery(const TextQuery& text_query);
  explicit TextQuery(TextQuery&& text_qeury) noexcept;
  TextQuery& operator=(const TextQuery& rhs) &;
  TextQuery& operator=(TextQuery&& rhs) & noexcept;
  ~TextQuery();

  [[nodiscard]] QueryResult query(const std::string&) const;

private:
  std::vector<std::string>* file_{nullptr};
  std::size_t* use_file_{nullptr};
  std::map<std::string, std::set<LineNo>*> word_map_;
  std::size_t* use_set_{nullptr};
};

TextQuery::TextQuery(std::ifstream& file)
  : file_(new std::vector<std::string>()), use_file_(new size_t(1))
{
  std::string text;

  while (std::getline(file, text)) {
    file_->push_back(text);
    
    const auto line_number = file_->size() - 1;
    std::istringstream line(text);
    
    std::string word;
    
    while (line >> word) {
      auto* lines = word_map_[word];
      
      if (lines == nullptr) {
        lines = new std::set<LineNo>();
        use_set_ = new size_t(1);
      }
      lines->insert(line_number);
    }
  }
}

TextQuery::TextQuery(const TextQuery& text_query) {
  file_ = text_query.file_;
  ++(*use_file_);
  --(*text_query.use_file_);
  word_map_ = text_query.word_map_;
  ++(*use_set_);
  --(*text_query.use_set_);
}



