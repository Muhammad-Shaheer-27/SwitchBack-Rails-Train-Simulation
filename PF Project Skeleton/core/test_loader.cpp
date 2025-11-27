#include <iostream>
#include "simulation_state.h" // Needed for memory (grid, variables)
#include "io.h"               // Needed for loading function

using namespace std;

int main(int argc, char* argv[]) {
    cout << "=== LEVEL LOADER DIAGNOSTIC TEST ===" << endl;

    // 1. Initialize Memory
    initializeSimulationState();

    // 2. Check Arguments
    if (argc < 2) {
        cout << "Usage: ./test_loader <level_file>" << endl;
        return 1;
    }

    // 3. Attempt Load
    cout << "Loading File: " << argv[1] << endl;
    if (loadLevelFile(argv[1])) {
        cout << "\n[SUCCESS] File Parsed Successfully!" << endl;
        
        // 4. Print Statistics
        cout << "-----------------------------------" << endl;
        cout << "Grid Dimensions : " << numRows << " x " << numColumns << endl;
        cout << "Trains Loaded   : " << numSpawn << endl;
        cout << "Switches Loaded : " << numSwitches << endl;
        cout << "Destinations    : " << numDestinations << endl;
        cout << "-----------------------------------" << endl;
        
        // 5. Print Visual Map
        cout << "Visual Map Check:" << endl;
        for(int r = 0; r < numRows; r++) {
            for(int c = 0; c < numColumns; c++) {
                cout << grid[r][c];
            }
            cout << endl;
        }
        cout << "-----------------------------------" << endl;
        
        return 0;
    } else {
        cout << "[FAILED] Could not load level." << endl;
        return 1;
    }
}