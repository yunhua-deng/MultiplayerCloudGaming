#include "ServerAllocationProblem.h"
#include "MatchmakingProblem.h"

int main(int argc, char *argv[])
{
	/*ServerAllocationProblem*/	
	/*for (double sessionSize : { 10, 50 })
	{
		ServerAllocationProblem::SimulateBasicProblem(75, 50, sessionSize);
		ServerAllocationProblem::SimulateBasicProblem(150, 100, sessionSize);
		ServerAllocationProblem::SimulateGeneralProblem(75, 50, sessionSize);
		ServerAllocationProblem::SimulateGeneralProblem(150, 100, sessionSize);
	}*/

	/*MaximumMatchingProblem*/
	//auto simulator = MatchmakingProblem::MaximumMatchingProblem();	
	//simulator.Initialize(); // initialize once
	//_mkdir(simulator.outputDirectory.c_str());
	//for (string algToRun : { "layered", "simple", "nearest", "random" })
	//{		
	//	for (int latencyThreshold : { 25, 50, 100 })
	//	{			
	//		for (int sessionSize : { 10, 20, 40 })
	//		{
	//			simulator.outFile = ofstream(simulator.outputDirectory + algToRun + "_" + std::to_string(latencyThreshold) + "_" + std::to_string(sessionSize) + ".csv");
	//			for (int clientCount = 50; clientCount <= 1000; clientCount += 50) { simulator.Simulate(algToRun, clientCount, latencyThreshold, sessionSize); }				
	//			simulator.outFile.close();
	//		}
	//	}
	//}

	/*ParetoMatchingProblem*/
	auto startTime = clock();
	auto simulator = MatchmakingProblem::ParetoMatchingProblem();
	simulator.Initialize();
	_mkdir(simulator.outputDirectory.c_str());
	for (bool controlled : { false, true })
	{
		for (int clientCount : { 100, 500 })
		{
			for (int latencyThreshold : { 25, 50 })
			{
				for (int sessionSize : { 10 })
				{
					for (int serverCapacity : { 5 })
					{
						cout << "\nSimulate() with setting: " << controlled << "." << clientCount << "." << latencyThreshold << "." << sessionSize << "." << serverCapacity << "\n";
						simulator.Simulate(controlled, clientCount, latencyThreshold, sessionSize, serverCapacity, 100);
					}
				}
			}
		}
	}
	std::printf("\n***total simulation running time: %.2f seconds***\n", std::difftime(clock(), startTime) / 1000);

	return 0;
}