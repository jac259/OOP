#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>
#include <vector>
#include <unordered_map>

struct Value {
  virtual ~Value() = default;
};

struct Null_Value : Value {};

template <class T> // used for numbers and bools
struct Primitive_Value : Value {
  Primitive_Value(T t) : value(t) {}
  T value;
};

struct String_Value : Value, std::string {
  using std::string::string;
};

struct Array_Value : Value, std::vector<Value *> {
  using std::vector<Value *>::vector;
};

struct Object_Value : Value, std::unordered_map<std::string, Value*> {
  using std::unordered_map<std::string, Value*>::unordered_map;
};
#endif
