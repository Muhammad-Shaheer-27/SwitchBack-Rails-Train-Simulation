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
const int maximum_rows=70;
const int maximum_Columns=70;
//Tiles
const char space=' ';
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
extern const int row_change[4];
extern const int column_change[4];


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
const int maximum_switches=26;
const int max_switches_state=2;
const char start_switch='A';
const char end_switch='Z';
//Switch Directions
const int K_up=0;
const int K_right=1;
const int K_down=2;
const int K_left=3;
const int switch_max_K=3;
//Switch modes
const int PER_DIR=0;
const int GLOBAL=1;
const int switchmode_per_dir=PER_DIR;     //Number of modes a switch can have per direction

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
extern int number_column;
extern int number_rows;
extern char grid[maximum_rows][maximum_Columns];
extern int safetyDelay[maximum_rows][maximum_Columns];    //Remaining ticks on a =tile 

// ----------------------------------------------------------------------------
// GLOBAL STATE: TRAINS
// ----------------------------------------------------------------------------
extern int numOf_trains;
extern int trainRow[max_trains];       //Row index of each train
extern int trainColumn[max_trains];    //Column index of each train
extern int trainColor[max_trains];
extern int trainDirection[max_trains];
extern int trainWait[max_trains]; //Ticks left to wait

// ----------------------------------------------------------------------------
// GLOBAL STATE: SWITCHES (A-Z mapped to 0-25)
// ----------------------------------------------------------------------------
extern int numSwitches;
extern int switchSignal[maximum_switches];
extern char switchLetter[maximum_switches];
extern int switchState[maximum_switches];
extern int switchMode[maximum_switches];
extern int switchCounter[maximum_switches][4];//Counter for perdirection for each switch(0,1,2,3)
extern int switchK[maximum_switches][4];//K value for each switch perdirection(entries left before flip)
extern int switchFlipped[maximum_switches];//Check if switch will flip
extern int switchRouting[maximum_switches][4][2];


// ----------------------------------------------------------------------------
// GLOBAL STATE: SPAWN POINTS
// ----------------------------------------------------------------------------
extern int num_spawn;
//Spawn Position
extern int spawnn_Row[max_trains];
extern int spawnn_Column[max_trains];
//Tick for Spawn of Train
extern int spawnTick[max_trains];
//Train index on spawn point
extern int spawnTrainID[max_trains];
extern int spawnDirection[max_trains];
extern int spawnColor[max_trains];

// ----------------------------------------------------------------------------
// GLOBAL STATE: DESTINATION POINTS
// ----------------------------------------------------------------------------
extern int numDest;
//Destination Position
extern int destinationRow[max_trains];
extern int destinationColumn[max_trains];
//Train index for destination
extern int destinationTrainID[max_trains];

// ----------------------------------------------------------------------------
// GLOBAL STATE: SIMULATION PARAMETERS
// ----------------------------------------------------------------------------
extern int currentTick;
extern int totalTicks;
extern int levelSeed;
extern int weather_type;
extern int simulationRunning;

// ----------------------------------------------------------------------------
// GLOBAL STATE: METRICS
// ----------------------------------------------------------------------------
extern int trainsReached;
extern int crashed_trains;
extern int totalWaitTicks;
extern int T_energy;
extern int switchFlips;
extern int signalViolations;

// ----------------------------------------------------------------------------
// GLOBAL STATE: EMERGENCY HALT
// ----------------------------------------------------------------------------
extern int emergencyHalt[maximum_rows][maximum_Columns];  //Ticks remaining for halt
extern int emergencyHaltActive;

// ----------------------------------------------------------------------------
// INITIALIZATION FUNCTION
// ----------------------------------------------------------------------------
// Resets all state before loading a new level.
void initializeSimulationState();

#endif