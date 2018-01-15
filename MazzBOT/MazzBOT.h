#pragma once

#include "sc2api/sc2_api.h"
#include "sc2renderer/sc2_renderer.h"

#include "GetExpansion.h"
#include "Debugging.h"
#include "AuxiliaryFunctions.h"
#include "AStar.h"


class MazzBOT : public sc2::Agent
{
private:

    // Variables to be defined:
    float kCameraWidth = 24.0f;

    int Map_Width;
    int Map_Height;
    std::vector< std::vector<int> >     PathBuildStatus;
    std::vector< std::vector<float> >   TerrainHeight;
    std::vector< std::vector<int> >     ScreenData;
    std::vector< std::vector<int> >     GetScreenData(std::vector< std::vector<int> > GlobalData, Point2D CameraLocation);

    void OnGameStart() override;
    void OnStep() override;
    void OnGameEnd() override;

    void FeatureRenderer();
    void PathBuildStatusUpdate();

public:
    Point2D GetOwnStartPosition();
    std::vector<Expansion>              ExpansionsVector;
    std::vector< std::vector< Point2D> > PathFromMain;
};