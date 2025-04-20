#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "Player.h"
#include "Texture.h"
#include "LevelObjs.h"
#include "Enums.h"
using namespace std;

// Window sizes
const int SCREEN_WIDTH=1280;
const int SCREEN_HEIGHT=720;

// Window to render to
SDL_Window *gWindow=nullptr;

// Renderer
SDL_Renderer *gRenderer=nullptr;

// Font
TTF_Font *gTinyFont=nullptr;
TTF_Font *gSmallFont=nullptr;
TTF_Font *gMediumFont=nullptr;
TTF_Font *gLargeFont=nullptr;
TTF_Font *gXtraFont=nullptr;
SDL_Color textColor={255, 255, 255};

// Textures
LTexture winMsgTexture;
LTexture gameTitleTexture;
LTexture instructionTexture[100];

LTexture cubeTexture;
LTexture blockSheetTexture;
LTexture orbPadSheetTexture;

Color selectedColor=PINK;
const int BG_TOTAL_COLOR=4;
SDL_Color bgColor[BG_TOTAL_COLOR]={
    {0xF6, 0x4A, 0x8A},
    {0x31, 0x8C, 0xE7},
    {0xEA, 0xC1, 0x42},
    {0x2A, 0x34, 0x39}
};

Background selectedBG=BLANK;
LTexture backgroundTexture[TOTAL_BG];

const int NUMBER_OF_BLOCKS=28;
const int NUMBER_OF_SPIKES=3;
const int NUMBER_OF_ORBS=3;
const int NUMBER_OF_PADS=3;
SDL_Rect blockClips[NUMBER_OF_BLOCKS];
SDL_Rect spikeClips[NUMBER_OF_SPIKES];
SDL_Rect orbClips[NUMBER_OF_ORBS];
SDL_Rect padClips[NUMBER_OF_PADS];

// Initialize
bool init() {
    bool success=true;

    // Initialize SDL.h
    if (SDL_Init(SDL_INIT_EVERYTHING)<0) {
        cout << "SDL could not initialize. " << SDL_GetError() << endl;
        success=false;
    }

    else {
        // Set texture filtering
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            cout << "Warning: Linear texture filtering not enabled." << endl;
        }

        // Create a window
        gWindow=SDL_CreateWindow("Die to Win", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow==nullptr) {
            cout << "Window could not be created. " << SDL_GetError() << endl;
            success=false;
        }

        else {
            // Create a renderer
            gRenderer=SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
            if (gRenderer==nullptr) {
                cout << "Renderer could not be created. " << SDL_GetError() << endl;
                success=false;
            }

            else {
                // Set white color
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

                // Initialize SDL_image.h
                int imgFlags=IMG_INIT_PNG;
                if (!(IMG_Init(imgFlags)&imgFlags)) {
                    cout << "SDL_image could not initialize. " << IMG_GetError() << endl;
                    success=false;
                }

                // Initialize SDL_ttf.h
                if (TTF_Init()==-1) {
                    cout << "SDL_ttf could not initialize. " << TTF_GetError() << endl;
                    success=false;
                }
            }
        }
    }

    return success;
}

