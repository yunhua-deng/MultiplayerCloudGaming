#include "Simulation.h"

int main(int argc, char *argv[])
{	
	double DELAY_BOUND_TO_G = 0, DELAY_BOUND_TO_R = 0, SESSION_SIZE = 0, BANDWIDTH_INTENSITY = 0; // parameters to be set by function arguments
	if (argc >= 5)	
	{
		DELAY_BOUND_TO_G = std::stod(argv[1]);
		DELAY_BOUND_TO_R = std::stod(argv[2]);
		SESSION_SIZE = std::stod(argv[3]);
		BANDWIDTH_INTENSITY = std::stod(argv[4]);

		if (DELAY_BOUND_TO_G <= 0 || DELAY_BOUND_TO_R <= 0 || SESSION_SIZE <= 0 || BANDWIDTH_INTENSITY <= 0)
		{
			printf("ERROR: invalid main() parameters\n");
			cin.get();
			return 0;
		}
	}

	SimulationBasicProblem(75, 50, 10, 2, 100);
	SimulationGeneralProblem(75, 50, 10, 2, 100);
	//SimulationBasicProblem(DELAY_BOUND_TO_G, DELAY_BOUND_TO_R, SESSION_SIZE, BANDWIDTH_INTENSITY);
	//SimulationGeneralProblem(DELAY_BOUND_TO_G, DELAY_BOUND_TO_R, SESSION_SIZE, BANDWIDTH_INTENSITY);

	return 0;
}