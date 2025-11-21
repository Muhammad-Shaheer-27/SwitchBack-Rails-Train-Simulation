#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

// ============================================================================
// SIMULATION_STATE.H - Global constants and state
// ============================================================================
// Global constants and arrays used by the game.
// ============================================================================

// ----------------------------------------------------------------------------
// GRID CONSTANTS
// ----------------------------------------------------------------------------
//Grid Size
const int max_Rows=50;
const int max_Columns=50;
//Tiles
const char empty_space='.';
const char horizontal_track='-';
const char vertical_track='|';
const char right_curve='/';
const char left_curve='\\';
const char crossing='+';
const char spawn='S';
const char destination='D';
//Direction variables
const int up_dir=0;
const int right_dir=1;
const int down_dir=2;
const int left_dir=3;
//Movement changes
int row_change[4]={-1,0,1,0};
int column_change[4]={0,1,0,-1};


// ----------------------------------------------------------------------------
// TRAIN CONSTANTS
// ----------------------------------------------------------------------------
const int max_trains=20;
const int max_colors=8;
//Train Directions
const int train_up=0;
const int train_right=1;
const int train_down=2;
const int train_left=3;
//TrainSpeed
const int train_speed=2;
const int train_speed_delay=1;


// ----------------------------------------------------------------------------
// SWITCH CONSTANTS
// ----------------------------------------------------------------------------
const int max_switches=26;
const int max_switches_state=2;
const char switch_start='A';
const char switch_end='Z';
//Switch Directions
const int K_up=0;
const int K_right=1;
const int K_down=2;
const int K_left=3;
const int switch_max_K=3;
//Switch modes
const int PER_DIR=0;
const int GLOBAL=1;
const int switchmode_per_dir=2;     //Number of modes a switch can have per direction

// ----------------------------------------------------------------------------
// WEATHER CONSTANTS
// ----------------------------------------------------------------------------
const int weather_normal=0;
const int weather_rain=1;
const int weather_fog=2;
const int weather_types=3;

// ----------------------------------------------------------------------------
// SIGNAL CONSTANTS
// ----------------------------------------------------------------------------
const int signal_green=0;
const int signal_yellow=1;
const int sigal_red=2;
const int max_signals=3;


// ----------------------------------------------------------------------------
// GLOBAL STATE: GRID
// ----------------------------------------------------------------------------
int numRows;
int numColumns;
char grid[max_Rows][max_Columns];
int safetyDelay[max_Rows][max_Columns];    //Remaining ticks on a =tile 

// ----------------------------------------------------------------------------
// GLOBAL STATE: TRAINS
// ----------------------------------------------------------------------------
int numTrains;
int trainRow[max_trains];       //Row index of each train
int trainColumn[max_trains];    //Column index of each train
int trainColor[max_trains];
int trainDirection[max_trains];
int trainWait[max_trains]; //Ticks left to wait

// ----------------------------------------------------------------------------
// GLOBAL STATE: SWITCHES (A-Z mapped to 0-25)
// ----------------------------------------------------------------------------
int numSwitches;
char switchLetter[max_switches];
int switchState[max_switches];
int switchMode[max_switches];
int switchCounter[max_switches][4];//Counter for perdirection for each switch(0,1,2,3)
int switchK[max_switches][4];//K value for each switch perdirection(entries left before flip)
int switchFlipped[max_switches];//Check if switch will flip

// ----------------------------------------------------------------------------
// GLOBAL STATE: SPAWN POINTS
// ----------------------------------------------------------------------------
int numSpawn;
//Spawn Position
int spawnRow[max_trains];
int spawnColumn[max_trains];
//Tick for Spawn of Train
int spawnTick[max_trains];
//Train index on spawn point
int spawnTrainID[max_trains];

// ----------------------------------------------------------------------------
// GLOBAL STATE: DESTINATION POINTS
// ----------------------------------------------------------------------------
int numDestinations;
//Destination Position
int destinationRow[max_trains];
int destinationColumn[max_trains];
//Train index for destination
int destinationTrainID[max_trains];

// ----------------------------------------------------------------------------
// GLOBAL STATE: SIMULATION PARAMETERS
// ----------------------------------------------------------------------------
int currentTick;
int totalTicks;
int levelSeed;
int weatherType;
int simulationRunning;

// ----------------------------------------------------------------------------
// GLOBAL STATE: METRICS
// ----------------------------------------------------------------------------
int trainsReached;
int trainsCrashed;
int totalWaitTicks;
int totalEnergy;
int switchFlips;
int signalViolations;

// ----------------------------------------------------------------------------
// GLOBAL STATE: EMERGENCY HALT
// ----------------------------------------------------------------------------
int emergencyHalt[max_Rows][max_Columns];  //Ticks remaining for halt
int emergencyHaltActive;

// ----------------------------------------------------------------------------
// INITIALIZATION FUNCTION
// ----------------------------------------------------------------------------
// Resets all state before loading a new level.
void initializeSimulationState();

#endif