// Load font + sprites
bool loadMedia() {
    bool success=true;

    gTinyFont=TTF_OpenFont("Resources/AmaticSC-Bold.ttf", 32);
    gSmallFont=TTF_OpenFont("Resources/AmaticSC-Bold.ttf", 48);
    gMediumFont=TTF_OpenFont("Resources/AmaticSC-Bold.ttf", 72);
    gLargeFont=TTF_OpenFont("Resources/AmaticSC-Bold.ttf", 120);
    gXtraFont=TTF_OpenFont("Resources/AmaticSC-Bold.ttf", 166);
    if (gSmallFont==nullptr || gMediumFont==nullptr || gLargeFont==nullptr) {
        cout << "Failed to load font. " << TTF_GetError() << endl;
        success=false;
    }
    else {
        if (!instructionTexture[0].loadFromRenderedText("Press any key to start", textColor, gMediumFont) ||
            !instructionTexture[1].loadFromRenderedText("Press left/right to customize background", textColor, gMediumFont) ||
            !instructionTexture[2].loadFromRenderedText("Press left/right to customize color", textColor, gMediumFont) ||
            !instructionTexture[3].loadFromRenderedText("Press up/down to select settings", textColor, gMediumFont) ||
            !instructionTexture[4].loadFromRenderedText("Press Enter to finish", textColor, gMediumFont) ||

            !instructionTexture[10].loadFromRenderedText("Press R to restart, ESC to exit", textColor, gMediumFont) ||

            !instructionTexture[20].loadFromRenderedText("Click block: ", textColor, gMediumFont) ||
            !instructionTexture[21].loadFromRenderedText("Passive income: ", textColor, gMediumFont) ||
            !instructionTexture[22].loadFromRenderedText("Total money: ", textColor, gMediumFont) ||

            !gameTitleTexture.loadFromRenderedText("Die to Win", textColor, gXtraFont) ||
            !winMsgTexture.loadFromRenderedText("Congratulations", textColor, gLargeFont)) {
            cout << "Failed to render text texture." << endl;
            success=false;
        }
    }

    if (!blockSheetTexture.loadFromFile("Resources/Block and Spike.png")) {
        cout << "Failed to load block and spike texture." << endl;
        success=false;
    }
    else {
        // Level corner, top left corner by default
        blockClips[0]={0, 0, 160, 160};

        // Wall block, horizontal by default
        blockClips[1]={160, 0, 160, 160};

        // T-block, wall side facing left by default
        blockClips[2]={0, 160, 160, 160};

        // Platform tip block, facing up by default
        blockClips[3]={160, 160, 160, 160};

        // No border block
        blockClips[4]={0, 320, 160, 160};

        // All border block
        blockClips[5]={160, 320, 160, 160};

        // Idle tycoon block 1 - misc upgrade
        blockClips[6]={0, 480, 160, 160};

        // Idle tycoon block 2 - lower the point block
        blockClips[7]={160, 480, 160, 160};

        // Idle tycoon block 3 - point upgrade
        blockClips[8]={320, 480, 160, 160};

        // Idle tycoon block 4 - passive income upgrade
        blockClips[9]={480, 480, 160, 160};

        // Idle tycoon - point block
        blockClips[10]={640, 480, 160, 160};

        // Menu block 1 - settings
        blockClips[11]={0, 640, 160, 160};

        // Menu block 2 - start
        blockClips[12]={160, 640, 160, 160};

        // Menu block 3 - credits
        blockClips[13]={320, 640, 160, 160};

        // Password puzzle - check solution
        blockClips[14]={0, 800, 160, 160};

        // Pool puzzle - add water
        blockClips[15]={160, 800, 160, 160};

        // Time puzzle - stop time
        blockClips[16]={320, 800, 160, 160};

        // Movable block
        blockClips[17]={480, 640, 320, 320};

        // Tic-tac-toe puzzle - move X to next position
        blockClips[18]={1280, 0, 160, 160};

        // Tic-tac-toe puzzle - X block (interactable)
        // Tic-tac-toe puzzle - X block
        blockClips[19]={1440, 0, 160, 160};

        // Tic-tac-toe puzzle - O block
        blockClips[20]={1600, 0, 160, 160};

        // Reset puzzle
        blockClips[21]={1760, 0, 160, 160};

        // Electricity puzzle - deplete
        blockClips[22]={1760, 160, 160, 160};

        // Corner block, top left corner by default
        blockClips[23]={320, 320, 160, 160};

        // Line block, facing up by default
        blockClips[24]={480, 320, 160, 160};

        // Platform tip with spike on top, platform tip facing left, spike facing up by default
        blockClips[25]={320, 160, 160, 160};
        spikeClips[0]={320, 0, 160, 160};

        // Platform with spike on top, spike facing up by default
        blockClips[26]={480, 160, 160, 160};
        spikeClips[1]={480, 0, 160, 160};

        // Platform with big spike on top, spike facing up by default
        blockClips[27]={640, 160, 160, 160};
        spikeClips[2]={640, 0, 160, 160};
    }

    if (!cubeTexture.loadFromFile("Resources/Player.png")) {
        cout << "Failed to load cube texture." << endl;
        success=false;
    }

    if (!orbPadSheetTexture.loadFromFile("Resources/Orb and Pad.png")) {
        cout << "Failed to load orb and pad texture." << endl;
        success=false;
    }
    else {
        orbClips[0]={0, 0, 160, 160};       // Yellow orb
        orbClips[1]={160, 0, 160, 160};     // Blue orb
        orbClips[2]={320, 0, 160, 160};     // Green orb

        padClips[0]={0, 160, 160, 160};     // Yellow pad
        padClips[1]={160, 160, 160, 160};   // Spider pad
        padClips[2]={320, 160, 160, 160};   // Pink pad
    }

    if (!backgroundTexture[STRIPE].loadFromFile("Resources/Stripe BG.png") ||
        !backgroundTexture[TETRIS].loadFromFile("Resources/Tetris BG.png") ||
        !backgroundTexture[BLANK].loadFromFile("Resources/Blank BG.png")) {

        cout << "Failed to load background texture." << endl;
        success=false;
    }

    return success;
}

