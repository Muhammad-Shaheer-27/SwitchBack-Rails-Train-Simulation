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

    // 1. Spawn trains scheduled for this tick
    spawnTrainsForTick();

    // 2. Update switch counters from trains on switches
    updateSwitchCounters();

    // 3. Queue automatic flips (based on counters and K-values)
    queueSwitchFlips();

    // 4. Determine directions for all trains (based on current tiles + switchState)
    determineAllRoutes();

    // 5. Compute signal lights based on *current* positions and directions.
    //    These will be used to decide which trains must stop at red.
    updateSignalLights();

    // 6. Move trains (obeying red signals, weather, safety, collisions)
    moveAllTrains();

    // 7. Apply queued switch flips AFTER movement
    applyDeferredFlips();

    // 8. Emergency halt logic
    applyEmergencyHalt();
    updateEmergencyHalt();

    // 9. Check arrivals
    checkArrivals();

    // 10. Recompute signals for visualization based on NEW positions
    updateSignalLights();

    // 11. Advance tick counter
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