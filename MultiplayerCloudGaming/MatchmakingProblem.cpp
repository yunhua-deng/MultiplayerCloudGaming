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
						client.eligibleDatacenters_G.push_back(&dc);
						dc.coverableClients_G.push_back(&client);
					}
				}
				if (!client.eligibleDatacenters_G.empty()) { totalEligibleClients++; }
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
			for (auto & dc : candidateDatacenters) { totalGroupedClients += std::floor((double)dc.assignedClients_G.size() / sessionSize) * sessionSize; }
			groupedRate.push_back(totalGroupedClients / clientCount);
		}
		
		/*dump to disk*/
		outFile << clientCount << "," << GetMeanValue(eligibleRate) << "," << GetMeanValue(groupedRate) << "," << GetMeanValue(groupingTime) << "\n";
		std::printf("%d,%.2f,%.2f,%.2f\n", clientCount, GetMeanValue(eligibleRate), GetMeanValue(groupedRate), GetMeanValue(groupingTime));
	}

	void MaximumMatchingProblem::RandomAssignmentGrouping()
	{
		/*reset assignedClients_G*/
		for (auto & dc : candidateDatacenters) { dc.assignedClients_G.clear(); }
		/*reset assignedDatacenter_G*/
		for (auto & client : candidateClients) { client.assignedDatacenter_G = nullptr; }

		/*determine each dc's assignedClients_G*/
		for (auto & client : candidateClients)
		{
			if (!client.eligibleDatacenters_G.empty()) { client.assignedDatacenter_G = client.eligibleDatacenters_G.at(GenerateRandomIndex(client.eligibleDatacenters_G.size())); }
			if (client.assignedDatacenter_G != nullptr) { client.assignedDatacenter_G->assignedClients_G.push_back(&client); }
		}
	}

	void MaximumMatchingProblem::NearestAssignmentGrouping()
	{	
		/*reset assignedClients_G*/
		for (auto & dc : candidateDatacenters) { dc.assignedClients_G.clear(); }
		/*reset assignedDatacenter_G*/
		for (auto & client : candidateClients) { client.assignedDatacenter_G = nullptr; }

		/*determine each dc's assignedClients_G*/
		for (auto & client : candidateClients)
		{			
			if (!client.eligibleDatacenters_G.empty()) client.assignedDatacenter_G = GetClientNearestEligibleDC(client);
			if (client.assignedDatacenter_G != nullptr) { client.assignedDatacenter_G->assignedClients_G.push_back(&client); }
		}
	}

	void MaximumMatchingProblem::SimpleGreedyGrouping(const int sessionSize)
	{
		/*reset assignedClients_G*/
		for (auto & dc : candidateDatacenters) { dc.assignedClients_G.clear(); }
		/*reset assignedDatacenter_G*/
		for (auto & client : candidateClients) { client.assignedDatacenter_G = nullptr; }

		/*sort coverableClients_G for each datacenter*/
		for (auto & dc : candidateDatacenters)
		{
			std::sort(dc.coverableClients_G.begin(), dc.coverableClients_G.end(), ClientComparatorByFewerEligibleDatacenters_G);
		}

		/*simple greedy*/
		while (true)
		{
			/*pick the maxDC*/
			auto maxDC = &(candidateDatacenters.front());
			int maxRank = 0;
			for (auto & client : maxDC->coverableClients_G) 
			{ 
				if (nullptr == client->assignedDatacenter_G) { maxRank++; } 
			}
			for (auto & dc : candidateDatacenters)
			{
				int thisRank = 0;
				for (auto & client : dc.coverableClients_G) 
				{ 
					if (nullptr == client->assignedDatacenter_G) { thisRank++; }
				}

				if (thisRank > maxRank)
				{
					maxRank = thisRank;
					maxDC = &dc; 
				}
			}

			/*determine how many unassigned clients to assign in this round*/
			double unassignedClients_GInMaxDC = (double)maxRank;
			int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClients_GInMaxDC / sessionSize) * sessionSize);			
			if (0 == clientsToBeGroupedInMaxDC) { break; }

			/*group (assign) clients in the maxDC*/		
			for (auto client : maxDC->coverableClients_G)
			{				
				/*group (assign) one not-yet-grouped client*/
				if (clientsToBeGroupedInMaxDC > 0)
				{
					if (nullptr == client->assignedDatacenter_G)
					{
						client->assignedDatacenter_G = maxDC;
						maxDC->assignedClients_G.push_back(client);
						clientsToBeGroupedInMaxDC--;						
					}
				}
				else break;
			}
		}
	}

	void MaximumMatchingProblem::LayeredGreedyGrouping(const int sessionSize)
	{
		/*reset assignedClients_G*/
		for (auto & dc : candidateDatacenters) { dc.assignedClients_G.clear(); }
		/*reset assignedDatacenter_G*/
		for (auto & client : candidateClients) { client.assignedDatacenter_G = nullptr; }

		/*sort coverableClients_G for each dc*/
		for (auto & dc : candidateDatacenters)
		{
			map<vector<DatacenterType*>, vector<ClientType*>, DatacenterPointerVectorCmp> clientSectors;
			for (auto & c : dc.coverableClients_G) { clientSectors[c->eligibleDatacenters_G].push_back(c); }

			/*cout << clientSectors.size() << " sectors\n";
			cout << "original coverableClients_G: { ";
			for (auto & c : dc.coverableClients_G) { cout << c->id << " "; }
			cout << "}\n";*/

			dc.coverableClients_G.clear();
			for (auto & sector : clientSectors) 
			{ 					
				for (auto & c : sector.second) 
				{ 
					dc.coverableClients_G.push_back(c);

					/*cout << c->id << ": { ";
					for (auto & it : c->eligibleDatacenters_G) { cout << it->id << " "; }
					cout << "}\n";*/
				}
			}

			/*cout << "sorted coverableClients_G: { ";
			for (auto & c : dc.coverableClients_G) { cout << c->id << " "; }
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
				for (auto & client : maxDC->coverableClients_G)
				{
					if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G.size() <= layerIndex) { maxRank++; }
				}
				for (auto & dc : candidateDatacenters)
				{
					int thisRank = 0;
					for (auto & client : dc.coverableClients_G)
					{
						if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G.size() <= layerIndex) { thisRank++; }
					}

					if (thisRank > maxRank)
					{ 						
						maxRank = thisRank;
						maxDC = &dc;						
					}
				}

				/*determine how many unassigned clients to assign in this round*/
				double unassignedClients_GInMaxDC = (double)maxRank;
				int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClients_GInMaxDC / sessionSize) * sessionSize);
				if (0 == clientsToBeGroupedInMaxDC) { break; }

				/*group (assign) clients in the maxDC*/
				for (auto & client : maxDC->coverableClients_G)
				{					
					if (clientsToBeGroupedInMaxDC > 0)
					{
						if (nullptr == client->assignedDatacenter_G)
						{
							client->assignedDatacenter_G = maxDC;
							maxDC->assignedClients_G.push_back(client);
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
		if (client.eligibleDatacenters_G.empty())
			return nullptr;

		auto nearest = client.eligibleDatacenters_G.front();
		for (auto & edc : client.eligibleDatacenters_G)
		{
			if (client.delayToDatacenter.at(edc->id) < client.delayToDatacenter.at(nearest->id))
			{
				nearest = edc;
			}
		}
		return nearest;
	}

	bool ClientComparatorByFewerEligibleDatacenters_G(const ClientType* a, const ClientType* b)
	{
		return (a->eligibleDatacenters_G.size() < b->eligibleDatacenters_G.size());
	}	

	bool DatacenterComparatorByPrice(const DatacenterType a, const DatacenterType b)
	{
		return (a.priceServer < b.priceServer);
	}	

	void ParetoMatchingProblem::SearchEligibleDatacenters4Clients(const int latencyThreshold)
	{					
		for (auto & client : globalClientList)
		{
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
				}			
				if (!eligibleDatacenters_R_indexed_by_G.empty())
				{
					client.eligibleDatacenters_G.push_back(&dc_g);
					client.eligibleDatacenters_R_indexed_by_G[dc_g.id] = eligibleDatacenters_R_indexed_by_G;
				}
			}

			for (auto & dc_r : client.eligibleDatacenters_R)
			{
				vector<DatacenterType*> eligibleDatacenters_G_indexed_by_R;
				for (auto & dc_g : client.eligibleDatacenters_G)
				{
					if (std::find(client.eligibleDatacenters_R_indexed_by_G.at(dc_g->id).begin(), client.eligibleDatacenters_R_indexed_by_G.at(dc_g->id).end(), dc_r) != client.eligibleDatacenters_R_indexed_by_G.at(dc_g->id).end())
					{
						eligibleDatacenters_G_indexed_by_R.push_back(dc_g);
					}
				}
				client.eligibleDatacenters_G_indexed_by_R[dc_r->id] = eligibleDatacenters_G_indexed_by_R;
			}
		}
	}

	void ParetoMatchingProblem::ResetAssignment()
	{
		for (auto & client : candidateClients)
		{
			client.assignedDatacenter_G = nullptr;
			client.assignedDatacenter_R = nullptr;			
		}

		for (auto & dc : candidateDatacenters)
		{
			dc.coverableClients_G.clear();
			dc.coverableClients_R.clear();
			dc.assignedClients_G.clear();
			dc.assignedClients_R.clear();			
		}
	}

	void ParetoMatchingProblem::G_Assignment_Random()
	{		
		srand(0); /*fix the random number engine*/

		ResetAssignment();

		if (!R_Assignment_Completed)
		{
			for (auto & client : candidateClients)
			{
				auto index = GenerateRandomIndex(client.eligibleDatacenters_G.size());
				client.assignedDatacenter_G = client.eligibleDatacenters_G.at(index);
			}			
		}
		else
		{
			for (auto & client : candidateClients)
			{
				auto index = GenerateRandomIndex(client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).size());
				client.assignedDatacenter_G = client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).at(index);
			}
		}

		G_Assignment_Completed = true;
	}

	void ParetoMatchingProblem::G_Assignment_Simple(const int sessionSize)
	{			
		ResetAssignment();
		
		/*search coverableClients_G*/
		for (auto & dc_g : candidateDatacenters)
		{			
			dc_g.coverableClients_G.clear();
			if (!R_Assignment_Completed)
			{
				for (auto & client : candidateClients)
				{
					if (std::find(client.eligibleDatacenters_G.begin(), client.eligibleDatacenters_G.end(), &dc_g) != client.eligibleDatacenters_G.end())
					{
						dc_g.coverableClients_G.push_back(&client);
					}
				}
			}
			else
			{
				for (auto & client : candidateClients)
				{
					if (std::find(client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).begin(), client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).end(), &dc_g) != client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).end())
					{
						dc_g.coverableClients_G.push_back(&client);
					}
				}
			}
			
			/*sort coverableClients_G*/
			std::sort(dc_g.coverableClients_G.begin(), dc_g.coverableClients_G.end(), ClientComparatorByFewerEligibleDatacenters_G);
		}
			
		/*G-Assignment*/
		while (true)
		{
			/*pick the maxDC*/
			auto maxDC = &(candidateDatacenters.front());
			int maxRank = 0;
			for (auto & client : maxDC->coverableClients_G)
			{
				if (nullptr == client->assignedDatacenter_G) { maxRank++; }
			}
			for (auto & dc : candidateDatacenters)
			{
				int thisRank = 0;
				for (auto & client : dc.coverableClients_G)
				{
					if (nullptr == client->assignedDatacenter_G) { thisRank++; }
				}
				if (thisRank > maxRank)
				{
					maxRank = thisRank;
					maxDC = &dc;
				}
			}

			/*determine how many unassigned clients to assign in this round*/
			double unassignedClients_G_InMaxDC = (double)maxRank;
			int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClients_G_InMaxDC / sessionSize) * sessionSize);
			if (0 == clientsToBeGroupedInMaxDC) { break; }

			/*group (assign) clients in the maxDC*/
			for (auto & client : maxDC->coverableClients_G)
			{
				/*group (assign) one not-yet-grouped client*/
				if (clientsToBeGroupedInMaxDC > 0)
				{
					if (nullptr == client->assignedDatacenter_G)
					{
						client->assignedDatacenter_G = maxDC;						
						clientsToBeGroupedInMaxDC--;
					}
				}
				else break;
			}
		}

		G_Assignment_Completed = true;
	}

	void ParetoMatchingProblem::G_Assignment_Layered(const int sessionSize)
	{	
		ResetAssignment();
		
		if (!R_Assignment_Completed)
		{			
			/*search coverableClients_G*/
			for (auto & dc_g : candidateDatacenters)
			{				
				dc_g.coverableClients_G.clear();
				for (auto & client : candidateClients)
				{
					if (std::find(client.eligibleDatacenters_G.begin(), client.eligibleDatacenters_G.end(), &dc_g) != client.eligibleDatacenters_G.end())
					{
						dc_g.coverableClients_G.push_back(&client);
					}					
				}

				/*sort coverableClients_G*/
				map<vector<DatacenterType*>, vector<ClientType*>, DatacenterPointerVectorCmp> clientSectors;
				for (auto & c : dc_g.coverableClients_G) 
				{ 
					clientSectors[c->eligibleDatacenters_G].push_back(c); 
				}
				dc_g.coverableClients_G.clear();
				for (auto & sector : clientSectors)
				{
					for (auto & c : sector.second)
					{
						dc_g.coverableClients_G.push_back(c);
					}
				}
			}

			/*G-Assignment*/
			for (size_t layerIndex = 1; layerIndex <= candidateClients.size(); layerIndex++)
			{
				while (true)
				{
					/*pick the maxDC*/
					auto maxDC = &(candidateDatacenters.front());
					int maxRank = 0;
					for (auto & client : maxDC->coverableClients_G)
					{
						if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G.size() <= layerIndex) { maxRank++; }
					}
					for (auto & dc : candidateDatacenters)
					{
						int thisRank = 0;
						for (auto & client : dc.coverableClients_G)
						{
							if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G.size() <= layerIndex) { thisRank++; }
						}

						if (thisRank > maxRank)
						{
							maxRank = thisRank;
							maxDC = &dc;
						}
					}

					/*determine how many unassigned clients to assign in this round*/
					double unassignedClients_GInMaxDC = (double)maxRank;
					int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClients_GInMaxDC / sessionSize) * sessionSize);
					if (0 == clientsToBeGroupedInMaxDC) { break; }

					/*group (assign) clients in the maxDC*/
					for (auto & client : maxDC->coverableClients_G)
					{
						if (clientsToBeGroupedInMaxDC > 0)
						{
							if (nullptr == client->assignedDatacenter_G)
							{
								client->assignedDatacenter_G = maxDC;
								clientsToBeGroupedInMaxDC--;
							}
						}
						else break;
					}
				}
			}
		}
		else
		{
			/*search coverableClients_G*/
			for (auto & dc_g : candidateDatacenters)
			{				
				dc_g.coverableClients_G.clear();
				for (auto & client : candidateClients)
				{
					if (std::find(client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).begin(), client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).end(), &dc_g) != client.eligibleDatacenters_G_indexed_by_R.at(client.assignedDatacenter_R->id).end())
					{
						dc_g.coverableClients_G.push_back(&client);
					}
				}

				/*sort coverableClients_G*/
				map<vector<DatacenterType*>, vector<ClientType*>, DatacenterPointerVectorCmp> clientSectors;
				for (auto & client : dc_g.coverableClients_G)
				{
					clientSectors[client->eligibleDatacenters_G_indexed_by_R.at(client->assignedDatacenter_R->id)].push_back(client);
				}
				dc_g.coverableClients_G.clear();
				for (auto & sector : clientSectors)
				{
					for (auto & client : sector.second)
					{
						dc_g.coverableClients_G.push_back(client);
					}
				}
			}

			/*G-Assignment*/
			for (size_t layerIndex = 1; layerIndex <= candidateClients.size(); layerIndex++)
			{
				while (true)
				{
					/*pick the maxDC*/
					auto maxDC = &(candidateDatacenters.front());
					int maxRank = 0;
					for (auto & client : maxDC->coverableClients_G)
					{
						if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G_indexed_by_R.at(client->assignedDatacenter_R->id).size() <= layerIndex) { maxRank++; }
					}
					for (auto & dc : candidateDatacenters)
					{
						int thisRank = 0;
						for (auto & client : dc.coverableClients_G)
						{
							if (nullptr == client->assignedDatacenter_G && client->eligibleDatacenters_G_indexed_by_R.at(client->assignedDatacenter_R->id).size() <= layerIndex) { thisRank++; }
						}

						if (thisRank > maxRank)
						{
							maxRank = thisRank;
							maxDC = &dc;
						}
					}

					/*determine how many unassigned clients to assign in this round*/
					double unassignedClients_G_InMaxDC = (double)maxRank;
					int clientsToBeGroupedInMaxDC = int(std::floor(unassignedClients_G_InMaxDC / sessionSize) * sessionSize);
					if (0 == clientsToBeGroupedInMaxDC) { break; }

					/*group (assign) clients in the maxDC*/
					for (auto & client : maxDC->coverableClients_G)
					{
						if (clientsToBeGroupedInMaxDC > 0)
						{
							if (nullptr == client->assignedDatacenter_G)
							{
								client->assignedDatacenter_G = maxDC;
								clientsToBeGroupedInMaxDC--;
							}
						}
						else break;
					}
				}
			}
		}

		G_Assignment_Completed = true;
	}

	void ParetoMatchingProblem::R_Assignment_Random()
	{
		srand(0); /*fix the random number engine*/

		ResetAssignment();

		if (!G_Assignment_Completed)
		{
			for (auto & client : candidateClients) 
			{ 
				auto index = GenerateRandomIndex(client.eligibleDatacenters_R.size());
				client.assignedDatacenter_R = client.eligibleDatacenters_R.at(index);
			}
		}
		else
		{
			for (auto & client : candidateClients)
			{
				auto index = GenerateRandomIndex(client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).size());
				client.assignedDatacenter_R = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).at(index);
			}
		}

		R_Assignment_Completed = true;
	}

	void ParetoMatchingProblem::R_Assignment_LSP()
	{
		ResetAssignment();
		
		if (!G_Assignment_Completed)
		{
			for (auto & client : candidateClients)
			{
				client.assignedDatacenter_R = client.eligibleDatacenters_R.front();
				for (auto & dc_r : client.eligibleDatacenters_R)
				{
					if (dc_r->priceServer < client.assignedDatacenter_R->priceServer)
					{
						client.assignedDatacenter_R = dc_r;
					}
				}
			}
		}
		else
		{
			for (auto & client : candidateClients)
			{
				client.assignedDatacenter_R = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).front();
				for (auto & dc_r : client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id))
				{
					if (dc_r->priceServer < client.assignedDatacenter_R->priceServer)
					{
						client.assignedDatacenter_R = dc_r;
					}
				}
			}
		}

		R_Assignment_Completed = true;
	}

	void ParetoMatchingProblem::R_Assignment_LCW(const int serverCapacity)
	{
		ResetAssignment();
		
		/*search coverableClients_R*/
		for (auto & dc_r : candidateDatacenters)
		{			
			dc_r.coverableClients_R.clear();
			if (!G_Assignment_Completed)
			{
				for (auto & client : candidateClients)
				{
					if (std::find(client.eligibleDatacenters_R.begin(), client.eligibleDatacenters_R.end(), &dc_r) != client.eligibleDatacenters_R.end())
					{
						dc_r.coverableClients_R.push_back(&client);
					}
				}
			}
			else
			{
				for (auto & client : candidateClients)
				{
					if (std::find(client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).begin(), client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).end(), &dc_r) != client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).end())
					{
						dc_r.coverableClients_R.push_back(&client);
					}
				}
			}
		}

		/*R-Assignment*/
		while (true)
		{				
			/*compute projectedWastage*/
			vector<int> projectedWastage(candidateDatacenters.size());
			for (int i = 0; i < candidateDatacenters.size(); i++)
			{											
				int numberOfNewClientsToAssign = 0;
				for (auto & client : candidateDatacenters.at(i).coverableClients_R)
				{
					if (nullptr == client->assignedDatacenter_R)
						numberOfNewClientsToAssign++;
				}				
				if (numberOfNewClientsToAssign > 0)
				{
					if (0 == numberOfNewClientsToAssign % serverCapacity)
						projectedWastage.at(i) = 0;
					else
						projectedWastage.at(i) = serverCapacity - numberOfNewClientsToAssign % serverCapacity;
				}
				else
				{
					projectedWastage.at(i) = serverCapacity;
				}			
			}

			/*find the index of the dc with the minimum projectedWastage*/
			int index_assignedDatacenter_R = 0;
			for (int i = 0; i < candidateDatacenters.size(); i++)
			{
				if (projectedWastage.at(i) < projectedWastage.at(index_assignedDatacenter_R))
					index_assignedDatacenter_R = i;
			}

			/*assign unassigned clients to index_assignedDatacenter_R*/
			int numOfNewlyAssignedClients = 0;
			for (auto & client : candidateDatacenters.at(index_assignedDatacenter_R).coverableClients_R)
			{
				if (nullptr == client->assignedDatacenter_R)
				{
					client->assignedDatacenter_R = &(candidateDatacenters.at(index_assignedDatacenter_R));
					numOfNewlyAssignedClients++;
				}
			}

			/*terminate*/
			if (0 == numOfNewlyAssignedClients)
				break;
		}

		R_Assignment_Completed = true;
	}

	void ParetoMatchingProblem::Simulate(const int latencyThreshold, const int clientCount, const int sessionSize, const int serverCapacity, const int simulationCount)
	{	
		/*handle bad parameters*/
		if (clientCount < sessionSize)
		{
			printf("clientCount < sessionSize!\n");
			return;
		}

		/*consider all datacenters as candidateDatacenters*/
		candidateDatacenters = globalDatacenterList;

		/*search eligible datacenters from candidateDatacenters for every client*/
		SearchEligibleDatacenters4Clients(latencyThreshold);

		/*run simulation round by round*/
		for (int round = 1; round <= simulationCount; round++)
		{
			/*generate random candidateClients based on eligibility*/
			candidateClients.clear();
			while (candidateClients.size() < clientCount)
			{ 
				auto oneRandomClient = globalClientList.at(GenerateRandomIndex(globalClientList.size()));
				if (!oneRandomClient.eligibleDatacenters_G.empty()) { candidateClients.push_back(oneRandomClient); }
			}
		}
	}

	//void ParetoMatchingProblem::Random(const int sessionSize)
	//{	
	//	/*reset*/
	//	for (auto & client : candidateClients) 
	//	{ 
	//		client.assignedDatacenter_G = nullptr;
	//		client.assignedDatacenter_R = nullptr;
	//		client.isGrouped = false;
	//	}
	//	
	//	/*determine assignedDatacenter_G for each client*/		
	//	for (auto & client : candidateClients)
	//	{			
	//		if (!client.eligibleDatacenters_G.empty())
	//		{
	//			auto index = GenerateRandomIndex(client.eligibleDatacenters_G.size());				
	//			client.assignedDatacenter_G = client.eligibleDatacenters_G.at(index);					
	//		}
	//	}

	//	/*determine assignedDatacenter_R for each client*/		
	//	for (auto & client : candidateClients)
	//	{
	//		if (!client.eligibleDatacenters_G.empty())
	//		{				
	//			auto index = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).size();
	//			client.assignedDatacenter_R = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).at(GenerateRandomIndex(index));
	//		}
	//	}

	//	/*grouping*/
	//	printf("\nsessionCounter,sessionCost\n");
	//	vector<double> allCosts;
	//	vector<vector<ClientType*>> allSessions;
	//	while (true)
	//	{
	//		double oneCost = 0;
	//		vector<ClientType*> oneSession;
	//		bool noMoreNewSessions = true;
	//		for (auto & dc_g : candidateDatacenters)
	//		{
	//			bool newSessionFound = false;
	//			oneSession.clear();
	//			for (auto & client : candidateClients)
	//			{
	//				if (client.eligibleDatacenters_G.empty()) continue;

	//				/*check if a new sesson is found and record stuff accordingly*/
	//				if (oneSession.size() == sessionSize)
	//				{
	//					for (auto & sessionClient : oneSession) { oneCost += sessionClient->assignedDatacenter_R->priceServer; }

	//					allCosts.push_back(oneCost);

	//					allSessions.push_back(oneSession);

	//					for (auto & sessionClient : oneSession) { sessionClient->isGrouped = true; }

	//					newSessionFound = true;

	//					break; /*stop as a new session is found*/
	//				}

	//				if (!client.isGrouped && client.assignedDatacenter_G->id == dc_g.id) { oneSession.push_back(&client); }
	//			}
	//			if (newSessionFound)
	//			{
	//				noMoreNewSessions = false;
	//				break;
	//			}
	//		}
	//		if (noMoreNewSessions)
	//		{
	//			break;
	//		}

	//		printf("%d,%.2f\n", (int)allSessions.size(), allCosts.back());			
	//	}

	//	/*int groupedClientCount = 0;
	//	for (auto & client : candidateClients)
	//	{
	//		if (client.isGrouped) groupedClientCount++;
	//	}
	//	printf("%d\n", groupedClientCount);*/
	//}

	//void ParetoMatchingProblem::Greedy_1(const int sessionSize)
	//{						
	//	/*reset*/
	//	for (auto & client : candidateClients)
	//	{
	//		client.assignedDatacenter_G = nullptr;
	//		client.assignedDatacenter_R = nullptr;
	//		client.isGrouped = false;
	//	}

	//	/*determine assignedDatacenter_G for each client*/		
	//	for (auto & client : candidateClients)
	//	{
	//		if (!client.eligibleDatacenters_G.empty())
	//		{
	//			auto index = GenerateRandomIndex(client.eligibleDatacenters_G.size());				
	//			client.assignedDatacenter_G = client.eligibleDatacenters_G.at(index);				
	//		}
	//	}		

	//	/*determine assignedDatacenter_R for each client*/		
	//	for (auto & client : candidateClients)
	//	{
	//		if (!client.eligibleDatacenters_G.empty())
	//		{
	//			client.assignedDatacenter_R = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).front();
	//			for (auto & d_r : client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id))
	//			{
	//				if (d_r->priceServer < client.assignedDatacenter_R->priceServer)
	//				{
	//					client.assignedDatacenter_R = d_r;
	//				}
	//			}
	//		}
	//	}		

	//	/*grouping*/
	//	printf("\nsessionCounter,sessionCost\n");
	//	vector<double> allCosts;
	//	vector<vector<ClientType*>> allSessions;
	//	while (true)
	//	{
	//		double oneCost = 0;
	//		vector<ClientType*> oneSession;
	//		bool noMoreNewSessions = true;
	//		for (auto & dc_g : candidateDatacenters)
	//		{
	//			bool newSessionFound = false;
	//			oneSession.clear();
	//			for (auto & client : candidateClients)
	//			{
	//				if (client.eligibleDatacenters_G.empty()) continue;

	//				/*check if a new sesson is found and record stuff accordingly*/
	//				if (oneSession.size() == sessionSize)
	//				{
	//					for (auto & sessionClient : oneSession) { oneCost += sessionClient->assignedDatacenter_R->priceServer; }

	//					allCosts.push_back(oneCost);

	//					allSessions.push_back(oneSession);

	//					for (auto & sessionClient : oneSession) { sessionClient->isGrouped = true; }

	//					newSessionFound = true;

	//					break; /*stop as a new session is found*/
	//				}

	//				if (!client.isGrouped && client.assignedDatacenter_G->id == dc_g.id) { oneSession.push_back(&client); }
	//			}
	//			if (newSessionFound)
	//			{
	//				noMoreNewSessions = false;
	//				break;
	//			}
	//		}
	//		if (noMoreNewSessions)
	//		{
	//			break;
	//		}

	//		printf("%d,%.2f\n", (int)allSessions.size(), allCosts.back());
	//	}

	//	/*int groupedClientCount = 0;
	//	for (auto & client : candidateClients)
	//	{
	//		if (client.isGrouped) groupedClientCount++;
	//	}
	//	printf("%d\n", groupedClientCount);*/
	//}

	//void ParetoMatchingProblem::Greedy_2(const int sessionSize)
	//{
	//	/*reset*/
	//	for (auto & client : candidateClients)
	//	{
	//		client.assignedDatacenter_G = nullptr;
	//		client.assignedDatacenter_R = nullptr;
	//		client.isGrouped = false;
	//	}

	//	/*determine assignedDatacenter_G for each client*/
	//	for (auto & client : candidateClients)
	//	{
	//		if (!client.eligibleDatacenters_G.empty())
	//		{
	//			auto index = GenerateRandomIndex(client.eligibleDatacenters_G.size());
	//			client.assignedDatacenter_G = client.eligibleDatacenters_G.at(index);
	//		}
	//	}

	//	/*determine assignedDatacenter_R for each client*/
	//	for (auto & client : candidateClients)
	//	{
	//		if (!client.eligibleDatacenters_G.empty())
	//		{
	//			client.assignedDatacenter_R = client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id).front();
	//			for (auto & d_r : client.eligibleDatacenters_R_indexed_by_G.at(client.assignedDatacenter_G->id))
	//			{
	//				if (d_r->priceServer < client.assignedDatacenter_R->priceServer)
	//				{
	//					client.assignedDatacenter_R = d_r;
	//				}
	//			}
	//		}
	//	}

	//	/*grouping*/
	//	printf("\nsessionCounter,sessionCost\n");
	//	vector<double> allCosts;
	//	vector<vector<ClientType*>> allSessions;
	//	auto copy_candidateDatacenters = candidateDatacenters;
	//	std::sort(copy_candidateDatacenters.begin(), copy_candidateDatacenters.end(), DatacenterComparatorByPrice);
	//	while (true)
	//	{
	//		double oneCost = 0;
	//		vector<ClientType*> oneSession;
	//		bool noMoreNewSessions = true;
	//		for (auto & dc_g : candidateDatacenters)
	//		{
	//			bool newSessionFound = false;
	//			oneSession.clear();
	//			for (auto & dc_r : copy_candidateDatacenters)
	//			{
	//				for (auto & client : candidateClients)
	//				{
	//					if (client.eligibleDatacenters_G.empty()) continue;

	//					/*check if a new sesson is found and record stuff accordingly*/
	//					if (oneSession.size() == sessionSize)
	//					{
	//						for (auto & sessionClient : oneSession) { oneCost += sessionClient->assignedDatacenter_R->priceServer; }

	//						allCosts.push_back(oneCost);

	//						allSessions.push_back(oneSession);

	//						for (auto & sessionClient : oneSession) { sessionClient->isGrouped = true; }

	//						newSessionFound = true;

	//						break; /*stop as a new session is found*/
	//					}

	//					if (!client.isGrouped && client.assignedDatacenter_G->id == dc_g.id && client.assignedDatacenter_R->id == dc_r.id) { oneSession.push_back(&client); }
	//				}
	//				if (newSessionFound)
	//				{
	//					break;
	//				}
	//			}
	//			if (newSessionFound)
	//			{
	//				noMoreNewSessions = false;
	//				break;
	//			}				
	//		}
	//		if (noMoreNewSessions)
	//		{
	//			break;
	//		}

	//		printf("%d,%.2f\n", (int)allSessions.size(), allCosts.back());
	//	}

	//	/*int groupedClientCount = 0;
	//	for (auto & client : candidateClients)
	//	{
	//		if (client.isGrouped) groupedClientCount++;
	//	}
	//	printf("%d\n", groupedClientCount);*/
	//}

	//void ParetoMatchingProblem::Greedy_3(const int sessionSize)
	//{
	//	/*reset*/
	//	for (auto & client : candidateClients)
	//	{
	//		client.assignedDatacenter_G = nullptr;
	//		client.assignedDatacenter_R = nullptr;
	//		client.isGrouped = false;
	//	}		

	//	/*determine assignedDatacenter_R for each client*/
	//	for (auto & client : candidateClients)
	//	{
	//		if (!client.eligibleDatacenters_G.empty())
	//		{
	//			client.assignedDatacenter_R = client.eligibleDatacenters_R.front();
	//			for (auto & d_r : client.eligibleDatacenters_R)
	//			{
	//				if (d_r->priceServer < client.assignedDatacenter_R->priceServer)
	//				{
	//					client.assignedDatacenter_R = d_r;
	//				}
	//			}
	//		}
	//	}

	//	/*determine assignedDatacenter_G for each client*/		
	//	for (auto & client : candidateClients)
	//	{			
	//		if (!client.eligibleDatacenters_G.empty())
	//		{
	//			for (auto & d_g : client.eligibleDatacenters_G)
	//			{
	//				if (client.eligibleDatacenters_R_indexed_by_G.at(d_g->id).end() != std::find(client.eligibleDatacenters_R_indexed_by_G.at(d_g->id).begin(), client.eligibleDatacenters_R_indexed_by_G.at(d_g->id).end(), client.assignedDatacenter_R))
	//				{
	//					client.assignedDatacenter_G = d_g;						
	//					break;
	//				}
	//			}
	//		}
	//	}

	//	/*grouping*/
	//	printf("\nsessionCounter,sessionCost\n");
	//	vector<double> allCosts;
	//	vector<vector<ClientType*>> allSessions;
	//	while (true)
	//	{
	//		double oneCost = 0;
	//		vector<ClientType*> oneSession;
	//		bool noMoreNewSessions = true;
	//		for (auto & dc_g : candidateDatacenters)
	//		{
	//			bool newSessionFound = false;
	//			oneSession.clear();
	//			for (auto & client : candidateClients)
	//			{
	//				if (client.eligibleDatacenters_G.empty()) continue;

	//				/*check if a new sesson is found and record stuff accordingly*/
	//				if (oneSession.size() == sessionSize)
	//				{
	//					for (auto & sessionClient : oneSession) { oneCost += sessionClient->assignedDatacenter_R->priceServer; }

	//					allCosts.push_back(oneCost);

	//					allSessions.push_back(oneSession);

	//					for (auto & sessionClient : oneSession) { sessionClient->isGrouped = true; }

	//					newSessionFound = true;

	//					break; /*stop as a new session is found*/
	//				}

	//				if (!client.isGrouped && client.assignedDatacenter_G->id == dc_g.id) { oneSession.push_back(&client); }
	//			}
	//			if (newSessionFound)
	//			{
	//				noMoreNewSessions = false;
	//				break;
	//			}
	//		}
	//		if (noMoreNewSessions)
	//		{
	//			break;
	//		}

	//		printf("%d,%.2f\n", (int)allSessions.size(), allCosts.back());
	//	}

	//	/*int groupedClientCount = 0;
	//	for (auto & client : candidateClients)
	//	{
	//		if (client.isGrouped) groupedClientCount++;
	//	}
	//	printf("%d\n", groupedClientCount);*/
	//}
}