// Cleanup
void close() {
    // Deal with textures
    blockSheetTexture.free();
    orbPadSheetTexture.free();
    cubeTexture.free();
    for (int i=0; i<TOTAL_BG; i++) {
        backgroundTexture[i].free();
    }

    for (int i=0; i<100; i++) {
        instructionTexture[i].free();
    }
    gameTitleTexture.free();
    winMsgTexture.free();

    // Deal with fonts
    TTF_CloseFont(gTinyFont);
    gTinyFont=nullptr;
    TTF_CloseFont(gSmallFont);
    gSmallFont=nullptr;
    TTF_CloseFont(gMediumFont);
    gMediumFont=nullptr;
    TTF_CloseFont(gLargeFont);
    gLargeFont=nullptr;
    TTF_CloseFont(gXtraFont);
    gXtraFont=nullptr;

    // Deal with window & renderer
    SDL_DestroyRenderer(gRenderer);
    gRenderer=nullptr;
    SDL_DestroyWindow(gWindow);
    gWindow=nullptr;

    // Deal with libraries
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// Split the level into tiles to place objects
const int TILE_SIZE=SCREEN_HEIGHT/10;
const int LEVEL_WIDTH=19;
const int LEVEL_HEIGHT=11;

// Vector to store objects
vector<Block> blocks;
vector<Spike> spikes;
vector<JumpOrb> jumpOrbs;
vector<JumpPad> jumpPads;

// Identify block type
struct BlockInfo {
    int clipIndex;
    double rotation;
    SDL_RendererFlip mirrored;
};
unordered_map<string, BlockInfo> blockLookup={
    {"1C0", {0, 0, SDL_FLIP_NONE}},     // Top left level corner
    {"1C1", {0, 90, SDL_FLIP_NONE}},    // Top right level corner
    {"1C2", {0, 180, SDL_FLIP_NONE}},   // Bottom right level corner
    {"1C3", {0, 270, SDL_FLIP_NONE}},   // Bottom left level corner

    {"1WH", {1, 0, SDL_FLIP_NONE}},     // Horizontal wall
    {"1WV", {1, 90, SDL_FLIP_NONE}},    // Vertical wall

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
    {"1MV", {17, 0, SDL_FLIP_NONE}},    // Movable block

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
struct SpikeInfo {
    int clipIndex;
    double rotation;
    SDL_RendererFlip mirrored;
};
unordered_map<string, SpikeInfo> spikeLookup={ // Note: Spike platform migrated to blocks (number 3 in front)
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

// Identify jump pad type
struct JumpPadInfo {
    int clipIndex;
    double rotation;
};
unordered_map<string, JumpPadInfo> jumpPadLookup={
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
void loadLevel(const string &path, vector<Block> &blocks, vector<Spike> &spikes, vector<JumpOrb> &jumpOrbs, vector<JumpPad> &jumpPads) {
    ifstream file(path);
    if (!file.is_open()) {
        cout << "Failed to open level file." << endl;
        return;
    }

    blocks.clear(); spikes.clear(); jumpOrbs.clear(); jumpPads.clear();

    // Read file and store objects
    string tile;
    int row=0;
    while (row<LEVEL_HEIGHT) {
        for (int col=0; col<LEVEL_WIDTH; col++) {
            float baseX=col*TILE_SIZE-TILE_SIZE*11/18;
            float baseY=row*TILE_SIZE-TILE_SIZE*9/18;

            file >> tile;

            // Store blocks
            if (blockLookup.find(tile)!=blockLookup.end()) {
                BlockInfo info=blockLookup[tile];
                blocks.emplace_back(baseX, baseY, TILE_SIZE, TILE_SIZE, info.rotation, info.mirrored, tile);
            }

            // Store spikes
            else if (spikeLookup.find(tile)!=spikeLookup.end()) {
                SpikeInfo info=spikeLookup[tile];
                if (tile[1]=='A' || tile[1]=='C') { // Small spike
                    if (info.rotation==0) {
                        spikes.emplace_back(baseX+TILE_SIZE*2/5.0f, baseY+TILE_SIZE*7/10.0f, TILE_SIZE/5.0f, TILE_SIZE/5.0f, info.rotation, info.mirrored, tile);
                    }
                    else if (info.rotation==90) {
                        spikes.emplace_back(baseX+TILE_SIZE/10.0f, baseY+TILE_SIZE*2/5.0f, TILE_SIZE/5.0f, TILE_SIZE/5.0f, info.rotation, info.mirrored, tile);
                    }
                    else if (info.rotation==180) {
                        spikes.emplace_back(baseX+TILE_SIZE*2/5.0f, baseY+TILE_SIZE/10.0f, TILE_SIZE/5.0f, TILE_SIZE/5.0f, info.rotation, info.mirrored, tile);
                    }
                    else if (info.rotation==270) {
                        spikes.emplace_back(baseX+TILE_SIZE*7/10.0f, baseY+TILE_SIZE*2/5.0f, TILE_SIZE/5.0f, TILE_SIZE/5.0f, info.rotation, info.mirrored, tile);
                    }
                }
                else if (tile[1]=='E') { // Big spike
                    if (info.rotation==0 || info.rotation==180) {
                        spikes.emplace_back(baseX+TILE_SIZE*2/5.0f, baseY+TILE_SIZE*3/10.0f, TILE_SIZE/5.0f, TILE_SIZE*2/5.0f, info.rotation, info.mirrored, tile);
                    }
                    else if (info.rotation==90 || info.rotation==270) {
                        spikes.emplace_back(baseX+TILE_SIZE*3/10.0f, baseY+TILE_SIZE*2/5.0f, TILE_SIZE*2/5.0f, TILE_SIZE/5.0f, info.rotation, info.mirrored, tile);
                    }
                }
            }

            // Store jump pads
            else if (jumpPadLookup.find(tile)!=jumpPadLookup.end()) {
                JumpPadInfo info=jumpPadLookup[tile];
                if (tile[0]=='J' || tile[0]=='P') {
                    if (info.rotation==0) {
                        jumpPads.emplace_back(baseX+TILE_SIZE/12.0f, baseY+TILE_SIZE*13/15.0f, TILE_SIZE*10/12.0f, TILE_SIZE/6.0f, info.rotation, tile);
                    }
                    else if (info.rotation==180) {
                        jumpPads.emplace_back(baseX+TILE_SIZE/12.0f, baseY-TILE_SIZE/30.0f, TILE_SIZE*10/12.0f, TILE_SIZE/6.0f, info.rotation, tile);
                    }
                }
                else if (tile[0]=='S') { // Spider pad
                    if (info.rotation==0) {
                        jumpPads.emplace_back(baseX+TILE_SIZE/30.0f, baseY+TILE_SIZE*3/4.0f, TILE_SIZE*14/15.0f, TILE_SIZE*2/5.0f, info.rotation, tile);
                    }
                    else if (info.rotation==90) {
                        jumpPads.emplace_back(baseX-TILE_SIZE*3/20.0f, baseY+TILE_SIZE/30.0f, TILE_SIZE*2/5.0f, TILE_SIZE*14/15.0f, info.rotation, tile);
                    }
                    else if (info.rotation==180) {
                        jumpPads.emplace_back(baseX+TILE_SIZE/30.0f, baseY-TILE_SIZE*3/20.0f, TILE_SIZE*14/15.0f, TILE_SIZE*2/5.0f, info.rotation, tile);
                    }
                    else if (info.rotation==270) {
                        jumpPads.emplace_back(baseX+TILE_SIZE*3/4.0f, baseY+TILE_SIZE/30.0f, TILE_SIZE*2/5.0f, TILE_SIZE*14/15.0f, info.rotation, tile);
                    }
                }
            }

            // Store jump orbs
            else {
                switch (tile[0]) {
                case 'Y': // Yellow orb
                case 'B': // Blue orb
                case 'G': { // Green orb
                    jumpOrbs.emplace_back(baseX-TILE_SIZE/10.0f, baseY-TILE_SIZE/10.0f, TILE_SIZE*12/10.0f, TILE_SIZE*12/10.0f, tile[0]);
                    break;
                    }
                }
            }
        }

        row++;
    }

    file.close();
}

void renderLevel(const vector<Block> &blocks, const vector<Spike> &spikes,
                 const vector<JumpOrb> &jumpOrbs, const vector<JumpPad> &jumpPads, double deltaTime) {
    // Render orbs
    for (const auto &orb : jumpOrbs) {
        SDL_FRect renderOrb={orb.getHitbox().x+TILE_SIZE/10.0f, orb.getHitbox().y+TILE_SIZE/10.0f, TILE_SIZE, TILE_SIZE};
        switch (orb.getType()) {
        case 'Y': // Yellow
            orbPadSheetTexture.render(renderOrb, &orbClips[0], 0, nullptr, SDL_FLIP_NONE);
            break;
        case 'B': // Blue
            orbPadSheetTexture.render(renderOrb, &orbClips[1], 0, nullptr, SDL_FLIP_NONE);
            break;
        case 'G': // Green
            orb.updateRotation(deltaTime);
            orbPadSheetTexture.render(renderOrb, &orbClips[2], orb.rotationAngle, nullptr, SDL_FLIP_NONE);
            break;
        }
    }

    // Render pads
    for (const auto &pad : jumpPads) {
        string type=pad.getType();
        if (jumpPadLookup.find(type)!=jumpPadLookup.end()) {
            JumpPadInfo info=jumpPadLookup[type];
            SDL_FRect renderPad;
            if (type[0]=='J' || type[0]=='P') {
                if (info.rotation==0) {
                    renderPad={pad.getHitbox().x-TILE_SIZE/12.0f, pad.getHitbox().y-TILE_SIZE*13/15.0f, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==180) {
                    renderPad={pad.getHitbox().x-TILE_SIZE/12.0f, pad.getHitbox().y+TILE_SIZE/30.0f, TILE_SIZE, TILE_SIZE};
                }
            }
            else if (type[0]=='S') { // Spider
                if (info.rotation==0) {
                    renderPad={pad.getHitbox().x-TILE_SIZE/30.0f, pad.getHitbox().y-TILE_SIZE*3/4.0f, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==90) {
                    renderPad={pad.getHitbox().x+TILE_SIZE*3/20.0f, pad.getHitbox().y-TILE_SIZE/30.0f, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==180) {
                    renderPad={pad.getHitbox().x-TILE_SIZE/30.0f, pad.getHitbox().y+TILE_SIZE*3/20.0f, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==270) {
                    renderPad={pad.getHitbox().x-TILE_SIZE*3/4.0f, pad.getHitbox().y-TILE_SIZE/30.0f, TILE_SIZE, TILE_SIZE};
                }
            }
            orbPadSheetTexture.render(renderPad, &padClips[info.clipIndex], info.rotation, nullptr, SDL_FLIP_NONE);
        }
    }

    // Render platforms (blocks)
    for (const auto& block : blocks) {
        string type=block.getType();
        if (blockLookup.find(type)!=blockLookup.end()) {
            BlockInfo info=blockLookup[type];
            SDL_FRect renderBlock=block.getHitbox();
            blockSheetTexture.render(renderBlock, &blockClips[info.clipIndex], info.rotation, nullptr, info.mirrored);
        }
    }

    // Render spikes
    for (const auto& spike : spikes) {
        string type=spike.getType();
        if (spikeLookup.find(type)!=spikeLookup.end()) {
            SpikeInfo info=spikeLookup[type];
            SDL_FRect renderSpike;
            if (type[1]=='A' || type[1]=='C') { // Small spike
                if (info.rotation==0) {
                    renderSpike={spike.getHitbox().x-TILE_SIZE*2/5.0f, spike.getHitbox().y-TILE_SIZE*7/10.0f, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==90) {
                    renderSpike={spike.getHitbox().x-TILE_SIZE/10.0f, spike.getHitbox().y-TILE_SIZE*2/5.0f, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==180) {
                    renderSpike={spike.getHitbox().x-TILE_SIZE*2/5.0f, spike.getHitbox().y-TILE_SIZE/10.0f, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==270) {
                    renderSpike={spike.getHitbox().x-TILE_SIZE*7/10.0f, spike.getHitbox().y-TILE_SIZE*2/5.0f, TILE_SIZE, TILE_SIZE};
                }
            }
            else if (type[1]=='E') { // Big spike
                if (info.rotation==0 || info.rotation==180) {
                    renderSpike={spike.getHitbox().x-TILE_SIZE*2/5.0f, spike.getHitbox().y-TILE_SIZE*3/10.0f, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==90 || info.rotation==270) {
                    renderSpike={spike.getHitbox().x-TILE_SIZE*3/10.0f, spike.getHitbox().y-TILE_SIZE*2/5.0f, TILE_SIZE, TILE_SIZE};
                }
            }
            blockSheetTexture.render(renderSpike, &spikeClips[info.clipIndex], info.rotation, nullptr, info.mirrored);
        }
    }
}

void displayTextInLevel(Player cube, vector<Block> &blocks, GameStatus currentStatus, GameSetting currentSetting) {
    /*instructionTexture[20].loadFromRenderedText("Click block: "+to_string(cube.getGainPerHit()), textColor, gMediumFont);
    instructionTexture[20].render(SCREEN_WIDTH-instructionTexture[20].getWidth(), TILE_SIZE);

    instructionTexture[21].loadFromRenderedText("Passive income: "+to_string(cube.getPassiveIncome()), textColor, gMediumFont);
    instructionTexture[21].render(SCREEN_WIDTH-instructionTexture[21].getWidth(), 2*TILE_SIZE);

    instructionTexture[22].loadFromRenderedText("Total money: "+to_string(cube.getTotalMoney()), textColor, gMediumFont);
    instructionTexture[22].render(SCREEN_WIDTH-instructionTexture[22].getWidth(), 3*TILE_SIZE);*/

    for (const Block &block : blocks) {
        if (block.getType()=="1I1" || block.getType()=="1I2" || block.getType()=="1I3" || block.getType()=="1I4") {
            instructionTexture[25].loadFromRenderedText(to_string(block.counter), textColor, gTinyFont);
            instructionTexture[25].render(block.getHitbox().x+TILE_SIZE/12, block.getHitbox().y);
        }
        if (block.getType()=="1S") {
            instructionTexture[30].loadFromRenderedText("Settings", textColor, gMediumFont);
            instructionTexture[30].render(block.getHitbox().x+(block.getHitbox().w-instructionTexture[30].getWidth())/2,
                                           block.getHitbox().y-instructionTexture[30].getHeight());
        }
        if (block.getType()=="1P") {
            instructionTexture[30].loadFromRenderedText("Play", textColor, gMediumFont);
            instructionTexture[30].render(block.getHitbox().x+(block.getHitbox().w-instructionTexture[30].getWidth())/2,
                                           block.getHitbox().y-instructionTexture[30].getHeight());
        }
        if (block.getType()=="1C") {
            instructionTexture[30].loadFromRenderedText("Credits", textColor, gMediumFont);
            instructionTexture[30].render(block.getHitbox().x+(block.getHitbox().w-instructionTexture[30].getWidth())/2,
                                           block.getHitbox().y-instructionTexture[30].getHeight());
        }
        if (block.getType()=="1K0") {
            gameTitleTexture.render(block.getHitbox().x+(9*TILE_SIZE-gameTitleTexture.getWidth())/2, block.getHitbox().y);
        }
        if (block.getType()=="1K2") {
            instructionTexture[30].loadFromRenderedText("v0.08 ", textColor, gSmallFont);
            instructionTexture[30].render(block.getHitbox().x+block.getHitbox().w-instructionTexture[30].getWidth(),
                                           block.getHitbox().y+block.getHitbox().h-instructionTexture[30].getHeight());
        }
    }
}

int main(int argc, char *argv[]) {
    if (!init()) {
        cout << "Failed to initialize." << endl;
    }
    else {
        if (!loadMedia()) {
            cout << "Failed to load media." << endl;
        }
        else {
            // Load level, will try to store move levels later
            loadLevel("Resources/Levels/Menu.txt", blocks, spikes, jumpOrbs, jumpPads);

            // Delta time, to keep physics consistent across all refresh rates
            Uint64 NOW=SDL_GetPerformanceCounter();
            Uint64 LAST=0;
            double deltaTime=0;

            Player cube;
            static double fadeAlpha=200;
            bool dead=false;
            float scrollingOffset=0;
            GameStatus currentStatus=MENU;
            GameSetting currentSetting=SETTING_BG;

            bool quit=false;
            SDL_Event e;

            // Running
            while (!quit) {
                // Calculate delta time
                LAST=NOW;
                NOW=SDL_GetPerformanceCounter();
                deltaTime=double((NOW-LAST)*1000)/SDL_GetPerformanceFrequency();
                deltaTime/=1000.0; // Convert to seconds

                // Handle game events
                while (SDL_PollEvent(&e)) {
                    if (e.type==SDL_QUIT) {
                        quit=true;
                    }
                    else if (currentStatus==SETTINGS && e.type==SDL_KEYDOWN) {
                        switch (e.key.keysym.sym) {
                        case SDLK_UP:
                        case SDLK_w:
                            currentSetting=static_cast<GameSetting>((currentSetting+1)%TOTAL_SETTING);
                            break;
                        case SDLK_DOWN:
                        case SDLK_s:
                            currentSetting=static_cast<GameSetting>((currentSetting-1+TOTAL_SETTING)%TOTAL_SETTING);
                            break;
                        case SDLK_LEFT:
                        case SDLK_a:
                            if (currentSetting==SETTING_BG) {
                                selectedBG=static_cast<Background>((selectedBG-1+TOTAL_BG)%TOTAL_BG);
                            }
                            else if (currentSetting==SETTING_COLOR) {
                                selectedColor=static_cast<Color>((selectedColor-1+TOTAL_COLOR)%TOTAL_COLOR);
                            }
                            break;
                        case SDLK_RIGHT:
                        case SDLK_d:
                            if (currentSetting==SETTING_BG) {
                                selectedBG=static_cast<Background>((selectedBG+1)%TOTAL_BG);
                            }
                            else if (currentSetting==SETTING_COLOR) {
                                selectedColor=static_cast<Color>((selectedColor+1)%TOTAL_COLOR);
                            }
                            break;
                        case SDLK_RETURN:
                            currentStatus=MENU;
                            break;
                        }
                    }
                    else if (currentStatus==WIN && e.type==SDL_KEYDOWN) {
                        if (e.key.keysym.sym==SDLK_r) {
                            currentStatus=RESTART;
                        }
                        else if (e.key.keysym.sym==SDLK_ESCAPE) {
                            quit=true;
                        }
                    }
                    else if (currentStatus==MENU || currentStatus==PLAYING) {
                        cube.handleEvent(e);
                    }
                }

                if (currentStatus==START) {
                    loadLevel("Resources/Levels/test.txt", blocks, spikes, jumpOrbs, jumpPads);
                    cube.reset();
                    fadeAlpha=0;
                    currentStatus=PLAYING;
                }

                // Playing
                if (currentStatus==PLAYING) {
                    // Handle player interactions
                    cube.move(blocks, jumpOrbs, currentStatus, deltaTime);
                    cube.interact(blocks, spikes, jumpOrbs, jumpPads, deltaTime, dead);
                    if (dead) {
                        currentStatus=WIN;
                    }
                }

                // Menu screen
                if (currentStatus==MENU) {
                    cube.move(blocks, jumpOrbs, currentStatus, deltaTime);
                    cube.interact(blocks, spikes, jumpOrbs, jumpPads, deltaTime, dead);
                }

                // Restart after win
                if (currentStatus==RESTART) {
                    dead=false;
                    fadeAlpha=0;
                    cube.reset();
                    loadLevel("Resources/Levels/test.txt", blocks, spikes, jumpOrbs, jumpPads);
                    currentStatus=PLAYING;
                    continue;
                }

                // Render level
                scrollingOffset+=60*deltaTime;
                if (scrollingOffset>SCREEN_HEIGHT) {
                    scrollingOffset=0;
                }

                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(gRenderer);

                SDL_Color currentBGColor=bgColor[selectedColor];
                backgroundTexture[selectedBG].setColor(currentBGColor);
                SDL_FRect backgroundRect={0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                if (selectedBG==STRIPE) {
                    for (int i=0; i<2; i++) {
                        backgroundRect.y=backgroundRect.h*i-scrollingOffset;
                        backgroundTexture[selectedBG].render(backgroundRect);
                    }
                }
                else if (selectedBG==TETRIS) {
                    for (int i=0; i<2; i++) {
                        backgroundRect.y=-backgroundRect.h*i+scrollingOffset;
                        backgroundTexture[selectedBG].render(backgroundRect);
                    }
                }
                else if (selectedBG==BLANK) {
                    backgroundTexture[selectedBG].render(backgroundRect);
                }

                renderLevel(blocks, spikes, jumpOrbs, jumpPads, deltaTime);
                cube.render();
                displayTextInLevel(cube, blocks, currentStatus, currentSetting);

                // Settings screen
                if (currentStatus==SETTINGS) {
                    cube.resetBool();

                    float textPosY=(SCREEN_HEIGHT-3*instructionTexture[3].getHeight())/2;
                    fadeAlpha=200;
                    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, static_cast<Uint8>(fadeAlpha));
                    SDL_FRect dimOverlay={SCREEN_WIDTH/10, textPosY, SCREEN_WIDTH*8/10, 3*instructionTexture[3].getHeight()};
                    SDL_RenderFillRectF(gRenderer, &dimOverlay);

                    instructionTexture[3].render((SCREEN_WIDTH-instructionTexture[3].getWidth())/2, textPosY);
                    textPosY+=instructionTexture[3].getHeight();
                    if (currentSetting==SETTING_BG) {
                        instructionTexture[1].render((SCREEN_WIDTH-instructionTexture[1].getWidth())/2, textPosY);
                    }
                    else if (currentSetting==SETTING_COLOR) {
                        instructionTexture[2].render((SCREEN_WIDTH-instructionTexture[2].getWidth())/2, textPosY);
                    }
                    textPosY+=instructionTexture[1].getHeight();
                    instructionTexture[4].render((SCREEN_WIDTH-instructionTexture[4].getWidth())/2, textPosY);
                }

                // Win screen
                if (currentStatus==WIN) {
                    if (fadeAlpha<200) {
                        fadeAlpha+=400*deltaTime;
                        if (fadeAlpha>200) {
                            fadeAlpha=200;
                        }
                    }

                    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, static_cast<Uint8>(fadeAlpha));
                    SDL_FRect dimOverlay={0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                    SDL_RenderFillRectF(gRenderer, &dimOverlay);

                    SDL_FRect winMsgRect={(SCREEN_WIDTH-winMsgTexture.getWidth())/2,
                                          (SCREEN_HEIGHT-winMsgTexture.getHeight()-instructionTexture[10].getHeight())/2,
                                          winMsgTexture.getWidth(),
                                          winMsgTexture.getHeight()};
                    winMsgTexture.setAlpha(static_cast<Uint8>(fadeAlpha)*255/200);
                    winMsgTexture.render(winMsgRect);

                    instructionTexture[10].setAlpha(static_cast<Uint8>(fadeAlpha)*255/200);
                    instructionTexture[10].render((SCREEN_WIDTH-instructionTexture[10].getWidth())/2, winMsgRect.y+winMsgRect.h);
                }

                SDL_RenderPresent(gRenderer);
            }
        }
    }
    close();
    return 0;
}
