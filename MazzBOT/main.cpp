#include "MazzBOT.h"

#include "sc2api/sc2_api.h"
#include "sc2utils/sc2_manage_process.h"

using namespace sc2;

int main(int argc, char* argv[])
{
	Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);

	const float kCameraWidth = 24.0f;
	const int kFeatureLayerSize = 80;
	const int kPixelDrawSize = 5;
	const int kDrawSize = kFeatureLayerSize * kPixelDrawSize;

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