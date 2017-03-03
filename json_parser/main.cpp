#include "parse.hpp"

#include <iostream>

int main() {
  std::string temp;
  std::stringstream json;
  Parser p;
  
  while(std::getline(std::cin, temp))
    json << temp;

  p.Parse(json.str());

  return 0;
  
}
