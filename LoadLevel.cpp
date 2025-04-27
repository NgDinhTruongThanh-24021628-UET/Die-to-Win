#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include "LevelObjs.h"
#include "LoadLevel.h"

// Split the level into tiles to place objects
const float TILE_SIZE=SCREEN_HEIGHT/10.0f;
const int LEVEL_WIDTH=19;
const int LEVEL_HEIGHT=11;

// Vector to store objects
std::vector<Block> blocks;
std::vector<Spike> spikes;
std::vector<JumpOrb> jumpOrbs;
std::vector<JumpPad> jumpPads;
std::vector<PushableBlock> pushableBlocks;

// Identify block type
std::unordered_map<std::string, BlockInfo> blockLookup={
    {"1C0", {0, 0, SDL_FLIP_NONE}},     // Top left level corner
    {"1C1", {0, 90, SDL_FLIP_NONE}},    // Top right level corner
    {"1C2", {0, 180, SDL_FLIP_NONE}},   // Bottom right level corner
    {"1C3", {0, 270, SDL_FLIP_NONE}},   // Bottom left level corner

    {"1WH", {1, 0, SDL_FLIP_NONE}},     // Horizontal wall
    {"1WV", {1, 90, SDL_FLIP_NONE}},    // Vertical wall
    {"1WVI", {1, 90, SDL_FLIP_NONE}},   // Vertical wall (interactable)

    {"1TL", {2, 0, SDL_FLIP_NONE}},     // T-block left
    {"1TU", {2, 90, SDL_FLIP_NONE}},    // T-block up
    {"1TR", {2, 180, SDL_FLIP_NONE}},   // T-block right
    {"1TD", {2, 270, SDL_FLIP_NONE}},   // T-block down

    {"1PU", {3, 0, SDL_FLIP_NONE}},     // Platform tip up
    {"1PR", {3, 90, SDL_FLIP_NONE}},    // Platform tip right
    {"1PD", {3, 180, SDL_FLIP_NONE}},   // Platform tip down
    {"1PL", {3, 270, SDL_FLIP_NONE}},   // Platform tip left

    {"1E", {4, 0, SDL_FLIP_NONE}},      // No border block
    {"1B", {5, 0, SDL_FLIP_NONE}},      // All border block
    {"1BI", {5, 0, SDL_FLIP_NONE}},     // All border block (interactable)
    {"1BG", {5, 0, SDL_FLIP_NONE}},     // All border block (green)
    {"1BO", {5, 0, SDL_FLIP_NONE}},     // All border block (orange)
    {"1BY", {5, 0, SDL_FLIP_NONE}},     // All border block (dio)

    {"1I1", {6, 0, SDL_FLIP_NONE}},     // Idle tycoon block 1 - misc upgrade
    {"1I2", {7, 0, SDL_FLIP_NONE}},     // Idle tycoon block 2 - lower the point block
    {"1I3", {8, 0, SDL_FLIP_NONE}},     // Idle tycoon block 3 - point upgrade
    {"1I4", {9, 0, SDL_FLIP_NONE}},     // Idle tycoon block 4 - passive income upgrade
    {"1IP", {10, 0, SDL_FLIP_NONE}},    // Idle tycoon - point block

    {"1S", {11, 0, SDL_FLIP_NONE}},     // Menu block 1 - settings
    {"1P", {12, 0, SDL_FLIP_NONE}},     // Menu block 2 - start
    {"1C", {13, 0, SDL_FLIP_NONE}},     // Menu block 3 - credits

    {"1IN", {14, 0, SDL_FLIP_NONE}},    // Password puzzle - check solution
    {"1BB", {15, 0, SDL_FLIP_NONE}},    // Pool puzzle - add water
    {"1SA", {16, 0, SDL_FLIP_NONE}},    // Time puzzle - stop time
    {"1MV", {17, 0, SDL_FLIP_NONE}},    // Pushable block

    {"1XM", {18, 0, SDL_FLIP_NONE}},    // Tic-tac-toe puzzle - move X to next position
    {"1XI", {19, 0, SDL_FLIP_NONE}},    // Tic-tac-toe puzzle - X block (interactable)
    {"1X", {19, 0, SDL_FLIP_NONE}},     // Tic-tac-toe puzzle - X block
    {"1O", {20, 0, SDL_FLIP_NONE}},     // Tic-tac-toe puzzle - O block
    {"1R", {21, 0, SDL_FLIP_NONE}},     // Reset puzzle

    {"1ZA", {22, 0, SDL_FLIP_NONE}},    // Electricity puzzle - deplete

    {"1K0", {23, 0, SDL_FLIP_NONE}},    // Top left corner block
    {"1K1", {23, 90, SDL_FLIP_NONE}},   // Top right corner block
    {"1K2", {23, 180, SDL_FLIP_NONE}},  // Bottom right corner block
    {"1K3", {23, 270, SDL_FLIP_NONE}},  // Bottom left corner block

    {"1LU", {24, 0, SDL_FLIP_NONE}},    // Line block up
    {"1LR", {24, 90, SDL_FLIP_NONE}},   // Line block right
    {"1LD", {24, 180, SDL_FLIP_NONE}},  // Line block down
    {"1LL", {24, 270, SDL_FLIP_NONE}},  // Line block left

    {"1JL", {28, 0, SDL_FLIP_NONE}},        // Jump-through platform attached to left wall
    {"1JR", {28, 0, SDL_FLIP_HORIZONTAL}},  // Jump-through platform attached to right wall
    {"1J", {29, 0, SDL_FLIP_NONE}},         // Jump-through platform in the air

    {"1Y", {30, 0, SDL_FLIP_NONE}},     // Invisible block

/***********************************************************************************************/

    // Platform tip[25] with spike
    {"3AU", {25, 0, SDL_FLIP_NONE}},    // Facing up
    {"3AR", {25, 90, SDL_FLIP_NONE}},   // Facing right
    {"3AD", {25, 180, SDL_FLIP_NONE}},  // Facing down
    {"3AL", {25, 270, SDL_FLIP_NONE}},  // Facing left

    // Platform tip[25] with spike, mirrored
    {"3AUM", {25, 0, SDL_FLIP_HORIZONTAL}},     // Facing up
    {"3ARM", {25, 90, SDL_FLIP_HORIZONTAL}},    // Facing right
    {"3ADM", {25, 180, SDL_FLIP_HORIZONTAL}},   // Facing down
    {"3ALM", {25, 270, SDL_FLIP_HORIZONTAL}},   // Facing left

    // Normal platform[26] with spike
    {"3CU", {26, 0, SDL_FLIP_NONE}},    // Facing up
    {"3CR", {26, 90, SDL_FLIP_NONE}},   // Facing right
    {"3CD", {26, 180, SDL_FLIP_NONE}},  // Facing down
    {"3CL", {26, 270, SDL_FLIP_NONE}},  // Facing left

    // Normal platform[27] with big spike
    {"3EU", {27, 0, SDL_FLIP_NONE}},    // Facing up
    {"3ER", {27, 90, SDL_FLIP_NONE}},   // Facing right
    {"3ED", {27, 180, SDL_FLIP_NONE}},  // Facing down
    {"3EL", {27, 270, SDL_FLIP_NONE}}   // Facing left
};

