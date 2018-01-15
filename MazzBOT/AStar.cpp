#include "AStar.h"

AStar::AStar()
{
	bot = nullptr;
}

AStar::~AStar()
{
}


std::vector<Point2D> AStar::SinglePath(Point2D Start, Point2D Goal)
{
	// Define some containers to work with:

	int mWidth = bot->Observation()->GetGameInfo().width;
	int mHeight = bot->Observation()->GetGameInfo().height;
	
	// Add the start node to the known nodes
	openSet.push_back(Start);

	// Resize the gScore and fScore matrices and set default value to infinite
	gScore.resize(mWidth);
	fScore.resize(mWidth);
	cameFrom.resize(mWidth);

	for (int w_ind = 0; w_ind < gScore.size(); w_ind++)
	{
		gScore[w_ind].resize(mHeight, inf);
		fScore[w_ind].resize(mHeight, inf);
		cameFrom[w_ind].resize(mHeight);
	}

	// The cost of getting from the start note to itself is 0
	gScore[Start.x][Start.y] = 0;

	// Set the heuristic start cost
	fScore[Start.x][Start.y] = HeuristicCostEstimate(Start, Goal);

	RendererSetup Setup;
	Setup.kPixelDrawSize = 5;
	Setup.kDrawSize = 400;
	sc2::renderer::Initialize("Feature layers", 50, 50, Setup.kDrawSize, Setup.kDrawSize);
	renderer::DrawHeatMap8BPP(gScore, 0, 0, Setup);


	// There are still nodes to be evaluated
	while (openSet.size() >= 1)
	{
		std::cout << "OpenSet.size(): " << openSet.size() << std::endl;
		// Current node is the node in the openSet with the lowest fScore Value
		Point2D Current = FindLowestFScoreNode();
		if (Current == Goal)
		{
			return ReconstructPath(Start, Goal);
			std::cout << "Reconstructed path!" << std::endl;
		}
		// Remove the current member from the openSet
		RemoveMember(Current);
		std::cout << "Removed Member!" << std::endl;
		closedSet.push_back(Current);
		std::cout << "Closed Set" << std::endl;
		
		// Expand the node
		ExpandNode(Current, Goal);
		std::cout << "Expanded note!" << std::endl;
	}
	

}

float AStar::HeuristicCostEstimate(Point2D Start, Point2D Goal)
{
	return Distance2D(Start, Goal);
}

Point2D AStar::FindLowestFScoreNode()
{
	// Get a handler for the actual best Node, and the Score of this node
	Point2D ReturnPoint;
	float MinScore = std::numeric_limits<float>::max();
	
	// Iterate through all nodes, and compare their cost with the actual best Node
	for (int ind = 0; ind < openSet.size(); ind++)
	{
		if (fScore[openSet[ind].x][openSet[ind].y] <= MinScore)
		{
			MinScore = fScore[openSet[ind].x][openSet[ind].y];
			ReturnPoint = openSet[ind];
		}
	}
	return ReturnPoint;
}

void AStar::RemoveMember(Point2D Member)
{
	for (int ind = 0; ind < openSet.size(); ind++)
	{
		if (openSet[ind] == Member)
		{
			openSet.erase(openSet.begin() + ind);
		}
	}
}

void AStar::ExpandNode(Point2D Current, Point2D Goal)
{
	// Check all neighbours of the current
	Point2D Neighbour;
	for (int x_iter = -1; x_iter <= 1; x_iter++)
	{
		Neighbour.x = Current.x + x_iter;
		if (Neighbour.x >= 0 && Neighbour.x <= cameFrom.size())
		{
			for (int y_iter = -1; y_iter <= 1; y_iter++)
			{
				Neighbour.y = Current.y + y_iter;
				// Check that x_iter or y_iter isn't 0 (which would be the current Point), and that the coordinate is on the map
				if ((x_iter != 0 || y_iter != 0) && Neighbour.y >= 0 && Neighbour.y <= cameFrom[Neighbour.x].size())
				{
					// Check if already in closedSet

					if (isNotClosedSetMember(Neighbour))
					{
						if (isNotOpenSetMember(Neighbour))
						{
							openSet.push_back(Neighbour);
						}
						float tentative_gScore = std::max(gScore[Current.x][Current.y] + MovingTo(Current, Neighbour), inf);
						if (tentative_gScore <= gScore[Neighbour.x][Neighbour.y])
						{
							cameFrom[Neighbour.x][Neighbour.y] = Current;
							gScore[Neighbour.x][Neighbour.y] = tentative_gScore;
							fScore[Neighbour.x][Neighbour.y] + HeuristicCostEstimate(Neighbour, Goal);
						}
					}
				}
			}
		}
	}
}

bool AStar::isNotClosedSetMember(Point2D Member)
{
	bool bReturn = true;
	
	for (auto& m : closedSet)
	{
		if (m == Member)
		{
			bReturn = false;
		}
	}
	
	return bReturn;
}

bool AStar::isNotOpenSetMember(Point2D Member)
{
	bool bReturn = true;

	for (auto& m : openSet)
	{
		if (m == Member)
		{
			bReturn = false;
		}
	}

	return bReturn;
}

float AStar::MovingTo(Point2D Current, Point2D Neighbour)
{
	if (bot->Observation()->IsPathable(Neighbour))
	{
		return Distance2D(Current, Neighbour);
	}
	else
	{
		return std::numeric_limits<float>::max();
	}
}

std::vector< Point2D > AStar::ReconstructPath(Point2D Start, Point2D Goal)
{
	std::cout << "In the Reconstruct Path Routine!" << std::endl;
	std::vector<Point2D> Path;
	Point2D Current = Goal;
	while (Current != Start)
	{
		Path.push_back(Current);
		Current = cameFrom[Current.x][Current.y];
	}

	std::reverse(Path.begin(), Path.end());

	return Path;
}