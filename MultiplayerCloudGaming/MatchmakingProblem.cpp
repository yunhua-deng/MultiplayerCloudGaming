#include "MatchmakingProblem.h"

namespace MatchmakingProblem
{
	void MatchmakingProblemBase::Initialize(const string given_dataDirectory)
	{
		string ClientDatacenterLatencyFile = "dc_to_pl_rtt.csv";
		string InterDatacenterLatencyFile = "dc_to_dc_rtt.csv";
		string BandwidthServerPricingFile = "dc_pricing_bandwidth_server.csv";
		
		/*string ClientDatacenterLatencyFile = "ping_to_prefix_median_matrix.csv";
		string InterDatacenterLatencyFile = "ping_to_dc_median_matrix.csv";
		string BandwidthServerPricingFile = "pricing_bandwidth_server.csv";*/
		
		this->dataDirectory = given_dataDirectory;
		this->globalClientList.clear();
		this->globalDatacenterList.clear();

		/* temporary stuff */
		vector<vector<double>> ClientToDatacenterDelayMatrix;
		vector<vector<double>> InterDatacenterDelayMatrix;
		vector<double> priceServerList;
		vector<double> priceBandwidthList;

		/* client-to-dc latency data */
		auto strings_read = ReadDelimitedTextFileIntoVector(dataDirectory + ClientDatacenterLatencyFile, ',', true);
		if (strings_read.empty())
		{
			printf("ERROR: empty file!\n");
			cin.get();
			return;
		}
		for (auto row : strings_read)
		{
			vector<double> ClientToDatacenterDelayMatrixOneRow;
			for (size_t col = 1; col < row.size(); col++)
			{
				ClientToDatacenterDelayMatrixOneRow.push_back(stod(row.at(col)) / 2);
			}
			ClientToDatacenterDelayMatrix.push_back(ClientToDatacenterDelayMatrixOneRow);
		}

		/*dc-to-dc latency data*/
		strings_read = ReadDelimitedTextFileIntoVector(dataDirectory + InterDatacenterLatencyFile, ',', true);
		if (strings_read.empty())
		{
			printf("ERROR: empty file!\n");
			cin.get();
			return;
		}
		for (auto row : strings_read)
		{
			vector<double> InterDatacenterDelayMatrixOneRow;
			for (size_t col = 1; col < row.size(); col++)
			{
				InterDatacenterDelayMatrixOneRow.push_back(stod(row.at(col)) / 2);
			}
			InterDatacenterDelayMatrix.push_back(InterDatacenterDelayMatrixOneRow);
		}
		const int totalClientCount = int(ClientToDatacenterDelayMatrix.size());

		/* bandwidth and server price data */
		strings_read = ReadDelimitedTextFileIntoVector(dataDirectory + BandwidthServerPricingFile, ',', true);
		if (strings_read.empty())
		{
			printf("ERROR: empty file!\n");
			cin.get();
			return;
		}
		for (auto row : strings_read)
		{
			priceBandwidthList.push_back(stod(row.at(1)));
			priceServerList.push_back(stod(row.at(2))); // 2: g2.8xlarge, 3: g2.2xlarge
		}
		const int totalDatacenterCount = int(ClientToDatacenterDelayMatrix.front().size());

		/* creating clients */
		for (int i = 0; i < totalClientCount; i++)
		{
			ClientType client(i);
			client.chargedTrafficVolume = 2;
			for (int j = 0; j < totalDatacenterCount; j++)
			{
				client.delayToDatacenter[j] = ClientToDatacenterDelayMatrix.at(i).at(j);
			}
			globalClientList.push_back(client);
		}
		//printf("%d clients loaded\n", int(globalClientList.size()));

		/* create datacenters */
		for (int i = 0; i < totalDatacenterCount; i++)
		{
			DatacenterType dc(i);
			dc.priceServer = priceServerList.at(i);
			dc.priceBandwidth = priceBandwidthList.at(i);
			for (auto client : globalClientList)
			{
				dc.delayToClient[client.id] = client.delayToDatacenter[dc.id];
			}
			for (int j = 0; j < totalDatacenterCount; j++)
			{
				dc.delayToDatacenter[j] = InterDatacenterDelayMatrix.at(i).at(j);
			}
			globalDatacenterList.push_back(dc);
		}
		//printf("%d datacenters loaded\n", int(globalDatacenterList.size()));
	}

	DatacenterType* GetClientNearestDC(ClientType & client)
	{
		if (client.eligibleDatacenters.empty())
			return nullptr;
		
		auto nearest = client.eligibleDatacenters.front();
		for (auto& edc : client.eligibleDatacenters)
		{
			if (client.delayToDatacenter.at(edc->id) < client.delayToDatacenter.at(nearest->id))
			{
				nearest = edc;
			}
		}
		return nearest;
	}

	void MaximumMatchingProblem::Simulate(const int latencyThreshold, const int clientCount, const int sessionSize, const int simulationCount)
	{
		printf("simulation settings: latencyThreshold = %d | clientCount = %d\n", latencyThreshold, clientCount);

		/*initialize global stuff*/
		Initialize();
		
		/*check if sth wrong*/
		if (clientCount > globalClientList.size())
		{
			printf("ERROR: clientCount too large!\n");
			cin.get();
			return;
		}

		/*random number seed*/
		std::srand(2);
		//std::srand(time(NULL));

		/*run simulation round by round*/
		for (int round = 0; round < simulationCount; round++)
		{	
			/*generate a set of random candidateClients according to the clientCount parameters*/
			auto globalClientListCopy = globalClientList; // avoid modifying the original globalClientList		 
			random_shuffle(globalClientListCopy.begin(), globalClientListCopy.end());
			candidateClients.assign(globalClientListCopy.begin(), globalClientListCopy.begin() + clientCount);

			/*every round we use the same candidate datacenters (i.e., all datacenters)*/
			candidateDatacenters = globalDatacenterList;

			/*find eligible datacenters for clients*/
			for (auto& client : candidateClients)
			{
				client.eligibleDatacenters.clear();
				for (auto& dc : candidateDatacenters)
				{
					if (client.delayToDatacenter.at(dc.id) <= latencyThreshold)
					{
						client.eligibleDatacenters.push_back(&dc);
					}
				}
			}

			/*nearest assignment*/
			NearestAssignmentGrouping();
			double totalEligibleClients = 0;
			double totalGroupedClients = 0;
			for (auto& dc : candidateDatacenters)
			{
				totalEligibleClients += dc.assignedClients.size();
				totalGroupedClients += std::floor(dc.assignedClients.size() / sessionSize) * sessionSize;
			}
			//printf("totalGroupedClients = %d vs totalEligibleClients = %d -> %.2f grouping rate\n", (int)totalGroupedClients, (int)totalEligibleClients, totalGroupedClients / totalEligibleClients);
			printf("grouping success rate = %.2f\n", totalGroupedClients / totalEligibleClients);
		}
	}

	void MaximumMatchingProblem::NearestAssignmentGrouping()
	{		
		/*reset each dc's assignedClients*/
		for (auto& dc : candidateDatacenters)
		{
			dc.assignedClients.clear();
		}

		/*determine each dc's assignedClients*/
		for (auto& client : candidateClients)
		{			
			client.assignedDatacenter = GetClientNearestDC(client);
			if (client.assignedDatacenter != nullptr) client.assignedDatacenter->assignedClients.push_back(&client);
		}
	}
}