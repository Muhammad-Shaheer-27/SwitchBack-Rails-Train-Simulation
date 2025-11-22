#include "io.h"
#include "simulation_state.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// ============================================================================
// IO.CPP - Level I/O and logging
// ============================================================================

// ----------------------------------------------------------------------------
// LOAD LEVEL FILE
// ----------------------------------------------------------------------------
// Load a .lvl file into global state.
// ----------------------------------------------------------------------------
bool loadLevelFile(string filename) {
    ifstream file(filename);
    
    if (!file.is_open()) {
        cout << "Error: Could not open level file: " << filename << endl;
        return false;
    }

    string key;
    while (file >> key) {
        
        // READ DIMENSIONS
        if (key == "ROWS:") file >> numRows;
        else if (key == "COLS:") file >> numColumns;
        else if (key == "SEED:") file >> levelSeed;
        
        // READ WEATHER
        else if (key == "WEATHER:") {
            string w;
            file >> w;
            if (w == "NORMAL") weatherType = weather_normal;
            else if (w == "RAIN") weatherType = weather_rain;
            else if (w == "FOG") weatherType = weather_fog;
        }
        
        // READ MAP
        else if (key == "MAP:") {
            for (int r = 0; r < numRows; r++) {
                string line;
                file >> line;
                for (int c = 0; c < numColumns; c++) {
                    grid[r][c] = line[c];
                }
            }
        }
        
        // READ SWITCHES (AND TRAINS)
        else if (key == "SWITCHES:") {
            while (true) {
                string temp;
                file >> temp; 

                // Check if we reached the TRAINS section
                if (temp == "TRAINS:") {
                    while (file >> spawnTick[numSpawn]) {
                        file >> spawnColumn[numSpawn];
                        file >> spawnRow[numSpawn];
                        file >> spawnDirection[numSpawn];
                        file >> spawnColor[numSpawn];
                        numSpawn++;
                    }
                    break;
                }

                // Process Switch
                int index = numSwitches;
                switchLetter[index] = temp[0]; 

                string modeStr;
                file >> modeStr;
                if (modeStr == "PER_DIR") switchMode[index] = switchmode_per_dir; 
                else switchMode[index] = 1; // Global
                
                file >> switchState[index]; 
                
                for(int i=0; i<4; i++) {
                    file >> switchK[index][i];
                    switchCounter[index][i] = 0;
                }
                switchFlipped[index] = 0;
                numSwitches++;
            }
        }
    }
    
    file.close();
    cout << "Level loaded: " << filename << endl;
    return true;
}

// ----------------------------------------------------------------------------
// INITIALIZE LOG FILES
// ----------------------------------------------------------------------------
// Create/clear CSV logs with headers.
// ----------------------------------------------------------------------------
void initializeLogFiles() {
    ofstream trace("trace.csv");
    if (trace.is_open()) {
        trace << "Tick,TrainID,X,Y,Direction,State\n";
        trace.close();
    }

    ofstream switches("switches.csv");
    if (switches.is_open()) {
        switches << "Tick,Switch,Mode,State\n";
        switches.close();
    }

    ofstream signals("signals.csv");
    if (signals.is_open()) {
        signals << "Tick,Switch,Signal\n";
        signals.close();
    }
}

// ----------------------------------------------------------------------------
// LOG TRAIN TRACE
// ----------------------------------------------------------------------------
// Append tick, train id, position, direction, state to trace.csv.
// ----------------------------------------------------------------------------
void logTrainTrace() {
    ofstream file("trace.csv", ios::app);
    if (!file.is_open()) return;

    for (int i = 0; i < numTrains; i++) {
        // Only log active trains (where Row is not -1)
        if (trainRow[i] != -1) { 
            file << currentTick << ","
                 << i << "," 
                 << trainColumn[i] << "," 
                 << trainRow[i] << ","   
                 << trainDirection[i] << ","
                 << trainWait[i] << "\n"; 
        }
    }
    file.close();
}

// ----------------------------------------------------------------------------
// LOG SWITCH STATE
// ----------------------------------------------------------------------------
// Append tick, switch id/mode/state to switches.csv.
// ----------------------------------------------------------------------------
void logSwitchState() {
    ofstream file("switches.csv", ios::app);
    if (!file.is_open()) return;

    for (int i = 0; i < numSwitches; i++) {
        file << currentTick << ","
             << switchLetter[i] << ","
             << switchMode[i] << ","
             << switchState[i] << "\n";
    }
    file.close();
}

// ----------------------------------------------------------------------------
// LOG SIGNAL STATE
// ----------------------------------------------------------------------------
// Append tick, switch id, signal color to signals.csv.
// ----------------------------------------------------------------------------
void logSignalState() {
    ofstream file("signals.csv", ios::app);
    if (!file.is_open()) return;

    for (int i = 0; i < numSwitches; i++) {
         // Placeholder: We will add actual signal logic later
         file << currentTick << ","
              << switchLetter[i] << ","
              << "GREEN" << "\n"; 
    }
    file.close();
}

// ----------------------------------------------------------------------------
// WRITE FINAL METRICS
// ----------------------------------------------------------------------------
// Write summary metrics to metrics.txt.
// ----------------------------------------------------------------------------
#include "io.h"
#include "simulation_state.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// ============================================================================
// IO.CPP - Level I/O and logging
// ============================================================================

