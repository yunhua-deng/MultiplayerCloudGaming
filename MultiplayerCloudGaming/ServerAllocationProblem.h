#pragma once

#include "Base.h"

namespace ServerAllocationProblem
{
	struct ClientType;
	struct DatacenterType;

	struct ClientType
	{
		int id; // global id (fixed once initialized)	
		double chargedTrafficVolume;
		map<int, double> delayToDatacenter; // delay values mapped with dc's id (fixed once initialized)		
		vector<DatacenterType*> eligibleDatacenters;
		int assignedDatacenterID; // the id of the dc to which it is assigned	

		ClientType(int givenID)
		{
			this->id = givenID;
			this->assignedDatacenterID = -1;
		}
	};

	struct DatacenterType
	{
		int id; // id of this dc (fixed once initilized)
		double priceServer; // server price (per server per session duration that is supposed to be up to 1 hour)
		double priceBandwidth; // bandwidth price per unit traffic volume (per GB)
		map<int, double> delayToClient; // delay value mapped with client's id (fixed once initialized)	
		map<int, double> delayToDatacenter; // delay value mapped with dc's id (fixed once initialized)		
		vector<ClientType*> coverableClients; // alternative way to access its coverable clients		
		vector<ClientType*> assignedClients;
		double openServerCount;
		vector<ClientType*> unassignedCoverableClients; // to be used by some algorithms
		double averageCostPerClient; // to be used by some algorithms
		DatacenterType(int givenID)
		{
			this->id = givenID;
			this->priceServer = 0;
			this->priceBandwidth = 0;
		}
	};
	
	bool Initialize(string, vector<ClientType*> &, vector<DatacenterType*> &);
	void SimulateBasicProblem(double, double, double, double SESSION_COUNT = 1000);
	void SimulateGeneralProblem(double, double, double, double SESSION_COUNT = 1000);

	void ResetEligibiltyCoverability(vector<ClientType*>, vector<DatacenterType*>);
	void ResetAssignment(vector<ClientType*>, vector<DatacenterType*> &);

	// matchmaking for basic problem
	// result: the datacenter for hosting the G-server, and a list of clients to be involved
	// return true if found
	bool Matchmaking4BasicProblem(vector<DatacenterType*>, vector<ClientType*>, int &, vector<ClientType*> &, double, double, double);

	// just to find if there are any eligible datacenters to open GS given the input (candidate datacenters, client group, delay bounds) 
	// record all of them if found
	// for general problem
	void SearchEligibleGDatacenter(vector<DatacenterType*>, vector<ClientType*>, vector<DatacenterType*> &, double, double);

	// result: a list of datacenters that are eligible for hosting the G-server, and a list of clients to be involved 
	// return true if found
	// for general problem
	bool Matchmaking4GeneralProblem(vector<DatacenterType*>, vector<ClientType*>, vector<ClientType*> &, vector<DatacenterType*> &, double, double, double);

	// used inside each strategy function
	// for general problem
	void SimulationSetup4GeneralProblem(DatacenterType*, vector<ClientType*>, vector<DatacenterType*>, double, double);

	// include G-server's cost into the total cost according to the group size
	// used inside the following strategy functions
	// for general problem only
	void IncludeGServerCost(DatacenterType*, double, bool, double &);

	// function to get the solution output 
	// return <cost_total, cost_server, cost_bandwidth, capacity_wastage, average_delay>
	tuple<double, double, double, double, double> GetSolutionOutput(vector<DatacenterType*>, double, vector<ClientType*>, int);

	// return true if and only if all clients are assigned and each client is assigned to one dc
	bool CheckIfAllClientsExactlyAssigned(vector<ClientType*>, vector<DatacenterType*>);

	void WriteCostWastageDelayData(int, vector<double>, double, vector<vector<vector<tuple<double, double, double, double, double>>>>&, string, string);	

	// Lower-Bound (LB)
	// for basic problem
	tuple<double, double, double, double, double> Alg_LB(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);

	// Lower-Bound (LB)
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_LB(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double, bool includingGServerCost = false);

	// Random-Assignment
	// for basic problem
	tuple<double, double, double, double, double> Alg_RANDOM(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);

	// Random-Assignment
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_RANDOM(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double, bool includingGServerCost = false);

	// Nearest-Assignment
	// for basic problem
	tuple<double, double, double, double, double> Alg_NEAREST(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);

	// Nearest-Assignment
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_NEAREST(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double, bool includingGServerCost = false);

	// Lowest-Server-Price-Datacenter-Assignment (LSP)
	// for basic problem
	tuple<double, double, double, double, double> Alg_LSP(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);

	// Lowest-Bandwidth-Price-Datacenter-Assignment (LBP)
	// for basic problem
	tuple<double, double, double, double, double> Alg_LBP(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);

	// Lowest-Combined-Price-Datacenter-Assignment (LCP)
	// for basic problem
	tuple<double, double, double, double, double> Alg_LCP(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);

	// Lowest-Server-Price-Datacenter-Assignment (LSP)
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_LSP(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double, bool includingGServerCost = false);

	// Lowest-Bandwidth-Price-Datacenter-Assignment (LBP)
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_LBP(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double, bool includingGServerCost = false);

	// Lowest-Combined-Price-Datacenter-Assignment (LCP)
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_LCP(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double, bool includingGServerCost = false);

	// Lowest-Capacity-Wastage-Assignment (LCW)
	// if server capacity < 2, reduce to LCP
	// for basic problem
	tuple<double, double, double, double, double> Alg_LCW(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);

	// Lowest-Capacity-Wastage-Assignment (LCW)
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_LCW(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double, bool includingGServerCost = false);

	// Lowest-Average-Cost-Assignment (LAC)
	// Idea: open exactly one server at each iteration, and where to open the server is determined based on the average cost contributed by all clients that are to be assigned to this server
	// if server capacity < 2, reduce to LCP
	// for basic problem
	tuple<double, double, double, double, double> Alg_LAC(const vector<ClientType*> &, const vector<DatacenterType*> &, double, int);

	// Lowest-Average-Cost-Assignment (LAC)
	// overloaded for general problem
	tuple<double, double, double, double, double> Alg_LAC(vector<DatacenterType*>, int &, const vector<ClientType*> &, const vector<DatacenterType*> &, double, double, double, bool includingGServerCost = false);		
}