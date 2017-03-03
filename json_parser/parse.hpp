#ifndef PARSE_HPP
#define PARSE_HPP

#include "value.hpp"

struct Parser {
private:
  std::vector<std::unique_ptr<Value>> items; // vector to accumulate items during parsing
  int _weight = 0;
  const std::string indent = "  ";

  // error message for invalid character
  const std::string& GetInvalidCharError() {
    static std::string InvalidCharError("Invalid character.");
    return InvalidCharError;
  }

  // Parsing methods
  void Ignore_Whitespace(std::string::iterator&, std::string::iterator&);
  std::unique_ptr<Value> Parse(std::string::iterator&, std::string::iterator&);
  std::unique_ptr<Value> Parse_Bool(bool, std::string::iterator&, std::string::iterator&);
  std::unique_ptr<Value> Parse_Null(std::string::iterator&, std::string::iterator&);
  std::unique_ptr<Value> Parse_String(std::string::iterator&, std::string::iterator&);
  std::unique_ptr<Value> Parse_Number(std::string::iterator&, std::string::iterator&);
  std::unique_ptr<Value> Parse_Object(std::string::iterator&, std::string::iterator&);
  std::unique_ptr<Value> Parse_Array(std::string::iterator&, std::string::iterator&);

  int Weight();
  std::string Print(std::unique_ptr<Value>, int&);
  std::string Indent(int);
  
public:
  Parser() {};
  ~Parser() {};

  // Helper function
  void Parse(std::string);
  void Print();
};

void Parser::Parse(std::string str) {
  // grab iterators for front and back of document
  std::string::iterator f = str.begin();
  std::string::iterator l = str.end();

  // parse each item in the document
  while(f != l)
    items.push_back(Parse(f, l));

  // display the weight of the document
  std::cout << "Weight of document: " << Weight() << "\n";
  std::cout << "------------------------------\n";
  Print();
}

void Parser::Print() {
  std::stringstream ss;
  int depth = 0;
  
  for(auto&& item : items)
    ss << Print(std::move(item), depth) << (items.back() != item ? "\n\n" : "");

  std::cout << ss.str();
}

std::string Parser::Print(std::unique_ptr<Value> value, int & depth) {
  std::stringstream ss;
  
  switch(value->type()) {
    
  case literal: {
    ss << value->print();
    break;
  }
    
  case array: {
    std::unique_ptr<Array_Value> arr(dynamic_cast<Array_Value*>(value.release()));
    
    if(!(*arr).size()) {
      ss << "[]";
      break;
    }
    else {
      ss << "[\n";
      ++depth;
    }
    
    for(auto&& a_val : *arr)
      ss << Indent(depth) << Print(std::move(a_val), depth) << (a_val != (*arr).back() ? ",\n" : "\n");
    
    --depth;
    ss << Indent(depth) << "]";
    break;
  }
      
  case object: {
    std::unique_ptr<Object_Value> obj(dynamic_cast<Object_Value*>(value.release()));
    
    if (!(*obj).size()) {
      ss << "{}";
      break;
    }
    else {
      ss << "{\n";
      ++depth;
    }
      
    for(auto&& o_val : *obj)
      ss << Indent(depth) << "\"" << o_val.first << "\": " << Print(std::move(o_val.second), depth) << (o_val != (*obj).back() ? ",\n" : "\n");
    --depth;
    ss << Indent(depth) << "}";
    break;
  }
  }
  
  return ss.str();
}

std::string Parser::Indent(int depth) {
  std::stringstream ss;
  for(int i = 0; i < depth; ++i)
    ss << indent;
  return ss.str();
}

int Parser::Weight() {
  // sums weight of each item in document
  int wt = 0;
  for(auto&& item : items)
    wt += item->weight();
  return wt;
}

std::unique_ptr<Value> Parser::Parse(std::string::iterator &f, std::string::iterator &l) {
  Ignore_Whitespace(f, l);
  
  if (f != l) {
    switch(*f) {
    case '"': return Parse_String(f, l);       // quote starts string
    case 't': return Parse_Bool(true, f, l);   // 't' starts true
    case 'f': return Parse_Bool(false, f, l);  // 'f' starts false
    case 'n': return Parse_Null(f, l);         // 'n' starts null
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': return Parse_Number(f, l);       // digits and minus start number
    case '[': return Parse_Array(f, l);        // square bracket starts array
    case '{': return Parse_Object(f, l);       // curly bracket starts object
      
    default: throw std::runtime_error(GetInvalidCharError());
    }
  }
}

void Parser::Ignore_Whitespace(std::string::iterator &f, std::string::iterator &l) {
  // skip spaces, newlines, indents, null characters, colons, and commas
  while(f != l) {
    switch(*f) {
    case ' ':
    case '\n':
    case '\t':
    case '\v':
    case '\0':
    case ':':
    case ',': ++f; continue;
    default: return;
    }
  }
}