// Identify spike type
std::unordered_map<std::string, SpikeInfo> spikeLookup={ // Note: Spike platform migrated to blocks (number 3 in front)
    // Platform tip with spike[0]
    {"2AU", {0, 0, SDL_FLIP_NONE}},     // Facing up
    {"2AR", {0, 90, SDL_FLIP_NONE}},    // Facing right
    {"2AD", {0, 180, SDL_FLIP_NONE}},   // Facing down
    {"2AL", {0, 270, SDL_FLIP_NONE}},   // Facing left

    // Platform tip with spike[0], mirrored
    {"2AUM", {0, 0, SDL_FLIP_HORIZONTAL}},      // Facing up
    {"2ARM", {0, 90, SDL_FLIP_HORIZONTAL}},     // Facing right
    {"2ADM", {0, 180, SDL_FLIP_HORIZONTAL}},    // Facing down
    {"2ALM", {0, 270, SDL_FLIP_HORIZONTAL}},    // Facing left

    // Normal platform with spike[1]
    {"2CU", {1, 0, SDL_FLIP_NONE}},     // Facing up
    {"2CR", {1, 90, SDL_FLIP_NONE}},    // Facing right
    {"2CD", {1, 180, SDL_FLIP_NONE}},   // Facing down
    {"2CL", {1, 270, SDL_FLIP_NONE}},   // Facing left

    // Normal platform with big spike[2]
    {"2EU", {2, 0, SDL_FLIP_NONE}},     // Facing up
    {"2ER", {2, 90, SDL_FLIP_NONE}},    // Facing right
    {"2ED", {2, 180, SDL_FLIP_NONE}},   // Facing down
    {"2EL", {2, 270, SDL_FLIP_NONE}},    // Facing left
};

