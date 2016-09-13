#include "ServerAllocationProblem.h"
#include "MatchmakingProblem.h"

int main(int argc, char *argv[])
{
	/*double DELAY_BOUND_TO_G = 75, DELAY_BOUND_TO_R = 50, SESSION_SIZE = 10;
	if (argc >= 4)
	{
	DELAY_BOUND_TO_G = std::stod(argv[1]);
	DELAY_BOUND_TO_R = std::stod(argv[2]);
	SESSION_SIZE = std::stod(argv[3]);

	if (DELAY_BOUND_TO_G <= 0 || DELAY_BOUND_TO_R <= 0 || SESSION_SIZE <= 0)
	{
	printf("ERROR: invalid main() parameters\n");
	cin.get();
	return 0;
	}
	}*/

	/*ServerAllocationProblem*/
	/*ServerAllocationProblem::SimulateBasicProblem(75, 50, 10);
	ServerAllocationProblem::SimulateBasicProblem(75, 50, 50);
	ServerAllocationProblem::SimulateBasicProblem(150, 100, 10);
	ServerAllocationProblem::SimulateBasicProblem(150, 100, 50);
	ServerAllocationProblem::SimulateGeneralProblem(75, 50, 10);
	ServerAllocationProblem::SimulateGeneralProblem(75, 50, 50);
	ServerAllocationProblem::SimulateGeneralProblem(150, 100, 10);
	ServerAllocationProblem::SimulateGeneralProblem(150, 100, 50);*/

	/*MatchmakingProblem*/	
	auto simulator = MatchmakingProblem::MaximumMatchingProblem();	
	simulator.Initialize();
	_mkdir(simulator.outputDirectory.c_str());
	for (string algName : { "nearest", "random" })
	{
		for (int latenchThreshold : { 25, 50, 100 })
		{
			simulator.groupingAlgorithm = algName;
			simulator.outFile = ofstream(simulator.outputDirectory + simulator.groupingAlgorithm + "_" + std::to_string(latenchThreshold) + ".csv");
			srand(0); // to ensure the random numbers genereted for each algorithm is the same
			for (int clientCount = 20; clientCount <= 400; clientCount += 20)
			{
				simulator.Simulate(clientCount, latenchThreshold);
			}
			simulator.outFile.close();
		}
	}
	
	return 0;
}