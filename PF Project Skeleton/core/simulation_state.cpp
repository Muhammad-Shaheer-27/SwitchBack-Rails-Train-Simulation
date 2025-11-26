#include "simulation_state.h"
#include <cstring>

//Movement changes
const int row_change[4]={-1,0,1,0};
const int column_change[4]={0,1,0,-1};

//Grid Variables
int numRows;
int numColumns;
char grid[max_Rows][max_Columns];
int safetyDelay[max_Rows][max_Columns];

//Trains variables
int numTrains;
int trainRow[max_trains];
int trainColumn[max_trains];
int trainColor[max_trains];
int trainDirection[max_trains];
int trainWait[max_trains];

//Switch Variables
int numSwitches;
char switchLetter[max_switches];
int switchState[max_switches];
int switchMode[max_switches];
int switchCounter[max_switches][4];
int switchK[max_switches][4];
int switchFlipped[max_switches];
int switchSignal[max_switches];

//Spawn point variables
int numSpawn;
int spawnRow[max_trains];
int spawnColumn[max_trains];
int spawnTick[max_trains];
int spawnTrainID[max_trains];
int spawnDirection[max_trains];
int spawnColor[max_trains];

//Destination point variables
int numDestinations;
int destinationRow[max_trains];
int destinationColumn[max_trains];
int destinationTrainID[max_trains];

//Simulation parameters
int currentTick;
int totalTicks;
int levelSeed;
int weatherType;
int simulationRunning;

//Data metric
int trainsReached;
int trainsCrashed;
int totalWaitTicks;
int totalEnergy;
int switchFlips;
int signalViolations;

//Emergency halt
int emergencyHalt[max_Rows][max_Columns];
int emergencyHaltActive;
// ============================================================================
// INITIALIZE SIMULATION STATE
// ============================================================================
// ----------------------------------------------------------------------------
// Resets all global simulation state.
// ----------------------------------------------------------------------------
// Called before loading a new level.
// ----------------------------------------------------------------------------
    // ============================================================================
// SIMULATION_STATE.CPP - Global state definitions
// ============================================================================
void initializeSimulationState(){
// ----------------------------------------------------------------------------
// GRID
// ----------------------------------------------------------------------------
    numRows=0;
    numColumns=0;
    for(int i=0;i<max_Rows;i++){
        for(int j=0;j<max_Columns;j++){
            grid[i][j]=empty_space;
            safetyDelay[i][j] = 0;
        }
    }

// ----------------------------------------------------------------------------
// TRAINS
// ----------------------------------------------------------------------------
    numTrains=0;
    for(int i=0;i<max_trains;i++){
        trainRow[i]=-1;
        trainColumn[i]=-1;
        trainDirection[i]=train_right;
        trainColor[i]=0;
        trainWait[i]=0; 
    }

// ----------------------------------------------------------------------------
// SWITCHES
// ----------------------------------------------------------------------------
    numSwitches=0;
    for(int i=0;i<max_switches;i++){
        switchLetter[i]='A'+i;
        switchState[i]=0;
        switchMode[i]=switchmode_per_dir;
        switchFlipped[i]=0;
        for(int j=0;j<4;j++){
            switchCounter[i][j]=0;
            switchK[i][j]=0;
        }
    }
// ----------------------------------------------------------------------------
// SPAWN AND DESTINATION POINTS
// ----------------------------------------------------------------------------
    numSpawn=0;
    numDestinations=0;
    for(int i=0;i<max_trains;i++){
        spawnRow[i]=-1;
        spawnColumn[i]=-1;
        spawnTick[i]=0;
        spawnTrainID[i]=-1;
        spawnDirection[i] = train_right; //Default direction for train
        spawnColor[i] = 0; 
        destinationRow[i]=-1;
        destinationColumn[i]=-1;
        destinationTrainID[i]=-1;
    }
// ----------------------------------------------------------------------------
// SIMULATION PARAMETERS
// ----------------------------------------------------------------------------
    currentTick=0;
    totalTicks=0;
    levelSeed=0;
    weatherType=weather_normal;
    simulationRunning=0;
// ----------------------------------------------------------------------------
// METRICS
// ----------------------------------------------------------------------------
    trainsReached=0;
    trainsCrashed=0;
    totalWaitTicks=0;
    totalEnergy=0;
    switchFlips=0;
    signalViolations=0;
// ----------------------------------------------------------------------------
// EMERGENCY HALT
// ----------------------------------------------------------------------------
    emergencyHaltActive=0;
    for(int i=0;i<max_Rows;i++){
        for(int j=0;j<max_Columns;j++){
            emergencyHalt[i][j]=0;
        }
    }
// ============================================================================
// INITIALIZE SIMULATION STATE
// ============================================================================
// ----------------------------------------------------------------------------
// Resets all global simulation state.
// ----------------------------------------------------------------------------
// Called before loading a new level.
// ----------------------------------------------------------------------------
}
