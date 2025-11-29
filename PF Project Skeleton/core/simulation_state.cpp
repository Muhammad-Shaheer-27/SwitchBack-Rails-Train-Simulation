#include "simulation_state.h"
#include <cstring>

//Movement changes
const int row_change[4]={-1,0,1,0};
const int column_change[4]={0,1,0,-1};

//Grid Variables
int number_column;
int number_rows;
char grid[maximum_rows][maximum_Columns];
int safetyDelay[maximum_rows][maximum_Columns];

//Trains variables
int numOf_trains;
int trainRow[max_trains];
int trainColumn[max_trains];
int trainColor[max_trains];
int trainDirection[max_trains];
int trainWait[max_trains];

//Switch Variables
int numSwitches;
char switchLetter[maximum_switches];
int switchState[maximum_switches];
int switchMode[maximum_switches];
int switchCounter[maximum_switches][4];
int switchK[maximum_switches][4];
int switchFlipped[maximum_switches];
int switchSignal[maximum_switches];

//Spawn point variables
int num_spawn;
int spawnn_Row[max_trains];
int spawnn_Column[max_trains];
int spawnTick[max_trains];
int spawnTrainID[max_trains];
int spawnDirection[max_trains];
int spawnColor[max_trains];

//Destination point variables
int numDest;
int destinationRow[max_trains];
int destinationColumn[max_trains];
int destinationTrainID[max_trains];

//Simulation parameters
int currentTick;
int totalTicks;
int levelSeed;
int weather_type;
int simulationRunning;

//Data metric
int trainsReached;
int crashed_trains;
int totalWaitTicks;
int T_energy;
int switchFlips;
int signalViolations;

//Emergency halt
int emergencyHalt[maximum_rows][maximum_Columns];
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
    number_column=0;
    number_rows=0;
    for(int i=0;i<maximum_rows;i++){
        for(int j=0;j<maximum_Columns;j++){
            grid[i][j]=space;
            safetyDelay[i][j] = 0;
        }
    }

// ----------------------------------------------------------------------------
// TRAINS
// ----------------------------------------------------------------------------
    numOf_trains=0;
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
    for(int i=0;i<maximum_switches;i++){
        switchLetter[i]='A'+i;
        switchState[i]=0;
        switchMode[i]=switchmode_per_dir;
        switchFlipped[i]=0;
        switchSignal[i] = signal_green;
        for(int j=0;j<4;j++){
            switchCounter[i][j]=0;
            switchK[i][j]=0;
        }
    }
// ----------------------------------------------------------------------------
// SPAWN AND DESTINATION POINTS
// ----------------------------------------------------------------------------
    num_spawn=0;
    numDest=0;
    for(int i=0;i<max_trains;i++){
        spawnn_Row[i]=-1;
        spawnn_Column[i]=-1;
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
    weather_type=weather_normal;
    simulationRunning=0;
// ----------------------------------------------------------------------------
// METRICS
// ----------------------------------------------------------------------------
    trainsReached=0;
    crashed_trains=0;
    totalWaitTicks=0;
    T_energy=0;
    switchFlips=0;
    signalViolations=0;
// ----------------------------------------------------------------------------
// EMERGENCY HALT
// ----------------------------------------------------------------------------
    emergencyHaltActive=0;
    for(int i=0;i<maximum_rows;i++){
        for(int j=0;j<maximum_Columns;j++){
            emergencyHalt[i][j]=0;
        }
    }
}
