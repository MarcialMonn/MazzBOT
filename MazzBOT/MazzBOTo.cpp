// First Version of the MazzBOT SC2 Bot
// Programmed using the official SC2 API by Blizzard Entertainment
// Ver0.0
// Last worked on: 15.12.2017
// Created by: Mazzader

// Include all needed libraries and define the used namespace
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2renderer/sc2_renderer.h"

#include "MazzBOTo.h"
#include "GetExpansion.h"

#include <iostream>
#include <chrono>
#include <thread>


using namespace sc2;

// Rendering some feature layer stuff
const float kCameraWidth = 24.0f;
const int kFeatureLayerSize = 80;
const int kPixelDrawSize = 5;
const int kDrawSize = kFeatureLayerSize * kPixelDrawSize;

void DrawFeatureLayer1BPP(const SC2APIProtocol::ImageData& image_data, int off_x, int off_y) 
{
	assert(image_data.bits_per_pixel() == 1);
	int width = image_data.size().x();
	int height = image_data.size().y();
	sc2::renderer::Matrix1BPP(image_data.data().c_str(), width, height, off_x, off_y, kPixelDrawSize, kPixelDrawSize);
}

void DrawFeatureLayerUnits8BPP(const SC2APIProtocol::ImageData& image_data, int off_x, int off_y) 
{
	assert(image_data.bits_per_pixel() == 8);
	int width = image_data.size().x();
	int height = image_data.size().y();
	sc2::renderer::Matrix8BPPPlayers(image_data.data().c_str(), width, height, off_x, off_y, kPixelDrawSize, kPixelDrawSize);
}

void DrawFeatureLayerHeightMap8BPP(const SC2APIProtocol::ImageData& image_data, int off_x, int off_y) 
{
	assert(image_data.bits_per_pixel() == 8);
	int width = image_data.size().x();
	int height = image_data.size().y();
	sc2::renderer::Matrix8BPPHeightMap(image_data.data().c_str(), width, height, off_x, off_y, kPixelDrawSize, kPixelDrawSize);
}

void DrawBuildPathMap(const std::vector<std::vector<int>> Matrix, int off_x, int off_y)
{
	int width = Matrix.size();
	int height = Matrix[0].size();
	sc2::renderer::MatrixBuildPathMap(Matrix, width, height, off_x, off_y, 3*kPixelDrawSize, 3*kPixelDrawSize);
}

// Main body of the bot
class MazzBOTo : public Agent
{
public:
	// Routine done on start of the game
	virtual void OnGameStart() final
	{
		std::cout << "Entered the OnGameStart Routine!" << std::endl;

		// Get some game information, preallocate some information containers, as well as run the first calculation
		const GameInfo& Info = Observation()->GetGameInfo();
		Map_Width = Info.width;
		Map_Height = Info.height;
		std::cout << "Got the game information handle and extracted some basic game information!" << std::endl;

		// Get the Pathable/Buildable Matrix
		PathBuildStatus.resize(Map_Width);
		TerrainHeight.resize(Map_Width);
		for (int i = 0; i < PathBuildStatus.size(); i++)
		{
			PathBuildStatus[i].resize(Map_Height);
			TerrainHeight[i].resize(Map_Height);
		}
		PathBuildUpdate();
		std::cout << "Updated the Pathable/Buildable Matrix" << std::endl;

		Units UnitList = Observation()->GetUnits();

		//ExpansionsArray = GetExpansions(UnitList, PathBuildStatus, TerrainHeight);
		std::cout << "GetExpansions Routine was exited correctly!" << std::endl;

		SPos = ExpansionsArray[0].pos;

		// Calculate the position of all expansions and get their according Mineral Fields
		std::cout << "Calculated all the expansions position and their Mineral Fields!" << std::endl;

		sc2::renderer::Initialize("Feature layers", 50, 50, 2 * kDrawSize, 2 * kDrawSize); 
	}

