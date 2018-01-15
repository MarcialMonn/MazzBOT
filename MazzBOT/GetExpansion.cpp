#include "GetExpansion.h"
#include "sc2api/sc2_client.h"

#include <iostream>
#include <cmath>


GetExpansions::GetExpansions()
{
	bot = nullptr;
}

GetExpansions::~GetExpansions()
{
}

std::vector<Expansion> GetExpansions::FindExpansions()
{
	DebugInterface* debug = bot->Debug();

	std::vector<Expansion> Expansions;
	Expansion StartExp;

	std::cout << "Entered the GetExpansions routine!" << std::endl;

	// First get the starting position:
	// Get a list of all own units
	// Iterate through the list of own units and find the command center
	for (const auto& unit : bot->Observation()->GetUnits(Unit::Alliance::Self))
	{
		if (unit->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER)
		{
			StartExp.pos = unit->pos;
		}
	}

	// Create an Expansion struct for the starting position, and add it to the vector of Expansions structs
	Expansions.push_back(StartExp);
	std::cout << "Calculated starting Position to be at: " << std::endl;
	std::cout << StartExp.pos.x << " x coords and: " << StartExp.pos.y << " y coords!" << std::endl;

	// Find all the different Expansion positions and the corresponding Mineral Fields
	// Get all neutral units, to find the Mineral Fields
	Units MF;
	bool BaseFound;
	std::vector<int> Base;

	// Iterate through all neutral units
	for (const auto& u : bot->Observation()->GetUnits(Unit::Alliance::Neutral))
	{ // Check if the unit is a Mineral Field
		if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD || u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD750 || u->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER)
		{
			if (DistanceSquared3D(u->pos, Expansions[0].pos) <= 250)
			{ // Check if the Mineral Field belongs to the starting base
				Expansions[0].Ressources.push_back(u);
				if (u->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER)
				{
					Expansions[0].GasGeyser.push_back(u);
				}
				else
				{
					Expansions[0].MineralFields.push_back(u);
				}
			}
			else
			{
				BaseFound = false;
				for (int i = 0; i < Expansions.size(); i++)
				{
					if (DistanceSquared3D(u->pos, Expansions[i].pos) <= 250)
					{
						Expansions[i].Ressources.push_back(u);
						if (u->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER)
						{
							Expansions[i].GasGeyser.push_back(u);
						}
						else
						{
							Expansions[i].MineralFields.push_back(u);
						}
						BaseFound = true;
					}
				}
				if (BaseFound == false)
				{
					Expansion NewExp;
					NewExp.pos = u->pos;
					NewExp.Ressources.push_back(u);
					if (u->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER)
					{
						NewExp.GasGeyser.push_back(u);
					}
					else
					{
						NewExp.MineralFields.push_back(u);
					}
					Expansions.push_back(NewExp);
				}
			}
		}
	}
	std::cout << "There were a total of: " << Expansions.size() << "Expansions found!" << std::endl;

	// Get the location of the expansion
	for (int ind = 1; ind < Expansions.size(); ind++)
	{
		// Chose one MineralFields randomly (First in the list)
		Expansion exp = Expansions[ind];
		float xpos = exp.MineralFields[0]->pos.x;
		float ypos = exp.MineralFields[0]->pos.y;

		// Circle around this MineralField and check, if the distance of all other MineralFields of this expansion exceeds a certain value
		bool PosFound;

		// Two positions to be checked (one circling clockwise, one counter clockwise)
		Point2D CheckPos1;
		Point2D CheckPos2;
		int iind = 8; // Minimal Required Distance
		std::vector<float> Distances;			// Vector containing the RMS Distance to a certain point to be checked
		std::vector<Point2D> ValidPositions;	// Vector containing valid positionsa

		// While there are no valid positions found
		while (Distances.size() == 0)
		{
			// Iterate through all possible x-coordinates
			for (int xsweep = -iind; xsweep <= iind; xsweep++)
			{
				// Iterate through all possible y-coordinates
				int ysweep = ceil(sqrt(pow(iind, 2) - pow(xsweep, 2)));
				CheckPos1.x = exp.MineralFields[0]->pos.x + xsweep;
				CheckPos2.x = CheckPos1.x;
				CheckPos1.y = exp.MineralFields[0]->pos.y + ysweep;
				CheckPos2.y = exp.MineralFields[0]->pos.y - ysweep;
				std::cout << "We are circling around: " << exp.MineralFields[0]->pos.x << " " << exp.MineralFields[0]->pos.y << std::endl;
				std::cout << "With a radius of " << iind << std::endl;

				/*
				if (xsweep == -iind)
				{
					int a;
					debug->DebugTextOut("O", exp.MineralFields[0]->pos, sc2::Colors::Green);
					debug->SendDebug();
					std::cout << "We are looking at position: " << CheckPos1.x << " " << CheckPos1.y << std::endl;
					std::cout << "And position: " << CheckPos2.x << " " << CheckPos2.y << std::endl;
					std::cout << "Ready?" << std::endl;
					//std::cin >> a;
				}

				// Debugging help:
				debug->DebugTextOut("X1", CheckPos1, sc2::Colors::White);
				debug->DebugTextOut("X2", CheckPos2, sc2::Colors::White);
				debug->SendDebug();
				*/

				std::vector<float> DistancesToMF;
				PosFound = true;
				for (const auto& mf : exp.MineralFields)
				{
					if (Distance2D(Convert3Dto2D(mf->pos), CheckPos1) < MinDistanceMin)
					{
						std::cout << "Distance is: " << Distance2D(Convert3Dto2D(mf->pos), CheckPos1) << std::endl;
						PosFound = false;
						break;
					}
					DistancesToMF.push_back(Distance2D(Convert3Dto2D(mf->pos), CheckPos1));
				}
				for (const auto& gg : exp.GasGeyser)
				{
					if (Distance2D(Convert3Dto2D(gg->pos), CheckPos1) < MinDistanceGas)
					{
						std::cout << "Distance is: " << Distance2D(Convert3Dto2D(gg->pos), CheckPos1) << std::endl;
						PosFound = false;
						break;
					}
					DistancesToMF.push_back(Distance2D(Convert3Dto2D(gg->pos), CheckPos1));
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
					if (Distance2D(Convert3Dto2D(mf->pos), CheckPos2) < MinDistanceMin)
					{
						std::cout << "Distance is: " << Distance2D(Convert3Dto2D(mf->pos), CheckPos2) << std::endl;
						PosFound = false;
						break;
					}
					DistancesToMF.push_back(Distance2D(Convert3Dto2D(mf->pos), CheckPos2));
				}

				for (const auto& gg : exp.GasGeyser)
				{
					if (Distance2D(Convert3Dto2D(gg->pos), CheckPos2) < MinDistanceGas)
					{
						std::cout << "Distance is: " << Distance2D(Convert3Dto2D(gg->pos), CheckPos2) << std::endl;
						PosFound = false;
						break;
					}
					DistancesToMF.push_back(Distance2D(Convert3Dto2D(gg->pos), CheckPos2));
				}

				if (PosFound)
				{
					Distances.push_back(RootMeanSquare(DistancesToMF));
					ValidPositions.push_back(CheckPos2);
				}
			}
			iind += 1;
		}
		std::cout << "Entered Refine Position routine!" << std::endl;
		exp.pos = RefinePosition(ValidPositions[FindMinimalValue(Distances).second], exp);
		std::cout << "Exited Refine Position routine!" << std::endl;
		Expansions[ind] = exp;
		
	}
	return Expansions;
}

