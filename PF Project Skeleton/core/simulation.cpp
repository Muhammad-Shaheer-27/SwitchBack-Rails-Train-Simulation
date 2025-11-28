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
    // FIX 1: Never finish at the very start (Tick 0)
    if (currentTick == 0) return false;

    // FIX 2: Only finish if (Reached + Crashed) equals TOTAL scheduled trains (numSpawn)
    // If numSpawn is 0 (empty level), we finish immediately.
    if (numSpawn > 0 && (trainsReached + trainsCrashed) >= numSpawn) {
        return true;
    }

    // Safety: Stop after 5000 ticks to prevent infinite loops
    if (currentTick > 5000) return true;

    return false;
}