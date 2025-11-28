#include "io.h"
#include "simulation_state.h"
#include "grid.h"
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

// ============================================================================
// IO.CPP - Level I/O and logging
// ============================================================================

bool loadLevelFile(string filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open level file." << endl;
        return false;
    }

    // Reset grid to empty dots
    for (int r = 0; r < max_Rows; r++) {
        for (int c = 0; c < max_Columns; c++) {
            grid[r][c] = '.';
        }
    }

    // Reset counters
    numSwitches = 0;
    numSpawn = 0;
    numDestinations = 0;

    string temp;
    string bufferedToken = "";      
    bool hasBufferedToken = false;  

    // Main loop: Reads file one word at a time
    while (true) {
        
        if (hasBufferedToken) {
            temp = bufferedToken;
            hasBufferedToken = false;
        } else {
            if (!(file >> temp)) {
                break; 
            }
        }

        // 1. READ DIMENSIONS
        if (temp == "ROWS:") file >> numRows;
        else if (temp == "COLS:") file >> numColumns;
        else if (temp == "SEED:") file >> levelSeed;
        else if (temp == "WEATHER:") {
            string w;
            file >> w;
            if (w == "RAIN") weatherType = weather_rain;
            else if (w == "FOG") weatherType = weather_fog;
            else weatherType = weather_normal;
        }

        // 2. READ MAP
        else if (temp == "MAP:") {
            string dummy;
            getline(file, dummy);

            for (int r = 0; r < numRows; r++) {
                string line;
                getline(file, line);

                int len = 0;
                while (line[len] != '\0') len++;

                bool isSwitchHeader = false;
                bool isTrainHeader = false;

                if (len >= 9) {
                    if (line[0]=='S' && line[1]=='W' && line[2]=='I' && line[3]=='T') isSwitchHeader = true;
                }
                if (len >= 7) {
                    if (line[0]=='T' && line[1]=='R' && line[2]=='A' && line[3]=='I') isTrainHeader = true;
                }

                if (isSwitchHeader) {
                    bufferedToken = "SWITCHES:";
                    hasBufferedToken = true;
                    break;
                }
                if (isTrainHeader) {
                    bufferedToken = "TRAINS:";
                    hasBufferedToken = true;
                    break;
                }

                for (int c = 0; c < numColumns; c++) {
                    if (c < len) {
                        if (line[c] != '\r') {
                            grid[r][c] = line[c];
                        }
                    } else {
                        grid[r][c] = '.';
                    }
                }
            }
        }

        // 3. READ SWITCHES
        else if (temp == "SWITCHES:") {
            while (true) {
                string nextWord;
                if (!(file >> nextWord)) break;

                if (nextWord == "TRAINS:") {
                    bufferedToken = "TRAINS:";
                    hasBufferedToken = true;
                    break; 
                }

                int index = numSwitches;
                switchLetter[index] = nextWord[0];

                string modeStr;
                file >> modeStr;
                if (modeStr == "PER_DIR") switchMode[index] = 0;
                else switchMode[index] = 1;

                file >> switchState[index];

                for (int k = 0; k < 4; k++) {
                    file >> switchK[index][k];
                    switchCounter[index][k] = 0; 
                }

                string skip1, skip2;
                file >> skip1 >> skip2; 

                switchFlipped[index] = 0;
                numSwitches++;
            }
        }

        // 4. READ TRAINS
        else if (temp == "TRAINS:") {
            while (file >> spawnTick[numSpawn]) {
                file >> spawnColumn[numSpawn];
                file >> spawnRow[numSpawn];
                file >> spawnDirection[numSpawn];
                file >> spawnColor[numSpawn];
                
                // --- FIX: Adjust Coordinate ---
                // The file seems to use 1-based indexing for rows, but C++ uses 0.
                // We subtract 1 to snap the train onto the correct track.
                spawnRow[numSpawn] = spawnRow[numSpawn] - 1; 
                
                spawnTrainID[numSpawn] = -1; 
                numSpawn++;
            }
        }
    }
    
    file.close();

    // AUTO-ASSIGN DESTINATIONS
    int dCount = 0;
    int dRows[max_trains];
    int dCols[max_trains];

    for (int r = 0; r < numRows; r++) {
        for (int c = 0; c < numColumns; c++) {
            if (grid[r][c] == 'D') {
                dRows[dCount] = r;
                dCols[dCount] = c;
                dCount++;
            }
        }
    }

    for (int i = 0; i < numSpawn; i++) {
        if (dCount > 0) {
            destinationRow[i] = dRows[i % dCount];
            destinationColumn[i] = dCols[i % dCount];
            destinationTrainID[i] = i; 
            numDestinations++;
        }
    }

    cout << "Level loaded: " << filename << endl;
    return true;
}

// ----------------------------------------------------------------------------
// LOGGING FUNCTIONS (Unchanged)
// ----------------------------------------------------------------------------
void initializeLogFiles() {
    ofstream f1("trace.csv");
    if(f1.is_open()) { f1 << "Tick,TrainID,X,Y,Direction,State" << endl; f1.close(); }
    ofstream f2("switches.csv");
    if(f2.is_open()) { f2 << "Tick,Switch,Mode,State" << endl; f2.close(); }
    ofstream f3("signals.csv");
    if(f3.is_open()) { f3 << "Tick,Switch,Signal" << endl; f3.close(); }
}

void logTrainTrace() {
    ofstream file("trace.csv", ios::app);
    if (file.is_open()) {
        for (int i = 0; i < numTrains; i++) {
            if (trainRow[i] != -1) {
                file << currentTick << "," << i << "," << trainColumn[i] << "," 
                     << trainRow[i] << "," << trainDirection[i] << "," << trainWait[i] << endl;
            }
        }
        file.close();
    }
}

void logSwitchState() {
    ofstream file("switches.csv", ios::app);
    if (file.is_open()) {
        for (int i = 0; i < numSwitches; i++) {
            file << currentTick << "," << switchLetter[i] << "," 
                 << switchMode[i] << "," << switchState[i] << endl;
        }
        file.close();
    }
}

void logSignalState() {
    ofstream file("signals.csv", ios::app);
    if (file.is_open()) {
        for (int i = 0; i < numSwitches; i++) {
            file << currentTick << "," << switchLetter[i] << "," << switchSignal[i] << endl;
        }
        file.close();
    }
}

void writeMetrics() {
    ofstream file("metrics.txt");
    if (file.is_open()) {
        file << "SIMULATION REPORT" << endl;
        file << "-----------------" << endl;
        file << "Total Ticks: " << currentTick << endl;
        file << "Trains Reached Destination: " << trainsReached << endl;
        file << "Trains Crashed: " << trainsCrashed << endl;
        file << "Total Wait Time: " << totalWaitTicks << endl;
        file << "Total Energy Used: " << totalEnergy << endl;
        file.close();
    }
}