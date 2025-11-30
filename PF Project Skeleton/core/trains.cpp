#include "trains.h"
#include "simulation_state.h"
#include "grid.h"
#include "switches.h"
#include <cstdlib>
#include <iostream>

using namespace std;

// ============================================================================
// TRAINS.CPP - Train logic
// ============================================================================

// ---------------------------------------------------------------------------
// Helper: get the destination assigned to a specific train.
// Destinations are stored by *destination index*, and mapped to trains via
// destinationTrainID[d] == trainID.
// ---------------------------------------------------------------------------
static bool getDestinationForTrain(int trainID, int &destRow, int &destCol) {
    for (int d = 0; d < numDest; ++d) {
        if (destinationTrainID[d] == trainID) {
            destRow = destinationRow[d];
            destCol = destinationColumn[d];
            return true;
        }
    }
    // Fallback: if no explicit assignment but destinations exist,
    // just use the first one. If there are no destinations at all,
    // return false.
    if (numDest > 0) {
        destRow = destinationRow[0];
        destCol = destinationColumn[0];
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
// SPAWN TRAINS FOR CURRENT TICK
// ----------------------------------------------------------------------------
// Activate trains scheduled for this tick.
// ----------------------------------------------------------------------------
void spawnTrainsForTick() {
    // Check all spawn instructions
    for (int i = 0; i < num_spawn; i++) {
        if (spawnTick[i] == currentTick && spawnTrainID[i] == -1) {

            // Find a free train slot
            int freeTrain = -1;
            for (int j = 0; j < max_trains; j++) {
                if (trainRow[j] == -1) {
                    freeTrain = j;
                    break;
                }
            }
            if (freeTrain == -1) {
                // No free slot; cannot spawn this train
                continue;
            }

            // If spawn tile is already occupied, skip this spawn this tick
            bool occupied = false;
            for (int k = 0; k < max_trains; k++) {
                if (trainRow[k] == spawnn_Row[i] &&
                    trainColumn[k] == spawnn_Column[i]) {
                    occupied = true;
                    break;
                }
            }
            if (occupied) continue;

            // Spawn train
            trainRow[freeTrain]       = spawnn_Row[i];
            trainColumn[freeTrain]    = spawnn_Column[i];
            trainDirection[freeTrain] = spawnDirection[i];
            trainColor[freeTrain]     = spawnColor[i];
            trainWait[freeTrain]      = 0;

            spawnTrainID[i] = freeTrain;  // Mark instruction as spawned

            // Re-map this destination from "spawn index" to actual train ID
            for (int d = 0; d < numDest; d++) {
                if (destinationTrainID[d] == i) {
                    destinationTrainID[d] = freeTrain;
                }
            }

            // NOTE: numOf_trains is treated as "highest used train index + 1".
            // It is *not* decremented when trains reach/crash, but that is OK
            // because we check trainRow[i] == -1 everywhere.
            if (freeTrain + 1 > numOf_trains) {
                numOf_trains = freeTrain + 1;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// DETERMINE NEXT POSITION for a train
// ----------------------------------------------------------------------------
// Compute next position from current tile and direction.
// Returns false if the move would go out of bounds or onto an invalid tile.
// ----------------------------------------------------------------------------
bool determineNextPosition(int trainID, int &nextRow, int &nextColumn) {
    nextRow    = trainRow[trainID]    + row_change[trainDirection[trainID]];
    nextColumn = trainColumn[trainID] + column_change[trainDirection[trainID]];

    // Correct bounds check (row vs number_rows, col vs number_column)
    if (!isInBounds(nextRow, nextColumn)) {
        return false;
    }

    char nextTile = grid[nextRow][nextColumn];

    // Only allow moving onto valid tiles
    if (!(isTrackTile(nextRow, nextColumn) ||
          isSwitchTile(nextRow, nextColumn) ||
          nextTile == spawn ||
          nextTile == destination ||
          nextTile == '=')) {
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
// GET NEXT DIRECTION based on current tile and direction
// ----------------------------------------------------------------------------
// Return new direction after entering the tile.
// ----------------------------------------------------------------------------
int getNextDirection(int trainID, int row, int col) {
    char track = grid[row][col];   // Tile train is currently on

     if(track=='='){
        track=originalGrid[trainRow[trainID]][trainColumn[trainID]];
        // If original is safety track treat as horizontal track
        if(track == '=') track = '-';
    }
    //Normal track logic
    switch (track) {
        case spawn:
        case destination:
        case '=':
        case horizontal_track:
            // Horizontal track: only LEFT or RIGHT is valid
            if (trainDirection[trainID] == left_dir) {
                return left_dir;
            } else {
                return right_dir;
            }

        case vertical_track:
            // Vertical track: only UP or DOWN is valid
            if (trainDirection[trainID] == up_dir) {
                return up_dir;
            } else {
                return down_dir;
            }

        case right_curve:
            if (trainDirection[trainID] == up_dir)        return right_dir;
            else if (trainDirection[trainID] == left_dir) return down_dir;
            break;

        case left_curve:
            if (trainDirection[trainID] == up_dir)         return left_dir;
            else if (trainDirection[trainID] == right_dir) return down_dir;
            break;

        case crossing:
            // Smart routing at '+'
            return getSmartDirectionAtCrossing(trainID);

        default:
            break;
    }

    // --- SWITCH LOGIC ---
    // If we are currently standing on a switch tile, choose direction
    // based on switchState[switchID].
    if (isSwitchTile(row, col)) {
        int switchID = getSwitchIndex(row, col);

        int currentDir = trainDirection[trainID];

        int straight = currentDir;
        int right    = (currentDir + 1) % 4;
        int left     = (currentDir + 3) % 4;
        int back     = (currentDir + 2) % 4;

        int candidates[4];
        candidates[0] = straight;

        // IMPORTANT:
        // Use switchState[switchID] to decide branch order.
        // 0 => prefer left then right
        // 1 => prefer right then left
        if (switchState[switchID] == 0) {
            candidates[1] = left;
            candidates[2] = right;
        } else {
            candidates[1] = right;
            candidates[2] = left;
        }
        candidates[3] = back; // last resort

        // Find the first candidate that leads to a valid tile
        for (int k = 0; k < 4; k++) {
            int dir    = candidates[k];
            int newRow = row + row_change[dir];
            int newCol = col + column_change[dir];

            if (!isInBounds(newRow, newCol)) continue;

            char newTile = grid[newRow][newCol];
            bool valid = (isTrackTile(newRow, newCol) ||
                          isSwitchTile(newRow, newCol) ||
                          newTile == spawn ||
                          newTile == destination ||
                          newTile == '=');

            if (valid) {
                return dir;
            }
        }
    }

    // Default: keep current direction
    return trainDirection[trainID];
}
// ----------------------------------------------------------------------------
// SMART ROUTING AT CROSSING - Route train to its matched destination
// ----------------------------------------------------------------------------
// Choose best direction at '+' toward the *correct* destination for this train.
// ----------------------------------------------------------------------------
int getSmartDirectionAtCrossing(int trainID) {
    int row    = trainRow[trainID];
    int column = trainColumn[trainID];

    int destRow, destCol;
    if (!getDestinationForTrain(trainID, destRow, destCol)) {
        // No destination assigned, just keep going straight
        return trainDirection[trainID];
    }

    int bestDir    = trainDirection[trainID];
    int minDistance = abs(row - destRow) + abs(column - destCol);    // Manhattan distance

    // Check all 4 possible directions
    for (int i = 0; i < 4; i++) {
        int nrow    = row    + row_change[i];
        int ncolumn = column + column_change[i];

        // this bug was causing the issues(corrected now)
        if (nrow < 0 || nrow >= number_rows ||
            ncolumn < 0 || ncolumn >= number_column)
            continue;

        char tile = grid[nrow][ncolumn];
        if (tile == space) continue; // Can't move onto empty tiles

        int dist = abs(nrow - destRow) + abs(ncolumn - destCol);
        if (dist < minDistance) {
            minDistance = dist;
            bestDir     = i;
        }
    }

    return bestDir;
}

// ----------------------------------------------------------------------------
// DETERMINE ALL ROUTES (PHASE 2)
// ----------------------------------------------------------------------------
// Fill next directions for all trains using local rules.
// ----------------------------------------------------------------------------
void determineAllRoutes() {
    for (int i = 0; i < numOf_trains; i++) {
        if (trainRow[i] == -1) continue; // inactive
        // If already at destination, do nothing
        if (isDestinationPoint(trainRow[i], trainColumn[i])) {
            continue;
        }
        // Determine next direction based on current tile
        trainDirection[i] = getNextDirection(i, trainRow[i], trainColumn[i]);
    }
}

// ----------------------------------------------------------------------------
// MOVE ALL TRAINS (PHASE 5)
// ----------------------------------------------------------------------------
// Move trains; resolve collisions and apply effects.
// ----------------------------------------------------------------------------
void moveAllTrains() {
    int nextRow[max_trains];
    int nextCol[max_trains];
    int nextDir[max_trains];
    int oldRow[max_trains];
    int oldCol[max_trains];

    // Plan moves
    for (int i = 0; i < numOf_trains; i++) {
        if (trainRow[i] == -1) {
            // inactive train
            nextRow[i] = -1;
            nextCol[i] = -1;
            nextDir[i] = trainDirection[i];
            continue;
        }

        // If the train is already waiting (safety tile/emergency halt) then it don't move
        if (trainWait[i] > 0) {
            nextRow[i] = trainRow[i];
            nextCol[i] = trainColumn[i];
            nextDir[i] = trainDirection[i];
            trainWait[i]--;
            totalWaitTicks++;
            continue;
        }

        // Compute next position
        if (!determineNextPosition(i, nextRow[i], nextCol[i])) {
            // IMPORTANT CHANGE:
            // we treatt leaving the track / out-of-bound  as a crash.
            // Otherwise the train would stay stuck forever and block others.
            trainRow[i]    = -1;
            trainColumn[i] = -1;
            nextRow[i]     = -1;
            nextCol[i]     = -1;
            crashed_trains++;
            continue;
        }

        // Compute next direction (based on the tile we're leaving)
        nextDir[i] = getNextDirection(i, trainRow[i], trainColumn[i]);
    }

    // Save previous positions for safety-tile detection
    for (int i = 0; i < numOf_trains; i++) {
        oldRow[i] = trainRow[i];
        oldCol[i] = trainColumn[i];
    }

    // Resolve collisions based on next positions
    detectCollisions(nextRow, nextCol, nextDir);

    // Apply movements
    for (int i = 0; i < numOf_trains; i++) {
        if (nextRow[i] != -1) {  // skip trains that are crashed
            trainRow[i]       = nextRow[i];
            trainColumn[i]    = nextCol[i];
            trainDirection[i] = nextDir[i];

            // Check if train has entered a safety tile
            char tile = grid[trainRow[i]][trainColumn[i]];
            if (tile == '=') {
                // Only set wait if we actually entered a new safety tile
                if (!(oldRow[i] == trainRow[i] && oldCol[i] == trainColumn[i])) {
                    trainWait[i] = 1;
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// DETECT COLLISIONS WITH PRIORITY SYSTEM
// ----------------------------------------------------------------------------
void detectCollisions(int nextRow[], int nextCol[], int nextDir[]) {
    for (int i = 0; i < numOf_trains; i++) {
        if (trainRow[i] == -1) continue; // inactive

        for (int j = i + 1; j < numOf_trains; j++) {
            if (trainRow[j] == -1) continue;

            // SAME-TILE collision: both try to move to same cell
            if (nextRow[i] == nextRow[j] && nextCol[i] == nextCol[j]) {
                // If one or both are already removed then we skip
                if (nextRow[i] == -1 || nextRow[j] == -1) continue;

                char tile = grid[nextRow[i]][nextCol[i]];

                int destRi, destCi, destRj, destCj;
                int dist_i = 0, dist_j = 0;

                if (getDestinationForTrain(i, destRi, destCi))
                    dist_i = abs(nextRow[i] - destRi) + abs(nextCol[i] - destCi);
                if (getDestinationForTrain(j, destRj, destCj))
                    dist_j = abs(nextRow[j] - destRj) + abs(nextCol[j] - destCj);

                if (dist_i == dist_j) {
                    // Both crash
                    nextRow[i]    = -1;
                    nextCol[i]    = -1;
                    nextRow[j]    = -1;
                    nextCol[j]    = -1;
                    trainRow[i]   = -1;
                    trainColumn[i]= -1;
                    trainRow[j]   = -1;
                    trainColumn[j]= -1;
                    crashed_trains += 2;
                } else if (dist_i > dist_j) {
                    // Train i is farther from its destination so it moves
                    nextRow[j] = trainRow[j];
                    nextCol[j] = trainColumn[j];
                } else {
                    // Train j is farther from its destination so it moves
                    nextRow[i] = trainRow[i];
                    nextCol[i] = trainColumn[i];
                }
            }

            // HEAD-ON swap collision: trains swap positions
            else if (nextRow[i] == trainRow[j] && nextCol[i] == trainColumn[j] &&
                     nextRow[j] == trainRow[i] && nextCol[j] == trainColumn[i]) {

                // If one or both already removed, skip
                if (nextRow[i] == -1 || nextRow[j] == -1) continue;

                //Check if straight track only
                char tile_i = grid[trainRow[i]][trainColumn[i]];
                char tile_j = grid[trainRow[j]][trainColumn[j]];
    
                // If current tile is safety, check original tile
                if (tile_i == '=') tile_i = originalGrid[trainRow[i]][trainColumn[i]];
                if (tile_j == '=') tile_j = originalGrid[trainRow[j]][trainColumn[j]];
    
                // If both trains are on straight tracks (not crossings), they MUST crash
                bool isStraightTrack_i = (tile_i == '|' || tile_i == '-');
                bool isStraightTrack_j = (tile_j == '|' || tile_j == '-');
                bool isSafety_i = (grid[trainRow[i]][trainColumn[i]] == '=');
                bool isSafety_j = (grid[trainRow[j]][trainColumn[j]] == '=');
    
                if(isStraightTrack_i && isStraightTrack_j || isSafety_i || isSafety_j){
                    //Crash Trains if no path avaliable
                    nextRow[i]=-1;
                    nextCol[i]=-1;
                    nextRow[j]=-1;
                    nextCol[j]=-1;
                    trainRow[i]=-1;
                    trainColumn[i]=-1;
                    trainRow[j]=-1;
                    trainColumn[j]=-1;
                    crashed_trains+=2;
                    continue; //Skip manhattan distance check
                }
                int destRi, destCi, destRj, destCj;
                int dist_i = 0, dist_j = 0;

                if (getDestinationForTrain(i, destRi, destCi))
                    dist_i = abs(nextRow[i] - destRi) + abs(nextCol[i] - destCi);
                if (getDestinationForTrain(j, destRj, destCj))
                    dist_j = abs(nextRow[j] - destRj) + abs(nextCol[j] - destCj);

                if (dist_i == dist_j) {
                    // Both crash
                    nextRow[i]     = -1;
                    nextCol[i]     = -1;
                    nextRow[j]     = -1;
                    nextCol[j]     = -1;
                    trainRow[i]    = -1;
                    trainColumn[i] = -1;
                    trainRow[j]    = -1;
                    trainColumn[j] = -1;
                    crashed_trains += 2;
                } else if (dist_i > dist_j) {
                    // i is farther from its destination so it moves
                    nextRow[j] = trainRow[j];
                    nextCol[j] = trainColumn[j];
                } else {
                    // j is farther from its destination so it moves
                    nextRow[i] = trainRow[i];
                    nextCol[i] = trainColumn[i];
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// CHECK ARRIVALS
// ----------------------------------------------------------------------------
// Mark trains that reached destinations.
// ----------------------------------------------------------------------------
void checkArrivals() {
    for (int i = 0; i < numOf_trains; i++) {
        if (trainRow[i] == -1) continue; // inactive

        for (int j = 0; j < numDest; j++) {
            // If thee train is exactly at a destination coordinate
            if (trainRow[i] == destinationRow[j] &&
                trainColumn[i] == destinationColumn[j]) {

                trainsReached++;
                trainRow[i]    = -1;
                trainColumn[i] = -1;  // Train becomes inactive

                // over here we clear this destination's train mapping if it belonged to this train
                if (destinationTrainID[j] == i)
                    destinationTrainID[j] = -1;

                break; // Stop checking other destinations for this train
            }
        }
    }
}

// ----------------------------------------------------------------------------
// APPLY EMERGENCY HALT
// ----------------------------------------------------------------------------
// Apply halt to trains in the active zone.
// ----------------------------------------------------------------------------
void applyEmergencyHalt() {
    for (int i = 0; i < numOf_trains; i++) {
        if (trainRow[i] != -1 &&
            emergencyHalt[trainRow[i]][trainColumn[i]] > 0) {

            int haltTicks = emergencyHalt[trainRow[i]][trainColumn[i]];
            if (trainWait[i] < haltTicks) {
                trainWait[i] = haltTicks;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// UPDATE EMERGENCY HALT
// ----------------------------------------------------------------------------
// Decrement timer and disable when done.
// NOTE: fixed iteration order to match emergencyHalt[row][col].
// ----------------------------------------------------------------------------
void updateEmergencyHalt() {
    for (int r = 0; r < number_rows; r++) {
        for (int c = 0; c < number_column; c++) {
            if (emergencyHalt[r][c] > 0) {
                emergencyHalt[r][c]--;
            }
        }
    }
}