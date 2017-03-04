#include "parse.hpp"

#include <iostream>

int main() {
  std::string temp;
  std::stringstream json;
  Parser p;

  // Concatenates each line of input to stringstream
  // This is meant to handle files passed in rather than text typed line by line
  while(std::getline(std::cin, temp))
    json << temp;

  // Parse it!
  p.Parse(json.str());

  return 0;
}
