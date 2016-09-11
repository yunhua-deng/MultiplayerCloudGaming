#pragma once

#include <vector>
#include <tuple>
#include <list>
#include <set>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm> // random_shuffle
#include <ctime>
#include <cmath>
#include <direct.h>

using namespace std;

vector<vector<string>> ReadDelimitedTextFileIntoVector(const string, const char, const bool);

double GetMeanValue(const vector<double> &v);
double GetStdValue(const vector<double> &v);
double GetMinValue(const vector<double> &v);
double GetMaxValue(const vector<double> &v);
double GetPercentile(vector<double>, const double);
double GetRatioOfGreaterThan(const vector<double>&, const double);