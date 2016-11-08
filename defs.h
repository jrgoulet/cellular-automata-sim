#ifndef FOREST_DEFS_H
#define FOREST_DEFS_H

#include "Node.h"

void quit();
void forest_fire(Node* n);
void conway(Node* n);
bool toss(double p);
int toss(int low, int high);
int get_density(Node* n);

typedef std::vector<unsigned char> langv;
typedef std::tuple<double,std::string> var;
typedef std::vector<var> ctrlv;

typedef std::uniform_int_distribution<> dist;

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
