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
			priceServerList.push_back(stod(row.at(3))); // 2: g2.8xlarge, 3: g2.2xlarge
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

	bool DatacenterComparatorByPrice(const DatacenterType a, const DatacenterType b)
	{
		return (a.priceServer < b.priceServer);
	}

	void ParetoOptimalMatchingProblem::FindEligibleClients(const int latencyThreshold)
	{		
		eligibleClients.clear();		
		for (auto & client : globalClientList)
		{
			/*reset for safety*/
			client.eligibleDatacenters_G.clear();
			client.eligibleDatacenters_R.clear();
			client.eligibleDatacenters_R_indexed_by_G.clear();

			/*update the above three stuff*/
			for (auto & dc_g : globalDatacenterList)
			{
				vector<DatacenterType*> eligibleDatacenters_R_indexed_by_G;
				for (auto & dc_r : globalDatacenterList)
				{
					if ((client.delayToDatacenter.at(dc_r.id) + dc_r.delayToDatacenter.at(dc_g.id)) <= latencyThreshold)
					{
						client.eligibleDatacenters_R.push_back(&dc_r);
						eligibleDatacenters_R_indexed_by_G.push_back(&dc_r);
					}
				}

				if (!eligibleDatacenters_R_indexed_by_G.empty())
				{
					client.eligibleDatacenters_G.push_back(&dc_g);
					client.eligibleDatacenters_R_indexed_by_G[dc_g.id] = eligibleDatacenters_R_indexed_by_G;
				}
			}

			/*check whether this client is eligible*/
			if (!client.eligibleDatacenters_G.empty()) { eligibleClients.push_back(client); }
		}
	}

	void ParetoOptimalMatchingProblem::Simulate(const string algToRun, const int clientCount, const int sessionSize, const int simulationCount)
	{
		/*fix the seed for random numnber generation for every algorithm*/
		srand(0);

		if (clientCount < sessionSize)
		{
			printf("clientCount < sessionSize!\n");
			return;
		}

		/*run simulation round by round*/
		for (int round = 1; round <= simulationCount; round++)
		{
			/*generate candidateClients (copies from globalClientList)*/
			candidateClients.clear();
			while (true) 
			{ 
				if (candidateClients.size() == clientCount) { break; }

				candidateClients.push_back(eligibleClients.at(GenerateRandomIndex(eligibleClients.size())));				
			}
						
			/*generate candidateDatacenters (copy from globalDatacenterList)*/
			candidateDatacenters = globalDatacenterList;		

			/*run grouping algorithm*/
			if ("random" == algToRun) Random(sessionSize);
			else if ("greedy-1" == algToRun) Greedy_1(sessionSize);
			else if ("greedy-2" == algToRun) Greedy_2(sessionSize);
			else if ("greedy-3" == algToRun) Greedy_3(sessionSize);
		}
	}

	void ParetoOptimalMatchingProblem::Random(const int sessionSize)
	{	
		/*reset*/
		for (auto & client : candidateClients) 
		{ 
			client.assignedDatacenter_G = nullptr;
			client.assignedDatacenter_R = nullptr;
			client.isGrouped = false;
		}
		
		/*determine assignedDatacenter_G for each client*/		
		for (auto & client : candidateClients)
		{			
			if (!client.eligibleDatacenters_G.empty())
			{
				auto index = GenerateRandomIndex(client.eligibleDatacenters_G.size());				
				client.assignedDatacenter_G = client.eligibleDatacenters_G.at(index);					
			}
		}

		/*determine assignedDatacenter_R for each client*/		
		for (auto & client : candidateClients)
		{
			if (!client.eligibleDatacenters_G.empty())
			{				
				auto index = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).size();
				client.assignedDatacenter_R = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).at(GenerateRandomIndex(index));
			}
		}

		/*grouping*/
		printf("\nsessionCounter,sessionCost\n");
		vector<double> allCosts;
		vector<vector<ClientType*>> allSessions;
		while (true)
		{
			double oneCost = 0;
			vector<ClientType*> oneSession;
			bool noMoreNewSessions = true;
			for (auto & dc_g : candidateDatacenters)
			{
				bool newSessionFound = false;
				oneSession.clear();
				for (auto & client : candidateClients)
				{
					if (client.eligibleDatacenters_G.empty()) continue;

					/*check if a new sesson is found and record stuff accordingly*/
					if (oneSession.size() == sessionSize)
					{
						for (auto & sessionClient : oneSession) { oneCost += sessionClient->assignedDatacenter_R->priceServer; }

						allCosts.push_back(oneCost);

						allSessions.push_back(oneSession);

						for (auto & sessionClient : oneSession) { sessionClient->isGrouped = true; }

						newSessionFound = true;

						break; /*stop as a new session is found*/
					}

					if (!client.isGrouped && client.assignedDatacenter_G->id == dc_g.id) { oneSession.push_back(&client); }
				}
				if (newSessionFound)
				{
					noMoreNewSessions = false;
					break;
				}
			}
			if (noMoreNewSessions)
			{
				break;
			}

			printf("%d,%.2f\n", (int)allSessions.size(), allCosts.back());			
		}

		int groupedClientCount = 0;
		for (auto & client : candidateClients)
		{
			if (client.isGrouped) groupedClientCount++;
		}
		printf("%d\n", groupedClientCount);
	}

	void ParetoOptimalMatchingProblem::Greedy_1(const int sessionSize)
	{						
		/*reset*/
		for (auto & client : candidateClients)
		{
			client.assignedDatacenter_G = nullptr;
			client.assignedDatacenter_R = nullptr;
			client.isGrouped = false;
		}

		/*determine assignedDatacenter_G for each client*/		
		for (auto & client : candidateClients)
		{
			if (!client.eligibleDatacenters_G.empty())
			{
				auto index = GenerateRandomIndex(client.eligibleDatacenters_G.size());				
				client.assignedDatacenter_G = client.eligibleDatacenters_G.at(index);				
			}
		}		

		/*determine assignedDatacenter_R for each client*/		
		for (auto & client : candidateClients)
		{
			if (!client.eligibleDatacenters_G.empty())
			{
				client.assignedDatacenter_R = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).front();
				for (auto & d_r : client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id))
				{
					if (d_r->priceServer < client.assignedDatacenter_R->priceServer)
					{
						client.assignedDatacenter_R = d_r;
					}
				}
			}
		}		

		/*grouping*/
		printf("\nsessionCounter,sessionCost\n");
		vector<double> allCosts;
		vector<vector<ClientType*>> allSessions;
		while (true)
		{
			double oneCost = 0;
			vector<ClientType*> oneSession;
			bool noMoreNewSessions = true;
			for (auto & dc_g : candidateDatacenters)
			{
				bool newSessionFound = false;
				oneSession.clear();
				for (auto & client : candidateClients)
				{
					if (client.eligibleDatacenters_G.empty()) continue;

					/*check if a new sesson is found and record stuff accordingly*/
					if (oneSession.size() == sessionSize)
					{
						for (auto & sessionClient : oneSession) { oneCost += sessionClient->assignedDatacenter_R->priceServer; }

						allCosts.push_back(oneCost);

						allSessions.push_back(oneSession);

						for (auto & sessionClient : oneSession) { sessionClient->isGrouped = true; }

						newSessionFound = true;

						break; /*stop as a new session is found*/
					}

					if (!client.isGrouped && client.assignedDatacenter_G->id == dc_g.id) { oneSession.push_back(&client); }
				}
				if (newSessionFound)
				{
					noMoreNewSessions = false;
					break;
				}
			}
			if (noMoreNewSessions)
			{
				break;
			}

			printf("%d,%.2f\n", (int)allSessions.size(), allCosts.back());
		}

		int groupedClientCount = 0;
		for (auto & client : candidateClients)
		{
			if (client.isGrouped) groupedClientCount++;
		}
		printf("%d\n", groupedClientCount);
	}

	void ParetoOptimalMatchingProblem::Greedy_2(const int sessionSize)
	{
		/*reset*/
		for (auto & client : candidateClients)
		{
			client.assignedDatacenter_G = nullptr;
			client.assignedDatacenter_R = nullptr;
			client.isGrouped = false;
		}

		/*determine assignedDatacenter_G for each client*/
		for (auto & client : candidateClients)
		{
			if (!client.eligibleDatacenters_G.empty())
			{
				auto index = GenerateRandomIndex(client.eligibleDatacenters_G.size());
				client.assignedDatacenter_G = client.eligibleDatacenters_G.at(index);
			}
		}

		/*determine assignedDatacenter_R for each client*/
		for (auto & client : candidateClients)
		{
			if (!client.eligibleDatacenters_G.empty())
			{
				client.assignedDatacenter_R = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).front();
				for (auto & d_r : client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id))
				{
					if (d_r->priceServer < client.assignedDatacenter_R->priceServer)
					{
						client.assignedDatacenter_R = d_r;
					}
				}
			}
		}

		/*grouping*/
		printf("\nsessionCounter,sessionCost\n");
		vector<double> allCosts;
		vector<vector<ClientType*>> allSessions;
		auto copy_candidateDatacenters = candidateDatacenters;
		std::sort(copy_candidateDatacenters.begin(), copy_candidateDatacenters.end(), DatacenterComparatorByPrice);
		while (true)
		{
			double oneCost = 0;
			vector<ClientType*> oneSession;
			bool noMoreNewSessions = true;
			for (auto & dc_g : candidateDatacenters)
			{
				bool newSessionFound = false;
				oneSession.clear();
				for (auto & dc_r : copy_candidateDatacenters)
				{
					for (auto & client : candidateClients)
					{
						if (client.eligibleDatacenters_G.empty()) continue;

						/*check if a new sesson is found and record stuff accordingly*/
						if (oneSession.size() == sessionSize)
						{
							for (auto & sessionClient : oneSession) { oneCost += sessionClient->assignedDatacenter_R->priceServer; }

							allCosts.push_back(oneCost);

							allSessions.push_back(oneSession);

							for (auto & sessionClient : oneSession) { sessionClient->isGrouped = true; }

							newSessionFound = true;

							break; /*stop as a new session is found*/
						}

						if (!client.isGrouped && client.assignedDatacenter_G->id == dc_g.id && client.assignedDatacenter_R->id == dc_r.id) { oneSession.push_back(&client); }
					}
					if (newSessionFound)
					{
						break;
					}
				}
				if (newSessionFound)
				{
					noMoreNewSessions = false;
					break;
				}				
			}
			if (noMoreNewSessions)
			{
				break;
			}

			printf("%d,%.2f\n", (int)allSessions.size(), allCosts.back());
		}

		int groupedClientCount = 0;
		for (auto & client : candidateClients)
		{
			if (client.isGrouped) groupedClientCount++;
		}
		printf("%d\n", groupedClientCount);
	}

	void ParetoOptimalMatchingProblem::Greedy_3(const int sessionSize)
	{
		/*reset*/
		for (auto & client : candidateClients)
		{
			client.assignedDatacenter_G = nullptr;
			client.assignedDatacenter_R = nullptr;
			client.isGrouped = false;
		}		

		/*determine assignedDatacenter_R for each client*/
		for (auto & client : candidateClients)
		{
			if (!client.eligibleDatacenters_G.empty())
			{
				client.assignedDatacenter_R = client.eligibleDatacenters_R.front();
				for (auto & d_r : client.eligibleDatacenters_R)
				{
					if (d_r->priceServer < client.assignedDatacenter_R->priceServer)
					{
						client.assignedDatacenter_R = d_r;
					}
				}
			}
		}

		/*determine assignedDatacenter_G for each client*/		
		for (auto & client : candidateClients)
		{			
			if (!client.eligibleDatacenters_G.empty())
			{
				for (auto & d_g : client.eligibleDatacenters_G)
				{
					if (client.eligibleDatacenters_R_indexed_by_G.at(d_g->id).end() != std::find(client.eligibleDatacenters_R_indexed_by_G.at(d_g->id).begin(), client.eligibleDatacenters_R_indexed_by_G.at(d_g->id).end(), client.assignedDatacenter_R))
					{
						client.assignedDatacenter_G = d_g;						
						break;
					}
				}
			}
		}

		/*grouping*/
		printf("\nsessionCounter,sessionCost\n");
		vector<double> allCosts;
		vector<vector<ClientType*>> allSessions;
		while (true)
		{
			double oneCost = 0;
			vector<ClientType*> oneSession;
			bool noMoreNewSessions = true;
			for (auto & dc_g : candidateDatacenters)
			{
				bool newSessionFound = false;
				oneSession.clear();
				for (auto & client : candidateClients)
				{
					if (client.eligibleDatacenters_G.empty()) continue;

					/*check if a new sesson is found and record stuff accordingly*/
					if (oneSession.size() == sessionSize)
					{
						for (auto & sessionClient : oneSession) { oneCost += sessionClient->assignedDatacenter_R->priceServer; }

						allCosts.push_back(oneCost);

						allSessions.push_back(oneSession);

						for (auto & sessionClient : oneSession) { sessionClient->isGrouped = true; }

						newSessionFound = true;

						break; /*stop as a new session is found*/
					}

					if (!client.isGrouped && client.assignedDatacenter_G->id == dc_g.id) { oneSession.push_back(&client); }
				}
				if (newSessionFound)
				{
					noMoreNewSessions = false;
					break;
				}
			}
			if (noMoreNewSessions)
			{
				break;
			}

			printf("%d,%.2f\n", (int)allSessions.size(), allCosts.back());
		}

		int groupedClientCount = 0;
		for (auto & client : candidateClients)
		{
			if (client.isGrouped) groupedClientCount++;
		}
		printf("%d\n", groupedClientCount);
	}
}