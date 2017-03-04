# Object-Oriented Programming Course

JSON Parser
----------------------------------
 * This program is a command line tool that
   * takes in a file containing json text on standard input,
   * parses and formats it, and
   * returns the formatted document on standard output.
 * The program can be run with commands such as:
   * ./build < jsonText.json > json.txt
   * curl https://reddit.com/r/front.json | ./build > json.txt
     * NOTE: using curl @ reddit.com seems to work intermittently
 * This program is intended to receive a file rather than parse line-by-line input from the command line. This behavior can be changed by editing main.cpp.
 * It is advised to print the output to a text file. The output can be printed to the command line for short documents but often the documents too large to display fully.