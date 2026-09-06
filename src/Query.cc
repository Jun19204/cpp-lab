#include "Query/Query.h"

#include <algorithm>
#include <sstream>

// TextQuery
TextQuery::TextQuery(std::ifstream& infile)
    : file_(std::make_shared<std::vector<std::string>>()) {
  std::string text;

  while (std::getline(infile, text)) {
    file_->push_back(text);

    const auto line_number = file_->size() - 1;
    std::istringstream line(text);

    std::string word;

    while (line >> word) {
      auto& lines = word_map_[word];

      if (!lines) {
        lines = std::make_shared<std::set<LineNo>>();
      }

      lines->insert(line_number);
    }
  }
}

QueryResult TextQuery::query(const std::string& sought) const {
  static const auto nodata =
      std::make_shared<std::set<LineNo>>();

  const auto loc = word_map_.find(sought);

  if (loc == word_map_.end()) {
    return QueryResult(sought, nodata, file_);
  }

  return QueryResult(sought, loc->second, file_);
}


// Query
Query::Query(const std::string& str)
    : query_(std::shared_ptr<QueryBase>(new WordQuery(str))) {}

QueryResult Query::eval(const TextQuery& text) const {
  return query_->eval(text);
}

std::string Query::rep() const {
  return query_->rep();
}

std::ostream& operator<<(std::ostream& os, const Query& query) {
  return os << query.rep();
}



// NotQuery
QueryResult NotQuery::eval(const TextQuery& text) const {
  auto result = query_.eval(text);

  auto ret_lines =
      std::make_shared<std::set<LineNo>>();

  const auto beg = result.begin();
  const auto end = result.end();

  auto all_lines = result.getFile();

  for (LineNo n = 0; n != all_lines->size(); ++n) {
    if (std::find(beg, end, n) == end) {
      ret_lines->insert(n);
    }
  }

  return QueryResult(rep(), ret_lines, all_lines);
}



// AndQuery / OrQuery
QueryResult AndQuery::eval(const TextQuery& text) const {
  auto left = lhs_.eval(text);
  auto right = rhs_.eval(text);

  auto ret_lines =
      std::make_shared<std::set<LineNo>>();

  std::set_intersection(
      left.begin(),
      left.end(),
      right.begin(),
      right.end(),
      std::inserter(*ret_lines, ret_lines->begin()));

  return QueryResult(rep(), ret_lines, left.getFile());
}

QueryResult OrQuery::eval(const TextQuery& text) const {
  auto right = rhs_.eval(text);
  auto left = lhs_.eval(text);

  auto ret_lines =
      std::make_shared<std::set<LineNo>>(
          left.begin(), left.end());

  ret_lines->insert(right.begin(), right.end());

  return QueryResult(rep(), ret_lines, left.getFile());
}



// Query Operators
Query operator~(const Query& operand) {
  return Query(
      std::shared_ptr<QueryBase>(
          new NotQuery(operand)));
}

Query operator&(const Query& lhs, const Query& rhs) {
  return Query(
      std::shared_ptr<QueryBase>(
          new AndQuery(lhs, rhs)));
}

Query operator|(const Query& lhs, const Query& rhs) {
  return Query(
      std::shared_ptr<QueryBase>(
          new OrQuery(lhs, rhs)));
}



// Output
std::ostream& print(
    std::ostream& os,
    const QueryResult& result) {
  os << result.sought_ << " occurs "
     << result.lines_->size() << " "
     << (result.lines_->size() == 1 ? "time" : "times")
     << '\n';

  for (const auto num : *result.lines_) {
    os << "\t(line "
       << num + 1
       << ") "
       << result.file_->at(num)
       << '\n';
  }

  return os;
}


