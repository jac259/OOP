#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <iostream>
#include <sstream>

enum Type {
  literal = 0,
  array = 1,
  object = 2
};

struct Value {
  virtual ~Value() = default;
  virtual int weight() const = 0;
  virtual std::string print() {}
  virtual int type() const = 0;
};

struct Literal_Value : Value {
  int weight() const { return 1; }
  int type() const { return literal; }
};

struct Null_Value : Literal_Value {
  std::string print() { return "null"; }
};

struct Number_Value : Literal_Value {
  double value;
  Number_Value(double d) : value(d) {}
  std::string print() {
    std::stringstream ss;
    ss << value;
    return ss.str();
  }
};

struct Boolean_Value : Literal_Value {
  bool value;
  Boolean_Value(bool b) : value(b) {}
  std::string print() { return value ? "true" : "false"; }
};

struct String_Value : Literal_Value, std::stringstream {
  using std::stringstream::stringstream;
  std::string getString() { return this->str(); }
  std::string print() { return "\"" + this->str() + "\""; }
};

struct Array_Value : Value, std::vector<std::unique_ptr<Value>> {
  using std::vector<std::unique_ptr<Value>>::vector;
  int weight() const;
  int type() const { return array; }
};

struct Object_Value : Value, std::vector<std::pair<std::string, std::unique_ptr<Value>>> {
  using std::vector<std::pair<std::string, std::unique_ptr<Value>>>::vector;
  int weight() const;
  int type() const { return object; }
};

int Array_Value::weight() const {
  int N = 1; // base weight of array
  for(auto&& v : * this)
    N += v->weight();
  return N;
}

int Object_Value::weight() const {
  int N = 1; // base weight of array
  for(auto&& v : * this)
    N += v.second->weight();
  return N;
}

#endif
