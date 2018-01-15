#pragma once

#include "sc2api/sc2_api.h"
#include "sc2renderer/sc2_renderer.h"
#include <thread>
#include <iostream>
#include <math.h>

using namespace sc2;

struct Expansion
{
    Point3D pos;
    std::vector<const Unit*> MineralFields;
    std::vector<const Unit*> GasGeyser;
    std::vector<const Unit*> Ressources;
};

struct RendererSetup
{
    int kPixelDrawSize;
    int kDrawSize;
};

std::pair<float, int> FindMinimalValue(std::vector<float> InVector);

std::pair<float, Point2D> FinMinimalValue(std::vector< std::vector<float> > InMatrix);

Point2D Convert3Dto2D(Point3D Pos3D);

Point3D Convert2Dto3D(Point2D Pos2D, std::vector<std::vector<float>> TerrainHeight);

Point3D FindNearestBuildingSpot(Point3D DesiredPosition, UNIT_TYPEID BuildingType, std::vector<std::vector<int>> PathBuildStatus, std::vector<std::vector<float>> TerrainHeight);

bool CheckBuildable(Point2D DesiredSpot, UNIT_TYPEID BuildingType, std::vector<std::vector<int> > PathBuildStatus);

float RootMeanSquare(std::vector<float> numbers);

std::string GetAbilityText(AbilityID ID);

const Unit* FindNearestUnit(Point2D Position, std::vector<const Unit*> UnitList);