#ifndef PARSE_HPP
#define PARSE_HPP

#include "value.hpp"

struct Parser {
private:
  std::vector<std::unique_ptr<Value>> items; // Vector to accumulate items during parsing
  const std::string indent = "  "; // Two-space indent for use in printing; \t is too long

  // Static error message for invalid character
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

  // Output methods
  int Weight();
  std::string Print(std::unique_ptr<Value>, int&);
  std::string Indent(int);
  
public:
  Parser() {};
  ~Parser() {};

  // Helper functions
  void Parse(std::string);
  void Print();
};

void Parser::Print() {
  // Helper function for recursive printing
  
  std::stringstream ss; // Stringstream over string for speed
  int depth = 0; // Track current depth and pass by reference to update during calls

  // Call recursive function on each root-level item in the document
  // Double space between root-level items in output
  for(auto&& item : items)
    ss << Print(std::move(item), depth) << (items.back() != item ? "\n\n" : "");

  std::cout << ss.str(); // Print formatted document
}

std::string Parser::Print(std::unique_ptr<Value> value, int & depth) {
  // Recursive printing function - returns formatted json from passed value
  
  std::stringstream ss;

  // Values contain a type to differentiate during printing
  switch(value->type()) {

  case literal: {
    // Literals simply add their formatted value -- see value.hpp
    ss << value->print();
    break;
  }
    
  case array: {
    // Arrays are cast as Array_Value
    std::unique_ptr<Array_Value> arr(dynamic_cast<Array_Value*>(value.release()));
    
    if(!(*arr).size()) {
      // Return empty brackets if no children exist in the array
      ss << "[]";
      break;
    }
    else {
      // Add an open bracket and prepare to print children
      ss << "[\n";
      ++depth;
    }

    // Print each child delimited with a comma and newline
    for(auto&& a_val : *arr)
      ss << Indent(depth) << Print(std::move(a_val), depth) << (a_val != (*arr).back() ? ",\n" : "\n");

    // Reduce depth and close bracket
    --depth;
    ss << Indent(depth) << "]";
    break;
  }
      
  case object: {
    // Objects are cast as Object_Value
    std::unique_ptr<Object_Value> obj(dynamic_cast<Object_Value*>(value.release()));
    
    if (!(*obj).size()) {
      // Return empty brackets if no children exist in the object
      ss << "{}";
      break;
    }
    else {
      // Add an open bracket and prepare to print children
      ss << "{\n";
      ++depth;
    }

    // Print each key-value pair and delimit pairs with a comma and newline
    for(auto&& o_val : *obj)
      ss << Indent(depth) << "\"" << o_val.first << "\": " << Print(std::move(o_val.second), depth) << (o_val != (*obj).back() ? ",\n" : "\n");

    // Reduce depth and close bracket
    --depth;
    ss << Indent(depth) << "}";
    break;
  }
  }
  
  return ss.str(); // Return the formatted json
}

std::string Parser::Indent(int depth) {
  // Returns a string of indents based on passed depth
  std::stringstream ss;
  for(int i = 0; i < depth; ++i)
    ss << indent;
  return ss.str();
}

int Parser::Weight() {
  // Sums the weight of each item in the document
  int wt = 0;
  for(auto&& item : items)
    wt += item->weight(); // weight() is recursive for objects/arrays -- see value.hpp
  return wt;
}

void Parser::Parse(std::string text) {
  // Helper function for recursive Parse
  
  // Iterators at front and back of document
  std::string::iterator f = text.begin();
  std::string::iterator l = text.end();

  // Parse each item in the document
  // Typical documents have one root-level object/array but as per standards can have multiple
  while(f != l)
    items.push_back(Parse(f, l));

  // Display the weight of the document
  std::cout << "Weight of document: " << Weight() << "\n";
  std::cout << "------------------------------\n";

  // Pretty-print the contents of the file
  Print();
}

std::unique_ptr<Value> Parser::Parse(std::string::iterator &f, std::string::iterator &l) {
  // Recursive method calls relevant Parse_* function -- these can call this method
  
  Ignore_Whitespace(f, l); // Skip whitespace and irrelevant characters
  
  if (f != l) {
    switch(*f) {
    case '"': return Parse_String(f, l);       // Quote starts string
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
    case '9': return Parse_Number(f, l);       // Digits and minus start number
    case '[': return Parse_Array(f, l);        // Square bracket starts array
    case '{': return Parse_Object(f, l);       // Curly bracket starts object
      
    default: throw std::runtime_error(GetInvalidCharError()); // Other characters throw exception
    }
  }
}

