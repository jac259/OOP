#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <iostream>
#include <sstream>

// List of types for use in printing
enum Type {
  literal = 0,
  array = 1,
  object = 2
};

// Abstract base class
struct Value {
  virtual ~Value() = default;
  virtual int weight() const = 0;
  virtual std::string print() {}
  virtual int type() const = 0;
};

// Intermediate class to define common behavior of all literals
// Includes strings, numbers, true, false, and null
struct Literal_Value : Value {
  int weight() const { return 1; }
  int type() const { return literal; }
};

// 'null' value
struct Null_Value : Literal_Value {
  std::string print() { return "null"; }
};

// 'number' value -- stores value as double
struct Number_Value : Literal_Value {
  double value;
  Number_Value(double d) : value(d) {}
  std::string print() {
    std::stringstream ss;
    ss << value;
    return ss.str();
  }
};

// 'true' and 'false' values -- differentiates using bool
struct Boolean_Value : Literal_Value {
  bool value;
  Boolean_Value(bool b) : value(b) {}
  std::string print() { return value ? "true" : "false"; }
};

// 'string' value -- inherits stringstream, prints content flanked by quotes
struct String_Value : Literal_Value, std::stringstream {
  using std::stringstream::stringstream;
  std::string getString() { return this->str(); }
  std::string print() { return "\"" + this->str() + "\""; }
};

// 'array' value -- inherits vector
struct Array_Value : Value, std::vector<std::unique_ptr<Value>> {
  using std::vector<std::unique_ptr<Value>>::vector;
  int weight() const;
  int type() const { return array; }
};

// 'object' value -- inherits vector -- maps key and value as string and value pointer pair
struct Object_Value : Value, std::vector<std::pair<std::string, std::unique_ptr<Value>>> {
  using std::vector<std::pair<std::string, std::unique_ptr<Value>>>::vector;
  int weight() const;
  int type() const { return object; }
};

// Recursively sums weight of children
int Array_Value::weight() const {
  int N = 1; // Base weight of array
  for(auto&& v : * this)
    N += v->weight();
  return N;
}

// Recursively sums weight of children
int Object_Value::weight() const {
  int N = 1; // Base weight of object
  for(auto&& v : * this)
    N += v.second->weight(); // Key-value pair weighs as much as its value
  return N;
}

#endif
