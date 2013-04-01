#include <vector>
#include <map>
#include <unordered_map>
#include "impl.h"

#ifndef _GRAM_H_
#define _GRAM_H_
#define QLENGTH 4
#ifdef __cplusplus
extern "C" {
#endif
using namespace std;

struct Gram{
char gram[QLENGTH];
int location; //strating location
int qw_id; //query word id
};

typedef vector<Gram> Grams;

extern unordered_map<int,Grams> grams;
extern map<int, Query > queries_map;

extern unordered_map<string, int> query_words;

void insert_word(char * s, int qid);

void build_grams();

#ifdef __cplusplus
}
#endif
#endif
