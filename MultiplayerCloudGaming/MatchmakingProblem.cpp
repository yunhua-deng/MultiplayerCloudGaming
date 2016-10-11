#include "MatchmakingProblem.h"

namespace MatchmakingProblem
{
	void MatchmakingProblemBase::Initialize()
	{				
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
	
	void MaximumMatchingProblem::Simulate(const string algToRun, const int clientCount, const int latencyThreshold, const int sessionSize, const int simulationCount)
	{		
		srand(0);
		
		/*stuff to record performance*/
		vector<double> eligibleRate;
		vector<double> groupedRate;
		vector<double> groupingTime;

		/*run simulation round by round*/		
		for (int round = 1; round <= simulationCount; round++)
		{	
			/*generate candidateClients (copy from globalClientList)*/
			candidateClients.clear();
			while (candidateClients.size() < clientCount) { candidateClients.push_back(globalClientList.at(GenerateRandomIndex(globalClientList.size()))); }

			/*generate candidateDatacenters (copy from globalDatacenterList)*/
			candidateDatacenters = globalDatacenterList;

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
				if (!client.eligibleDatacenters.empty()) { totalEligibleClients++; }
			}

			/*cancel this round*/
			if (totalEligibleClients < sessionSize)
			{
				std::printf("totalEligibleClients < sessionSize -> redo this round\n");
				round--;
				continue;
			}
			else { eligibleRate.push_back(totalEligibleClients / clientCount); }
			//printf("round=%d\n", round);

			/*run grouping algorithm*/
			auto timeStart = clock();
			if ("nearest" == algToRun) NearestAssignmentGrouping();
			else if ("random" == algToRun) RandomAssignmentGrouping();
			else if ("simple" == algToRun) SimpleGreedyGrouping(sessionSize);			
			else if ("layered" == algToRun) LayeredGreedyGrouping(sessionSize);
			else std::printf("invalid algoritm name!\n");
			groupingTime.push_back((double)difftime(clock(), timeStart));
				
			/*get the result of this round*/
			double totalGroupedClients = 0;			
			for (auto & dc : candidateDatacenters) { totalGroupedClients += std::floor((double)dc.assignedClients.size() / sessionSize) * sessionSize; }
			groupedRate.push_back(totalGroupedClients / clientCount);
		}
		
		/*dump to disk*/
		outFile << clientCount << "," << GetMeanValue(eligibleRate) << "," << GetMeanValue(groupedRate) << "," << GetMeanValue(groupingTime) << "\n";
		std::printf("%d,%.2f,%.2f,%.2f\n", clientCount, GetMeanValue(eligibleRate), GetMeanValue(groupedRate), GetMeanValue(groupingTime));
	}

	void MaximumMatchingProblem::RandomAssignmentGrouping()
	{
		/*reset assignedClients*/
		for (auto & dc : candidateDatacenters) { dc.assignedClients.clear(); }
		/*reset assignedDatacenter*/
		for (auto & client : candidateClients) { client.assignedDatacenter = nullptr; }

		/*determine each dc's assignedClients*/
		for (auto & client : candidateClients)
		{
			if (!client.eligibleDatacenters.empty()) { client.assignedDatacenter = client.eligibleDatacenters.at(GenerateRandomIndex(client.eligibleDatacenters.size())); }
			if (client.assignedDatacenter != nullptr) { client.assignedDatacenter->assignedClients.push_back(&client); }
		}
	}

	void MaximumMatchingProblem::NearestAssignmentGrouping()
	{	
		/*reset assignedClients*/
		for (auto & dc : candidateDatacenters) { dc.assignedClients.clear(); }
		/*reset assignedDatacenter*/
		for (auto & client : candidateClients) { client.assignedDatacenter = nullptr; }

		/*determine each dc's assignedClients*/
		for (auto & client : candidateClients)
		{			
			if (!client.eligibleDatacenters.empty()) client.assignedDatacenter = GetClientNearestEligibleDC(client);
			if (client.assignedDatacenter != nullptr) { client.assignedDatacenter->assignedClients.push_back(&client); }
		}
	}

