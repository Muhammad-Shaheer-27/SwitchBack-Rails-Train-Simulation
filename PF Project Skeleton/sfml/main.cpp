#include "app.h"
#include "../core/simulation_state.h"
#include "../core/simulation.h"
#include "../core/io.h"
#include <iostream>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: ./switchback <level_file>" << endl;
        return 1;
    }

    // initialize simulation core
    initializeSimulation();

    if (!loadLevelFile(argv[1])) {
        cout << "Error: Failed to load level file: " << argv[1] << endl;
        return 1;
    }

    initializeLogFiles();

    cout << "Level Loaded: " << argv[1] << endl;
    cout << "Starting Graphics..." << endl;

    if (!initializeApp()) {
        cout << "Failed to initialize graphics app" << endl;
        return 1;
    }

    runApp();
    cleanupApp();

    writeMetrics();
    cout << "Simulation Finished. Metrics saved." << endl;
    return 0;
}