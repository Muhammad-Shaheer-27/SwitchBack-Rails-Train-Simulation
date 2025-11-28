#include "grid.h"
#include "simulation_state.h"

// ============================================================================
// GRID.CPP - Grid utilities
// ============================================================================

// ----------------------------------------------------------------------------
// Check if a position is inside the grid.
// ----------------------------------------------------------------------------
// Returns true if x,y are within bounds.
// ----------------------------------------------------------------------------
bool isInBounds(int i,int j) {
    return(i>=0 && i<numRows && j>=0 && j<numColumns);
}
// ----------------------------------------------------------------------------
// Check if a tile is a track tile.
// ----------------------------------------------------------------------------
// Returns true if the tile can be traversed by trains.
// ----------------------------------------------------------------------------
bool isTrackTile(int i,int j) {
    if(!isInBounds(i,j)) return 0;
    char tile=grid[i][j];
    return(tile==horizontal_track ||tile==vertical_track||tile==right_curve||
    tile==left_curve||tile==crossing||tile==spawn||tile==destination||tile=='=');
}

// ----------------------------------------------------------------------------
// Check if a tile is a switch.
// ----------------------------------------------------------------------------
// Returns true if the tile is 'A'..'Z'.
// ----------------------------------------------------------------------------
bool isSwitchTile(int i,int j) {
    if(!isInBounds(i,j)) return 0;
    char tile=grid[i][j];
    return(tile>=switch_start && tile<=switch_end);
}

// ----------------------------------------------------------------------------
// Get switch index from character.
// ----------------------------------------------------------------------------
// Maps 'A'..'Z' to 0..25, else -1.
// ----------------------------------------------------------------------------
int getSwitchIndex(int i,int j) {
    if(!isSwitchTile(i,j)) return -1;
    return grid[i][j]-switch_start;
}

// ----------------------------------------------------------------------------
// Check if a position is a spawn point.
// ----------------------------------------------------------------------------
// Returns true if x,y is a spawn.
// ----------------------------------------------------------------------------
bool isSpawnPoint(int i,int j) {
    if(!isInBounds(i,j)) return 0;
    for(int k=0;k<numSpawn;k++){
        if(spawnRow[k]==i && spawnColumn[k]==j){
            return 1;
        }
    }
    return 0;
}

// ----------------------------------------------------------------------------
// Check if a position is a destination.
// ----------------------------------------------------------------------------
// Returns true if x,y is a destination.
// ----------------------------------------------------------------------------
bool isDestinationPoint(int i,int j) {
    if(!isInBounds(i,j)) return 0;
    for(int k=0;k<numDestinations;k++){
        if(destinationRow[k]==i && destinationColumn[k]==j){
            return 1;
        }
    }
    return 0;
}

// ----------------------------------------------------------------------------
// Toggle a safety tile.
// ----------------------------------------------------------------------------
// Returns true if toggled successfully.
// ----------------------------------------------------------------------------
bool toggleSafetyTile(int i,int j) {
    if(!isInBounds(i,j)) return 0;
    if(grid[i][j]==empty_space){
        grid[i][j]='=';
        safetyDelay[i][j]=1;
    }
    else if(grid[i][j]=='='){
        grid[i][j]=empty_space;
        safetyDelay[i][j]=0;
    }
    else{
        return 0;
    }
    return 1;
}
