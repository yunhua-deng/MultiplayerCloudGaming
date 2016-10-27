#pragma once

#include "Base.h"

namespace MatchmakingProblem
{	
	struct ClientType;
	struct DatacenterType;
	
	struct ClientType
	{
		int id;
		double chargedTrafficVolume;
		map<int, double> delayToDatacenter;

		/*for MaximumMatchingProblem*/
		vector<DatacenterType*> eligibleDatacenters;
		DatacenterType* assignedDatacenter = nullptr;

		/*for ParetoOptimalMatchingProblem*/
		vector<DatacenterType*> eligibleDatacenters_G;
		vector<DatacenterType*> eligibleDatacenters_R;
		map<int, vector<DatacenterType*>> eligibleDatacenters_R_indexed_by_G;
		DatacenterType* assignedDatacenter_G = nullptr;
		DatacenterType* assignedDatacenter_R = nullptr;
		bool isGrouped;
		
		ClientType(int givenID)
		{
			this->id = givenID;
		}		
	};	
	bool ClientComparatorByFewerEligibleDatacenters(const ClientType* a, const ClientType* b);
	
	struct DatacenterType
	{
		int id; // id of this dc (fixed once initilized)
		double priceServer; // server price (per server per session duration that is supposed to be up to 1 hour)
		double priceBandwidth; // bandwidth price per unit traffic volume (per GB)
		map<int, double> delayToClient; // delay value mapped with client's id (fixed once initialized)	
		map<int, double> delayToDatacenter; // delay value mapped with dc's id (fixed once initialized)		
		
		/*for MaximumMatchingProblem*/
		vector<ClientType*> coverableClients;
		vector<ClientType*> assignedClients;

		/*for ParetoOptimalMatchingProblem*/
		vector<ClientType*> assignedClients_G;
		vector<ClientType*> assignedClients_R;
		
		DatacenterType(int givenID)
		{
			this->id = givenID;
		}
	};	

	struct DatacenterPointerVectorCmp
	{
		bool operator()(const vector<DatacenterType*>& a, const vector<DatacenterType*>& b) const
		{
			if (a.size() > b.size()) return false;
			else if (a.size() == b.size())
			{
				if (a.empty()) return false;
				else
				{
					for (size_t i = 0; i < a.size(); i++)
					{
						if (i < (a.size() - 1))
						{
							if (a.at(i)->id > b.at(i)->id) return false;
							else if (a.at(i)->id < b.at(i)->id) return true;
						}
						else return (a.back()->id < b.back()->id);
					}
				}
			}
			else return true;
		}
	};
	
	class MatchmakingProblemBase
	{			
	public:
		string dataDirectory = ".\\Data\\"; // root path for input and output
		void Initialize();
	protected:
		vector<ClientType> globalClientList; // read from input
		vector<DatacenterType> globalDatacenterList; // read from input		
	};

	class MaximumMatchingProblem : public MatchmakingProblemBase
	{
	public:		
		string outputDirectory = dataDirectory + "MaximumMatchingProblem\\";
		ofstream outFile;
		void Simulate(const string algToRun, const int clientCount = 100, const int latencyThreshold = 100, const int sessionSize = 10, const int simulationCount = 100);
	private:
		vector<ClientType> candidateClients;
		vector<DatacenterType> candidateDatacenters;
		DatacenterType* GetClientNearestEligibleDC(ClientType & client);
		void RandomAssignmentGrouping();
		void NearestAssignmentGrouping();
		void SimpleGreedyGrouping(const int sessionSize);
		void LayeredGreedyGrouping(const int sessionSize);
	};

	class ParetoOptimalMatchingProblem : public MatchmakingProblemBase
	{
	public:
		string outputDirectory = dataDirectory + "MaximumMatchingProblem\\";
		ofstream outFile;
		void FindEligibleClients(const int latencyThreshold = 100);
		void Simulate(const string algToRun, const int clientCount = 100, const int sessionSize = 10, const int simulationCount = 100);
	private:
		vector<ClientType> eligibleClients; // subset of globalClientList
		vector<ClientType> candidateClients; // subset of eligibleClients
		vector<DatacenterType> candidateDatacenters; // not a subset of globalDatacenterList but a copy of it
		void RandomAssignmentGrouping(const int sessionSize);
		void RGreedyGrouping(const int sessionSize);
	};
}