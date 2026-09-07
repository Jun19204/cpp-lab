#pragma once

#include <cstddef>
#include <fstream>
#include <set>
#include <map>
#include <sstream>
#include <vector>
#include <string>

class QueryResult;

class SharedData {
public:
  using LineNo = std::vector<std::string>::size_type;
  explicit SharedData(std::ifstream& file);
  SharedData(const SharedData&) = delete;
  SharedData(SharedData&&) = delete;
  SharedData& operator=(const SharedData&) = delete;
  SharedData& operator=(SharedData&&) = delete;
  ~SharedData() = default;

  void addRef() {
    ++use_count_;
  }
  bool release() {
    return --use_count_ == 0;
  }

private:
  std::vector<std::string> file_;
  std::map<std::string, std::set<LineNo>> word_map_;
  std::size_t use_count_{1};
};

SharedData::SharedData(std::ifstream& infile) {
  std::string text;
  while (std::getline(infile, text)) {
    file_.push_back(text);

    const auto line_number{file_.size() - 1};
    std::istringstream line(text);

    std::string word;
    while (line >> word) {
      auto& lines = word_map_[word];
      lines.insert(line_number);
    }
  }
}


// file을 읽고 map을 생성하는 클래스
class TextQuery {
public:
  explicit TextQuery(std::ifstream& infile);
  explicit TextQuery(const TextQuery& text_query);
  explicit TextQuery(TextQuery&& text_query) noexcept;
  TextQuery& operator=(const TextQuery& rhs) &;
  TextQuery& operator=(TextQuery&& rhs) & noexcept;
  ~TextQuery();

  [[nodiscard]] QueryResult query(const std::string&) const;

private:
  SharedData* data_;
  void release() {
    if (data_ == nullptr) {
      return;
    }
    if (data_->release()) {
      delete data_;
      data_ = nullptr;
    }
  }
  void addRef() {
    if (data_ != nullptr) {
      data_->addRef();
    }
  }
};

TextQuery::TextQuery(std::ifstream& infile) 
  : data_(new SharedData(infile)) {}

TextQuery::TextQuery(const TextQuery& text_query) 
  : data_(text_query.data_)
{
  addRef();
}

TextQuery::TextQuery(TextQuery&& text_query) noexcept {
  data_ = text_query.data_;
  text_query.data_ = nullptr;
}

TextQuery& TextQuery::operator=(const TextQuery& rhs) & {
  if (this == &rhs) {
    return *this;
  }
  release();
  data_ = rhs.data_;
  addRef();
  return *this;
}

TextQuery& TextQuery::operator=(TextQuery&& rhs) & noexcept {
  if (this == &rhs) {
    return *this;
  }
  release();
  data_ = rhs.data_;
  rhs.data_ = nullptr;
  return *this;
}

TextQuery::~TextQuery() {
  release();
}

