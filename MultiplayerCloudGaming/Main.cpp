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
	auto simulator = MatchmakingProblem::ParetoMatchingProblem();
	simulator.Initialize();
	_mkdir(simulator.outputDirectory.c_str());
	for (bool sharedCostAcrossSessions : { true, false })
	{
		for (int clientCount : { 100, 500 })
		{
			for (int latencyThreshold : { 100 })
			{
				for (int sessionSize : { 10 })
				{
					for (int serverCapacity : { 1, 4 })
					{
						simulator.Simulate(sharedCostAcrossSessions, clientCount, latencyThreshold, sessionSize, serverCapacity);
					}
				}
			}
		}
	}

	return 0;
}