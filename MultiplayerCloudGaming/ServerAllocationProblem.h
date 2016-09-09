#pragma once

#include "Base.h"

class ClientClass;
class DatacenterClass;

class ClientClass
{
public:
	int id; // global id (fixed once initialized)	
	double trafficVolume = 2; // the default value

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
	vector<ClientClass*> assignedClients;
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

bool DCComparatorByPriceServer(DatacenterClass* A, DatacenterClass* B);
bool DCComparatorByPriceBandwidth(DatacenterClass* A, DatacenterClass* B);
bool DCComparatorByPriceCombined(DatacenterClass* A, DatacenterClass* B);
bool DCComparatorByAverageCostPerClient(DatacenterClass* A, DatacenterClass* B);
bool EligibleDCComparatorByDelay(const tuple<int, double, double, double, double> &A, const tuple<int, double, double, double, double> &B);
bool EligibleDCComparatorByPriceServer(const tuple<int, double, double, double, double> &A, const tuple<int, double, double, double, double> &B);
bool EligibleDCComparatorByPriceBandwidth(const tuple<int, double, double, double, double> &A, const tuple<int, double, double, double, double> &B);
bool EligibleDCComparatorByPriceCombined(const tuple<int, double, double, double, double> &A, const tuple<int, double, double, double, double> &B);

void ResetEligibleDatacentersCoverableClients(const vector<ClientClass*> &, const vector<DatacenterClass*> &);
void ResetAssignment(const vector<ClientClass*> &, const vector<DatacenterClass*> &);

bool InputData(string, vector<vector<double>> &, vector<vector<double>> &, vector<double> &, vector<double> &);

// matchmaking for basic problem
// result: the datacenter for hosting the G-server, and a list of clients to be involved
// return true if found
bool Matchmaking4BasicProblem(vector<DatacenterClass*>, vector<ClientClass*>, int &, vector<ClientClass*> &, double, double, double);

// just to find if there are any eligible datacenters to open GS given the input (candidate datacenters, client group, delay bounds) 
// record all of them if found
// for general problem
void SearchEligibleGDatacenter(vector<DatacenterClass*>, vector<ClientClass*>, vector<DatacenterClass*> &, double, double);

// result: a list of datacenters that are eligible for hosting the G-server, and a list of clients to be involved 
// return true if found
// for general problem
bool Matchmaking4GeneralProblem(vector<DatacenterClass*>, vector<ClientClass*>, vector<ClientClass*> &, vector<DatacenterClass*> &, double, double, double);

// used inside each strategy function
// for general problem
void SimulationSetup4GeneralProblem(DatacenterClass*, vector<ClientClass*>, vector<DatacenterClass*>, double, double);

// include G-server's cost into the total cost according to the group size
// used inside the following strategy functions
// for general problem only
void IncludeGServerCost(DatacenterClass*, double, bool, double &);

// function to get the solution output 
// return <cost_total, cost_server, cost_bandwidth, capacity_wastage, average_delay>
tuple<double, double, double, double, double> GetSolutionOutput(vector<DatacenterClass*>, double, vector<ClientClass*>, int);

// return true if and only if all clients are assigned and each client is assigned to one dc
bool CheckIfAllClientsExactlyAssigned(vector<ClientClass*>, vector<DatacenterClass*>);

void WriteCostWastageDelayData(int, vector<double>, double, vector<vector<vector<tuple<double, double, double, double, double>>>>&, string, string);

void SimulateBasicProblem(double, double, double, double SESSION_COUNT = 1);

void SimulateGeneralProblem(double, double, double, double SESSION_COUNT = 1);

// Lower-Bound (LB)
// for basic problem
tuple<double, double, double, double, double> LB(const vector<ClientClass*> &, const vector<DatacenterClass*> &, double, int);

// Lower-Bound (LB)
// overloaded for general problem
tuple<double, double, double, double, double> LB(vector<DatacenterClass*>, int &, const vector<ClientClass*> &, const vector<DatacenterClass*> &, double, double, double, bool includingGServerCost = false);

// Random-Assignment
// for basic problem
tuple<double, double, double, double, double> RANDOM(const vector<ClientClass*> &, const vector<DatacenterClass*> &, double, int);

// Random-Assignment
// overloaded for general problem
tuple<double, double, double, double, double> RANDOM(vector<DatacenterClass*>, int &, const vector<ClientClass*> &, const vector<DatacenterClass*> &, double, double, double, bool includingGServerCost = false);

// Nearest-Assignment
// for basic problem
tuple<double, double, double, double, double> NEAREST(const vector<ClientClass*> &,	const vector<DatacenterClass*> &, double, int);

// Nearest-Assignment
// overloaded for general problem
tuple<double, double, double, double, double> NEAREST(vector<DatacenterClass*>,	int &, const vector<ClientClass*> &, const vector<DatacenterClass*> &, double, double, double, bool includingGServerCost = false);

// Lowest-Server-Price-Datacenter-Assignment (LSP)
// for basic problem
tuple<double, double, double, double, double> LSP(const vector<ClientClass*> &,	const vector<DatacenterClass*> &, double, int);

// Lowest-Bandwidth-Price-Datacenter-Assignment (LBP)
// for basic problem
tuple<double, double, double, double, double> LBP(const vector<ClientClass*> &,	const vector<DatacenterClass*> &, double, int);

// Lowest-Combined-Price-Datacenter-Assignment (LCP)
// for basic problem
tuple<double, double, double, double, double> LCP(const vector<ClientClass*> &,	const vector<DatacenterClass*> &, double, int);

// Lowest-Server-Price-Datacenter-Assignment (LSP)
// overloaded for general problem
tuple<double, double, double, double, double> LSP(vector<DatacenterClass*>,	int &, const vector<ClientClass*> &, const vector<DatacenterClass*> &, double, double, double, bool includingGServerCost = false);

// Lowest-Bandwidth-Price-Datacenter-Assignment (LBP)
// overloaded for general problem
tuple<double, double, double, double, double> LBP(vector<DatacenterClass*>, int &, const vector<ClientClass*> &, const vector<DatacenterClass*> &, double, double, double, bool includingGServerCost = false);

// Lowest-Combined-Price-Datacenter-Assignment (LCP)
// overloaded for general problem
tuple<double, double, double, double, double> LCP(vector<DatacenterClass*>,	int &, const vector<ClientClass*> &, const vector<DatacenterClass*> &, double, double, double, bool includingGServerCost = false);

// Lowest-Capacity-Wastage-Assignment (LCW)
// if server capacity < 2, reduce to LCP
// for basic problem
tuple<double, double, double, double, double> LCW(const vector<ClientClass*> &, const vector<DatacenterClass*> &, double, int);

// Lowest-Capacity-Wastage-Assignment (LCW)
// overloaded for general problem
tuple<double, double, double, double, double> LCW(vector<DatacenterClass*>, int &, const vector<ClientClass*> &, const vector<DatacenterClass*> &, double, double, double, bool includingGServerCost = false);

// Lowest-Average-Cost-Assignment (LAC)
// Idea: open exactly one server at each iteration, and where to open the server is determined based on the average cost contributed by all clients that are to be assigned to this server
// if server capacity < 2, reduce to LCP
// for basic problem
tuple<double, double, double, double, double> LAC(const vector<ClientClass*> &,	const vector<DatacenterClass*> &, double, int);

// Lowest-Average-Cost-Assignment (LAC)
// overloaded for general problem
tuple<double, double, double, double, double> LAC(vector<DatacenterClass*>,	int &, const vector<ClientClass*> &, const vector<DatacenterClass*> &, double, double, double, bool includingGServerCost = false);