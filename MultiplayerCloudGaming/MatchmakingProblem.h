#pragma once

#include "Base.h"

namespace MatchmakingProblem
{	
	struct ClientType;
	struct DatacenterType;
	struct SessionType;
	
	struct ClientType
	{
		int id;
		double chargedTrafficVolume;
		map<int, double> delayToDatacenter;

		vector<DatacenterType*> eligibleDatacenters_G;
		vector<DatacenterType*> eligibleDatacenters_R;		
		map<int, vector<DatacenterType*>> eligibleDatacenters_R_indexed_by_G;
		map<int, vector<DatacenterType*>> eligibleDatacenters_G_indexed_by_R;
		DatacenterType* assignedDatacenter_G = nullptr;
		DatacenterType* assignedDatacenter_R = nullptr;
		bool isGrouped = false;
		
		ClientType(int givenID)
		{
			this->id = givenID;
		}		
	};	
	bool ClientComparatorByFewerEligibleDatacenters_G(const ClientType* a, const ClientType* b);
	
	struct DatacenterType
	{
		int id; // id of this dc (fixed once initilized)
		double priceServer; // server price (per server per session duration that is supposed to be up to 1 hour)
		double priceBandwidth; // bandwidth price per unit traffic volume (per GB)
		map<int, double> delayToClient; // delay value mapped with client's id (fixed once initialized)	
		map<int, double> delayToDatacenter; // delay value mapped with dc's id (fixed once initialized)		
		
		vector<ClientType*> coverableClients_G;
		vector<ClientType*> coverableClients_R;
		vector<ClientType*> assignedClients_G;
		vector<ClientType*> assignedClients_R;
		
		DatacenterType(int givenID)
		{
			this->id = givenID;
		}
	};
	bool DatacenterComparatorByPrice(const DatacenterType a, const DatacenterType b);

	struct SessionType
	{
		vector<ClientType*> sessionClients;
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

	class ParetoMatchingProblem : public MatchmakingProblemBase
	{
	public:
		string outputDirectory = dataDirectory + "MaximumMatchingProblem\\";
		ofstream outFile;		
		void Simulate(const int latencyThreshold = 100, const int clientCount = 100, const int sessionSize = 10, const int serverCapacity = 4, const int simulationCount = 100);
	private:		
		void SearchEligibleDatacenters4Clients(const int latencyThreshold = 100);
		vector<ClientType> candidateClients; // copy of a subset of globalClientList
		vector<DatacenterType> candidateDatacenters; // copy of globalDatacenterList
		vector<SessionType> allSessions;
		
		void ResetAssignment();

		/*control flags*/
		bool G_Assignment_Completed = false;
		bool R_Assignment_Completed = false;

		/*G_Assignment algorithms*/
		void G_Assignment_Random();
		void G_Assignment_Simple(const int sessionSize);
		void G_Assignment_Layered(const int sessionSize);
		
		/*R_Assignment algorithms*/
		void R_Assignment_Random();
		void R_Assignment_LSP();
		void R_Assignment_LCW(const int serverCapacity);

		/*GroupingAllocation algorithms*/
		void Grouping_Random(const int sessionSize);
		void Grouping_Greedy(const int sessionSize, const int serverCapacity, bool shareCostAcrossSessions = true);

		/*main procedures*/
		void ClientAssignment(const int sessionSize, const int serverCapacity, const string algFirstStage, const string algSecondStage);	
		double ComputeCost(const int serverCapacity, bool shareCostAcrossSessions = true);
		
		/*void Random(const int sessionSize);
		void Greedy_1(const int sessionSize);
		void Greedy_2(const int sessionSize);
		void Greedy_3(const int sessionSize);*/
	};
}