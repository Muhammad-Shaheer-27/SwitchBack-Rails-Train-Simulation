#include "app.h"
#include "../core/simulation_state.h"
#include "../core/simulation.h"
#include "../core/grid.h"
#include "../core/switches.h"
#include "../core/io.h"
#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES FOR APP STATE
// ----------------------------------------------------------------------------
static sf::RenderWindow* g_window = nullptr;
static sf::View g_camera;
static bool g_isPaused = false;
static bool g_stepOnce = false;
static bool g_isDragging = false;
static sf::Vector2i g_lastDrag;
static float g_cellSize = 24.0f;        // world units per grid cell
static float g_gridOffsetX = 8.0f;      // margin from left
static float g_gridOffsetY = 8.0f;      // margin from top

// ----------------------------------------------------------------------------
// INITIALIZATION
// ----------------------------------------------------------------------------
bool initializeApp() {
    // Variable Name Fix: numColumns -> number_column, numRows -> number_rows
    int cols = (number_column > 0) ? number_column : 40;
    int rows = (number_rows > 0) ? number_rows : 25;
    
    int width = (int)(cols * g_cellSize + g_gridOffsetX * 2);
    int height = (int)(rows * g_cellSize + g_gridOffsetY * 2);

    // Safety clamp for window size
    if (width > 1920) width = 1920;
    if (height > 1080) height = 1080;

    g_window = new sf::RenderWindow(sf::VideoMode(width, height), "Switchback Rails");
    if (!g_window) {
        cout << "Failed to create window\n";
        return false;
    }

    g_window->setFramerateLimit(60);

    // initialize view to whole window
    g_camera = sf::View(sf::FloatRect(0.0f, 0.0f, (float)width, (float)height));
    g_window->setView(g_camera);

    return true;
}

// ----------------------------------------------------------------------------
// HELPER: DRAW TILE
// ----------------------------------------------------------------------------
static void drawTile(sf::RenderWindow &win, int r, int c, char ch) {
    sf::RectangleShape rect(sf::Vector2f(g_cellSize - 1.0f, g_cellSize - 1.0f));
    rect.setPosition(g_gridOffsetX + c * g_cellSize, g_gridOffsetY + r * g_cellSize);

    // Background
    rect.setFillColor(sf::Color(30, 30, 30));
    win.draw(rect);

    // Inner shape for track/objects
    sf::RectangleShape inner(sf::Vector2f(g_cellSize - 6.0f, g_cellSize - 6.0f));
    inner.setPosition(g_gridOffsetX + c * g_cellSize + 3.0f, g_gridOffsetY + r * g_cellSize + 3.0f);

    if (ch == '-' || ch == '=' || ch == '|' || ch == '+' || ch == '/' || ch == '\\') {
        inner.setFillColor(sf::Color(170,170,170)); // Tracks
        win.draw(inner);
    } else if (ch == 'S') {
        inner.setFillColor(sf::Color(0,160,0)); // Spawn (Green)
        win.draw(inner);
    } else if (ch == 'D') {
        inner.setFillColor(sf::Color(160,0,0)); // Destination (Red)
        win.draw(inner);
    } else if (ch >= 'A' && ch <= 'Z') {
        inner.setFillColor(sf::Color(200,140,0)); // Switch (Orange)
        win.draw(inner);
        
        // BONUS: Add Signal Light on top of switch if needed
        int sIdx = getSwitchIndex(r, c);
        if (sIdx != -1) {
            sf::CircleShape signal(g_cellSize / 6.0f);
            signal.setPosition(g_gridOffsetX + c * g_cellSize + g_cellSize/2.0f, 
                               g_gridOffsetY + r * g_cellSize + g_cellSize/2.0f);
            // 0=Green, 1=Yellow, 2=Red
            if (switchSignal[sIdx] == 0) signal.setFillColor(sf::Color::Green);
            else if (switchSignal[sIdx] == 1) signal.setFillColor(sf::Color::Yellow);
            else signal.setFillColor(sf::Color::Red);
            win.draw(signal);
        }
    } else if (ch == '=') {
        inner.setFillColor(sf::Color(0, 200, 200)); // Safety Tile
        win.draw(inner);
    }
}

// ----------------------------------------------------------------------------
// HELPER: DRAW TRAIN
// ----------------------------------------------------------------------------
static void drawTrain(sf::RenderWindow &win, int r, int c, int colorIdx) {
    sf::CircleShape circ(g_cellSize/2.5f);
    // Center calculation
    float offset = (g_cellSize - circ.getRadius()*2)/2.0f;
    circ.setPosition(g_gridOffsetX + c * g_cellSize + offset,
                     g_gridOffsetY + r * g_cellSize + offset);

    // simple color palette for colorIdx
    sf::Color cols[8] = {
        sf::Color(200,40,40),   // Red
        sf::Color(40,120,200),  // Blue
        sf::Color(40,200,80),   // Green
        sf::Color(200,180,40),  // Yellow
        sf::Color(160,80,200),  // Purple
        sf::Color(40,200,200),  // Cyan
        sf::Color(220,120,40),  // Orange
        sf::Color(200,200,200)  // White
    };
    int idx = (colorIdx >=0 && colorIdx < 8) ? colorIdx : 0;
    circ.setFillColor(cols[idx]);
    win.draw(circ);
}

