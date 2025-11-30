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

bool loadLevelFile(string filename)
{
    ifstream file(filename);

    if(!file.is_open())
    {
        cout<<"Error:Could not open level file." << endl;
        return false;
    }

    //Print empty space tiles for remainign rows and columns
    for (int r=0;r<maximum_rows;r++)
    {
        for(int c=0;c<maximum_Columns;c++)
        {
            grid[r][c]=space;
        }
    }

    //Reset counting values
    numSwitches=0;
    num_spawn=0;
    numDest=0;

    string current_word;
    string pending_header= "";
    bool hasBufferedToken=false;

    //Read file word by word
    while(true)
    {
        if(hasBufferedToken)
        {
            current_word=pending_header;
            hasBufferedToken = false;
        }
        else
        {
            if(!(file>>current_word))
            {
                break;
            }
        }
        //Read level information
        if(current_word=="ROWS:")
        {
            file>>number_rows;
        }
        else if(current_word=="COLS:")
        {
            file>>number_column;
        }
        else if(current_word=="SEED:")
        {
            file>>levelSeed;
        }
        else if(current_word=="WEATHER:")
        {
            string weather_code;
            file>>weather_code;
            if(weather_code=="RAIN")      weather_type = weather_rain;
            else if (weather_code=="FOG")  weather_type = weather_fog;
            else weather_type = weather_normal;
        }

        //Map reading
        else if(current_word == "MAP:")
        {
            string dummy;
            getline(file, dummy); //Remove rest of the line

            for (int row=0;row<number_rows;row++)
            {
                string line;
                if(!getline(file,line))
                {
                    break;
                }

                int length=0;
                //Measure line length except /r
                while(length<(int)line.size())
                {
                    if(line[length]=='\r') break;
                    length++;
                }

                bool isSwitchHeader=false;
                bool isTrainHeader=false;

                //Check trains or switches headers
                if(length>=9)
                {
                    if(line[0]=='S'&&line[1]=='W'&&line[2]=='I'&&line[3]=='T')
                        isSwitchHeader=true;
                }
                if(length>=7)
                {
                    if(line[0]=='T'&&line[1]=='R'&&line[2]=='A'&&line[3]=='I')
                        isTrainHeader=true;
                }

                if(isSwitchHeader)
                {
                   //Dont include swithces line in map
                    pending_header="SWITCHES:";
                    hasBufferedToken=true;
                    break;
                }
                if(isTrainHeader)
                {
                    pending_header="TRAINS:";
                    hasBufferedToken=true;
                    break;
                }

                //Copy characters from map to grid
                for(int c=0;c<number_column;c++)
                {
                    if(c<length)
                    {
                        //Avoid misprinting track and space
                        if(line[c]!='\r'&&line[c]!=' ')
                        {
                            grid[row][c]=line[c];
                        }
                        else
                        {
                            grid[row][c]=space;
                        }
                    }
                    else
                    {
                        grid[row][c]=space;
                    }
                }
            }
        }

        //Swich reading
        else if(current_word=="SWITCHES:")
        {
            while (true)
            {
                string nextWord;
                if(!(file >> nextWord)) break;

                //Prevent reading trains in switches
                if(nextWord=="TRAINS:")
                {
                    pending_header="TRAINS:";
                    hasBufferedToken=true;
                    break;
                }

                int index=numSwitches;
                switchLetter[index]=nextWord[0];

                string modeStr;
                file>>modeStr;
                //0 for Per dir and 1 for global
                if(modeStr=="PER_DIR")
                    switchMode[index]=0;
                else
                    switchMode[index]=1;

                file>>switchState[index];

                for(int k=0;k<4;k++)
                {
                    file>>switchK[index][k];
                    switchCounter[index][k]=0;
                }

                //Remove extralines
                string skip1, skip2;
                file>>skip1>>skip2;

                switchFlipped[index]=0;
                numSwitches++;
            }
        }

        //Train reading
        else if(current_word=="TRAINS:")
        {
            int rawRow;
            while (file>>spawnTick[num_spawn])
            {
                file>>spawnn_Column[num_spawn];
                file>>rawRow;
                file>>spawnDirection[num_spawn];
                file>>spawnColor[num_spawn];

                //Store raw row
                spawnn_Row[num_spawn] = rawRow;

                //Add actual train id
                spawnTrainID[num_spawn] = -1;
                num_spawn++;
            }
        }
    }

    file.close();

    // ------------------------------------------------------------------------
    // DESTINATIONS (D on the map)
    // ------------------------------------------------------------------------
    int destination_Count=0;
    int destRows[max_trains];
    int destCols[max_trains];

    for(int r=0;r<number_rows;r++)
    {
        for(int c=0;c<number_column;c++)
        {
            if(grid[r][c]=='D')
            {
                destRows[destination_Count]=r;
                destCols[destination_Count]=c;
                destination_Count++;
                if(destination_Count>=max_trains) break;
            }
        }
        if(destination_Count>=max_trains) break;
    }

    // ------------------------------------------------------------------------
    // IDENTIFYING SPAWN POINTS (S on the map)
    // ------------------------------------------------------------------------
    for(int i=0;i<num_spawn;i++)
    {
        int originalRow=spawnn_Row[i];
        int originalColumn=spawnn_Column[i];
        bool fixed=false;
        int search_row[8];
        int searchColumn[8];

        //Try to find rows and columns of spawn point
        search_row[0]=originalRow-1; 
        searchColumn[0]=originalColumn-1;
        search_row[1]=originalRow-1; 
        searchColumn[1]=originalColumn;
        search_row[2]=originalRow;   
        searchColumn[2]=originalColumn-1;
        search_row[3]=originalRow;   
        searchColumn[3]=originalColumn;

        //Check if row and column were swapped
        search_row[4]=originalColumn-1;
        searchColumn[4]=originalRow-1;
        search_row[5]=originalColumn-1;
        searchColumn[5]=originalRow;
        search_row[6]=originalColumn;
        searchColumn[6]=originalRow-1;
        search_row[7]=originalColumn;
        searchColumn[7]=originalRow;

        for (int k = 0; k < 8; k++)
        {
            int mapRow=search_row[k];
            int mapColumn=searchColumn[k];
            if(mapRow<0||mapRow>=number_rows||mapColumn<0||mapColumn>=number_column)
                continue;

            if(grid[mapRow][mapColumn]=='S')
            {
                spawnn_Row[i]=mapRow;
                spawnn_Column[i]=mapColumn;
                fixed=true;
                break;
            }
        }

        if (!fixed)
        {
            //Return back to original position
            int safe_row=originalRow-1;
            int safe_Col=originalColumn-1;
            if (safe_row<0)safe_row=0;
            if (safe_Col< 0)safe_Col=0;
            if (safe_row>=number_rows)safe_row=0;
            if (safe_Col>=number_column)safe_Col=0;
            spawnn_Row[i]=safe_row;
            spawnn_Column[i]=safe_Col;
        }
    }

    //Map spawn points to track S tiles
    int foundstart_count=0;
    int sRows[max_trains];
    int sCols[max_trains];

    for(int r=0;r<number_rows;r++)
    {
        for (int c=0;c<number_column;c++)
        {
            if(grid[r][c]=='S')
            {
                sRows[foundstart_count]=r;
                sCols[foundstart_count]=c;
                foundstart_count++;
                if (foundstart_count >= max_trains) break;
            }
        }
        if (foundstart_count >= max_trains) break;
    }

    if (foundstart_count>0)
    {
        //Check if spawn location and S tile dont match
        bool needFallback = false;
        for (int i = 0; i < num_spawn; i++)
        {
            int mapRow = spawnn_Row[i];
            int mapColumn = spawnn_Column[i];
            if (mapRow < 0 || mapRow >= number_rows ||
                mapColumn < 0 || mapColumn >= number_column ||
                grid[mapRow][mapColumn] != 'S')
            {
                needFallback = true;
                break;
            }
        }
        //Map all spawn points to S tiles
        if (needFallback)
        {
            for (int i = 0; i < num_spawn; i++)
            {
                int idx=i%foundstart_count;
                spawnn_Row[i]=sRows[idx];
                spawnn_Column[i]=sCols[idx];
            }
        }
    }

    // ------------------------------------------------------------------------
    // ASSIGN DESTINATIONS TO SPAWNS
    // ------------------------------------------------------------------------
    for (int i = 0; i < num_spawn; i++)
    {
        if (destination_Count > 0)
        {
            //Assign destinations
            destinationRow[i]     = destRows[i % destination_Count];
            destinationColumn[i]  = destCols[i % destination_Count];
            destinationTrainID[i] = i;
            numDest++;
        }
    }

    cout << "Level loaded: " << filename << endl;
    return true;
}

