#include "MatchmakingProblem.h"

namespace MatchmakingProblem
{
	void MatchmakingProblemBase::Initialize(const string given_dataDirectory)
	{
		this->dataDirectory = given_dataDirectory;
		this->globalClientList.clear();
		this->globalDatacenterList.clear();

		/* temporary stuff */
		vector<vector<double>> ClientToDatacenterDelayMatrix;
		vector<vector<double>> InterDatacenterDelayMatrix;
		vector<double> priceServerList;
		vector<double> priceBandwidthList;

		/* client-to-dc latency data */
		auto strings_read = ReadDelimitedTextFileIntoVector(dataDirectory + "dc_to_pl_rtt.csv", ',', true);
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
		strings_read = ReadDelimitedTextFileIntoVector(dataDirectory + "dc_to_dc_rtt.csv", ',', true);
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
		strings_read = ReadDelimitedTextFileIntoVector(dataDirectory + "dc_pricing_bandwidth_server.csv", ',', true);
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
		printf("%d clients loaded\n", int(globalClientList.size()));

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
		printf("%d datacenters loaded\n", int(globalDatacenterList.size()));
	}

	void MaximumMatchingProblem::RunSimulation()
	{		
		SetupSimulation();
	}

	void MaximumMatchingProblem::SetupSimulation(const double clientPopulation, const double latencyThreshold, const double sessionSize)
	{
		if (clientPopulation > globalClientList.size())
		{
			printf("ERROR: clientPopulation too large!\n");
			cin.get();
			return;
		}

		//std::srand(2); // seed for random number generator
		std::srand(time(NULL)); // seed for random number generator

		auto globalClientListCopy = globalClientList; // avoid modifying the original		
		random_shuffle(globalClientListCopy.begin(), globalClientListCopy.end());
		candidateClients.assign(globalClientListCopy.begin(), globalClientListCopy.begin() + (size_t)clientPopulation);
		candidateDatacenters = globalDatacenterList;
		
		for (auto& client : candidateClients)
		{
			client.eligibleDatacenters.clear();
			for (auto& dc : candidateDatacenters)
			{
				client.eligibleDatacenters.push_back(&dc);
			}
			//printf("client %d : nearest dc = %d\n", client.id, GetClientNearestDC(client)->id);
			client.assignedDatacenter = GetClientNearestDC(client);
			client.assignedDatacenter->assignedClients.push_back(&client);
		}

		double totalSessionNumber = 0;
		for (auto& dc : candidateDatacenters)
		{
			totalSessionNumber += std::floor(dc.assignedClients.size() / sessionSize);
		}
		printf("totalSessionNumber = %d\n", (int)totalSessionNumber);
	}

	DatacenterType* GetClientNearestDC(ClientType & client)
	{
		DatacenterType* nearest = client.eligibleDatacenters.front();
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