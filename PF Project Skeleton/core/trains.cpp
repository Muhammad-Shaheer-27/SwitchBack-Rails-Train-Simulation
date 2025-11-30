#include "trains.h"
#include "simulation_state.h"
#include "grid.h"
#include "switches.h"
#include <cstdlib>
#include <iostream>

using namespace std;

// ============================================================================
// TRAINS.CPP - Train logic WITH WEATHER EFFECTS + SIGNAL OBEYING
// ============================================================================

// Track move counter for rain slowdowns (per train)
static int trainMoveCounter[max_trains] = {0};

// ---------------------------------------------------------------------------
// Helper: get the destination assigned to a specific train.
// ---------------------------------------------------------------------------
static bool getDestinationForTrain(int trainID, int &destRow, int &destCol) {
    for (int d = 0; d < numDest; ++d) {
        if (destinationTrainID[d] == trainID) {
            destRow = destinationRow[d];
            destCol = destinationColumn[d];
            return true;
        }
    }
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
void spawnTrainsForTick() {
    for (int i = 0; i < num_spawn; i++) {
        if (spawnTick[i] == currentTick && spawnTrainID[i] == -1) {

            int freeTrain = -1;
            for (int j = 0; j < max_trains; j++) {
                if (trainRow[j] == -1) {
                    freeTrain = j;
                    break;
                }
            }
            if (freeTrain == -1) continue;

            bool occupied = false;
            for (int k = 0; k < max_trains; k++) {
                if (trainRow[k] == spawnn_Row[i] &&
                    trainColumn[k] == spawnn_Column[i]) {
                    occupied = true;
                    break;
                }
            }
            if (occupied) continue;

            trainRow[freeTrain]       = spawnn_Row[i];
            trainColumn[freeTrain]    = spawnn_Column[i];
            trainDirection[freeTrain] = spawnDirection[i];
            trainColor[freeTrain]     = spawnColor[i];
            trainWait[freeTrain]      = 0;
            trainMoveCounter[freeTrain] = 0;

            spawnTrainID[i] = freeTrain;

            for (int d = 0; d < numDest; d++) {
                if (destinationTrainID[d] == i) {
                    destinationTrainID[d] = freeTrain;
                }
            }

            if (freeTrain + 1 > numOf_trains) {
                numOf_trains = freeTrain + 1;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// DETERMINE NEXT POSITION
// ----------------------------------------------------------------------------
bool determineNextPosition(int trainID, int &nextRow, int &nextColumn) {
    nextRow    = trainRow[trainID]    + row_change[trainDirection[trainID]];
    nextColumn = trainColumn[trainID] + column_change[trainDirection[trainID]];

    if (!isInBounds(nextRow, nextColumn)) {
        return false;
    }

    char nextTile = grid[nextRow][nextColumn];

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
// GET NEXT DIRECTION
// ----------------------------------------------------------------------------
int getNextDirection(int trainID, int row, int col) {
    char track = grid[row][col];

    // Treat safety tiles '=' as "keep current direction"
    if (track == '=') {
        return trainDirection[trainID];
    }
    
    switch (track) {
        case spawn:
        case destination:
        case horizontal_track:
            if (trainDirection[trainID] == left_dir) {
                return left_dir;
            } else if (trainDirection[trainID] == right_dir) {
                return right_dir;
            } else {
                return trainDirection[trainID];
            }

        case vertical_track:
            if (trainDirection[trainID] == up_dir) {
                return up_dir;
            } else if (trainDirection[trainID] == down_dir) {
                return down_dir;
            } else {
                return trainDirection[trainID];
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
            return getSmartDirectionAtCrossing(trainID);

        default:
            break;
    }

    // Switch logic (based on switchState)
    if (isSwitchTile(row, col)) {
        int switchID = getSwitchIndex(row, col);
        int currentDir = trainDirection[trainID];

        int straight = currentDir;
        int right    = (currentDir + 1) % 4;
        int left     = (currentDir + 3) % 4;
        int back     = (currentDir + 2) % 4;

        int candidates[4];
        candidates[0] = straight;

        if (switchState[switchID] == 0) {
            candidates[1] = left;
            candidates[2] = right;
        } else {
            candidates[1] = right;
            candidates[2] = left;
        }
        candidates[3] = back;

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

    return trainDirection[trainID];
}

// ----------------------------------------------------------------------------
// SMART ROUTING AT CROSSING
// ----------------------------------------------------------------------------
int getSmartDirectionAtCrossing(int trainID) {
    int row    = trainRow[trainID];
    int column = trainColumn[trainID];

    int destRow, destCol;
    if (!getDestinationForTrain(trainID, destRow, destCol)) {
        return trainDirection[trainID];
    }

    int bestDir     = trainDirection[trainID];
    int minDistance = abs(row - destRow) + abs(column - destCol);

    for (int i = 0; i < 4; i++) {
        int nrow    = row    + row_change[i];
        int ncolumn = column + column_change[i];

        if (nrow < 0 || nrow >= number_rows ||
            ncolumn < 0 || ncolumn >= number_column)
            continue;

        char tile = grid[nrow][ncolumn];
        if (tile == space) continue;

        int dist = abs(nrow - destRow) + abs(ncolumn - destCol);
        if (dist < minDistance) {
            minDistance = dist;
            bestDir     = i;
        }
    }

    return bestDir;
}

// ----------------------------------------------------------------------------
// DETERMINE ALL ROUTES
// ----------------------------------------------------------------------------
void determineAllRoutes() {
    for (int i = 0; i < numOf_trains; i++) {
        if (trainRow[i] == -1) continue;
        if (isDestinationPoint(trainRow[i], trainColumn[i])) {
            continue;
        }
        trainDirection[i] = getNextDirection(i, trainRow[i], trainColumn[i]);
    }
}

// ----------------------------------------------------------------------------
// MOVE ALL TRAINS - WITH WEATHER EFFECTS + OBEY RED SIGNALS
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
            nextRow[i] = -1;
            nextCol[i] = -1;
            nextDir[i] = trainDirection[i];
            continue;
        }

        // If already waiting, decrement and stay put
        if (trainWait[i] > 0) {
            nextRow[i] = trainRow[i];
            nextCol[i] = trainColumn[i];
            nextDir[i] = trainDirection[i];
            trainWait[i]--;
            totalWaitTicks++;
            continue;
        }

        // === SIGNAL CHECK ON SWITCH ===
        // If train is on a switch and that switch shows RED, stop for 1 tick.
        if (isSwitchTile(trainRow[i], trainColumn[i])) {
            int swID = getSwitchIndex(trainRow[i], trainColumn[i]);
            if (swID >= 0 && swID < maximum_switches) {
                if (switchSignal[swID] == sigal_red) {  // RED
                    nextRow[i] = trainRow[i];
                    nextCol[i] = trainColumn[i];
                    nextDir[i] = trainDirection[i];
                    trainWait[i] = 1;   // stop for one tick
                    totalWaitTicks++;
                    continue;
                }
            }
        }

        // === WEATHER EFFECT: RAIN ===
        if (weather_type == weather_rain) {
            trainMoveCounter[i]++;
            if (trainMoveCounter[i] >= 5) {
                trainWait[i] = 1;
                trainMoveCounter[i] = 0;

                nextRow[i] = trainRow[i];
                nextCol[i] = trainColumn[i];
                nextDir[i] = trainDirection[i];
                totalWaitTicks++;
                continue;
            }
        }

        // Compute next position
        if (!determineNextPosition(i, nextRow[i], nextCol[i])) {
            // Invalid next tile: treat as crash (leaves map)
            trainRow[i]    = -1;
            trainColumn[i] = -1;
            nextRow[i]     = -1;
            nextCol[i]     = -1;
            crashed_trains++;
            continue;
        }

        nextDir[i] = getNextDirection(i, trainRow[i], trainColumn[i]);
    }

    // Save previous positions
    for (int i = 0; i < numOf_trains; i++) {
        oldRow[i] = trainRow[i];
        oldCol[i] = trainColumn[i];
    }

    // Resolve collisions
    detectCollisions(nextRow, nextCol, nextDir);

    // Apply movements
    for (int i = 0; i < numOf_trains; i++) {
        if (nextRow[i] != -1) {
            trainRow[i]       = nextRow[i];
            trainColumn[i]    = nextCol[i];
            trainDirection[i] = nextDir[i];

            char tile = grid[trainRow[i]][trainColumn[i]];

            // Safety tile: 1 tick wait when entering '='
            if (tile == '=') {
                if (!(oldRow[i] == trainRow[i] && oldCol[i] == trainColumn[i])) {
                    trainWait[i] = 1;
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// DETECT COLLISIONS
// ----------------------------------------------------------------------------
void detectCollisions(int nextRow[], int nextCol[], int nextDir[]) {
    for (int i = 0; i < numOf_trains; i++) {
        if (trainRow[i] == -1) continue;

        for (int j = i + 1; j < numOf_trains; j++) {
            if (trainRow[j] == -1) continue;

            // SAME-TILE collision
            if (nextRow[i] == nextRow[j] && nextCol[i] == nextCol[j]) {
                if (nextRow[i] == -1 || nextRow[j] == -1) continue;

                int destRi, destCi, destRj, destCj;
                int dist_i = 0, dist_j = 0;

                if (getDestinationForTrain(i, destRi, destCi))
                    dist_i = abs(nextRow[i] - destRi) + abs(nextCol[i] - destCi);
                if (getDestinationForTrain(j, destRj, destCj))
                    dist_j = abs(nextRow[j] - destRj) + abs(nextCol[j] - destCj);

                if (dist_i == dist_j) {
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
                    nextRow[i] = trainRow[i];
                    nextCol[i] = trainColumn[i];
                } else {
                    nextRow[j] = trainRow[j];
                    nextCol[j] = trainColumn[j];
                }
            }

            // HEAD-ON swap collision
            else if (nextRow[i] == trainRow[j] && nextCol[i] == trainColumn[j] &&
                     nextRow[j] == trainRow[i] && nextCol[j] == trainColumn[i]) {

                if (nextRow[i] == -1 || nextRow[j] == -1) continue;

                char tile_i = grid[trainRow[i]][trainColumn[i]];
                char tile_j = grid[trainRow[j]][trainColumn[j]];
    
                bool isStraightTrack_i = (tile_i == vertical_track || tile_i == horizontal_track);
                bool isStraightTrack_j = (tile_j == vertical_track || tile_j == horizontal_track);
                bool isSafety_i        = (tile_i == '=');
                bool isSafety_j        = (tile_j == '=');
    
                if ((isStraightTrack_i && isStraightTrack_j) || isSafety_i || isSafety_j) {
                    nextRow[i] = -1;
                    nextCol[i] = -1;
                    nextRow[j] = -1;
                    nextCol[j] = -1;
                    trainRow[i] = -1;
                    trainColumn[i] = -1;
                    trainRow[j] = -1;
                    trainColumn[j] = -1;
                    crashed_trains += 2;
                    continue;
                }
                
                int destRi, destCi, destRj, destCj;
                int dist_i = 0, dist_j = 0;

                if (getDestinationForTrain(i, destRi, destCi))
                    dist_i = abs(nextRow[i] - destRi) + abs(nextCol[i] - destCi);
                if (getDestinationForTrain(j, destRj, destCj))
                    dist_j = abs(nextRow[j] - destRj) + abs(nextCol[j] - destCj);

                if (dist_i == dist_j) {
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
                    nextRow[i] = trainRow[i];
                    nextCol[i] = trainColumn[i];
                } else {
                    nextRow[j] = trainRow[j];
                    nextCol[j] = trainColumn[j];
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// CHECK ARRIVALS
// ----------------------------------------------------------------------------
void checkArrivals() {
    for (int i = 0; i < numOf_trains; i++) {
        if (trainRow[i] == -1) continue;

        for (int j = 0; j < numDest; j++) {
            if (trainRow[i] == destinationRow[j] &&
                trainColumn[i] == destinationColumn[j]) {

                trainsReached++;
                trainRow[i]    = -1;
                trainColumn[i] = -1;

                if (destinationTrainID[j] == i)
                    destinationTrainID[j] = -1;

                break;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// APPLY EMERGENCY HALT
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
void updateEmergencyHalt() {
    for (int r = 0; r < number_rows; r++) {
        for (int c = 0; c < number_column; c++) {
            if (emergencyHalt[r][c] > 0) {
                emergencyHalt[r][c]--;
            }
        }
    }
}