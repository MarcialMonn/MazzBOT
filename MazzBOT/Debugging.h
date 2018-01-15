#pragma once

#include "sc2api/sc2_api.h"
#include "MazzBOT.h"

#include <string>

class MazzBOT;

class Debugging
{

public:
    // Constructor & Deconstructor
    Debugging();
    ~Debugging();

    MazzBOT *bot; // Zeiger!?
    
    void main();  
    void ShowAllUnitsAbilityID();
    void ShowStartingPosition();
    void ShowExpansionsInformation();
    void PrintPath();

    void PrintCCPositions();
};