// ----------------------------------------------------------------------------
// LOGGING FUNCTIONS
// ----------------------------------------------------------------------------

void initializeLogFiles()
{
    ofstream f1("trace.csv");
    if (f1.is_open())
    {
        f1 << "time_Tick,Id_train,X_cord,Y_cord,Direction,State" << endl;
        f1.close();
    }

    ofstream f2("switches.csv");
    if (f2.is_open())
    {
        f2 << "time_Tick,Switch,mode,State" << endl;
        f2.close();
    }

    ofstream f3("signals.csv");
    if (f3.is_open())
    {
        f3 << "time_Tick,Switch,Signal" << endl;
        f3.close();
    }
}

void logTrainTrace()
{
    ofstream file("trace.csv", ios::app);
    if (file.is_open())
    {
        for (int i=0;i<numOf_trains;i++)
        {
            if (trainRow[i] != -1)
            {
                file <<currentTick<<","
                     <<i << ","
                     <<trainColumn[i]<<","
                     <<trainRow[i]<<","
                     <<trainDirection[i]<<","
                     <<trainWait[i]<<endl;
            }
        }
        file.close();
    }
}

void logSwitchState()
{
    ofstream file("switches.csv", ios::app);
    if (file.is_open())
    {
        for (int i = 0; i < numSwitches; i++)
        {
            file<<currentTick<<","
                 <<switchLetter[i]<<","
                 <<switchMode[i]<<","
                 <<switchState[i]<<endl;
        }
        file.close();
    }
}

