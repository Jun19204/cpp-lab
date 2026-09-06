#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class QueryResult;

// file을 읽고 map을 생성하는 클래스
class TextQuery {
public:
  using LineNo = std::vector<std::string>::size_type;

  explicit TextQuery(std::ifstream &);
  [[nodiscard]] QueryResult query(const std::string &) const;

private:
  std::shared_ptr<std::vector<std::string>> file_;
  std::map<std::string, std::shared_ptr<std::set<LineNo>>> word_map_;
};




class QueryBase {
  friend class Query;

protected:
  using LineNo = TextQuery::LineNo;
  virtual ~QueryBase() = default;

private:
  [[nodiscard]] virtual QueryResult eval(const TextQuery &) const = 0;
  [[nodiscard]] virtual std::string rep() const = 0;
};




class QueryResult {
  friend std::ostream &print(std::ostream &, const QueryResult &);

public:
  QueryResult(const std::string &sought,
              std::shared_ptr<std::set<TextQuery::LineNo>> lines,
              std::shared_ptr<std::vector<std::string>> file)
      : sought_(sought), 
        lines_(std::move(lines)), 
        file_(std::move(file)) {}

  [[nodiscard]] auto getFile() const 
    -> std::shared_ptr<std::vector<std::string>> {
    return file_;
  }

  [[nodiscard]] auto begin() const { 
    return lines_->begin(); 
  }

  [[nodiscard]] auto end() const { 
    return lines_->end(); 
  }

private:
  std::string sought_;
  std::shared_ptr<std::set<TextQuery::LineNo>> lines_;
  std::shared_ptr<std::vector<std::string>> file_;
};




class Query {
  friend Query operator~(const Query &);
  friend Query operator|(const Query &, const Query &);
  friend Query operator&(const Query &, const Query &);

public:
  explicit Query(const std::string &);

  [[nodiscard]] QueryResult eval(const TextQuery &) const;
  [[nodiscard]] std::string rep() const;

private:
  explicit Query(std::shared_ptr<QueryBase> query) 
    : query_(std::move(query)) {}

  std::shared_ptr<QueryBase> query_;
};

std::ostream &operator<<(std::ostream &, const Query &);




class WordQuery : public QueryBase {
  friend class Query;

  explicit WordQuery(const std::string &str) 
    : query_word_(str) {}

  [[nodiscard]] QueryResult eval(const TextQuery &text) const override {
    return text.query(query_word_);
  }

  [[nodiscard]] std::string rep() const override { 
    return query_word_; 
  }

  std::string query_word_;
};




class NotQuery : public QueryBase {
  friend Query operator~(const Query &);

  explicit NotQuery(const Query &query) 
    : query_(query) {}

  [[nodiscard]] std::string rep() const override {
    return "~(" + query_.rep() + ")";
  }

  [[nodiscard]] QueryResult eval(const TextQuery &) const;

  Query query_;
};




class BinaryQuery : public QueryBase {
protected:
  BinaryQuery(const Query &lhs, 
              const Query &rhs, 
              const std::string &op_sym)
      : lhs_(lhs), 
        rhs_(rhs), 
        op_sym_(op_sym) {}

  [[nodiscard]] std::string rep() const override {
    return "(" + lhs_.rep() + " " + op_sym_ + " " + rhs_.rep() + ")";
  }

  Query lhs_;
  Query rhs_;
  std::string op_sym_;
};




class AndQuery : public BinaryQuery {
  friend Query operator&(const Query &, const Query &);

  AndQuery(const Query &lhs, const Query &rhs) 
    : BinaryQuery(lhs, rhs, "&") {}

  [[nodiscard]] QueryResult eval(const TextQuery &) const override;
};

class OrQuery : public BinaryQuery {
  friend Query operator|(const Query &, const Query &);

  OrQuery(const Query &lhs, const Query &rhs) 
    : BinaryQuery(lhs, rhs, "|") {}

  [[nodiscard]] QueryResult eval(const TextQuery &) const override;
};

