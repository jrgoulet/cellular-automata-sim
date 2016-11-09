#ifndef FOREST_DEFS_H
#define FOREST_DEFS_H

#include "Node.h"
#include <random>

void quit();
void forest_fire(Node* n);
void conway(Node* n);
bool toss(double p);
int toss(int low, int high);
int get_density(Node* n);
std::string get_color(int i);
int get_color(std::string s);

typedef std::vector<char> langv;
typedef std::tuple<double,std::string> var;
typedef std::vector<var> ctrlv;

typedef std::uniform_int_distribution<> dist;

#define FG_BLACK    "\x1b[30m"
#define FG_RED      "\x1b[31m"
#define FG_GREEN    "\x1b[32m"
#define FG_YELLOW   "\x1b[33m"
#define FG_BLUE     "\x1b[34m"
#define FG_MAGENTA  "\x1b[35m"
#define FG_CYAN     "\x1b[36m"
#define FG_WHITE    "\x1b[37m"
#define FG_DEFAULT  "\x1b[39m"
#define BG_BLACK    "\x1b[40m"
#define BG_RED      "\x1b[41m"
#define BG_GREEN    "\x1b[42m"
#define BG_YELLOW   "\x1b[43m"
#define BG_BLUE     "\x1b[44m"
#define BG_MAGENTA  "\x1b[45m"
#define BG_CYAN     "\x1b[46m"
#define BG_WHITE    "\x1b[47m"
#define BG_DEFAULT  "\x1b[49m"

#define ERROR_ARGV_C "Improper argument count"
#define ERROR_ARGV_2 "Generation count must be at least 1"
#define ERROR_ARGV_3 "Ignition probability must be between 0 and 1"
#define ERROR_ARGV_4 "Growth probability must be between 0 and 1"
#define ERROR_ARGV_5 "Width must be at least 5"
#define ERROR_ARGV_6 "Height must be at least 5"
#define ERROR_ARGV_7 "Density must be between 0 and 1"
#define ERROR_ARGV_T "Improper argument types"
#define ERROR_FILE "An error occurred while accessing the input file."
#endif //FOREST_DEFS_H
