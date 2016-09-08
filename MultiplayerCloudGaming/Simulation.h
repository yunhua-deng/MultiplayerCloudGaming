#ifndef SIMULATION_H_INCLUDED
#define SIMULATION_H_INCLUDED
#endif
#pragma once

#include "Base.h"

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

bool DCComparatorByPriceServer(DatacenterClass* A, DatacenterClass* B);
bool DCComparatorByPriceBandwidth(DatacenterClass* A, DatacenterClass* B);
bool DCComparatorByPriceCombined(DatacenterClass* A, DatacenterClass* B);
bool DCComparatorByAverageCostPerClient(DatacenterClass* A, DatacenterClass* B);
bool EligibleDCComparatorByDelay(const tuple<int, double, double, double, double> &A, const tuple<int, double, double, double, double> &B);
bool EligibleDCComparatorByPriceServer(const tuple<int, double, double, double, double> &A, const tuple<int, double, double, double, double> &B);
bool EligibleDCComparatorByPriceBandwidth(const tuple<int, double, double, double, double> &A, const tuple<int, double, double, double, double> &B);
bool EligibleDCComparatorByPriceCombined(const tuple<int, double, double, double, double> &A, const tuple<int, double, double, double, double> &B);
void ResetEligibleDatacentersCoverableClients(const vector<ClientClass*> &clients, const vector<DatacenterClass*> &datacenters);
void ResetAssignment(const vector<ClientClass*> &clients, const vector<DatacenterClass*> &datacenters);

// matchmaking for basic problem
// result: the datacenter for hosting the G-server, and a list of clients to be involved
// return true if found
bool MatchmakingProblemBasic(vector<DatacenterClass*> allDatacenters,
	vector<ClientClass*> allClients,
	int &GDatacenterID,
	vector<ClientClass*> &sessionClients,
	double SESSION_SIZE,
	double DELAY_BOUND_TO_G,
	double DELAY_BOUND_TO_R);

// just to find if there are any eligible datacenters to open GS given the input (candidate datacenters, client group, delay bounds) 
// record all of them if found
// for general problem
void SearchEligibleGDatacenter(vector<DatacenterClass*> allDatacenters,
	vector<ClientClass*> sessionClients,
	vector<DatacenterClass*> &eligibleGDatacenters,
	double DELAY_BOUND_TO_G,
	double DELAY_BOUND_TO_R);

// result: a list of datacenters that are eligible for hosting the G-server, and a list of clients to be involved 
// return true if found
// for general problem
bool MatchmakingProblemGeneral(vector<DatacenterClass*> allDatacenters,
	vector<ClientClass*> allClients,
	vector<ClientClass*> &sessionClients,
	vector<DatacenterClass*> &eligibleGDatacenters,
	double SESSION_SIZE,
	double DELAY_BOUND_TO_G,
	double DELAY_BOUND_TO_R);

// used inside each strategy function
// for general problem
void ExperimentSetupProblemGeneral(DatacenterClass *GDatacenter,
	vector<ClientClass*> sessionClients,
	vector<DatacenterClass*> allDatacenters,
	double DELAY_BOUND_TO_G,
	double DELAY_BOUND_TO_R);

// include G-server's cost into the total cost according to the group size
// used inside the following strategy functions
// for general problem only
void IncludeGServerCost(DatacenterClass *GDatacenter, double SESSION_SIZE, bool includingGServerCost, double &tempTotalCost);

// function to get the solution output 
// return <cost_total, cost_server, cost_bandwidth, capacity_wastage, average_delay>
tuple<double, double, double, double, double> GetSolutionOutput(
	vector<DatacenterClass*> allDatacenters,
	double serverCapacity,
	vector<ClientClass*> sessionClients,
	int GDatacenterID);

// return true if and only if all clients are assigned and each client is assigned to one dc
bool CheckIfAllClientsExactlyAssigned(vector<ClientClass*> sessionClients, vector<DatacenterClass*> allDatacenters);

bool SimulationPreparation(string dataDirectory, double BANDWIDTH_INTENSITY,
	vector<vector<double>>& ClientToDatacenterDelayMatrix,
	vector<vector<double>>& InterDatacenterDelayMatrix,
	vector<double>& priceServerList,
	vector<double>& priceBandwidthList);

void WriteCostWastageDelayData(int STRATEGY_COUNT, vector<double> SERVER_CAPACITY_LIST, double SESSION_COUNT,
	vector<vector<vector<tuple<double, double, double, double, double>>>>& outcomeAtAllSessions,
	string dataDirectory, string experimentSettings);

void SimulationBasicProblem(double DELAY_BOUND_TO_G = 150, double DELAY_BOUND_TO_R = 100, double SESSION_SIZE = 10, double BANDWIDTH_INTENSITY = 2, double SESSION_COUNT = 1000);