	// Routine entered on each game step
	virtual void OnStep() final
	{
		PathBuildUpdate();

		const SC2APIProtocol::Observation* observation = Observation()->GetRawObservation();

		const SC2APIProtocol::FeatureLayers& m = observation->feature_layer_data().renders();
		DrawFeatureLayerUnits8BPP(m.unit_density(), 0, 0);
		DrawFeatureLayer1BPP(m.selected(), kDrawSize, 0);

		
		const SC2APIProtocol::FeatureLayersMinimap& mi = observation->feature_layer_data().minimap_renders();
		DrawFeatureLayerHeightMap8BPP(mi.height_map(), 0, kDrawSize);
		/*
		DrawFeatureLayer1BPP(mi.camera(), kDrawSize, kDrawSize);
		*/
		Point2D CameraPos = Observation()->GetCameraPos();

		ScreenData = GetScreenData(PathBuildStatus, CameraPos);

		DrawBuildPathMap(ScreenData, kDrawSize, kDrawSize);

		sc2::renderer::Render();
		
		BuildSCV = true;
		TryBuildSupplyDepot();

		// Very fast, short thingie implemented to check if Expansions Position is found at correct spot!
		Point2D NatPos;
		float Distance = std::numeric_limits<float>::max();
		for (auto& e : ExpansionsArray)
		{
			if (DistanceSquared3D(e.pos, ExpansionsArray[0].pos) > 5 || DistanceSquared3D(e.pos, ExpansionsArray[0].pos) <= Distance)
			{
				NatPos = Convert3Dto2D(e.pos);
				Distance = DistanceSquared3D(e.pos, ExpansionsArray[0].pos);
			}
		}
		
		if (CountUnitType(UNIT_TYPEID::TERRAN_SCV) >= 20 || CountUnitType(UNIT_TYPEID::TERRAN_COMMANDCENTER) < 2)
		{
			const Unit* BuildingUnit = FindNearestMiningSCV(NatPos);
			TryBuildStructure(ABILITY_ID::BUILD_COMMANDCENTER, NatPos, BuildingUnit);
			//TryBuildStructure();
		}
		
		// Debugging
		Debugging();
	}

	virtual void OnGameEnd() final 
	{
		sc2::renderer::Shutdown();
	}

	virtual void OnUnitIdle(const Unit* unit) final
	{
		// Checks all Idle Units, and dependent on the unit type, decides what to do
		switch (unit->unit_type.ToType())
		{
		case UNIT_TYPEID::TERRAN_COMMANDCENTER:
		{
			// Trains SCV from Command Centers, if more SCVs are desired (set by BuildSCV bool)
			if (BuildSCV)
			{
				Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_SCV);
			}
		}
		case UNIT_TYPEID::TERRAN_SCV:
		{
			// If there are idle SCVs, find the nearest Mineral Patch, and sent the SCV to mine
			const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
			if (!mineral_target)
			{
				break;
			}
			Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
		}
		}
	}

