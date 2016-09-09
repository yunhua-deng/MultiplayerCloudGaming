#include "ServerAllocationProblem.h"

int main(int argc, char *argv[])
{	
	double DELAY_BOUND_TO_G = 75, DELAY_BOUND_TO_R = 50, SESSION_SIZE = 10;
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
	}

	ServerAllocationProblem::SimulateBasicProblem(DELAY_BOUND_TO_G, DELAY_BOUND_TO_R, SESSION_SIZE, 100);
	ServerAllocationProblem::SimulateGeneralProblem(DELAY_BOUND_TO_G, DELAY_BOUND_TO_R, SESSION_SIZE, 100);

	return 0;
}