// ============================================================================
// Signal State Logging with Actual Colors
// ============================================================================
void logSignalState()
{
    ofstream file("signals.csv", ios::app);
    if (file.is_open())
    {
        for(int i=0;i<numSwitches;i++)
        {
            //Convert signal number to string
            string color;
            if (switchSignal[i]==signal_green)
                color="GREEN";
            else if(switchSignal[i]==signal_yellow)
                color="YELLOW";
            else if (switchSignal[i]==sigal_red)
                color="RED";
            else
                color="GREEN";  //Return to Default
            
            file<<currentTick<<","
                 <<switchLetter[i]<<","
                 <<color<<endl;
        }
        file.close();
    }
}

void writeMetrics()
{
    ofstream file("metrics.txt");
    if (file.is_open())
    {
        file<<"SIMULATION REPORT"<<endl;
        file<<"-----------------"<<endl;
        file<<"Total Ticks: "<<currentTick<<endl;
        file<<"Trains Reached to Destination: "<<trainsReached<<endl;
        file<<"Trains Crashed: "<<crashed_trains<<endl;
        file<<"Total Waiting Time: "<<totalWaitTicks<<endl;
        file<<"Total Energy Used: "<<T_energy<<endl;
        file<<"Switch Flips: "<<switchFlips<<endl;
        file.close();
    }
}