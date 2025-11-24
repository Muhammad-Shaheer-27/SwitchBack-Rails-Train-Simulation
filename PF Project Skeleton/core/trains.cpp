#include "trains.h"
#include "simulation_state.h"
#include "grid.h"
#include "switches.h"
#include <cstdlib>

// ============================================================================
// TRAINS.CPP - Train logic
// ============================================================================

// Storage for planned moves (for collisions).

// Previous positions (to detect switch entry).

// ----------------------------------------------------------------------------
// SPAWN TRAINS FOR CURRENT TICK
// ----------------------------------------------------------------------------
// Activate trains scheduled for this tick.
// ----------------------------------------------------------------------------
void spawnTrainsForTick() {
    for(int i=0;i<numSpawn;i++){    //Checking all spawn points
        if(spawnTick[i]==currentTick && spawnTrainID[i]==-1){   //Check if train scheduled for spawn
             // Find a free train slot
            int freeTrain=-1;
            for(int j=0;j<max_trains;j++) {
                if(trainRow[j]==-1){
                    freeTrain=j;    //If slot is avaliable then store the free slot index in free train
                    break;
                }
            }
            if(freeTrain==-1) {
                //Free spot not availiable then dont spawn
                continue;
            }
             bool occupied = false;
            for(int k=0;k<max_trains;k++) {     //Check if a train is already present on spawn tile
                if(trainRow[k]==spawnRow[i] && trainColumn[k]==spawnColumn[i]) {
                    occupied = true;
                    break;
                }
            }
            if(occupied) continue;  //If tile  is occupied dont spawn
            // Spawn train by filling in the spawn indexes
            trainRow[freeTrain]=spawnRow[i];
            trainColumn[freeTrain]=spawnColumn[i];
            trainDirection[freeTrain]=spawnDirection[i];
            trainColor[freeTrain]=spawnColor[i];
            trainWait[freeTrain]=0;

            spawnTrainID[i] = freeTrain;  // Mark train as spawned
            numTrains++;
        }
    }
}

// ----------------------------------------------------------------------------
// DETERMINE NEXT POSITION for a train
// ----------------------------------------------------------------------------
// Compute next position/direction from current tile and rules.
// ----------------------------------------------------------------------------
bool determineNextPosition(int trainID,int &nextRow,int &nextColumn) {
    nextRow=trainRow[trainID]+row_change[trainDirection[trainID]];  //Calculate next row 
    nextColumn=trainColumn[trainID]+column_change[trainDirection[trainID]]; //Calculate next column

    if(!isInBounds(nextRow,nextColumn)||!isTrackTile(nextRow,nextColumn))
    return false;
     if(nextRow<0 || nextRow>=numRows || nextColumn<0 || nextColumn>=numColumns)
        return false;

    return true;
}

// ----------------------------------------------------------------------------
// GET NEXT DIRECTION based on current tile and direction
// ----------------------------------------------------------------------------
// Return new direction after entering the tile.
// ----------------------------------------------------------------------------
int getNextDirection(int trainID) {
    char track=grid[trainRow[trainID]][trainColumn[trainID]];   //Check the current tile a train is present on
    switch(track){
    case horizontal_track:
    if(trainDirection[trainID]==left_dir){
        return left_dir;
    }
    else{
        return right_dir;
    }
    case vertical_track:
    if(trainDirection[trainID]==up_dir){
        return up_dir;
    }
    else{
        return down_dir;
    }
    case right_curve:
    if(trainDirection[trainID]==up_dir) return right_dir;
    if(trainDirection[trainID]==left_dir) return down_dir;
    return trainDirection[trainID];
    case left_curve:
    if(trainDirection[trainID]==up_dir) return left_dir;
    if(trainDirection[trainID]==right_dir) return down_dir;
    return trainDirection[trainID];
    case crossing:
    return getSmartDirectionAtCrossing(trainID);
    default:return trainDirection[trainID];
    }
}

// ----------------------------------------------------------------------------
// SMART ROUTING AT CROSSING - Route train to its matched destination
// ----------------------------------------------------------------------------
// Choose best direction at '+' toward destination.
// ----------------------------------------------------------------------------
int getSmartDirectionAtCrossing(int trainID) {
    int row=trainRow[trainID];  //Current row and colum positions
    int column=trainColumn[trainID];
    int destRow=destinationRow[trainID];    //Destination train tries to reach  
    int destColumn=destinationColumn[trainID];
    int bestDir=trainDirection[trainID];    //Best path the train know to reach destination
    int minDistance=abs(row-destRow)+abs(column-destColumn);   //Manhattan distance
    for(int i=0;i<4;i++){
        int nrow= row+row_change[i];
        int ncolumn=column+column_change[i];
        // Check bounds
        if(nrow<0||nrow>=numRows||ncolumn<0||ncolumn>=numColumns) continue;
        //Check if tile is a rail
        char tile=grid[nrow][ncolumn];
        if(tile==empty_space) continue; //Train does not move on empty tiles

        int dist= abs(nrow-destRow)+abs(ncolumn-destColumn);    //Calculate distance from neigbouring tile to destination
        if(dist<minDistance){
            minDistance=dist;   //If smaller distance move towards it
            bestDir=i;
        }
    }

    return bestDir;
}

