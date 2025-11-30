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
    // We track severity per switch letter A..Z:
    // 0 = green, 1 = yellow, 2 = red
    int severity[maximum_switches];
    bool hasSwitch[maximum_switches];

    for (int i = 0; i < maximum_switches; i++) {
        severity[i] = signal_green;   // default: green
        hasSwitch[i] = false;
    }

    // Scan entire grid for switch tiles and trains around them
    for (int r = 0; r < number_rows; r++) {
        for (int c = 0; c < number_column; c++) {
            char tile = grid[r][c];
            if (tile < 'A' || tile > 'Z') continue;   // not a switch tile

            int swID = tile - 'A';
            if (swID < 0 || swID >= maximum_switches) continue;
            hasSwitch[swID] = true;

            int sev = severity[swID]; // current severity for this letter

            // 1) Check distance 1 for any train -> RED
            for (int dir = 0; dir < 4; dir++) {
                int r1 = r + row_change[dir];
                int c1 = c + column_change[dir];
                if (!isInBounds(r1, c1)) continue;

                for (int t = 0; t < numOf_trains; t++) {
                    if (trainRow[t] == r1 && trainColumn[t] == c1) {
                        sev = sigal_red;    // 2
                        goto done_near;     // no need to check more for this tile
                    }
                }
            }

            // 2) If not RED, check distance 2 for any train -> YELLOW
            if (sev != sigal_red) {
                for (int dir = 0; dir < 4; dir++) {
                    int r2 = r + 2 * row_change[dir];
                    int c2 = c + 2 * column_change[dir];
                    if (!isInBounds(r2, c2)) continue;

                    for (int t = 0; t < numOf_trains; t++) {
                        if (trainRow[t] == r2 && trainColumn[t] == c2) {
                            if (sev < signal_yellow) {
                                sev = signal_yellow;  // 1
                            }
                        }
                    }
                }
            }

        done_near:
            severity[swID] = sev;
        }
    }

    // Apply severity to switchSignal[]
    for (int i = 0; i < maximum_switches; i++) {
        if (hasSwitch[i]) {
            switchSignal[i] = severity[i];
        } else {
            switchSignal[i] = signal_green;
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