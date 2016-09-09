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

class ClientClass;
class DatacenterClass;

class ClientClass
{
public:
	int id; // global id (fixed once initialized)	

	map<int, double> delayToDatacenter; // delay values mapped with dc's id (fixed once initialized)

	vector<tuple<int, double, double, double, double>> eligibleDatacenterList; // <dc's id, dc's delay, dc's server price, dc's bandwidth price, dc's combined price>
	vector<DatacenterClass*> eligibleDatacenters; // alternative way to access its eligible datacenters	

	int assignedDatacenterID; // the id of the dc to which it is assigned

	ClientClass(int givenID)
	{
		this->id = givenID;
		this->assignedDatacenterID = -1;
	}
};

class DatacenterClass
{
public:
	int id; // id of this dc (fixed once initilized)
	double priceServer; // server price (per server per session)
	double priceBandwidth; // bandwidth price (per user per session)
	double priceCombined; // for lower bound only	

	map<int, double> delayToClient; // delay value mapped with client's id (fixed once initialized)	
	map<int, double> delayToDatacenter; // delay value mapped with dc's id (fixed once initialized)

	ClientClass* nearestClient; // its nearest client (fixed once initialized)

	vector<int> coverableClientList; // clients within its coverage according to delay bounds
	vector<ClientClass*> coverableClients; // alternative way to access its coverable clients

	vector<int> assignedClientList;
	double openServerCount;

	vector<ClientClass*> unassignedCoverableClients; // to be used by some algorithms

	double averageCostPerClient; // to be used by some algorithms

	DatacenterClass(int givenID)
	{
		this->id = givenID;
		this->priceServer = 0;
		this->priceBandwidth = 0;
		this->priceCombined = 0;
	}
};

double GetMeanValue(const vector<double> &v);
double GetStdValue(const vector<double> &v);
double GetMinValue(const vector<double> &v);
double GetMaxValue(const vector<double> &v);
double GetPercentile(vector<double>, const double);
double GetRatioOfGreaterThan(const vector<double>&, const double);
vector<vector<string>> ReadDelimitedTextFileIntoVector(const string, const char, const bool);