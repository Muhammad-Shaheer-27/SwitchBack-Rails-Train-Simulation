#include "app.h"
#include "../core/simulation_state.h"
#include "../core/simulation.h"
#include "../core/io.h"
#include "../core/grid.h" 
#include <iostream>
#include <cstdlib>  // For tick update on terminal
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
void printTerminalGrid(){
    //Clear the screen to animate the frame
    #ifdef _WIN32
        system("cls");
    #else
       system("clear");
    #endif

    cout << "Tick: " << currentTick << endl;
    cout << "Trains Reached: " << trainsReached << " | Crashed: " << crashed_trains << endl;
    cout << "------------------------------------------------------------" << endl;

    //Print grid
    for(int r=0;r<number_column;r++){
        for(int c=0;c<number_rows;c++){
            bool hasTrain=false;

            //Check if active train present at this position
            for(int i=0;i<numOf_trains;i++) {
                //Only draw valid trains
                if(trainRow[i]!=-1&&trainRow[i]==r&&trainColumn[i]==c){
                    //Print the last digit of the train ID to distinguish them
                    cout<<(i%10);
                    hasTrain=true;
                    break;
                }
            }

            //If no train print the map tile
            if(!hasTrain){
                //Print switch
                cout<<grid[r][c];
            }
        }
        cout<<endl;
    }
    cout<<"------------------------------------------------------------"<<endl;
}

// ----------------------------------------------------------------------------
// MAIN ENTRY POINT
// ----------------------------------------------------------------------------
int main(int argc, char* argv[]){
    //Check argument
    if(argc<2){
        cout<<"Usage: ./switchback_console <level_file>"<<endl;
        //Fall back to default level file if none provided
        cout<<"No file provided. Exiting."<<endl;
        return 1;
    }

    // 2. Initialize Simulation
    initializeSimulation();

    //Load level file
    if (!loadLevelFile(argv[1])) {
        cout<<"Error: Failed to load level file: "<<argv[1]<<endl;
        return 1;
    }

    //Add log files
    initializeLogFiles();

    //Call function to print grid
    printTerminalGrid();

    cout<<"Level Loaded Successfully: "<< argv[1]<<endl;
    cout<<"Press ENTER to start the simulation..."<<endl;
    cin.get(); //Wait for user input
for(int i=0;i<numOf_trains;i++) {
    cout<<"Train "<<i<<": row="<<trainRow[i]<<" col="<<trainColumn[i]<<endl;
}

cout<<"isSimulationComplete() returns: " <<(isSimulationComplete()?"TRUE":"FALSE")<<endl;
cout<<"==============================\n\n";
    //Train simulation on terminal
    while (!isSimulationComplete()) {
        
        // A. Run one tick of simulation logic
        simulateOneTick();
        // B. Log data to CSV files
        logTrainTrace();
        logSwitchState();
        logSignalState();

        //Print grid to terminal
        printTerminalGrid();

        //Add wait time before each tick
        usleep(500000); 

        //Prevent infinite loop
        if (currentTick>1000) {
            cout << "Simulation timed out (Max Ticks Reached)." << endl;
            break;
        }
    }

    //Write metrics
    writeMetrics();
    cout<<"Simulation Finished!"<<endl;
    cout<<"Final metrics written to metrics.txt"<<endl;

    return 0;
}