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

    if (!file.is_open())
    {
        cout << "Error: Could not open level file." << endl;
        return false;
    }

    // Reset grid to empty "space" tiles.
    // NOTE: using the same 'space' constant used in grid.cpp etc
    // instead of '.' so the meaning of "empty" is consistent everywhere.
    for (int r = 0; r < maximum_rows; r++)
    {
        for (int c = 0; c < maximum_Columns; c++)
        {
            grid[r][c] = space;
        }
    }

    // Reset counting values
    numSwitches = 0;
    num_spawn   = 0;
    numDest     = 0;

    string current_word;
    string pending_header = "";
    bool hasBufferedToken = false;

    // Main loop: Reading the file one word at a time
    while (true)
    {
        if (hasBufferedToken)
        {
            current_word = pending_header;
            hasBufferedToken = false;
        }
        else
        {
            if (!(file >> current_word))
            {
                break; // EOF
            }
        }

        // 1) LEVEL PARAMETERS / DIMENSIONS
        if (current_word == "ROWS:")
        {
            file >> number_rows;
        }
        else if (current_word == "COLS:")
        {
            file >> number_column;
        }
        else if (current_word == "SEED:")
        {
            file >> levelSeed;
        }
        else if (current_word == "WEATHER:")
        {
            string weather_code;
            file >> weather_code;
            if (weather_code == "RAIN")      weather_type = weather_rain;
            else if (weather_code == "FOG")  weather_type = weather_fog;
            else                             weather_type = weather_normal;
        }

        // 2) MAP
        else if (current_word == "MAP:")
        {
            string dummy;
            getline(file, dummy); // consume rest of "MAP:" line

            for (int row = 0; row < number_rows; row++)
            {
                string line;
                if (!getline(file, line))
                {
                    break; // unexpected EOF
                }

                int length = 0;
                // Measure line length excluding possible '\r'
                while (length < (int)line.size())
                {
                    if (line[length] == '\r') break;
                    length++;
                }

                bool isSwitchHeader = false;
                bool isTrainHeader  = false;

                // Detect when the map section is over and we've hit
                // the "SWITCHES:" or "TRAINS:" header line
                if (length >= 9)
                {
                    if (line[0] == 'S' && line[1] == 'W' && line[2] == 'I' && line[3] == 'T')
                        isSwitchHeader = true;
                }
                if (length >= 7)
                {
                    if (line[0] == 'T' && line[1] == 'R' && line[2] == 'A' && line[3] == 'I')
                        isTrainHeader = true;
                }

                if (isSwitchHeader)
                {
                    // We have read the "SWITCHES:" line as a map row.
                    // Don't use this row in the map; instead schedule that
                    // the next iteration of the outer loop handles SWITCHES.
                    pending_header   = "SWITCHES:";
                    hasBufferedToken = true;
                    break;
                }
                if (isTrainHeader)
                {
                    pending_header   = "TRAINS:";
                    hasBufferedToken = true;
                    break;
                }

                // Normal map row: copy characters into grid
                for (int c = 0; c < number_column; c++)
                {
                    if (c < length)
                    {
                        // non-space char becomes tile; space becomes 'space'
                        if (line[c] != '\r' && line[c] != ' ')
                        {
                            grid[row][c] = line[c];
                        }
                        else
                        {
                            grid[row][c] = space;
                        }
                    }
                    else
                    {
                        grid[row][c] = space;
                    }
                }
            }
        }

        // 3) SWITCHES
        else if (current_word == "SWITCHES:")
        {
            while (true)
            {
                string nextWord;
                if (!(file >> nextWord)) break; // EOF

                // If we hit the TRAINS header while reading switches,
                // stop and buffer it for the outer loop
                if (nextWord == "TRAINS:")
                {
                    pending_header   = "TRAINS:";
                    hasBufferedToken = true;
                    break;
                }

                int index = numSwitches;
                switchLetter[index] = nextWord[0];

                string modeStr;
                file >> modeStr;
                // NOTE: 0 = PER_DIR, 1 = GLOBAL (must match enums in switches.h)
                if (modeStr == "PER_DIR")
                    switchMode[index] = 0;
                else
                    switchMode[index] = 1;

                file >> switchState[index];

                for (int k = 0; k < 4; k++)
                {
                    file >> switchK[index][k];
                    switchCounter[index][k] = 0;
                }

                // Two extra string fields in level file that are not used here
                string skip1, skip2;
                file >> skip1 >> skip2;

                switchFlipped[index] = 0;
                numSwitches++;
            }
        }

        // 4) TRAINS
        else if (current_word == "TRAINS:")
        {
            // File format (per line, for example):
            // spawnTick  column  row  direction  color
            int rawRow;
            while (file >> spawnTick[num_spawn])
            {
                file >> spawnn_Column[num_spawn];
                file >> rawRow;
                file >> spawnDirection[num_spawn];
                file >> spawnColor[num_spawn];

                // Store raw row; we will resolve this to actual 'S' tile
                // after the full map is parsed (below).
                spawnn_Row[num_spawn] = rawRow;

                // This will later be filled with the actual train ID
                // when the train is spawned in trains.cpp
                spawnTrainID[num_spawn] = -1;
                num_spawn++;
            }
        }
    }

    file.close();

    // ------------------------------------------------------------------------
    // DESTINATIONS (D on the map)
    // ------------------------------------------------------------------------
    int destination_Count = 0;
    int dRows[max_trains];
    int dCols[max_trains];

    for (int r = 0; r < number_rows; r++)
    {
        for (int c = 0; c < number_column; c++)
        {
            if (grid[r][c] == 'D')
            {
                dRows[destination_Count] = r;
                dCols[destination_Count] = c;
                destination_Count++;
                if (destination_Count >= max_trains) break;
            }
        }
        if (destination_Count >= max_trains) break;
    }

    // ------------------------------------------------------------------------
    // IDENTIFYING SPAWN POINTS (S on the map)
    // ------------------------------------------------------------------------
    for (int i = 0; i < num_spawn; i++)
    {
        int rawR = spawnn_Row[i];
        int rawC = spawnn_Column[i];
        bool fixed = false;
        int search_row[8];
        int searchColumn[8];

        // Try a 2x2 neighbourhood around (rawR, rawC)
        search_row[0]   = rawR - 1; searchColumn[0] = rawC - 1;
        search_row[1]   = rawR - 1; searchColumn[1] = rawC;
        search_row[2]   = rawR;     searchColumn[2] = rawC - 1;
        search_row[3]   = rawR;     searchColumn[3] = rawC;

        // Also try the "transposed" neighbourhood around (rawC, rawR).
        // This allows the input to accidentally swap row/col and still work.
        search_row[4]   = rawC - 1; searchColumn[4] = rawR - 1;
        search_row[5]   = rawC - 1; searchColumn[5] = rawR;
        search_row[6]   = rawC;     searchColumn[6] = rawR - 1;
        search_row[7]   = rawC;     searchColumn[7] = rawR;

        for (int k = 0; k < 8; k++)
        {
            int rr = search_row[k];
            int cc = searchColumn[k];
            if (rr < 0 || rr >= number_rows || cc < 0 || cc >= number_column)
                continue;

            if (grid[rr][cc] == 'S')
            {
                spawnn_Row[i]    = rr;
                spawnn_Column[i] = cc;
                fixed = true;
                break;
            }
        }

        if (!fixed)
        {
            // Fallback: assume rawR, rawC are 1-based (row, col)
            int safe_row = rawR - 1;
            int safe_Col = rawC - 1;
            if (safe_row < 0)           safe_row = 0;
            if (safe_Col < 0)           safe_Col = 0;
            if (safe_row >= number_rows)   safe_row = 0;
            if (safe_Col >= number_column) safe_Col = 0;
            spawnn_Row[i]    = safe_row;
            spawnn_Column[i] = safe_Col;
        }
    }

    // ------------------------------------------------------------------------
    // If needed, map spawn points to existing 'S' tiles in a fallback way
    // ------------------------------------------------------------------------
    int foundstart_count = 0;
    int sRows[max_trains];
    int sCols[max_trains];

    for (int r = 0; r < number_rows; r++)
    {
        for (int c = 0; c < number_column; c++)
        {
            if (grid[r][c] == 'S')
            {
                sRows[foundstart_count] = r;
                sCols[foundstart_count] = c;
                foundstart_count++;
                if (foundstart_count >= max_trains) break;
            }
        }
        if (foundstart_count >= max_trains) break;
    }

    if (foundstart_count > 0)
    {
        // Check if any spawn location does not actually land on an 'S' tile.
        bool needFallback = false;
        for (int i = 0; i < num_spawn; i++)
        {
            int rr = spawnn_Row[i];
            int cc = spawnn_Column[i];
            if (rr < 0 || rr >= number_rows ||
                cc < 0 || cc >= number_column ||
                grid[rr][cc] != 'S')
            {
                needFallback = true;
                break;
            }
        }

        // If so, cycle spawn positions over all found S tiles.
        if (needFallback)
        {
            for (int i = 0; i < num_spawn; i++)
            {
                int idx = i % foundstart_count;
                spawnn_Row[i]    = sRows[idx];
                spawnn_Column[i] = sCols[idx];
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
            // Simple policy: assign destinations round-robin across all D tiles.
            destinationRow[i]     = dRows[i % destination_Count];
            destinationColumn[i]  = dCols[i % destination_Count];

            // IMPORTANT:
            // Initially, destinationTrainID[i] stores the *spawn index* (i).
            // When the actual train is spawned, trains.cpp remaps this to the
            // real train ID (freeTrain).
            //
            // See trains.cpp: in spawnTrainsForTick():
            //   if (destinationTrainID[d] == i) { destinationTrainID[d] = freeTrain; }
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
        // time_Tick, train id, X (col), Y (row), direction, wait/state
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
        for (int i = 0; i < numOf_trains; i++)
        {
            if (trainRow[i] != -1)
            {
                file << currentTick << ","
                     << i << ","
                     << trainColumn[i] << ","
                     << trainRow[i] << ","
                     << trainDirection[i] << ","
                     << trainWait[i] << endl;
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
            file << currentTick << ","
                 << switchLetter[i] << ","
                 << switchMode[i] << ","
                 << switchState[i] << endl;
        }
        file.close();
    }
}

void logSignalState()
{
    ofstream file("signals.csv", ios::app);
    if (file.is_open())
    {
        for (int i = 0; i < numSwitches; i++)
        {
            file << currentTick << ","
                 << switchLetter[i] << ","
                 << switchSignal[i] << endl;
        }
        file.close();
    }
}

void writeMetrics()
{
    ofstream file("metrics.txt");
    if (file.is_open())
    {
        file << "SIMULATION REPORT" << endl;
        file << "-----------------" << endl;
        file << "Total Ticks: " << currentTick << endl;
        file << "Trains Reached to Destination: " << trainsReached << endl;
        file << "Trains Crashed: " << crashed_trains << endl;
        file << "Total Waiting Time: " << totalWaitTicks << endl;
        file << "Total Energy Used: " << T_energy << endl;
        file.close();
    }
}