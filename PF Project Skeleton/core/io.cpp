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

// ----------------------------------------------------------------------------
// LOAD LEVEL FILE
// ----------------------------------------------------------------------------
// Load a .lvl file into global state.
// ----------------------------------------------------------------------------
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
    string bufferedToken = "";      // Stores a word if we read it too early
    bool hasBufferedToken = false;  // Flag to check if we have a stored word

    // Main loop: Reads file one word at a time
    while (true) {
        
        // 1. Handle Buffered Token (The Logic to fix the Map bug)
        // If we found "SWITCHES:" inside the map loop, we process it here.
        if (hasBufferedToken) {
            temp = bufferedToken;
            hasBufferedToken = false;
        } else {
            // Otherwise, read normally
            if (!(file >> temp)) {
                break; // Stop at end of file
            }
        }

        // 2. READ DIMENSIONS
        if (temp == "ROWS:") {
            file >> numRows;
        }
        else if (temp == "COLS:") {
            file >> numColumns;
        }
        else if (temp == "SEED:") {
            file >> levelSeed;
        }
        else if (temp == "WEATHER:") {
            string w;
            file >> w;
            if (w == "RAIN") weatherType = weather_rain;
            else if (w == "FOG") weatherType = weather_fog;
            else weatherType = weather_normal;
        }

        // 3. READ MAP
        else if (temp == "MAP:") {
            // Eat the newline after "MAP:"
            string dummy;
            getline(file, dummy);

            for (int r = 0; r < numRows; r++) {
                string line;
                getline(file, line);

                // ---LENGTH CALCULATION ---
                int len = 0;
                while (line[len] != '\0') {
                    len++;
                }

                // --- MANUAL KEYWORD DETECTION ---
                // Check if this line is actually "SWITCHES:" or "TRAINS:"
                // We check character by character.
                bool isSwitchHeader = false;
                bool isTrainHeader = false;

                if (len >= 9) {
                    if (line[0]=='S' && line[1]=='W' && line[2]=='I' && line[3]=='T' && 
                        line[4]=='C' && line[5]=='H' && line[6]=='E' && line[7]=='S' && line[8]==':') {
                        isSwitchHeader = true;
                    }
                }
                if (len >= 7) {
                    if (line[0]=='T' && line[1]=='R' && line[2]=='A' && line[3]=='I' && 
                        line[4]=='N' && line[5]=='S' && line[6]==':') {
                        isTrainHeader = true;
                    }
                }

                // If we hit a header, stop reading map immediately
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

                // Fill Grid Row
                for (int c = 0; c < numColumns; c++) {
                    if (c < len) {
                        // Ignore Carriage Return '\r' manually
                        if (line[c] != '\r') {
                            grid[r][c] = line[c];
                        }
                    } else {
                        grid[r][c] = '.';
                    }
                }
            }
        }

        // 4. READ SWITCHES
        else if (temp == "SWITCHES:") {
            while (true) {
                string nextWord;
                // Try reading next word
                if (!(file >> nextWord)) break;

                // Check if we hit TRAINS
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
                file >> skip1 >> skip2; // Eat "STRAIGHT" "TURN"

                switchFlipped[index] = 0;
                numSwitches++;
            }
        }

        // 5. READ TRAINS
        else if (temp == "TRAINS:") {
            while (file >> spawnTick[numSpawn]) {
                file >> spawnColumn[numSpawn];
                file >> spawnRow[numSpawn];
                file >> spawnDirection[numSpawn];
                file >> spawnColor[numSpawn];
                
                spawnTrainID[numSpawn] = -1; 
                numSpawn++;
            }
        }
    }
    
    file.close();

    // AUTO-ASSIGN DESTINATIONS (Manual Logic)
    int dCount = 0;
    int dRows[max_trains];
    int dCols[max_trains];

    // Find 'D' tiles
    for (int r = 0; r < numRows; r++) {
        for (int c = 0; c < numColumns; c++) {
            if (grid[r][c] == 'D') {
                dRows[dCount] = r;
                dCols[dCount] = c;
                dCount++;
            }
        }
    }

    // Assign to trains
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
// INITIALIZE LOG FILES
// ----------------------------------------------------------------------------
void initializeLogFiles() {
    ofstream f1("trace.csv");
    if(f1.is_open()) { f1 << "Tick,TrainID,X,Y,Direction,State" << endl; f1.close(); }

    ofstream f2("switches.csv");
    if(f2.is_open()) { f2 << "Tick,Switch,Mode,State" << endl; f2.close(); }

    ofstream f3("signals.csv");
    if(f3.is_open()) { f3 << "Tick,Switch,Signal" << endl; f3.close(); }
}

// ----------------------------------------------------------------------------
// LOG TRAIN TRACE
// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
// LOG SWITCH STATE
// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
// LOG SIGNAL STATE
// ----------------------------------------------------------------------------
void logSignalState() {
    ofstream file("signals.csv", ios::app);
    if (file.is_open()) {
        for (int i = 0; i < numSwitches; i++) {
            file << currentTick << "," << switchLetter[i] << "," << switchSignal[i] << endl;
        }
        file.close();
    }
}

// ----------------------------------------------------------------------------
// WRITE FINAL METRICS
// ----------------------------------------------------------------------------
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