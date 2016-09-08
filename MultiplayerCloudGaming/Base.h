#ifndef BASE_H_INCLUDED
#define BASE_H_INCLUDED
#endif
#pragma once

#include <vector>
#include <tuple>
#include <list>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <ctime>
#include <cmath>
#include <direct.h>

using namespace std;

double GetMeanValue(const vector<double> &v);
double GetStdValue(const vector<double> &v);
double GetMinValue(const vector<double> &v);
double GetMaxValue(const vector<double> &v);
double GetPercentile(vector<double>, const double);
double GetRatioOfGreaterThan(const vector<double>&, const double);
vector<vector<string>> ReadDelimitedTextFileIntoVector(const string, const char, const bool);