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
	
	void MaximumMatchingProblem::Simulate(const int clientCount, const int latencyThreshold, const int simulationCount, const int sessionSize)
	{
		/*initialize global stuff*/
		Initialize();		

		/*stuff to record performance*/
		vector<double> groupingRate;

		/*run simulation round by round*/
		for (int round = 1; round <= simulationCount; round++)
		{	
			/*generate a set of random candidateClients according to the clientCount parameters*/
			auto globalClientListCopy = globalClientList; // avoid modifying the original globalClientList	 
			if (clientCount <= globalClientListCopy.size())
			{
				random_shuffle(globalClientListCopy.begin(), globalClientListCopy.end());
				candidateClients.assign(globalClientListCopy.begin(), globalClientListCopy.begin() + clientCount);
			}
			else // in case if the clientCount is greater than the total number of clients loaded from the dataset
			{
				while (candidateClients.size() < clientCount)
				{
					random_shuffle(globalClientListCopy.begin(), globalClientListCopy.end());
					for (auto& client : globalClientListCopy)
					{
						candidateClients.push_back(client);
					}
				}
			}

			/*every round we use the same candidate datacenters (i.e., all datacenters)*/
			candidateDatacenters = globalDatacenterList;

			/*find eligible datacenters for clients and handle the case where all clients have no eligible datacenters*/
			double totalEligibleClients = 0;
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
				if (!client.eligibleDatacenters.empty())
				{
					totalEligibleClients++;
				}
			}
			if (totalEligibleClients < sessionSize)
			{
				printf("totalEligibleClients < sessionSize, ignore this round and generate another candidateDatacenters\n");
				round--;
				continue;
			}

			/*grouping algorithm*/
			if ("random" == groupingAlgorithm)
				RandomAssignmentGrouping();
			else if ("nearest" == groupingAlgorithm)
				NearestAssignmentGrouping();
			else
				printf("invalid grouping algoritm name!\n");
				
			/*record the result of this round*/
			double totalGroupedClients = 0;			
			for (auto& dc : candidateDatacenters)
			{				
				totalGroupedClients += std::floor((double)dc.assignedClients.size() / sessionSize) * sessionSize;
			}			
			groupingRate.push_back(totalGroupedClients / totalEligibleClients);
		}
		
		/*print out the result*/
		printf("clientCount=%d\t", clientCount);
		printf(groupingAlgorithm.c_str());
		printf("\tgroupingRate=%.2f\n", GetMeanValue(groupingRate));
	}

	void MaximumMatchingProblem::RandomAssignmentGrouping()
	{
		/*reset assignedClients*/
		for (auto& dc : candidateDatacenters)
		{
			dc.assignedClients.clear();
		}

		/*reset assignedDatacenter*/
		for (auto& client : candidateClients)
		{
			client.assignedDatacenter = nullptr;
		}

		/*determine each dc's assignedClients*/
		for (auto& client : candidateClients)
		{
			if (!client.eligibleDatacenters.empty())
			{
				client.assignedDatacenter = client.eligibleDatacenters.at(GenerateRandomIndex(client.eligibleDatacenters.size()));
			}

			if (client.assignedDatacenter != nullptr)
			{
				client.assignedDatacenter->assignedClients.push_back(&client);
			}
		}
	}

	void MaximumMatchingProblem::NearestAssignmentGrouping()
	{		
		/*reset assignedClients*/
		for (auto& dc : candidateDatacenters)
		{
			dc.assignedClients.clear();
		}

		/*reset assignedDatacenter*/
		for (auto& client : candidateClients)
		{
			client.assignedDatacenter = nullptr;
		}

		/*determine each dc's assignedClients*/
		for (auto& client : candidateClients)
		{			
			if (!client.eligibleDatacenters.empty())
			{
				client.assignedDatacenter = GetClientNearestDC(client);
			}

			if (client.assignedDatacenter != nullptr)
			{
				client.assignedDatacenter->assignedClients.push_back(&client);
			}
		}
	}

	DatacenterType* MatchmakingProblemBase::GetClientNearestDC(ClientType & client)
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
}