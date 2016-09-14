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
		vector<DatacenterType*> eligibleDatacenters;
		DatacenterType* assignedDatacenter = nullptr;

		ClientType(int givenID)
		{
			this->id = givenID;
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

		DatacenterType(int givenID)
		{
			this->id = givenID;
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
		DatacenterType* GetClientNearestDC(ClientType & client);
	};

	class MaximumMatchingProblem : public MatchmakingProblemBase
	{
	public:		
		string outputDirectory = dataDirectory + "MaximumMatchingProblem\\";		
		ofstream outFile;
		void Simulate(const string algToRun, const int clientCount = 100, const int latencyThreshold = 100, const int simulationCount = 100, const int sessionSize = 10);
	private:
		vector<ClientType> candidateClients;
		vector<DatacenterType> candidateDatacenters;
		void RandomAssignmentGrouping();
		void NearestAssignmentGrouping();		
	};
}