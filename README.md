# OOP
Object-Oriented Programming Course
----------------------------------

JSON Parser
 * This program
   * takes in a file containing json text on standard input,
   * parses and formats it, and
   * returns the formatted document on standard output.
 * The program can be run with commands such as:
   * ./build < jsonText.json > json.txt
   * curl https://reddit.com/r/front.json | ./build > json.txt
     * NOTE: using curl @ reddit.com seems to work intermittently
 * It is advised to print the output to a text file. While it can be output to the command line the documents are often too large to display fully.