// Identify jump orb type
std::unordered_map<std::string, JumpOrbInfo> jumpOrbLookup={
    // Yellow orb
    {"Y", {0, 0, 0}},                           // Normal
    {"YX", {0, TILE_SIZE/2, 0}},                // X increase
    {"YY", {0, 0, TILE_SIZE/2}},                // Y increase
    {"YXY", {0, TILE_SIZE/2, TILE_SIZE/2}},     // Both increase

    // Blue orb
    {"B", {1, 0, 0}},
    {"BX", {1, TILE_SIZE/2, 0}},
    {"BY", {1, 0, TILE_SIZE/2}},
    {"BXY", {1, TILE_SIZE/2, TILE_SIZE/2}},

    // Green orb
    {"G", {2, 0, 0}},
    {"GX", {2, TILE_SIZE/2, 0}},
    {"GY", {2, 0, TILE_SIZE/2}},
    {"GXY", {2, TILE_SIZE/2, TILE_SIZE/2}},

    // Dash orb
    {"D", {3, 0, 0}},
    {"DX", {3, TILE_SIZE/2, 0}},
    {"DY", {3, 0, TILE_SIZE/2}},
    {"DXY", {3, TILE_SIZE/2, TILE_SIZE/2}},
};

// Identify jump pad type
std::unordered_map<std::string, JumpPadInfo> jumpPadLookup={
    {"JU", {0, 0}},      // Yellow pad up
    {"JD", {0, 180}},    // Yellow pad down

    {"SU", {1, 0}},      // Spider pad up
    {"SR", {1, 90}},     // Spider pad right
    {"SD", {1, 180}},    // Spider pad down
    {"SL", {1, 270}},    // Spider pad left

    {"PU", {2, 0}},      // Pink pad up
    {"PD", {2, 180}},    // Pink pad down
};

