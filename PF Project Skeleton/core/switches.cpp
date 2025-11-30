#include "switches.h"
#include "simulation_state.h"
#include "grid.h"
#include "io.h"
#include <iostream>

using namespace std;

// ============================================================================
// SWITCHES.CPP - Switch management + SIGNAL LIGHTS
// ============================================================================

// ----------------------------------------------------------------------------
// UPDATE SWITCH COUNTERS
// ----------------------------------------------------------------------------
void updateSwitchCounters() {
    for (int i = 0; i < numOf_trains; i++) {
        if (trainRow[i] == -1) continue;

        char tile = grid[trainRow[i]][trainColumn[i]];

        if (tile >= 'A' && tile <= 'Z') {
            int swID = tile - 'A';
            int dir = trainDirection[i];

            if (switchMode[swID] == GLOBAL) {
                switchCounter[swID][0]++;
            } else {
                if (dir >= 0 && dir < 4) {
                    switchCounter[swID][dir]++;
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// QUEUE SWITCH FLIPS
// ----------------------------------------------------------------------------
void queueSwitchFlips() {
    for (int i = 0; i < numSwitches; i++) {
        if (switchMode[i] == GLOBAL) {
            if (switchCounter[i][0] >= switchK[i][0]) {
                switchFlipped[i] = 1;
                switchCounter[i][0] = 0;
                switchFlips++;
            }
        } else {
            for (int dir = 0; dir < 4; dir++) {
                if (switchCounter[i][dir] >= switchK[i][dir]) {
                    switchFlipped[i] = 1;
                    switchCounter[i][dir] = 0;
                    switchFlips++;
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// APPLY DEFERRED FLIPS
// ----------------------------------------------------------------------------
void applyDeferredFlips() {
    for (int i = 0; i < numSwitches; i++) {
        if (switchFlipped[i] == 1) {
            switchState[i] = !switchState[i];
            switchFlipped[i] = 0;
        }
    }
}

// ----------------------------------------------------------------------------
// UPDATE SIGNAL LIGHTS
// ----------------------------------------------------------------------------
// For each train that is currently ON a switch tile, we compute the signal
// for that switch ID (A..Z):
//
// Green (0): next tile along the train's current direction is free & valid.
// Yellow (1): next tile is free, but there is a train two tiles ahead.
// Red (2): next tile is blocked/out-of-bounds/not track OR next tile is occupied.
// ----------------------------------------------------------------------------
void updateSignalLights() {
    // Default all signals to GREEN for all possible letters A..Z
    for (int i = 0; i < maximum_switches; i++) {
        switchSignal[i] = signal_green;
    }

    // For each train, if it is standing on a switch, compute that switch's signal
    for (int t = 0; t < numOf_trains; t++) {
        if (trainRow[t] == -1) continue;

        int r = trainRow[t];
        int c = trainColumn[t];

        if (!isSwitchTile(r, c)) continue;

        int swID = getSwitchIndex(r, c);
        if (swID < 0 || swID >= maximum_switches) continue;

        int dir = trainDirection[t];

        bool red = false;
        bool yellow = false;

        // 1 tile ahead in current direction
        int r1 = r + row_change[dir];
        int c1 = c + column_change[dir];

        // If out-of-bounds or not traversable, it's RED
        if (!isInBounds(r1, c1) ||
            !(isTrackTile(r1, c1) || isSwitchTile(r1, c1) ||
              grid[r1][c1] == spawn || grid[r1][c1] == destination || grid[r1][c1] == '=')) {
            red = true;
        } else {
            // If another train is currently on the next tile, it's RED
            for (int k = 0; k < numOf_trains; k++) {
                if (k == t) continue;
                if (trainRow[k] == r1 && trainColumn[k] == c1) {
                    red = true;
                    break;
                }
            }

            // If not RED, check two tiles ahead for YELLOW
            if (!red) {
                int r2 = r1 + row_change[dir];
                int c2 = c1 + column_change[dir];

                if (isInBounds(r2, c2)) {
                    for (int k = 0; k < numOf_trains; k++) {
                        if (trainRow[k] == r2 && trainColumn[k] == c2) {
                            yellow = true;
                            break;
                        }
                    }
                }
            }
        }

        int newStatus = signal_green;
        if (red) newStatus = sigal_red;         // NOTE: constant name 'sigal_red' in header
        else if (yellow) newStatus = signal_yellow;

        // Use highest severity per switch: green(0) < yellow(1) < red(2)
        if (newStatus > switchSignal[swID]) {
            switchSignal[swID] = newStatus;
        }
    }
}

// ----------------------------------------------------------------------------
// TOGGLE SWITCH STATE (Manual)
// ----------------------------------------------------------------------------
void toggleSwitchState(int switchID) {
    if (switchID >= 0 && switchID < numSwitches) {
        switchState[switchID] = !switchState[switchID];
    }
}

// ----------------------------------------------------------------------------
// GET SWITCH STATE FOR DIRECTION
// ----------------------------------------------------------------------------
int getSwitchStateForDirection(int switchID, int direction) {
    if (switchID >= 0 && switchID < numSwitches) {
        return switchState[switchID];
    }
    return 0;
}