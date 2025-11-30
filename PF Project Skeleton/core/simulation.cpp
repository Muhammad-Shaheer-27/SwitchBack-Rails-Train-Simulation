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
    if (levelSeed != 0)
        srand(levelSeed);
    else
        srand((unsigned int)time(0));
    simulationRunning = 1;
    currentTick = 0;
}

void simulateOneTick() {
    if (!simulationRunning) return;

    spawnTrainsForTick();
    updateSwitchCounters();
    queueSwitchFlips();
    determineAllRoutes();

    // Update signals BEFORE moving – trains will obey these
    updateSignalLights();

    moveAllTrains();
    applyDeferredFlips();
    applyEmergencyHalt();
    updateEmergencyHalt();
    checkArrivals();

    // Update signals AFTER moving – for correct visualization of new positions
    updateSignalLights();

    currentTick++;
}
// ----------------------------------------------------------------------------
// CHECK IF SIMULATION IS COMPLETE
// ----------------------------------------------------------------------------
bool isSimulationComplete() {
    if (currentTick == 0) return false;

    if (num_spawn > 0 && (trainsReached + crashed_trains) >= num_spawn)
        return true;

    if (currentTick > 1000) return true;

    return false;
}