	void MaximumMatchingProblem::SimpleGreedyGrouping(const int sessionSize)
	{
		/*reset assignedClients*/
		for (auto & dc : candidateDatacenters) { dc.assignedClients.clear(); }
		/*reset assignedDatacenter*/
		for (auto & client : candidateClients) { client.assignedDatacenter = nullptr; }

		/*sort coverableClients for each datacenter*/
		for (auto & dc : candidateDatacenters)
		{
			std::sort(dc.coverableClients.begin(), dc.coverableClients.end(), ClientComparatorByFewerEligibleDatacenters);
		}

		/*simple greedy*/
		while (true)
		{
			/*pick the maxDC*/
			auto maxDC = &(candidateDatacenters.front());
			int maxRank = 0;
			for (auto & client : maxDC->coverableClients) 
			{ 
				if (nullptr == client->assignedDatacenter) { maxRank++; } 
			}
			for (auto & dc : candidateDatacenters)
			{
				int thisRank = 0;
				for (auto & client : dc.coverableClients) 
				{ 
					if (nullptr == client->assignedDatacenter) { thisRank++; }
				}

				if (thisRank > maxRank)
				{
					maxRank = thisRank;
					maxDC = &dc; 
				}
			}

			/*determine how many unassigned clients to assign in this round*/
			double unassignedClientsInMaxDC = (double)maxRank;
			int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClientsInMaxDC / sessionSize) * sessionSize);			
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
				else break;
			}
		}
	}

	void MaximumMatchingProblem::LayeredGreedyGrouping(const int sessionSize)
	{
		/*reset assignedClients*/
		for (auto & dc : candidateDatacenters) { dc.assignedClients.clear(); }
		/*reset assignedDatacenter*/
		for (auto & client : candidateClients) { client.assignedDatacenter = nullptr; }

		/*sort coverableClients for each dc*/
		for (auto & dc : candidateDatacenters)
		{
			map<vector<DatacenterType*>, vector<ClientType*>, DatacenterPointerVectorCmp> clientSectors;
			for (auto & c : dc.coverableClients) { clientSectors[c->eligibleDatacenters].push_back(c); }

			/*cout << clientSectors.size() << " sectors\n";
			cout << "original coverableClients: { ";
			for (auto & c : dc.coverableClients) { cout << c->id << " "; }
			cout << "}\n";*/

			dc.coverableClients.clear();
			for (auto & sector : clientSectors) 
			{ 					
				for (auto & c : sector.second) 
				{ 
					dc.coverableClients.push_back(c);

					/*cout << c->id << ": { ";
					for (auto & it : c->eligibleDatacenters) { cout << it->id << " "; }
					cout << "}\n";*/
				}
			}

			/*cout << "sorted coverableClients: { ";
			for (auto & c : dc.coverableClients) { cout << c->id << " "; }
			cout << "}\n";
			cin.get();*/
		}

		/*layered greedy*/
		for (size_t layerIndex = 1; layerIndex <= candidateClients.size(); layerIndex++)
		{
			while (true)
			{
				/*pick the maxDC*/
				auto maxDC = &(candidateDatacenters.front());
				int maxRank = 0;
				for (auto & client : maxDC->coverableClients)
				{
					if (nullptr == client->assignedDatacenter && client->eligibleDatacenters.size() <= layerIndex) { maxRank++; }
				}
				for (auto & dc : candidateDatacenters)
				{
					int thisRank = 0;
					for (auto & client : dc.coverableClients)
					{
						if (nullptr == client->assignedDatacenter && client->eligibleDatacenters.size() <= layerIndex) { thisRank++; }
					}

					if (thisRank > maxRank)
					{ 						
						maxRank = thisRank;
						maxDC = &dc;						
					}
				}

				/*determine how many unassigned clients to assign in this round*/
				double unassignedClientsInMaxDC = (double)maxRank;
				int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClientsInMaxDC / sessionSize) * sessionSize);
				if (0 == clientsToBeGroupedInMaxDC) { break; }

				/*group (assign) clients in the maxDC*/
				for (auto & client : maxDC->coverableClients)
				{					
					if (clientsToBeGroupedInMaxDC > 0)
					{
						if (nullptr == client->assignedDatacenter)
						{
							client->assignedDatacenter = maxDC;
							maxDC->assignedClients.push_back(client);
							clientsToBeGroupedInMaxDC--;
						}
					}
					else break;
				}
			}
		}
	}

	DatacenterType* MaximumMatchingProblem::GetClientNearestEligibleDC(ClientType & client)
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

	bool ClientComparatorByFewerEligibleDatacenters(const ClientType* a, const ClientType* b)
	{
		return (a->eligibleDatacenters.size() < b->eligibleDatacenters.size());
	}

	void ParetoOptimalMatchingProblem::Simulate(const string algToRun, const int clientCount, const int latencyThreshold, const int sessionSize, const int simulationCount)
	{
		/*fix the seed for random numnber generation for every run*/
		srand(0);		

		/*run simulation round by round*/
		for (int round = 1; round <= simulationCount; round++)
		{
			/*generate candidateClients (copy from globalClientList)*/
			candidateClients.clear();
			while (candidateClients.size() < clientCount) { candidateClients.push_back(globalClientList.at(GenerateRandomIndex(globalClientList.size()))); }

			/*generate candidateDatacenters (copy from globalDatacenterList)*/
			candidateDatacenters = globalDatacenterList;

			/*update eligible G and R datacenters for clients*/
			double totalEligibleClients = 0;
			for (auto & client : candidateClients)
			{
				/*reset for safety*/
				client.eligibleDatacenters_G.clear();
				client.eligibleDatacenters_R.clear();
				client.eligibleDatacenters_R_indexed_by_G.clear();

				/*update the above three stuff*/
				for (auto & dc_g : candidateDatacenters)
				{
					vector<DatacenterType*> eligibleDatacenters_R_indexed_by_G;
					for (auto & dc_r : candidateDatacenters)
					{
						if ((client.delayToDatacenter.at(dc_r.id) + dc_r.delayToDatacenter.at(dc_g.id)) <= latencyThreshold)
						{
							client.eligibleDatacenters_R.push_back(&dc_r);
							eligibleDatacenters_R_indexed_by_G.push_back(&dc_r);
						}
						if (!client.eligibleDatacenters_R.empty())
						{
							client.eligibleDatacenters_G.push_back(&dc_g);
						}
					}
					
					if (!eligibleDatacenters_R_indexed_by_G.empty()) { client.eligibleDatacenters_R_indexed_by_G[dc_g.id] = eligibleDatacenters_R_indexed_by_G; }
				}

				/*check whether this client is eligible*/
				if (!client.eligibleDatacenters_G.empty()) { totalEligibleClients++; }
			}

			/*cancel this round*/
			if (totalEligibleClients < sessionSize)
			{
				std::printf("totalEligibleClients < sessionSize -> redo this round\n");
				round--;
				continue;
			}			
			
			//printf("round=%d\n", round);

			/*run grouping algorithm*/
			RandomAssignmentGrouping(sessionSize);			
		}
	}

	void ParetoOptimalMatchingProblem::RandomAssignmentGrouping(const int sessionSize)
	{
		/*reset*/
		for (auto & client : candidateClients) { client.isGrouped = false; }
		
		/*determine assignedDatacenter_G and assignedDatacenter_R for each client*/
		for (auto & client : candidateClients)
		{			
			if (!client.eligibleDatacenters_G.empty() && !client.eligibleDatacenters_R_indexed_by_G.empty())
			{
				client.assignedDatacenter_G = client.eligibleDatacenters_G.at(GenerateRandomIndex(client.eligibleDatacenters_G.size()));
				auto temp = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).size(); /*bug*/
				client.assignedDatacenter_R = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).at(GenerateRandomIndex(temp));
			}
		}

		/*grouping*/		
		vector<double> totalCost;
		vector<vector<ClientType*>> totalSessions;
		while (true)
		{
			double oneCost = 0;
			vector<ClientType*> oneSession;
			for (auto & dc_g : candidateDatacenters)
			{
				oneSession.clear();
				for (auto & client : candidateClients)
				{
					if (oneSession.size() == sessionSize) 
					{ 
						for (auto & sessionClient : oneSession) { oneCost += sessionClient->assignedDatacenter_R->priceServer; } /*capacity is one*/
						
						totalCost.push_back(oneCost);

						totalSessions.push_back(oneSession);

						for (auto & sessionClient : oneSession) { sessionClient->isGrouped = true; } /*update client's grouping state*/

						break;
					}

					if (!client.isGrouped && client.assignedDatacenter_G->id == dc_g.id) { oneSession.push_back(&client); }
				}
			}
		}

		/*output*/
		for (int sessionCount = 1; sessionCount <= totalSessions.size(); sessionCount++)
		{
			printf("sessionCount = %d, totalCost = %f\n", sessionCount, totalCost.at(sessionCount - 1));
		}
	}
}