// ----------------------------------------------------------------------------
// DETERMINE ALL ROUTES (PHASE 2)
// ----------------------------------------------------------------------------
// Fill next positions/directions for all trains.
// ----------------------------------------------------------------------------
void determineAllRoutes() {
    for(int i=0;i<numTrains;i++){   //Move through all trains
        if(trainRow[i]==-1) continue;   //Skipping inactive trains(reached destination or not spawned yet)
        if(trainRow[i]==destinationRow[i] && trainColumn[i]==destinationColumn[i]){
            continue;   //Train already present at destination
        }
        //Determine next direction
        trainDirection[i] = getNextDirection(i);
    }
}

// ----------------------------------------------------------------------------
// MOVE ALL TRAINS (PHASE 5)
// ----------------------------------------------------------------------------
// Move trains; resolve collisions and apply effects.
// ----------------------------------------------------------------------------
void moveAllTrains() {
    //Store next positions for trains
    int nextRow[max_trains];
    int nextCol[max_trains];
    int nextDir[max_trains];
    
    for(int i = 0;i<numTrains;i++) {
        if(trainRow[i]==-1) { // inactive trains set to default 
            nextRow[i]=-1;
            nextCol[i]=-1;
            nextDir[i]=trainDirection[i];
            continue;
        }
        //Calculate next positions
       nextRow[i]=trainRow[i]+row_change[trainDirection[i]];
        nextCol[i]=trainColumn[i]+column_change[trainDirection[i]];
        nextDir[i]=trainDirection[i];
        //Check bounds 
        if (nextRow[i]<0||nextRow[i]>=numRows||nextCol[i]<0||nextCol[i]>=numColumns){
            nextRow[i]=trainRow[i];
            nextCol[i]=trainColumn[i];
        }
    }
    detectCollisions(nextRow,nextCol,nextDir);
    //Apply movement
    for(int i=0;i<numTrains;i++){
        if(nextRow[i]!=-1){     //Inactive trains skipped
            trainRow[i]=nextRow[i]; //Applies new position for all trains
            trainColumn[i]=nextCol[i];
            trainDirection[i]=nextDir[i];
        }
    }
}

// ----------------------------------------------------------------------------
// DETECT COLLISIONS WITH PRIORITY SYSTEM
// ----------------------------------------------------------------------------
// Resolve same-tile, swap, and crossing conflicts.
// ----------------------------------------------------------------------------
void detectCollisions(int nextRow[],int nextCol[],int nextDir[]) {
    for(int i=0;i<numTrains;i++){
        if(trainRow[i]==-1) continue;
        for(int j=i+1;j<numTrains;j++){
            if(nextRow[j]==-1) continue;
            if(nextRow[i]==nextRow[j] && nextCol[i]==nextCol[j]){
            char tile=grid[nextRow[i]][nextCol[i]];
            int dist_i=abs(nextRow[i]-destinationRow[i])+abs(nextCol[i]-destinationColumn[i]);
            int dist_j=abs(nextRow[j]-destinationRow[j])+abs(nextCol[j]-destinationColumn[j]);
            if(tile==crossing) {
            if(dist_i==dist_j) { //Collison
                nextRow[i]=trainRow[i];
                nextCol[i]=trainColumn[i];
                nextRow[j]=trainRow[j];
                nextCol[j]=trainColumn[j];
                trainsCrashed+=2;
            } else if(dist_i>dist_j) {
                nextRow[j]=trainRow[j];
                nextCol[j]=trainColumn[j];
            } else {
                nextRow[i]=trainRow[i];
                nextCol[i]=trainColumn[i];
            }
            } 
            else { //normal tile collision
                if(dist_i == dist_j) {
                //Both equally far then collision
                nextRow[i] = trainRow[i];
                nextCol[i] = trainColumn[i];
                nextRow[j] = trainRow[j];
                nextCol[j] = trainColumn[j];
                trainsCrashed += 2;
            } else if(dist_i > dist_j) {
                //i is farther then move i stop j
                nextRow[j] = trainRow[j];
                nextCol[j] = trainColumn[j];
            } 
            else {
                //j is farther then move j stop i
                nextRow[i] = trainRow[i];
                nextCol[i] = trainColumn[i];
                    }
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
    for(int i=0;i<numTrains;i++){   //Check all trains
        if(trainRow[i]==-1) continue;   //Skip inactive trains
        for(int j=0;j<numDestinations;j++){ 
            if(destinationTrainID[j]==i && trainRow[i]==destinationRow[j] &&
                trainColumn[i]==destinationColumn[j]){  //Check if train reached destination
                trainsReached++;    //If reached then increament
                trainRow[i]=-1;
                trainColumn[i]=-1;  //Mark that train as inactive
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
    for(int i=0;i<numTrains;i++){   //Check all trains
        if (trainRow[i]!=-1 && emergencyHalt[trainRow[i]][trainColumn[i]]>0){   //Check if tile has emergency halt
        if (trainWait[i]<emergencyHalt[trainRow[i]][trainColumn[i]]){   //Apply timer if halt is applied
            trainWait[i]=emergencyHalt[trainRow[i]][trainColumn[i]];
            }
        }
    }
}
// ----------------------------------------------------------------------------
// UPDATE EMERGENCY HALT
// ----------------------------------------------------------------------------
// Decrement timer and disable when done.
// ----------------------------------------------------------------------------
void updateEmergencyHalt() {
    for(int i=0;i<numRows;i++){ //Check every tile on grid
        for(int j=0;j<numColumns;j++){
            if(emergencyHalt[i][j]>0){      //Check if emergency halt is present
                emergencyHalt[i][j]--;      //Reduce the tick duration by 1
            }
        }
    }
}