// ----------------------------------------------------------------------------
// LOAD LEVEL FILE
// ----------------------------------------------------------------------------
// Load a .lvl file into global state.
// ----------------------------------------------------------------------------
bool loadLevelFile(string filename) {
    ifstream file(filename);
    
    if (!file.is_open()) {
        cout << "Error: Could not open level file: " << filename << endl;
        return false;
    }

    string key;
    while (file >> key) {
        
        // READ DIMENSIONS
        if (key == "ROWS:") file >> numRows;
        else if (key == "COLS:") file >> numColumns;
        else if (key == "SEED:") file >> levelSeed;
        
        // READ WEATHER
        else if (key == "WEATHER:") {
            string w;
            file >> w;
            if (w == "NORMAL") weatherType = weather_normal;
            else if (w == "RAIN") weatherType = weather_rain;
            else if (w == "FOG") weatherType = weather_fog;
        }
        
        // READ MAP
        else if (key == "MAP:") {
            for (int r = 0; r < numRows; r++) {
                string line;
                file >> line;
                for (int c = 0; c < numColumns; c++) {
                    grid[r][c] = line[c];
                }
            }
        }
        
        // READ SWITCHES (AND TRAINS)
        else if (key == "SWITCHES:") {
            while (true) {
                string temp;
                file >> temp; 

                // Check if we reached the TRAINS section
                if (temp == "TRAINS:") {
                    while (file >> spawnTick[numSpawn]) {
                        file >> spawnColumn[numSpawn];
                        file >> spawnRow[numSpawn];
                        file >> spawnDirection[numSpawn];
                        file >> spawnColor[numSpawn];
                        numSpawn++;
                    }
                    break;
                }

                // Process Switch
                int index = numSwitches;
                switchLetter[index] = temp[0]; 

                string modeStr;
                file >> modeStr;
                if (modeStr == "PER_DIR") switchMode[index] = switchmode_per_dir; 
                else switchMode[index] = 1; // Global
                
                file >> switchState[index]; 
                
                for(int i=0; i<4; i++) {
                    file >> switchK[index][i];
                    switchCounter[index][i] = 0;
                }
                switchFlipped[index] = 0;
                numSwitches++;
            }
        }
    }
    
    file.close();
    cout << "Level loaded: " << filename << endl;
    return true;
}

// ----------------------------------------------------------------------------
// INITIALIZE LOG FILES
// ----------------------------------------------------------------------------
// Create/clear CSV logs with headers.
// ----------------------------------------------------------------------------
void initializeLogFiles() {
    ofstream trace("trace.csv");
    if (trace.is_open()) {
        trace << "Tick,TrainID,X,Y,Direction,State\n";
        trace.close();
    }

    ofstream switches("switches.csv");
    if (switches.is_open()) {
        switches << "Tick,Switch,Mode,State\n";
        switches.close();
    }

    ofstream signals("signals.csv");
    if (signals.is_open()) {
        signals << "Tick,Switch,Signal\n";
        signals.close();
    }
}

// ----------------------------------------------------------------------------
// LOG TRAIN TRACE
// ----------------------------------------------------------------------------
// Append tick, train id, position, direction, state to trace.csv.
// ----------------------------------------------------------------------------
void logTrainTrace() {
    ofstream file("trace.csv", ios::app);
    if (!file.is_open()) return;

    for (int i = 0; i < numTrains; i++) {
        // Only log active trains (where Row is not -1)
        if (trainRow[i] != -1) { 
            file << currentTick << ","
                 << i << "," 
                 << trainColumn[i] << "," 
                 << trainRow[i] << ","   
                 << trainDirection[i] << ","
                 << trainWait[i] << "\n"; 
        }
    }
    file.close();
}

// ----------------------------------------------------------------------------
// LOG SWITCH STATE
// ----------------------------------------------------------------------------
// Append tick, switch id/mode/state to switches.csv.
// ----------------------------------------------------------------------------
void logSwitchState() {
    ofstream file("switches.csv", ios::app);
    if (!file.is_open()) return;

    for (int i = 0; i < numSwitches; i++) {
        file << currentTick << ","
             << switchLetter[i] << ","
             << switchMode[i] << ","
             << switchState[i] << "\n";
    }
    file.close();
}

// ----------------------------------------------------------------------------
// LOG SIGNAL STATE
// ----------------------------------------------------------------------------
// Append tick, switch id, signal color to signals.csv.
// ----------------------------------------------------------------------------
void logSignalState() {
    ofstream file("signals.csv", ios::app);
    if (!file.is_open()) return;

    for (int i = 0; i < numSwitches; i++) {
         // Placeholder: We will add actual signal logic later
         file << currentTick << ","
              << switchLetter[i] << ","
              << "GREEN" << "\n"; 
    }
    file.close();
}

// ----------------------------------------------------------------------------
// WRITE FINAL METRICS
// ----------------------------------------------------------------------------
// Write summary metrics to metrics.txt.
// ----------------------------------------------------------------------------
void writeMetrics() {
    ofstream file("metrics.txt");
    if (!file.is_open()) return;

    file << "Simulation Metrics\n";
    file << "------------------\n";
    file << "Trains Reached: " << trainsReached << "\n";
    file << "Trains Crashed: " << trainsCrashed << "\n";
    file << "Total Wait Ticks: " << totalWaitTicks << "\n";
    file << "Total Energy: " << totalEnergy << "\n";
    file << "Switch Flips: " << switchFlips << "\n";
    file << "Signal Violations: " << signalViolations << "\n";
    
    file.close();
}