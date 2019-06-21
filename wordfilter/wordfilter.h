#pragma once
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <string>
using namespace std;


bool FilterDirtyWord(const std::string &input, char *output);
int AddWord(const std::string &w);
void ClearWordLib();
void initKeyWords();
