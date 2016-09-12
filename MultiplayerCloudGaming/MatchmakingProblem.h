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
	protected:		
		string dataDirectory; // root path for input and output
		vector<ClientType> globalClientList; // read from input
		vector<DatacenterType> globalDatacenterList; // read from input
		void Initialize(const string givenDataDirectory = ".\\Data\\");
	};

	class MaximumMatchingProblem : public MatchmakingProblemBase
	{
	public:
		void Simulate(const int latencyThreshold = 100, const int clientCount = 100, const int sessionSize = 10, const int simulationCount = 100);
	private:
		vector<ClientType> candidateClients;
		vector<DatacenterType> candidateDatacenters;
		void NearestAssignmentGrouping();
	};

	DatacenterType* GetClientNearestDC(ClientType & client);
}