private:
	bool TryBuildSupplyDepot()
	{
		const ObservationInterface* observation = Observation();

		// If we are not supply capped, don't build any supply depots.
		if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2)
		{
			return false;
		}
		else
		{
			// If soon supply capped, find a supply building position, the nearest minig scv, and send the SCV to build the supply depot
			const Point2D TargetBuildPosition = TargetSupplyDepotPosition();
			const Unit* BuildingUnit = FindNearestMiningSCV(TargetBuildPosition);
			return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT, TargetBuildPosition, BuildingUnit);
		}
	}

	const Point2D TargetSupplyDepotPosition()
	{
		// Finds a suitable position to build a supply depot
		Point2D TargetPosition;
		TargetPosition.x = GetRandomScalar()*50.0f + ExpansionsArray[0].pos.x;
		TargetPosition.y = GetRandomScalar()*50.0f + ExpansionsArray[0].pos.y;
		return TargetPosition;
	}

	const Unit* FindNearestMiningSCV(const Point2D TargetPosition)
	{
		// Finds the nearest mining SCV to a target position.
		// Iterates through all the units, and checks the distance to the target positions. If it is closed than the previous found closest scv, sets this scv to return
		float OldDistance = std::numeric_limits<float>::max();
		Units units = Observation()->GetUnits(Unit::Alliance::Self);
		const Unit* target = nullptr;
		for (const auto& u : units)
		{
			for (const auto& order : u->orders)
			{
				if (order.ability_id == ABILITY_ID::HARVEST_GATHER)
				{
					float NewDistance = DistanceSquared2D(u->pos, TargetPosition);
					if (NewDistance <= OldDistance)
					{
						target = u;
						OldDistance = NewDistance;
					}
				}
			}
		}
		return target;
	}

	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, const Point2D TargetBuildPosition, const Unit* BuildingUnit)
	{
		// Tries to build the structure definied in ABILITY_ID by the unit BuildingUnit at position TargetBuildPositiona
		Actions()->UnitCommand(BuildingUnit, ability_type_for_structure, TargetBuildPosition);
		return true;
	}

	const Unit* FindNearestMineralPatch(const Point2D& start)
	{
		// Finds the nearest Mineral Patch to a given position

		// Get all the neutral units, iterate through all of them and check which one is the nearest to the given position
		Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
		float distance = std::numeric_limits<float>::max();
		const Unit* target = nullptr;
		for (const auto& u : units)
		{
			if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD)
			{
				float d = DistanceSquared2D(u->pos, start);
				if (d < distance)
				{
					distance = d;
					target = u;
				}
			}
		}
		return target;
	}

	void PathBuildUpdate()
	{
		// Update the Status of the terrain, regarding placeability and pathability
		// 0: Neither placeable nor pathable
		// 1: Placable
		// 2: Pathable
		// 3: Placeable and Pathable

		Point2D UpdatePoint; // Allocate the point at which the status is momentarily been updated

		// Iterate through map width and map height
		for (int i = 0; i < Map_Width; i++)
		{
			UpdatePoint.x = i;
			for (int j = 0; j < Map_Height; j++)
			{
				UpdatePoint.y = j;
				// Update the status
				PathBuildStatus[i][j] = Observation()->IsPlacable(UpdatePoint) + 2 * Observation()->IsPathable(UpdatePoint);
				TerrainHeight[i][j] = Observation()->TerrainHeight(UpdatePoint);
			}
		}
	}

	// Some auxiliary functions
	size_t CountUnitType(UNIT_TYPEID unit_type) {
		return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
	}

	static std::string GetAbilityText(sc2::AbilityID ability_id)
	{
		std::string str;
		str += sc2::AbilityTypeToName(ability_id);
		str += " (";
		str += std::to_string(uint32_t(ability_id));
		str += ")";
		return str;
	}

	static std::string GetPositionText(Point3D Position)
	{
		std::string str;
		str += "X: ";
		str += std::to_string(Position.x);
		str += "Y: ";
		str += std::to_string(Position.y);
		return str;
	}

	std::vector< std::vector<int> > GetScreenData(std::vector< std::vector<int> > GlobalData, Point2D CameraLocation)
	{
		std::vector< std::vector<int> > ScreenData;
		ScreenData.resize(kCameraWidth);

		for (int i = 0; i < kCameraWidth; i++)
		{
			ScreenData[i].resize(kCameraWidth);
			for (int j = 0; j < kCameraWidth; j++)
			{
				ScreenData[i][j] = GlobalData[CameraLocation.x-0.5*kCameraWidth+i][CameraLocation.y-0.5*kCameraWidth+j];
			}
		}
		return ScreenData;
	}

	// Debugging part:
	void Debugging()
	{
		ShowAllUnitsAbilityID();
		ShowStartingPosition();
		ShowExpansionsInformation();
		debug->SendDebug();
	}

	void ShowAllUnitsAbilityID()
	{
		Point3D UnitPos;
		Units OwnUnits = Observation()->GetUnits(Unit::Alliance::Self);

		for (const auto& unit : OwnUnits)
		{
			for (const auto& order : unit->orders)
			{
				debug -> DebugTextOut(GetAbilityText(order.ability_id), unit->pos, sc2::Colors::Green);
			}
		}
	}

	void ShowStartingPosition()
	{
		Point3D DebugPosMin, DebugPosMax;
		DebugPosMin = SPos;
		DebugPosMin.x -= 1;
		DebugPosMin.y -= 1;
		DebugPosMin.z -= 1;
		DebugPosMax = SPos;
		DebugPosMax.x += 1;
		DebugPosMax.y += 1;
		DebugPosMax.z += 1;
		debug->DebugBoxOut(DebugPosMin, DebugPosMax, sc2::Colors::Red);
	}

	void ShowExpansionsInformation()
	{
		Units NeutralUnits = Observation()->GetUnits(Unit::Alliance::Neutral);
		for (auto& exp : ExpansionsArray)
		{
			debug->DebugTextOut("X", exp.pos, sc2::Colors::Red);
			for (auto& mf : exp.MineralFields)
			{
				debug->DebugTextOut("MF HERE", mf->pos, sc2::Colors::Green);
			}
		}
	}

public:
	//const GameInfo& Info;	// Contains information of the game, extracted on game start
	int Map_Width;			// Width of the map
	int Map_Height;			// Height of the map
	std::vector< std::vector<int> > PathBuildStatus;
	std::vector< std::vector<float> > TerrainHeight;
	std::vector< std::vector<int> > ScreenData;
	Point3D SPos;
	std::vector<Expansion> ExpansionsArray;

	// Some command booleans
	bool BuildSCV; // Check if it is desired to build more SCVs

	// Debug Interface
	DebugInterface* debug = Debug();
};

/*
int main(int argc, char* argv[])
{
	Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);

	FeatureLayerSettings settings(kCameraWidth, kFeatureLayerSize, kFeatureLayerSize, kFeatureLayerSize, kFeatureLayerSize);
	coordinator.SetFeatureLayers(settings);

	MazzBOT bot;
	coordinator.SetParticipants(
	{
		CreateParticipant(Race::Terran, &bot),
		CreateComputer(Race::Zerg)
	});

	coordinator.LaunchStarcraft();
	coordinator.StartGame(sc2::kMapBelShirVestigeLE);

	while (coordinator.Update())
	{
	}

	return 0;
}
*/