void SimulationGeneralProblem(double DELAY_BOUND_TO_G, double DELAY_BOUND_TO_R, double SESSION_SIZE, double BANDWIDTH_INTENSITY, double SESSION_COUNT);

// Lower-Bound (LB)
// for basic problem
tuple<double, double, double, double, double> LB(
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double serverCapacity,
	int GDatacenterID);

// Lower-Bound (LB)
// overloaded for general problem
tuple<double, double, double, double, double> LB(
	vector<DatacenterClass*> eligibleGDatacenters,
	int &finalGDatacenter,
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double DELAY_BOUND_TO_G,
	double DELAY_BOUND_TO_R,
	double serverCapacity,
	bool includingGServerCost);

// Random-Assignment
// for basic problem
tuple<double, double, double, double, double> RANDOM(
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double serverCapacity,
	int GDatacenterID);

// Random-Assignment
// overloaded for general problem
tuple<double, double, double, double, double> RANDOM(
	vector<DatacenterClass*> eligibleGDatacenters,
	int &finalGDatacenter,
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double DELAY_BOUND_TO_G,
	double DELAY_BOUND_TO_R,
	double serverCapacity,
	bool includingGServerCost);

// Nearest-Assignment
// for basic problem
tuple<double, double, double, double, double> NEAREST(
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double serverCapacity,
	int GDatacenterID);

// Nearest-Assignment
// overloaded for general problem
tuple<double, double, double, double, double> NEAREST(
	vector<DatacenterClass*> eligibleGDatacenters,
	int &finalGDatacenter,
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double DELAY_BOUND_TO_G,
	double DELAY_BOUND_TO_R,
	double serverCapacity,
	bool includingGServerCost);

// Lowest-Server-Price-Datacenter-Assignment (LSP)
// for basic problem
tuple<double, double, double, double, double> LSP(
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double serverCapacity,
	int GDatacenterID);

// Lowest-Bandwidth-Price-Datacenter-Assignment (LBP)
// for basic problem
tuple<double, double, double, double, double> LBP(
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double serverCapacity,
	int GDatacenterID);

// Lowest-Combined-Price-Datacenter-Assignment (LCP)
// for basic problem
tuple<double, double, double, double, double> LCP(
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double serverCapacity,
	int GDatacenterID);

// Lowest-Server-Price-Datacenter-Assignment (LSP)
// overloaded for general problem
tuple<double, double, double, double, double> LSP(
	vector<DatacenterClass*> eligibleGDatacenters,
	int &finalGDatacenter,
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double DELAY_BOUND_TO_G,
	double DELAY_BOUND_TO_R,
	double serverCapacity,
	bool includingGServerCost);

// Lowest-Bandwidth-Price-Datacenter-Assignment (LBP)
// overloaded for general problem
tuple<double, double, double, double, double> LBP(
	vector<DatacenterClass*> eligibleGDatacenters,
	int &finalGDatacenter,
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double DELAY_BOUND_TO_G,
	double DELAY_BOUND_TO_R,
	double serverCapacity,
	bool includingGServerCost);

// Lowest-Combined-Price-Datacenter-Assignment (LCP)
// overloaded for general problem
tuple<double, double, double, double, double> LCP(
	vector<DatacenterClass*> eligibleGDatacenters,
	int &finalGDatacenter,
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double DELAY_BOUND_TO_G,
	double DELAY_BOUND_TO_R,
	double serverCapacity,
	bool includingGServerCost);

// Lowest-Capacity-Wastage-Assignment (LCW)
// if server capacity < 2, reduce to LCP
// for basic problem
tuple<double, double, double, double, double> LCW(
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double serverCapacity,
	int GDatacenterID);

// Lowest-Capacity-Wastage-Assignment (LCW)
// overloaded for general problem
tuple<double, double, double, double, double> LCW(
	vector<DatacenterClass*> eligibleGDatacenters,
	int &finalGDatacenter,
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double DELAY_BOUND_TO_G,
	double DELAY_BOUND_TO_R,
	double serverCapacity,
	bool includingGServerCost);

// Lowest-Average-Cost-Assignment (LAC)
// Idea: open exactly one server at each iteration, and where to open the server is determined based on the average cost contributed by all clients that are to be assigned to this server
// if server capacity < 2, reduce to LCP
// for basic problem
tuple<double, double, double, double, double> LAC(
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double serverCapacity,
	int GDatacenterID);

// Lowest-Average-Cost-Assignment (LAC)
// overloaded for general problem
tuple<double, double, double, double, double> LAC(
	vector<DatacenterClass*> eligibleGDatacenters,
	int &finalGDatacenter,
	const vector<ClientClass*> &sessionClients,
	const vector<DatacenterClass*> &allDatacenters,
	double DELAY_BOUND_TO_G,
	double DELAY_BOUND_TO_R,
	double serverCapacity,
	bool includingGServerCost);