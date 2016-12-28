#include "ServerAllocationProblem.h"
#include "MatchmakingProblem.h"

using namespace MatchmakingProblem;

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
	auto simulator = ParetoMatchingProblem();
	simulator.Initialize();
	_mkdir(simulator.outputDirectory.c_str());
	for (bool regionControl : { false/*, true*/ })
	{
		for (int clientCount : { 100, 200, 400 })
		{
			for (int latencyThreshold : { 25, 50, 100 })
			{
				for (int sessionSize : { 10 })
				{
					for (int serverCapacity : { 4 })
					{
						cout << "\nSimulate() with setting: " << regionControl << "." << clientCount << "." << latencyThreshold << "." << sessionSize << "." << serverCapacity << "\n";
						simulator.Simulate(Setting(regionControl, clientCount, latencyThreshold, sessionSize, serverCapacity, 1000));
					}
				}
			}
		}
	}
	std::printf("\n***total simulation running time: %.2f seconds***\n", std::difftime(clock(), startTime) / 1000);

	return 0;
}