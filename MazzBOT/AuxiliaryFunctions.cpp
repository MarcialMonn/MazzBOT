#include "AuxiliaryFunctions.h"
#include <numeric>

using namespace sc2;

std::pair<float, int> FindMinimalValue(std::vector<float> InVector)
{
	float minValue = std::numeric_limits<float>::max();
	int minIndex;
	// Change for it to run with whatever values are in the vector
	for (int i = 0; i < InVector.size(); i++)
	{
		if (InVector[i] < minValue)
		{
			minValue = InVector[i];
			minIndex = i;
		}
	}
	return std::make_pair(minValue, minIndex);
}

std::pair<float, Point2D> FinMinimalValue(std::vector< std::vector<float> > InMatrix)
{
	float minValue = std::numeric_limits<float>::max();
	Point2D minPosition;

	for (int x = 0; x < InMatrix.size(); x++)
	{
		for (int y = 0; y < InMatrix.size(); y++)
		{
			if (InMatrix[x][y] <= minValue)
			{
				minValue = InMatrix[x][y];
				minPosition.x = x;
				minPosition.y = y;
			}
		}
	}
	return std::make_pair(minValue, minPosition);

}

Point2D Convert3Dto2D(Point3D Pos3D)
{
	Point2D OutPos;
	OutPos.x = Pos3D.x;
	OutPos.y = Pos3D.y;
	return OutPos;
}

Point3D Convert2Dto3D(Point2D Pos2D, std::vector<std::vector<float>> TerrainHeight)
{
	Point3D OutPos;
	OutPos.x = Pos2D.x;
	OutPos.y = Pos2D.y;
	OutPos.z = TerrainHeight[Pos2D.x][Pos2D.y];
	return OutPos;
}

Point3D FindNearestBuildingSpot(Point3D DesiredPosition, UNIT_TYPEID BuildingType, std::vector<std::vector<int>> PathBuildStatus, std::vector<std::vector<float>> TerrainHeight)
{
	Point2D CheckPosition;
	CheckPosition.x = DesiredPosition.x;
	CheckPosition.y = DesiredPosition.y;
	std::vector<Point2D> PositionVector;
	std::vector<float> DistanceVector;

	if (!CheckBuildable(CheckPosition, BuildingType, PathBuildStatus))
	{
		return DesiredPosition;
	}
	else
	{
		bool Found = false;
		int SweepDistance = 1;
		while (!Found)
		{
			for (int xsweep = -SweepDistance; xsweep <= SweepDistance; xsweep++)
			{
				for (int ysweep = -SweepDistance; ysweep <= SweepDistance; ysweep++)
				{
					if (std::abs(xsweep) == SweepDistance || std::abs(ysweep) == SweepDistance)
					{
						CheckPosition.x = DesiredPosition.x + xsweep;
						CheckPosition.y = DesiredPosition.y + ysweep;
						if (CheckBuildable(CheckPosition, BuildingType, PathBuildStatus))
						{
							PositionVector.push_back(CheckPosition);
							DistanceVector.push_back(DistanceSquared3D(Convert2Dto3D(CheckPosition, TerrainHeight), DesiredPosition));
							Found = true;
						}
					}
				}
			}
			SweepDistance++;
		}

		std::pair<float, int> MinimalValues = FindMinimalValue(DistanceVector);
		return Convert2Dto3D(PositionVector[MinimalValues.second], TerrainHeight);
	}
}

bool CheckBuildable(Point2D DesiredSpot, UNIT_TYPEID BuildingType, std::vector<std::vector<int> > PathBuildStatus)
{
	int BuildCount = 0;
	int CheckCount = 0;

	// Add an algorithm to dynamically adapt to building space needed
	// Furthermore should be able to detect changes in pathing (e.g. completely closed space)
	if (BuildingType == UNIT_TYPEID::TERRAN_COMMANDCENTER)
	{
		for (int i = -2; i <= 2; i++)
		{
			for (int j = -2; j <= 2; j++)
			{
				CheckCount++;
				if (PathBuildStatus[DesiredSpot.x + i][DesiredSpot.y + j] == 1 || PathBuildStatus[DesiredSpot.x + i][DesiredSpot.y + j] == 3)
				{
					BuildCount++;
				}
			}
		}
	}
	if (BuildCount == CheckCount)
	{
		return true;
	}
	else if (BuildCount < CheckCount)
	{
		return false;
	}
	else
	{
		// Throw an error?!?
		std::cout << "Something did really not go as planned in the CheckBuildable algorithm" << std::endl;
	}
}

float RootMeanSquare(std::vector<float> numbers)
{
	float RMS = sqrt((std::inner_product(numbers.begin(), numbers.end(), numbers.begin(), 0) / static_cast<float>(numbers.size())));
	return RMS;
}

std::string GetAbilityText(AbilityID ID)
{
	std::string str;
	str += sc2::AbilityTypeToName(ID);
	str += " (";
	str += std::to_string(uint32_t(ID));
	str += ")";
	return str;
}

const Unit* FindNearestUnit(Point2D Position, std::vector<const Unit*> UnitList)
{
	float Distance = std::numeric_limits<float>::max();
	const Unit* ReturnUnit;

	for (const auto& u : UnitList)
	{
		if (Distance2D(Position, u->pos))
		{
			Distance = Distance2D(Position, u->pos);
			ReturnUnit = u;
		}
	}

	return ReturnUnit;
}

void DrawFeatureLayer1BPP(const SC2APIProtocol::ImageData& image_data, int off_x, int off_y, RendererSetup Setup)
{
	assert(image_data.bits_per_pixel() == 1);
	int width = image_data.size().x();
	int height = image_data.size().y();
	Matrix1BPP(image_data.data().c_str(), width, height, off_x, off_y, Setup.kPixelDrawSize, Setup.kPixelDrawSize);
}

void DrawFeatureLayerUnits8BPP(const SC2APIProtocol::ImageData& image_data, int off_x, int off_y, RendererSetup Setup)
{
	assert(image_data.bits_per_pixel() == 8);
	int width = image_data.size().x();
	int height = image_data.size().y();
	Matrix8BPPPlayers(image_data.data().c_str(), width, height, off_x, off_y, Setup.kPixelDrawSize, Setup.kPixelDrawSize);
}

void DrawFeatureLayerHeightMap8BPP(const SC2APIProtocol::ImageData& image_data, int off_x, int off_y, RendererSetup Setup)
{
	assert(image_data.bits_per_pixel() == 8);
	int width = image_data.size().x();
	int height = image_data.size().y();
	Matrix8BPPHeightMap(image_data.data().c_str(), width, height, off_x, off_y, Setup.kPixelDrawSize, Setup.kPixelDrawSize);
}

void DrawBuildPathMap(const std::vector<std::vector<int>> Matrix, int off_x, int off_y, RendererSetup Setup)
{
	int width = Matrix.size();
	int height = Matrix[0].size();
	MatrixBuildPathMap(Matrix, width, height, off_x, off_y, 3 * Setup.kPixelDrawSize, 3 * Setup.kPixelDrawSize);
}

void DrawHeatMap8BPP(std::vector< std::vector<float> > Matrix, int off_x, int off_y, RendererSetup Setup)
{
	int width = Matrix.size();
	int height = Matrix[0].size();
	Matrix8BPPHeatMap(Matrix, width, height, off_x, off_y, Setup.kPixelDrawSize, Setup.kPixelDrawSize);

}