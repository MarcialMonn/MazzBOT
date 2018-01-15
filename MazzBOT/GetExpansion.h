#pragma once;

#include "sc2api/sc2_api.h"
#include "MazzBOT.h"
#include "AuxiliaryFunctions.h"

using namespace sc2;

// Point3D StartingPosition;

class MazzBOT;

class GetExpansions
{
public:
    // Constructor & Deconstructor
    GetExpansions();
    ~GetExpansions();
    MazzBOT *bot;
    std::vector<Expansion> FindExpansions();
private:
    Point3D RefinePosition(Point2D InitPos, Expansion exp);
    float MinDistanceMin = 6;
    float MinDistanceGas = 6.5;
};
//std::vector<Expansion> GetExpansions(Units InUnits, std::vector<std::vector<int>> PathBuildStatus, std::vector<std::vector<float>> TerrainHeight);