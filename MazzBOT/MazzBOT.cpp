#include "MazzBOT.h"

#include <iostream>
#include <vector>

const float kCameraWidth = 24.0f;

void MazzBOT::OnGameStart()
{
	std::cout << "Entered the OnGameStart Routine!" << std::endl;

	// Setup some objects to be used later
	GetExpansions GE;
	GE.bot = this;

	AStar PathFind;
	PathFind.bot = this;


	// Get some game information, preallocate some information containers, as well as run the first calculation
	const sc2::GameInfo& Info = Observation()->GetGameInfo();
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
	PathBuildStatusUpdate();
	std::cout << "Updated the Pathable/Buildable Matrix" << std::endl;

	sc2::Units UnitList = Observation()->GetUnits();	

	ExpansionsVector = GE.FindExpansions();
	std::cout << "GetExpansion Routine was exited!" << std::endl;


	for (int ind = 1; ind < ExpansionsVector.size(); ind++)
	{
		PathFromMain.push_back(PathFind.SinglePath(ExpansionsVector[0].pos, ExpansionsVector[ind].pos));
		std::cout << "Found pathing!" << std::endl;
	}


}

void MazzBOT::OnStep()
{
	// Update the PathBuildStatus Matrix
	PathBuildStatusUpdate();

	// 
	FeatureRenderer();

	//DebugInfo DInfo;
	const sc2::GameInfo& Info = Observation()->GetGameInfo();
	
	Debugging db;
	db.bot = this;
	db.ShowAllUnitsAbilityID();
	db.ShowStartingPosition();
	db.ShowExpansionsInformation();
	db.PrintCCPositions();
	db.PrintPath();

}

void MazzBOT::OnGameEnd()
{
	sc2::renderer::Shutdown();
}

void MazzBOT::FeatureRenderer()
{
	// Get all the needed information for the renderer
	const SC2APIProtocol::Observation* observation = Observation()->GetRawObservation();
	const SC2APIProtocol::FeatureLayers& m = observation->feature_layer_data().renders();
	const SC2APIProtocol::FeatureLayersMinimap& mi = observation->feature_layer_data().minimap_renders();
	const SC2APIProtocol::ImageData ID;

	//DrawFeatureLayerUnits8BPP(m.unit_density(), 0, 0);
	//DrawFeatureLayer1BPP(m.selected(), kDrawSize, 0);
	//DrawFeatureLayerHeightMap8BPP(mi.height_map(), 0, kDrawSize);

	Point2D CameraPos = Observation()->GetCameraPos();
	ScreenData = GetScreenData(PathBuildStatus, CameraPos);

}

void MazzBOT::PathBuildStatusUpdate()
{
	// Update the Status of the terrain, regarding placeability and pathability
	// 0: Neither placeable nor pathable
	// 1: Placable
	// 2: Pathable
	// 3: Placeable and Pathable

	// Furthermore gets the Height of the Terrain at the specific point

	sc2::Point2D UpdatePoint; // Allocate the point at which the status is momentariliy been updated

	// Iterate through map width and map height
	for (int i = 0; i < Map_Width; i++)
	{
		UpdatePoint.x = i;
		for (int j = 0; j < Map_Height; j++)
		{
			UpdatePoint.y = j;
			PathBuildStatus[i][j] = Observation()->IsPlacable(UpdatePoint) + 2 * Observation()->IsPathable(UpdatePoint);
			TerrainHeight[i][j] = Observation()->TerrainHeight(UpdatePoint);
		}
	}
}

std::vector< std::vector<int> > MazzBOT::GetScreenData(std::vector< std::vector<int> > GlobalData, Point2D CameraLocation)
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

Point2D MazzBOT::GetOwnStartPosition()
{
	float distance = std::numeric_limits<float>::max();
	Point2D Position;
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units)
	{
		if (unit->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER)
		{
			for (const auto& spos : Observation()->GetGameInfo().start_locations)
			{
				if (DistanceSquared2D(spos, unit->pos) <= distance)
				{
					Position = unit->pos;
					distance = DistanceSquared2D(spos, unit->pos);
				}
			}
		}
	}
	return Position;
}