#include "io.h"
#include "simulation_state.h"
#include <iostream>
#include <fstream>
#include <string>
#include <limits>

using namespace std;

// ============================================================================
// IO.CPP - Level I/O (Smart Version)
// ============================================================================

// Helper to read trains
void readTrains(ifstream &file) {
    cout << "DEBUG: Reading TRAINS section..." << endl;
    while (file >> spawnTick[numSpawn]) {
        file >> spawnColumn[numSpawn];    
        file >> spawnRow[numSpawn];       
        file >> spawnDirection[numSpawn]; 
        file >> spawnColor[numSpawn];     
        
        spawnTrainID[numSpawn] = -1;      
        cout << "DEBUG: Loaded Train " << numSpawn << " at Tick " << spawnTick[numSpawn] << endl;
        numSpawn++;
    }
}

bool loadLevelFile(string filename) {
    ifstream file(filename);
    
    if (!file.is_open()) {
        cout << "Error: Could not open level file: " << filename << endl;
        return false;
    }

    string key;
    while (file >> key) {
        
        if (key == "ROWS:") file >> numRows;
        else if (key == "COLS:") file >> numColumns;
        else if (key == "SEED:") file >> levelSeed;
        
        else if (key == "WEATHER:") {
            string w;
            file >> w;
            if (w == "NORMAL") weatherType = weather_normal;
            else if (w == "RAIN") weatherType = weather_rain;
            else if (w == "FOG") weatherType = weather_fog;
        }
        
        // --- SMART MAP READER ---
        else if (key == "MAP:") {
            cout << "DEBUG: Reading MAP..." << endl;
            file.ignore(numeric_limits<streamsize>::max(), '\n'); // Skip rest of "MAP:" line
            
            for (int r = 0; r < numRows; r++) {
                // 1. PEEK AHEAD: Is the next word a Keyword?
                // We save the current file position
                streampos oldPos = file.tellg();
                string checkKeyword;
                file >> checkKeyword; 

                if (checkKeyword == "SWITCHES:" || checkKeyword == "TRAINS:") {
                    // If we found a keyword, STOP reading the map!
                    // Go back to before we read the keyword so the main loop handles it.
                    file.seekg(oldPos); 
                    break; 
                }

                // 2. If not a keyword, go back and read the line as a Map Row
                file.seekg(oldPos);
                string line;
                getline(file, line);
                
                for (int c = 0; c < numColumns; c++) {
                    if (c < (int)line.length()) {
                        grid[r][c] = line[c];
                    } else {
                        grid[r][c] = '.'; 
                    }
                }
            }
        }
        // ------------------------
        
        else if (key == "SWITCHES:") {
            cout << "DEBUG: Reading SWITCHES..." << endl;
            while (true) {
                string temp;
                if (!(file >> temp)) break; 

                if (temp == "TRAINS:") {
                    readTrains(file); 
                    break; 
                }

                int index = numSwitches;
                switchLetter[index] = temp[0]; 

                string modeStr;
                file >> modeStr;
                if (modeStr == "PER_DIR") switchMode[index] = switchmode_per_dir; 
                else switchMode[index] = 1; 
                
                file >> switchState[index]; 
                
                for(int i=0; i<4; i++) {
                    file >> switchK[index][i];
                    switchCounter[index][i] = 0;
                }
                
                // Consume "STRAIGHT TURN" labels
                string label1, label2;
                file >> label1 >> label2;
                
                switchFlipped[index] = 0;
                numSwitches++;
            }
        }

        else if (key == "TRAINS:") {
            readTrains(file);
        }
    }
    
    file.close();
    
    // AUTO ASSIGN DESTINATIONS
    numDestinations = 0;
    int dRows[max_trains];
    int dCols[max_trains];
    int dCount = 0;

    for(int r = 0; r < numRows; r++) {
        for(int c = 0; c < numColumns; c++) {
            if(grid[r][c] == 'D') {
                dRows[dCount] = r;
                dCols[dCount] = c;
                dCount++;
            }
        }
    }

    for(int i = 0; i < numSpawn; i++) { 
        int targetIndex = 0; 
        for(int k = 0; k < dCount; k++) {
            if(dRows[k] == spawnRow[i]) {
                targetIndex = k;
                break;
            }
        }
        destinationRow[numDestinations] = dRows[targetIndex];
        destinationColumn[numDestinations] = dCols[targetIndex];
        destinationTrainID[numDestinations] = i;
        numDestinations++;
    }

    cout << "Level loaded: " << filename << endl;
    return true;
}

// ----------------------------------------------------------------------------
// LOGGING FUNCTIONS
// ----------------------------------------------------------------------------
void initializeLogFiles() {
    ofstream trace("trace.csv");
    if (trace.is_open()) { trace << "Tick,TrainID,X,Y,Direction,State\n"; trace.close(); }
    ofstream switches("switches.csv");
    if (switches.is_open()) { switches << "Tick,Switch,Mode,State\n"; switches.close(); }
    ofstream signals("signals.csv");
    if (signals.is_open()) { signals << "Tick,Switch,Signal\n"; signals.close(); }
}

void logTrainTrace() {
    ofstream file("trace.csv", ios::app);
    if (!file.is_open()) return;
    for (int i = 0; i < numTrains; i++) {
        if (trainRow[i] != -1) { 
            file << currentTick << "," << i << "," << trainColumn[i] << "," 
                 << trainRow[i] << "," << trainDirection[i] << "," << trainWait[i] << "\n"; 
        }
    }
    file.close();
}

void logSwitchState() {
    ofstream file("switches.csv", ios::app);
    if (!file.is_open()) return;
    for (int i = 0; i < numSwitches; i++) {
        file << currentTick << "," << switchLetter[i] << "," << switchMode[i] << "," << switchState[i] << "\n";
    }
    file.close();
}

void logSignalState() {
    ofstream file("signals.csv", ios::app);
    if (!file.is_open()) return;
    for (int i = 0; i < numSwitches; i++) {
         file << currentTick << "," << switchLetter[i] << "," << "GREEN" << "\n"; 
    }
    file.close();
}

void writeMetrics() {
    ofstream file("metrics.txt");
    if (!file.is_open()) return;
    file << "Simulation Metrics\n------------------\n";
    file << "Trains Reached: " << trainsReached << "\n";
    file << "Trains Crashed: " << trainsCrashed << "\n";
    file.close();
}