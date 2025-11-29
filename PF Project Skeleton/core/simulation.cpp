#include "simulation.h"
#include "simulation_state.h"
#include "trains.h"
#include "switches.h"
#include "io.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;

// ============================================================================
// SIMULATION.CPP - Implementation of main simulation logic
// ============================================================================

void initializeSimulation() {
    initializeSimulationState();
    if(levelSeed != 0) srand(levelSeed);
    else srand((unsigned int)time(0));
    simulationRunning = 1;
    currentTick = 0;
}

void simulateOneTick() {
    if(!simulationRunning) return;

    spawnTrainsForTick();
    updateSwitchCounters();
    queueSwitchFlips();
    determineAllRoutes();
    moveAllTrains();
    applyDeferredFlips();
    updateSignalLights();
    applyEmergencyHalt();
    updateEmergencyHalt();
    checkArrivals();

    currentTick++;
}

// ----------------------------------------------------------------------------
// FIXED: CHECK IF SIMULATION IS COMPLETE
// ----------------------------------------------------------------------------
bool isSimulationComplete() {
    //Do not end at tick 0
    if(currentTick==0)return false;
    //End if trains reached+crashed= total trains spawned
    if(num_spawn>0&&(trainsReached+crashed_trains)>=num_spawn){
        return true;
    }
    //Prevent infinite simulation
    if(currentTick>1000)return true;

    return false;
}