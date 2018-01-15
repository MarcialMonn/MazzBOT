#pragma once

#include "sc2api/sc2_api.h"
#include "MazzBOT.h"
#include "sc2renderer/sc2_renderer.h"

#include <vector>

using namespace sc2;

class MazzBOT;

class AStar {
public:
    AStar();
    ~AStar();
    MazzBOT* bot;
    std::vector<Point2D> SinglePath(Point2D Start, Point2D Goal);

private:
    // Define infitine as the maximal float value possible, and define map width and height
    float inf = std::numeric_limits<float>::max();

    float HeuristicCostEstimate(Point2D Start, Point2D Goal);
    Point2D FindLowestFScoreNode();
    void RemoveMember(Point2D Member);
    void ExpandNode(Point2D Current, Point2D Goal);
    bool isNotClosedSetMember(Point2D Member);
    bool isNotOpenSetMember(Point2D Member);
    float MovingTo(Point2D Current, Point2D Neighbour);
    std::vector< Point2D > ReconstructPath(Point2D Start, Point2D Goal);

    // For each node, the cost of getting from the start to that node. Initiate as infinite (max possible value)
    std::vector < std::vector<float> > gScore;

    // For each node, the cost of getting from the start node to the goal node by passing by that node
    // That value is partly known and partly heuristic
    std::vector< std::vector<float> > fScore;

    // For each node, which node it can most efficiently be reached from.
    // If a node can be reached from many nodes, cameFrom will eventually contain the most efficiten previous step
    std::vector< std::vector<Point2D> > cameFrom;

    // Set of currently discovered nodes that are not evalauted yet.
    // Initially, only the start node is known
    std::vector<Point2D> openSet;

    // Set of nodes already evaluated
    std::vector<Point2D> closedSet;
};