Point3D GetExpansions::RefinePosition(Point2D InitPos, Expansion exp)
{
	Point2D CheckPos;
	Point3D ReturnPos;
	
	if (ceil(InitPos.x) == InitPos.x)
	{
		InitPos.x += 0.5;
	}
	if (ceil(InitPos.y) == InitPos.y)
	{
		InitPos.y += 0.5;
	}

	bool Optimal = false;
	bool ToNear;
	while (!Optimal)
	{
		std::vector<Point2D> ValidPositions;
		std::vector<float> RMSDistance;
		for (int xsweep = -1; xsweep <= 1; xsweep++)
		{
			CheckPos.x = InitPos.x + xsweep;
			for (int ysweep = -1; ysweep <= 1; ysweep++)
			{
				CheckPos.y = InitPos.y + ysweep;

				std::vector<float> DistancesToMF;
				ToNear = false;
				for (const auto& rf : exp.MineralFields)
				{
					float Distance = Distance2D(rf->pos, CheckPos);
					if (Distance < MinDistanceMin)
					{
						ToNear = true;
						break;
					}
					DistancesToMF.push_back(Distance);
				}
				for (const auto& rf : exp.GasGeyser)
				{
					float Distance = Distance2D(rf->pos, CheckPos);
					if (Distance < MinDistanceGas)
					{
						ToNear = true;
						break;
					}
					DistancesToMF.push_back(Distance);
				}
				if (!ToNear)
				{
					ValidPositions.push_back(CheckPos);
					RMSDistance.push_back(RootMeanSquare(DistancesToMF));
				}
			}
		}
		
		std::pair<float, int> MinVal = FindMinimalValue(RMSDistance);

		if (ValidPositions[MinVal.second] == InitPos)
		{
			Optimal = true;
		}
		else
		{
			InitPos = ValidPositions[MinVal.second];
		}

	}
	
	ReturnPos.x = InitPos.x;
	ReturnPos.y = InitPos.y;
	ReturnPos.z = bot->Observation()->TerrainHeight(InitPos);
	return ReturnPos;
}