std::unique_ptr<Value> Parser::Parse_Array(std::string::iterator &f, std::string::iterator &l) {
  Array_Value * arr = new Array_Value();

  ++f; // skip initial bracket

  while(f != l && *f != ']') {
    arr->push_back(Parse(f, l)); // parse the next item and add it to the array

    if (f != l && *f != ']')
      ++f; // only increment if within doc or array    
  }

  if(*f == ']' && f != l)
    ++f; // step over end bracket if not eof

  //_weight += arr->weight();
  return std::unique_ptr<Value>(arr);
}

std::unique_ptr<Value> Parser::Parse_Object(std::string::iterator &f, std::string::iterator &l) {
  Object_Value * obj = new Object_Value();
  std::unique_ptr<Value> key;
  std::unique_ptr<Value> val;

  ++f; // skip initial bracket

  while(f != l && *f != '}') {
    key = Parse(f, l); // string
    val = Parse(f, l); // any valid json type

    auto sv = key.release(); // release key from unique_ptr to raw pointer
    std::string str = dynamic_cast<String_Value *>(sv)->getString(); // get string from key
    
    obj->push_back(std::make_pair(str, std::move(val))); // add key-value pair to object

    if (f != l && *f != '}')
      ++f; // only increment if within doc or object

  }

  if(*f == '}' && f != l)
    ++f; // step over end bracket if not eof

  //_weight += obj->weight();
  return std::unique_ptr<Value>(obj);
}

std::unique_ptr<Value> Parser::Parse_String(std::string::iterator &f, std::string::iterator &l) {
  String_Value * buffer = new String_Value();
  ++f; // skip initial quote
  
  while(f != l && *f != '"') {
    if(*f == '\\') {
      ++f; // handle escape character (immediately consume next character, not handling specials)
      if(f == l)
	break;
    }
    buffer->put(*f);
    //buffer << *f; // add char to buffer
    ++f;
  }

  if(*f == '"' && f != l)
    ++f; // if the loop finished because it hit a quote, step over the end quote

  //_weight += buffer->weight();
  return std::unique_ptr<Value>(buffer);
}

std::unique_ptr<Value> Parser::Parse_Number(std::string::iterator &f, std::string::iterator &l) {
  std::stringstream buffer;
  buffer.put(*f);
  ++f;
  while(f!=l && (isdigit(*f) || *f == '.' || *f == 'e' || *f == 'E' || *f == '+' || *f == '-' )) {
    // accept digits, decimal points, exponentials & exponential power signs
    buffer.put(*f); // add char to buffer
    ++f;
  }

  //Primitive_Value<double> * val = new Primitive_Value<double>(stod(buffer.str()));
  Number_Value * val = new Number_Value(stod(buffer.str()));
  _weight += val->weight();
  return std::unique_ptr<Value>(val);
}

std::unique_ptr<Value> Parser::Parse_Bool(bool b, std::string::iterator &f, std::string::iterator &l) {
  ++f;
  
  if(b) { // if 't' hit, check for 'rue'
    if(*f == 'r') {
      ++f;
      if(*f == 'u') {
	++f;
	if(*f == 'e') {
	  ++f;
	  //Primitive_Value<bool> * val = new Primitive_Value<bool>(true);
	  Boolean_Value * val = new Boolean_Value(true);
	  //_weight += val->weight();
	  return std::unique_ptr<Value>(val);
	}
      }
    }
  }
  else { // if 'f' hit, check for 'alse'
    if(*f == 'a') {
      ++f;
      if(*f == 'l') {
	++f;
	if(*f == 's') {
	  ++f;
	  if(*f == 'e') {
	    ++f;
	    //Primitive_Value<bool> * val = new Primitive_Value<bool>(false);
	    Boolean_Value * val = new Boolean_Value(false);
	    //_weight += val->weight();
	    return std::unique_ptr<Value>(val);
	  }
	}
      }
    }
  }
  // if 'true' or 'false' were not successfully completed, throw error
  throw std::runtime_error(GetInvalidCharError());
}

std::unique_ptr<Value> Parser::Parse_Null(std::string::iterator &f, std::string::iterator &l) {
  ++f;

  // if 'n' hit check for 'ull'
  if(*f == 'u') {
    ++f;
    if(*f == 'l') {
      ++f;
      if(*f == 'l') {
	++f;
	Null_Value * val = new Null_Value();
	//_weight += val->weight();
	return std::unique_ptr<Value>(new Null_Value());
      }
    }
  }
  // if 'null' was not successfully completed, throw error
  throw std::runtime_error(GetInvalidCharError());
}

#endif