// Load level from a file
void loadLevel(const std::string &path, std::vector<Block> &blocks, std::vector<PushableBlock> &pushableBlocks,
               std::vector<Spike> &spikes, std::vector<JumpOrb> &jumpOrbs, std::vector<JumpPad> &jumpPads) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open level file." << std::endl;
        return;
    }

    blocks.clear(); pushableBlocks.clear(); spikes.clear(); jumpOrbs.clear(); jumpPads.clear();

    // Read file and store objects
    std::string tile;
    int row=0;
    while (row<LEVEL_HEIGHT) {
        for (int col=0; col<LEVEL_WIDTH; col++) {
            float baseX=col*TILE_SIZE-TILE_SIZE*11/18;
            float baseY=row*TILE_SIZE-TILE_SIZE*9/18;

            file >> tile;

            // Store blocks
            if (blockLookup.find(tile)!=blockLookup.end()) {
                BlockInfo info=blockLookup[tile];
                if (tile!="1MV") blocks.emplace_back(baseX, baseY, TILE_SIZE, TILE_SIZE, info.rotation, info.mirrored, tile);
                else pushableBlocks.emplace_back(baseX, baseY, TILE_SIZE, TILE_SIZE);
            }

            // Store spikes
            else if (spikeLookup.find(tile)!=spikeLookup.end()) {
                SpikeInfo info=spikeLookup[tile];
                if (tile[1]=='A' || tile[1]=='C') { // Small spike
                    if (info.rotation==0) {
                        spikes.emplace_back(baseX+TILE_SIZE*2/5, baseY+TILE_SIZE*7/10, TILE_SIZE/5, TILE_SIZE/5, info.rotation, info.mirrored, tile);
                    }
                    else if (info.rotation==90) {
                        spikes.emplace_back(baseX+TILE_SIZE/10, baseY+TILE_SIZE*2/5, TILE_SIZE/5, TILE_SIZE/5, info.rotation, info.mirrored, tile);
                    }
                    else if (info.rotation==180) {
                        spikes.emplace_back(baseX+TILE_SIZE*2/5, baseY+TILE_SIZE/10, TILE_SIZE/5, TILE_SIZE/5, info.rotation, info.mirrored, tile);
                    }
                    else if (info.rotation==270) {
                        spikes.emplace_back(baseX+TILE_SIZE*7/10, baseY+TILE_SIZE*2/5, TILE_SIZE/5, TILE_SIZE/5, info.rotation, info.mirrored, tile);
                    }
                }
                else if (tile[1]=='E') { // Big spike
                    if (info.rotation==0 || info.rotation==180) {
                        spikes.emplace_back(baseX+TILE_SIZE*2/5, baseY+TILE_SIZE*3/10, TILE_SIZE/5, TILE_SIZE*2/5, info.rotation, info.mirrored, tile);
                    }
                    else if (info.rotation==90 || info.rotation==270) {
                        spikes.emplace_back(baseX+TILE_SIZE*3/10, baseY+TILE_SIZE*2/5, TILE_SIZE*2/5, TILE_SIZE/5, info.rotation, info.mirrored, tile);
                    }
                }
            }

            // Store jump pads
            else if (jumpPadLookup.find(tile)!=jumpPadLookup.end()) {
                JumpPadInfo info=jumpPadLookup[tile];
                if (tile[0]=='J' || tile[0]=='P') {
                    if (info.rotation==0) {
                        jumpPads.emplace_back(baseX+TILE_SIZE/12, baseY+TILE_SIZE*13/15, TILE_SIZE*10/12, TILE_SIZE/6, info.rotation, tile);
                    }
                    else if (info.rotation==180) {
                        jumpPads.emplace_back(baseX+TILE_SIZE/12, baseY-TILE_SIZE/30, TILE_SIZE*10/12, TILE_SIZE/6, info.rotation, tile);
                    }
                }
                else if (tile[0]=='S') { // Spider pad
                    if (info.rotation==0) {
                        jumpPads.emplace_back(baseX+TILE_SIZE/30, baseY+TILE_SIZE*3/4, TILE_SIZE*14/15, TILE_SIZE*2/5, info.rotation, tile);
                    }
                    else if (info.rotation==90) {
                        jumpPads.emplace_back(baseX-TILE_SIZE*3/20, baseY+TILE_SIZE/30, TILE_SIZE*2/5, TILE_SIZE*14/15, info.rotation, tile);
                    }
                    else if (info.rotation==180) {
                        jumpPads.emplace_back(baseX+TILE_SIZE/30, baseY-TILE_SIZE*3/20, TILE_SIZE*14/15, TILE_SIZE*2/5, info.rotation, tile);
                    }
                    else if (info.rotation==270) {
                        jumpPads.emplace_back(baseX+TILE_SIZE*3/4, baseY+TILE_SIZE/30, TILE_SIZE*2/5, TILE_SIZE*14/15, info.rotation, tile);
                    }
                }
            }

            // Store jump orbs
            else if (jumpOrbLookup.find(tile)!=jumpOrbLookup.end()) {
                JumpOrbInfo info=jumpOrbLookup[tile];
                jumpOrbs.emplace_back(baseX-TILE_SIZE/10+info.offsetX, baseY-TILE_SIZE/10+info.offsetY, TILE_SIZE*12/10, TILE_SIZE*12/10, tile[0]);
            }
        }

        row++;
    }

    file.close();
}