// ----------------------------------------------------------------------------
// HELPER: CONVERT WORLD MOUSE POS TO GRID COORDS
// ----------------------------------------------------------------------------
static bool worldToGrid(const sf::RenderWindow &win, sf::Vector2i pix, int &outR, int &outC) {
    sf::Vector2f world = win.mapPixelToCoords(pix);
    float wx = world.x - g_gridOffsetX;
    float wy = world.y - g_gridOffsetY;
    
    if (wx < 0.0f || wy < 0.0f) return false;
    
    int c = (int)(wx / g_cellSize);
    int r = (int)(wy / g_cellSize);
    
    // Variable Name Fix: numRows -> number_rows, numColumns -> number_column
    if (r < 0 || r >= number_rows || c < 0 || c >= number_column) return false;
    
    outR = r; outC = c;
    return true;
}

// ----------------------------------------------------------------------------
// MAIN APP LOOP
// ----------------------------------------------------------------------------
void runApp() {
    if (!g_window) return;

    const int TICKS_PER_SEC = 4;
    const int MS_PER_TICK = 1000 / TICKS_PER_SEC;
    sf::Clock clock;
    int accumulator = 0;

    // Optional: Print initial grid to terminal once
    // printTerminalGrid(); // Requires making printTerminalGrid accessible or copying logic

    while (g_window->isOpen()) {
        // --- 1. EVENTS ---
        sf::Event event;
        while (g_window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                writeMetrics();
                g_window->close();
                break;
            }

            // Keyboard
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    writeMetrics();
                    g_window->close();
                    break;
                }
                if (event.key.code == sf::Keyboard::Space) {
                    g_isPaused = !g_isPaused;
                }
                if (event.key.code == sf::Keyboard::Period) {
                    g_stepOnce = true;
                }
            }

            // Mouse Click
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mp = sf::Mouse::getPosition(*g_window);
                
                // Middle Drag (Pan)
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    g_isDragging = true;
                    g_lastDrag = mp;
                } else {
                    int gr, gc;
                    if (worldToGrid(*g_window, mp, gr, gc)) {
                        // Left Click: Safety Tile
                        if (event.mouseButton.button == sf::Mouse::Left) {
                            toggleSafetyTile(gr, gc);
                        } 
                        // Right Click: Toggle Switch
                        else if (event.mouseButton.button == sf::Mouse::Right) {
                            int sidx = getSwitchIndex(gr, gc);
                            // Variable Name Fix: maximum_switches
                            if (sidx >= 0 && sidx < maximum_switches) {
                                switchState[sidx] = 1 - switchState[sidx]; // Toggle
                                cout << "Switch toggled manually." << endl;
                            }
                        }
                    }
                }
            }

            // Mouse Release
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    g_isDragging = false;
                }
            }

            // Mouse Move (Pan)
            if (event.type == sf::Event::MouseMoved) {
                if (g_isDragging) {
                    sf::Vector2i now = sf::Mouse::getPosition(*g_window);
                    sf::Vector2f p1 = g_window->mapPixelToCoords(g_lastDrag);
                    sf::Vector2f p2 = g_window->mapPixelToCoords(now);
                    sf::Vector2f delta = p1 - p2;
                    g_camera.move(delta);
                    g_window->setView(g_camera);
                    g_lastDrag = now;
                }
            }

            // Zoom
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) g_camera.zoom(0.9f);
                else g_camera.zoom(1.1f);
                g_window->setView(g_camera);
            }
        }

        // --- 2. UPDATE ---
        int elapsed = (int)clock.restart().asMilliseconds();
        accumulator += elapsed;

        while (accumulator >= MS_PER_TICK) {
            if (!g_isPaused || g_stepOnce) {
                // Variable Name Fix: updateSimulation() -> simulateOneTick()
                simulateOneTick(); 
                
                // Logging
                logTrainTrace();
                logSwitchState();
                logSignalState();
                
                if (g_stepOnce) g_stepOnce = false;
            }
            accumulator -= MS_PER_TICK;
        }

        // --- 3. RENDER ---
        g_window->clear(sf::Color(16,16,16));
        g_window->setView(g_camera);

        // A. Draw Grid
        // Variable Name Fix: number_rows, number_column
        for (int r = 0; r < number_rows; r++) {
            for (int c = 0; c < number_column; c++) {
                drawTile(*g_window, r, c, grid[r][c]);
            }
        }

        // B. Draw Active Trains
        // Variable Name Fix: numOf_trains, trainRow, trainColumn
        for (int i = 0; i < numOf_trains; i++) {
            if (trainRow[i] >= 0 && trainColumn[i] >= 0) {
                // Use i % 8 for color if trainColor not reliable, or use trainColor[i]
                int colorIdx = (i >= 0 && i < max_trains) ? (i % 8) : 0;
                drawTrain(*g_window, trainRow[i], trainColumn[i], colorIdx);
            }
        }
        
        // C. Draw Scheduled/Spawn Points
        // Variable Name Fix: num_spawn, spawnn_Row, spawnn_Column
        for (int i = 0; i < num_spawn; i++) {
            // Only draw if NOT spawned yet (index >= numOf_trains logic used in your snippet)
            if (i >= numOf_trains) {
                int sr = spawnn_Row[i], sc = spawnn_Column[i];
                if (sr >= 0 && sc >= 0) {
                    drawTrain(*g_window, sr, sc, spawnColor[i]);
                }
            }
        }

        g_window->display();
    }
}

// ----------------------------------------------------------------------------
// CLEANUP
// ----------------------------------------------------------------------------
void cleanupApp() {
    if (g_window) {
        if (g_window->isOpen()) g_window->close();
        delete g_window;
        g_window = nullptr;
    }
}