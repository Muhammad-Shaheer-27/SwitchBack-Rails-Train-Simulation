#include "app.h"
#include "../core/simulation_state.h"
#include "../core/simulation.h"
#include "../core/io.h"
#include "../core/grid.h" 
#include <iostream>
#include <cstdlib>  // For system("cls") or system("clear")
#include <unistd.h> // For usleep()

using namespace std;

// ============================================================================
// MAIN.CPP - Entry point of the application (TERMINAL VERSION)
// ============================================================================

// ----------------------------------------------------------------------------
// HELPER: PRINT TERMINAL GRID
// ----------------------------------------------------------------------------
// Clears the console and prints the current state of the grid.
// ----------------------------------------------------------------------------
void printTerminalGrid() {
    // 1. Clear the screen to animate the frame
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    // 2. Print Header
    cout << "Tick: " << currentTick << endl;
    cout << "Trains Reached: " << trainsReached << " | Crashed: " << trainsCrashed << endl;
    cout << "------------------------------------------------------------" << endl;

    // 3. Print Grid
    for (int r = 0; r < numRows; r++) {
        for (int c = 0; c < numColumns; c++) {
            bool hasTrain = false;

            // Check if active train is on this tile
            for (int i = 0; i < numTrains; i++) {
                // Only draw valid trains
                if (trainRow[i] != -1 && trainRow[i] == r && trainColumn[i] == c) {
                    // Print the last digit of the train ID to distinguish them
                    cout << (i % 10);
                    hasTrain = true;
                    break;
                }
            }

            // If no train, print the map tile
            if (!hasTrain) {
                // If it's a switch, print its current direction state if needed, 
                // but for now, we just print the char from the grid.
                cout << grid[r][c];
            }
        }
        cout << endl; // Newline at end of row
    }
    cout << "------------------------------------------------------------" << endl;
}

// ----------------------------------------------------------------------------
// MAIN ENTRY POINT
// ----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    // 1. Check Command Line Arguments
    if (argc < 2) {
        cout << "Usage: ./switchback_console <level_file>" << endl;
        // Fallback for testing if no argument provided (Optional)
        cout << "No file provided. Exiting." << endl;
        return 1;
    }

    // 2. Initialize Simulation
    initializeSimulation();

    // 3. Load Level File
    // We pass argv[1] which is the path to the level file provided in the terminal
    if (!loadLevelFile(argv[1])) {
        cout << "Error: Failed to load level file: " << argv[1] << endl;
        return 1;
    }

    // 4. Initialize Logs
    initializeLogFiles();

    // 5. Print Initial State (So you can see the map before starting)
    printTerminalGrid();

    cout << "Level Loaded Successfully: " << argv[1] << endl;
    cout << "Press ENTER to start the simulation..." << endl;
    cin.get(); // Wait for user input

    // 6. Main Application Loop
    while (!isSimulationComplete()) {
        
        // A. Run one tick of simulation logic
        simulateOneTick();

        // B. Log data to CSV files
        logTrainTrace();
        logSwitchState();
        logSignalState();

        // C. Render to Terminal
        printTerminalGrid();

        // D. Delay for visual effect (0.1s = 100000 microseconds)
        // Adjust this value to speed up or slow down the terminal animation
        usleep(100000); 

        // Safety break to prevent infinite loops if logic fails
        if (currentTick > 5000) {
            cout << "Simulation timed out (Max Ticks Reached)." << endl;
            break;
        }
    }

    // 7. Cleanup & Final Stats
    writeMetrics();
    cout << "Simulation Finished!" << endl;
    cout << "Final metrics written to metrics.txt" << endl;

    return 0;
}