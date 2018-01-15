#include <vector>
#include "MazzBOT.h"
#include "sc2api/sc2_api.h"

std::vector<Expansion> GetExpansions(Units InUnits, std::vector<std::vector<int>> PathBuildStatus, std::vector<std::vector<float>> TerrainHeight)
{
	Client client = Client();
	DebugInterface* debug = Client().Debug();

	Point3D StartingPosition;

	std::vector<Expansion> Expansions;

	std::cout << "Entering the GetExpansions routine!" << std::endl;

	// First get the starting position:
	// Get a list of all own units
	// Iterate through the list of own units and find the command center
	for (const auto& unit : InUnits)
	{
		if (unit->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER)
		{
			// The position of the command center equals the starting position
			StartingPosition = unit->pos;
		}
	}

	// Create an Expansion struct for the starting position, and add it to the vector ef Expansion structs
	Expansion StartExp;
	StartExp.pos = StartingPosition;
	Expansions.push_back(StartExp);
	std::cout << "Calculated starting Position to be at: " << std::endl;
	std::cout << StartExp.pos.x << " x coords and: " << StartExp.pos.y << "y coords!" << std::endl;

	// Finds all the different Expansion positions and the corresponding Mineral Fields
	// Get all neutral units, to find the Mineral Fields
	//Units NeutralUnits = Observation()->GetUnits(Unit::Alliance::Neutral);
	Units MF; // Array to contain all already found Mineral Fields
	bool BaseFound; // Bool indicating if momentarily chosen Mineral Field can be associated with an already found base
	std::vector<int> Base; // Vector containing the base nr for every Mineral Field

						   // Iterate through all neutral units
	for (const auto& u : InUnits)
	{ // Check if the unit is a Mineral Field
		if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD || u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD750)
		{
			if (DistanceSquared3D(u->pos, StartingPosition) <= 200)
			{ // Check if the Mineral Field belongs to the starting base
				Expansions[0].MineralFields.push_back(u);
			}
			else
			{
				BaseFound = false;
				for (int i = 0; i < Expansions.size(); i++)
				{
					if (DistanceSquared3D(u->pos, Expansions[i].pos) <= 200)
					{
						Expansions[i].MineralFields.push_back(u);
						BaseFound = true;
					}
				}
				if (BaseFound == false)
				{
					Expansion NewExp;
					NewExp.pos = u->pos; // Only first position estimate to have this value filled
					NewExp.MineralFields.push_back(u);
					Expansions.push_back(NewExp);
				}
			}
		}
	}
	std::cout << "There were a total of: " << Expansions.size() << " Expansions found!" << std::endl;

	// Get the location to build the expansion
	int fe = 1;
	for (auto& exp : Expansions)
	{
		if (fe == 1)
		{ // If first Expansion, the location has already been found, and nothing needs to be done
			fe = 0;
		}
		else
		{
			// Chose one MineralField randomly (First in the list)
			float xpos = exp.MineralFields[0]->pos.x;
			float ypos = exp.MineralFields[0]->pos.y;

			// Circle around this Mineralfield and check if the distance of all other mineralfields of this expansion exceeds a certain value
			bool PosFound;

			// Two positions to be checked (one circling clockwise, one counter clockwise)
			Point2D CheckPos1;
			Point2D CheckPos2;
			int iind = 6;	// Minimal required Distance
			std::vector<float> Distances;	// Vector containing the RMS Distance to a certain point to be checked
			std::vector<Point2D> ValidPositions;	// Vector containint valid positions

													// While there are no valid positions found:
			while (Distances.size() == 0)
			{
				// Iterate though all possible x-coordinates
				for (int xsweep = -iind; xsweep <= iind; xsweep++)
				{
					// Iterate through all possible y-coordinates
					float ysweep = ceil(sqrt(pow(iind, 2) - pow(xsweep, 2)));
					CheckPos1.x = exp.MineralFields[0]->pos.x + xsweep;
					CheckPos2.x = CheckPos1.x;
					CheckPos1.y = exp.MineralFields[0]->pos.y + ysweep;
					CheckPos2.y = exp.MineralFields[0]->pos.y - ysweep;


					// Debugging help:
					debug->DebugTextOut("X1", CheckPos1, sc2::Colors::White);
					debug->DebugTextOut("X2", CheckPos2, sc2::Colors::White);
					debug->SendDebug();


					std::vector<float> DistancesToMF;
					PosFound = true;
					for (const auto& mf : exp.MineralFields)
					{
						if (Distance2D(Convert3Dto2D(mf->pos), CheckPos1) < iind)
						{
							PosFound = false;
							break;
						}
						DistancesToMF.push_back(Distance2D(Convert3Dto2D(mf->pos), CheckPos1));
					}
					if (PosFound)
					{
						Distances.push_back(RootMeanSquare(DistancesToMF));
						ValidPositions.push_back(CheckPos1);
					}

					DistancesToMF.resize(0);
					PosFound = true;
					for (const auto& mf : exp.MineralFields)
					{
						if (Distance2D(Convert3Dto2D(mf->pos), CheckPos2) < iind)
						{
							PosFound = false;
							break;
						}
						DistancesToMF.push_back(Distance2D(Convert3Dto2D(mf->pos), CheckPos2));
					}
					if (PosFound)
					{
						Distances.push_back(RootMeanSquare(DistancesToMF));
						ValidPositions.push_back(CheckPos2);
					}
				}
				std::cout << iind << std::endl;
				iind++;
			}
			std::pair<float, int> MinDistances;
			MinDistances = FindMinimalValue(Distances);
			exp.pos = Convert2Dto3D(ValidPositions[MinDistances.second], TerrainHeight);
		}
	}
	return Expansions;
}