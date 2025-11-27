#include "simulation.h"
#include "simulation_state.h"
#include "trains.h"
#include "switches.h"
#include "io.h"
#include <cstdlib>
#include <ctime>

// ============================================================================
// SIMULATION.CPP - Implementation of main simulation logic
// ============================================================================

// ----------------------------------------------------------------------------
// INITIALIZE SIMULATION
// ----------------------------------------------------------------------------

void initializeSimulation() {
    //Reset all variables
    initializeSimulationState();
    //Random seed generation
    if(levelSeed!=0){
        srand(levelSeed);
    }
    else{
        srand((unsigned int)time(0));
    }
    //Begin simulation
    simulationRunning=1;
    currentTick=0;
}

// ----------------------------------------------------------------------------
// SIMULATE ONE TICK
// ----------------------------------------------------------------------------

void simulateOneTick() {
    if(!simulationRunning) return;
    //Spawn trains for current tick
    spawnTrainsForTick();
    //Update switch counters and determine flips
    updateSwitchCounters();
    queueSwitchFlips();
    //Determine next direction for all trains
    determineAllRoutes();
    //Move trains
    moveAllTrains();
    //Implement deferred switch flips
    applyDeferredFlips();
    //Signal lights
    updateSignalLights();
    //Emergency halt and update timers
    applyEmergencyHalt();
    updateEmergencyHalt();
    //Check trains reached destination
    checkArrivals();
    //Increase simulation tick
    currentTick++;
}

// ----------------------------------------------------------------------------
// CHECK IF SIMULATION IS COMPLETE
// ----------------------------------------------------------------------------

bool isSimulationComplete() {
    for(int i=0;i<numTrains;i++){   //Check all trains
        if(trainRow[i]!=-1){
            return false;   //Atleast one train running
        }
    }
    return true;    //If all trains inactive then simulation complete
}
