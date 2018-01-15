#include "Debugging.h"
#include "MazzBOT.h"
#include "AuxiliaryFunctions.h"

#include <iostream>

Debugging::Debugging()
{
	bot = nullptr;
	std::vector<Point2D> Path;
	Path.resize(2);
}

Debugging::~Debugging()
{
}


void Debugging::main()
{
	/*
	ShowAllUnitsAbilityID(bot);
	ShowStartingPosition(bot);
	ShowExpansionsInformation(bot);
	debug->SendDebug();
	*/
}

void Debugging::ShowAllUnitsAbilityID()
{
	Point3D UnitPos;
	Units OwnUnits = bot->Observation()->GetUnits(Unit::Alliance::Self);
	DebugInterface* debug = bot->Debug();
	for (const auto& unit : OwnUnits)
	{
		for (const auto& order : unit->orders)
		{
			debug->DebugTextOut(GetAbilityText(order.ability_id), unit->pos, sc2::Colors::Green);
			//debug->DebugTextOut(GetAbilityText(order.ability_id), unit->pos, sc2::Colors::Green);
		}
	}
	debug->SendDebug();
}

void Debugging::ShowStartingPosition()
{
	const ObservationInterface* obs = bot->Observation();
	DebugInterface* debug = bot->Debug();
	Point2D SPos = bot->GetOwnStartPosition();
	Point3D DebugPosMin, DebugPosMax;
	DebugPosMin.x = SPos.x - 2.0f;
	DebugPosMin.y = SPos.y - 2.0f;
	DebugPosMin.z = obs->TerrainHeight(SPos) - 2.0f;
	DebugPosMax.x = SPos.x + 2.0f;
	DebugPosMax.y = SPos.y + 2.0f;
	DebugPosMax.z = obs->TerrainHeight(SPos) + 2.0f;
	debug->DebugBoxOut(DebugPosMin, DebugPosMax, sc2::Colors::Red);
	debug->SendDebug();
	
}

void Debugging::ShowExpansionsInformation()
{
	DebugInterface* debug = bot->Debug();

	for (int i = 0; i < bot->ExpansionsVector.size(); i++)
	{
		Expansion exp = bot->ExpansionsVector[i];
		debug->DebugTextOut("X", exp.pos, sc2::Colors::White);
		for (auto& mf : exp.MineralFields)
		{
			debug->DebugTextOut(std::to_string(i), mf->pos, sc2::Colors::Green);
		}
		for (auto& gg : exp.GasGeyser)
		{
			debug->DebugTextOut(std::to_string(i), gg->pos, sc2::Colors::Blue);
		}
	}
	debug->SendDebug();
}

void Debugging::PrintCCPositions()
{
	Units OwnUnits = bot->Observation()->GetUnits(Unit::Alliance::Self);
	std::vector<const Unit*> CCList;

	int CCCount = 0;
	for (const auto u : OwnUnits)
	{
		if (u->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER || u->unit_type == UNIT_TYPEID::TERRAN_ORBITALCOMMAND)
		{
			CCList.push_back(u);
		}
	}

	if (CCList.size() >= 10)
	{
		for (const auto& exp : bot->ExpansionsVector)
		{
			const Unit* NearestCC = FindNearestUnit(exp.pos, CCList);

			std::cout << "Exp Pos at: " << exp.pos.x << " " << exp.pos.y <<"! " << "Nearest CC at: " << NearestCC->pos.x << " " << NearestCC->pos.y << "!" << std::endl;
		}
	}
}

void Debugging::PrintPath()
{
	DebugInterface* debug = bot->Debug();

	Point3D Pos1;
	Point3D Pos2;


	for (const auto& Path : bot->PathFromMain)
	{
		for (int ind = 0; ind < Path.size() - 1; ind++)
		{
			Pos1.x = Path[ind].x;
			Pos1.y = Path[ind].y;
			Pos1.z = bot->Observation()->TerrainHeight(Path[ind]) + 1.0f;
			Pos2.x = Path[ind + 1].x;
			Pos2.y = Path[ind + 1].y;
			Pos2.z = bot->Observation()->TerrainHeight(Path[ind + 1]) + 1.0f;
			debug->DebugLineOut(Pos1, Pos2, sc2::Colors::Purple);
		}
	}

	debug->SendDebug();
}