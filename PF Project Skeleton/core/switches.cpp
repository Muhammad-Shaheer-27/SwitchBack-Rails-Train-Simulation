#include "switches.h"
#include "simulation_state.h"
#include "grid.h"
#include "io.h"
#include <iostream>

using namespace std;

// ============================================================================
// SWITCHES.CPP - Switch management
// ============================================================================

// ----------------------------------------------------------------------------
// UPDATE SWITCH COUNTERS
// ----------------------------------------------------------------------------
// Increment counters for trains entering switches.
// ----------------------------------------------------------------------------
void updateSwitchCounters() {
    // Loop through all active trains
    for (int i = 0; i < numOf_trains; i++) {
        // Skip if train is not on the map
        if (trainRow[i] == -1) continue;

        // Get the tile the train is currently standing on
        char tile = grid[trainRow[i]][trainColumn[i]];

        // Check if the tile is a switch (A-Z)
        if (tile >= 'A' && tile <= 'Z') {
            int swID = tile - 'A'; // Convert 'A'->0, 'B'->1, etc.
            int dir = trainDirection[i];

            // Update counter based on the switch mode
            if (switchMode[swID] == GLOBAL) {
                // Global mode: increment the 0-index counter
                switchCounter[swID][0]++;
            } else {
                // Per-Direction mode: increment the counter for this specific direction
                switchCounter[swID][dir]++;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// QUEUE SWITCH FLIPS
// ----------------------------------------------------------------------------
// Queue flips when counters hit K.
// ----------------------------------------------------------------------------
void queueSwitchFlips() {
    for (int i = 0; i < numSwitches; i++) {
        // Check all 4 directions (Up, Right, Down, Left)
        for (int dir = 0; dir < 4; dir++) {
            
            // If Global mode, we only care about index 0
            if (switchMode[i] == GLOBAL && dir > 0) continue;

            // If counter has reached the K-value limit
            if (switchCounter[i][dir] >= switchK[i][dir]) {
                
                // Mark the switch to flip later (Deferred Flip)
                switchFlipped[i] = 1;
                
                // Reset the counter immediately so it can start counting again
                switchCounter[i][dir] = 0;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// APPLY DEFERRED FLIPS
// ----------------------------------------------------------------------------
// Apply queued flips after movement.
// ----------------------------------------------------------------------------
void applyDeferredFlips() {
    for (int i = 0; i < numSwitches; i++) {
        // If the switch was marked to flip in the queue step
        if (switchFlipped[i] == 1) {
            
            // Toggle state: 0 becomes 1, 1 becomes 0
            switchState[i] = !switchState[i];
            
            // Reset the flip flag
            switchFlipped[i] = 0;
        }
    }
}

// ----------------------------------------------------------------------------
// UPDATE SIGNAL LIGHTS
// ----------------------------------------------------------------------------
// Update signal colors for switches.
// ----------------------------------------------------------------------------
void updateSignalLights() {
    for (int i = 0; i < numSwitches; i++) {
        // Basic Logic: Set all signals to GREEN (0) for now.
        // (Advanced logic requires checking track occupancy ahead)
        switchSignal[i] = 0; 
    }
}

// ----------------------------------------------------------------------------
// TOGGLE SWITCH STATE (Manual)
// ----------------------------------------------------------------------------
// Manually toggle a switch state.
// ----------------------------------------------------------------------------
void toggleSwitchState(int switchID) {
    // Check bounds to be safe
    if (switchID >= 0 && switchID < numSwitches) {
        switchState[switchID] = !switchState[switchID];
    }
}

// ----------------------------------------------------------------------------
// GET SWITCH STATE FOR DIRECTION
// ----------------------------------------------------------------------------
// Return the state for a given direction.
// ----------------------------------------------------------------------------
int getSwitchStateForDirection(int switchID, int direction) {
    // Check bounds
    if (switchID >= 0 && switchID < numSwitches) {
        return switchState[switchID];
    }
    return 0; // Default to straight if invalid ID
}