void Parser::Ignore_Whitespace(std::string::iterator &f, std::string::iterator &l) {
  // Skip spaces, newlines, indents, null characters, colons, and commas
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
  // Parsing an array
  
  Array_Value * arr = new Array_Value();

  ++f; // Skip initial bracket

  while(f != l && *f != ']') {
    arr->push_back(Parse(f, l)); // Parse the next item and add it to the array

    if (f != l && *f != ']')
      ++f; // Only increment if within doc or array    
  }

  if(*f == ']' && f != l)
    ++f; // Step over end bracket if not eof

  return std::unique_ptr<Value>(arr);
}

std::unique_ptr<Value> Parser::Parse_Object(std::string::iterator &f, std::string::iterator &l) {
  // Parsing an object
  
  Object_Value * obj = new Object_Value();
  std::unique_ptr<Value> key;
  std::unique_ptr<Value> val;

  ++f; // skip initial bracket

  while(f != l && *f != '}') {
    key = Parse(f, l); // String
    val = Parse(f, l); // Any valid json type

    auto sv = key.release(); // Release key from unique_ptr to raw pointer
    std::string str = dynamic_cast<String_Value *>(sv)->getString(); // Get string from key
    
    obj->push_back(std::make_pair(str, std::move(val))); // Add key-value pair to object

    if (f != l && *f != '}')
      ++f; // Only increment if within doc or object
  }

  if(*f == '}' && f != l)
    ++f; // Step over end bracket if not eof

  return std::unique_ptr<Value>(obj);
}

std::unique_ptr<Value> Parser::Parse_String(std::string::iterator &f, std::string::iterator &l) {
  // Parsing a string
  
  String_Value * buffer = new String_Value(); // Inherits stringstream
  ++f; // Skip initial quote
  
  while(f != l && *f != '"') {
    if(*f == '\\') {
      ++f; // Handle escape character (immediately consume next character, not handling specials)
      if(f == l)
	break;
    }
    buffer->put(*f);
    ++f;
  }

  if(*f == '"' && f != l)
    ++f; // If the loop finished because it hit a quote, step over the end quote

  return std::unique_ptr<Value>(buffer);
}

std::unique_ptr<Value> Parser::Parse_Number(std::string::iterator &f, std::string::iterator &l) {
  // Parsing a number
  
  std::stringstream buffer; // Temporarily store number as stringstream
  bool exp = false, dec = false; // Ensure no duplicate exponentials or decimal points
  
  while(f!=l && (isdigit(*f) || *f == '.' || *f == 'e' || *f == 'E' || *f == '+' || *f == '-' )) {
    // Accept digits, decimal points, exponentials & exponential power signs
    if((exp && (*f == 'e' || *f == 'E')) || (dec && *f == '.'))
      throw std::runtime_error(GetInvalidCharError()); // Duplicate exponential / decimal point

    if(*f == 'e' || *f == 'E')
      exp = true;

    if(*f == '.')
      dec = true;

    buffer.put(*f);
    ++f;
  }

  Number_Value * val = new Number_Value(stod(buffer.str())); // Convert stringstream to double
  return std::unique_ptr<Value>(val);
}

std::unique_ptr<Value> Parser::Parse_Bool(bool b, std::string::iterator &f, std::string::iterator &l) {
  // Parsing a boolean
  
  ++f;
  
  if(b) { // If 't' hit, check for 'rue'
    if(*f == 'r') {
      ++f;
      if(*f == 'u') {
	++f;
	if(*f == 'e') {
	  ++f;
	  Boolean_Value * val = new Boolean_Value(true);
	  return std::unique_ptr<Value>(val);
	}
      }
    }
  }
  else { // If 'f' hit, check for 'alse'
    if(*f == 'a') {
      ++f;
      if(*f == 'l') {
	++f;
	if(*f == 's') {
	  ++f;
	  if(*f == 'e') {
	    ++f;
	    Boolean_Value * val = new Boolean_Value(false);
	    return std::unique_ptr<Value>(val);
	  }
	}
      }
    }
  }
  // If 'true' or 'false' were not successfully completed, throw exception
  throw std::runtime_error(GetInvalidCharError());
}

std::unique_ptr<Value> Parser::Parse_Null(std::string::iterator &f, std::string::iterator &l) {
  // Parsing a null
  
  ++f;

  // 'n' hit, check for 'ull'
  if(*f == 'u') {
    ++f;
    if(*f == 'l') {
      ++f;
      if(*f == 'l') {
	++f;
	Null_Value * val = new Null_Value();
	return std::unique_ptr<Value>(new Null_Value());
      }
    }
  }
  // If 'null' was not successfully completed, throw error
  throw std::runtime_error(GetInvalidCharError());
}

#endif
