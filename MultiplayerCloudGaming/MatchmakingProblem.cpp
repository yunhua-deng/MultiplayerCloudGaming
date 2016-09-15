#include "MatchmakingProblem.h"

namespace MatchmakingProblem
{
	void MatchmakingProblemBase::Initialize()
	{
		/*string ClientDatacenterLatencyFile = "dc_to_pl_rtt.csv";
		string InterDatacenterLatencyFile = "dc_to_dc_rtt.csv";
		string BandwidthServerPricingFile = "dc_pricing_bandwidth_server.csv";*/		
		string ClientDatacenterLatencyFile = "ping_to_prefix_median_matrix.csv";
		string InterDatacenterLatencyFile = "ping_to_dc_median_matrix.csv";
		string BandwidthServerPricingFile = "pricing_bandwidth_server.csv";		
		
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
	
	void MaximumMatchingProblem::Simulate(const string algToRun, const int clientCount, const int latencyThreshold, const int simulationCount, const int sessionSize)
	{		
		srand(0);
		
		/*stuff to record performance*/
		vector<double> eligibleRate;
		vector<double> groupedRate;

		/*run simulation round by round*/
		auto globalClientListCopy = globalClientList; // prevent the original globalClientList from being modifed by random_shuffle
		for (int round = 1; round <= simulationCount; round++)
		{	
			/*generate candidateClients*/
			candidateClients.clear();
			while (candidateClients.size() < clientCount)
			{
				candidateClients.push_back(globalClientListCopy.at(GenerateRandomIndex(globalClientListCopy.size())));
			}

			/*generate candidateDatacenters*/
			candidateDatacenters= globalDatacenterList;

			/*update eligible datacenters for clients and coverable clients for datacenters*/
			double totalEligibleClients = 0;
			for (auto & client : candidateClients)
			{				
				for (auto & dc : candidateDatacenters)
				{
					if (client.delayToDatacenter.at(dc.id) <= latencyThreshold)
					{
						client.eligibleDatacenters.push_back(&dc);
						dc.coverableClients.push_back(&client);
					}
				}
				if (!client.eligibleDatacenters.empty())
				{
					totalEligibleClients++;
				}
			}

			/*cancel this round*/
			if (totalEligibleClients < sessionSize)
			{
				printf("totalEligibleClients < sessionSize -> redo this round\n");
				round--;
				continue;
			}
			else
			{
				eligibleRate.push_back(totalEligibleClients / clientCount);
			}

			printf("round=%d\n", round);

			/*grouping algorithm*/
			if ("nearest" == algToRun)
				NearestAssignmentGrouping();
			else if ("random" == algToRun)
				RandomAssignmentGrouping();
			else if ("simple" == algToRun)
				SimpleGreedyGrouping(sessionSize);
			else
				printf("invalid algoritm name!\n");
				
			/*get the result of this round*/
			double totalGroupedClients = 0;			
			for (auto & dc : candidateDatacenters)
			{				
				totalGroupedClients += std::floor((double)dc.assignedClients.size() / sessionSize) * sessionSize;
			}			
			groupedRate.push_back(totalGroupedClients / clientCount);			
		}
		
		/*dump to disk*/
		outFile << clientCount << "," << GetMeanValue(eligibleRate) << "," << GetMeanValue(groupedRate) << "\n";
		printf("%d,%.2f,%.2f\n", clientCount, GetMeanValue(eligibleRate), GetMeanValue(groupedRate));
	}

	void MaximumMatchingProblem::RandomAssignmentGrouping()
	{
		/*reset assignedClients*/
		for (auto & dc : candidateDatacenters)
		{
			dc.assignedClients.clear();
		}

		/*reset assignedDatacenter*/
		for (auto & client : candidateClients)
		{
			client.assignedDatacenter = nullptr;
		}

		/*determine each dc's assignedClients*/
		for (auto & client : candidateClients)
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
		for (auto & dc : candidateDatacenters)
		{
			dc.assignedClients.clear();
		}

		/*reset assignedDatacenter*/
		for (auto & client : candidateClients)
		{
			client.assignedDatacenter = nullptr;
		}

		/*determine each dc's assignedClients*/
		for (auto & client : candidateClients)
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

	void MaximumMatchingProblem::SimpleGreedyGrouping(const int sessionSize)
	{
		/*reset assignedClients*/
		for (auto & dc : candidateDatacenters)
		{
			dc.assignedClients.clear();
		}

		/*reset assignedDatacenter*/
		for (auto & client : candidateClients)
		{
			client.assignedDatacenter = nullptr;
		}

		/*sort coverableClients for each datacenter*/
		for (auto & dc : candidateDatacenters)
		{
			std::sort(dc.coverableClients.begin(), dc.coverableClients.end(), ClientComparatorByEligibleDatacenterSize);
		}

		/*simple datacenter-based hill-climbing greedy*/		
		while (true)
		{
			/*pick the maxDC*/
			auto maxDC = &(candidateDatacenters.front());
			for (auto & dc : candidateDatacenters)
			{
				int rankMaxDC = 0;
				for (auto & client : maxDC->coverableClients)
				{
					if (nullptr == client->assignedDatacenter) { rankMaxDC++; }
				}

				int rankThisDC = 0;
				for (auto & client : dc.coverableClients)
				{
					if (nullptr == client->assignedDatacenter) { rankThisDC++; }
				}

				if (rankThisDC > rankMaxDC) { maxDC = &dc; }
			}

			double unassignedClientsInMaxDC = 0;
			for (auto & client : maxDC->coverableClients)
			{
				if (nullptr == client->assignedDatacenter) { unassignedClientsInMaxDC++; }
			}
			int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClientsInMaxDC / sessionSize) * sessionSize);			

			/*check termination condition*/
			if (0 == clientsToBeGroupedInMaxDC) { break; }

			/*group (assign) clients in the maxDC*/		
			for (auto client : maxDC->coverableClients)
			{				
				/*group (assign) one not-yet-grouped client*/
				if (clientsToBeGroupedInMaxDC > 0)
				{
					if (nullptr == client->assignedDatacenter)
					{
						client->assignedDatacenter = maxDC;
						maxDC->assignedClients.push_back(client);
						clientsToBeGroupedInMaxDC--;						
					}
				}
				else { break; }
			}
		}
	}

	DatacenterType* MatchmakingProblemBase::GetClientNearestDC(ClientType & client)
	{
		if (client.eligibleDatacenters.empty())
			return nullptr;

		auto nearest = client.eligibleDatacenters.front();
		for (auto & edc : client.eligibleDatacenters)
		{
			if (client.delayToDatacenter.at(edc->id) < client.delayToDatacenter.at(nearest->id))
			{
				nearest = edc;
			}
		}
		return nearest;
	}

	bool ClientComparatorByEligibleDatacenterSize(const ClientType * a, const ClientType * b)
	{
		return (a->eligibleDatacenters.size() >= b->eligibleDatacenters.size());
	}
}