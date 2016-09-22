#include "ServerAllocationProblem.h"
#include "MatchmakingProblem.h"

struct MyVectorCmp 
{
	bool operator()(const vector<int> a, const vector<int> b) const
	{
		if (a.size() < b.size()) { return true; }
		else if ((a.size() == b.size()) && !a.empty())
		{
			for (size_t i = 0; i < a.size(); i++)
			{
				if (a.at(i) < b.at(i)) { return true; }
				else if (a.at(i) == b.at(i))
				{
					if (i < (a.size() - 1)) { continue; }
					else { return false; }
				}
				else { return false; }
			}
		}
		else { return false; }
	}
};

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
	/*for (double sessionSize : { 10, 50 })
	{
		ServerAllocationProblem::SimulateBasicProblem(75, 50, sessionSize);
		ServerAllocationProblem::SimulateBasicProblem(150, 100, sessionSize);
		ServerAllocationProblem::SimulateGeneralProblem(75, 50, sessionSize);
		ServerAllocationProblem::SimulateGeneralProblem(150, 100, sessionSize);
	}*/

	/*MatchmakingProblem*/
	auto simulator = MatchmakingProblem::MaximumMatchingProblem();	
	simulator.Initialize(); // initialize once
	_mkdir(simulator.outputDirectory.c_str());
	for (string algToRun : { "layered", "simple", "nearest", "random" })
	{		
		for (int latencyThreshold : { 25, 50, 100 })
		{			
			for (int sessionSize : { 10, 20, 40 })
			{
				simulator.outFile = ofstream(simulator.outputDirectory + algToRun + "_" + std::to_string(latencyThreshold) + "_" + std::to_string(sessionSize) + ".csv");
				for (int clientCount = 50; clientCount <= 1000; clientCount += 50) { simulator.Simulate(algToRun, clientCount, latencyThreshold, sessionSize); }				
				simulator.outFile.close();
			}
		}
	}

	return 0;
}