#ifndef Y86_64_TRANS_H
#define Y86_64_TRANS_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include "nlohmann/json.hpp"
#include "nlohmann/fifo_map.hpp"

using namespace nlohmann;

template<class Key, class Value, class dummy_compare, class trans>
using my_workaround_fifo_map = fifo_map<Key, Value, fifo_map_compare<Key>, trans>;
using my_json = basic_json<my_workaround_fifo_map>;
using Json = my_json;

#include "Y86.h"

void yo2ed(std::string &filePath);
void yo2json(std::string &filePath);

int hex2dec(char);
int getAdr(char*);
int getVal(char*, char*);

#endif  // Y86_64_TRANS_H