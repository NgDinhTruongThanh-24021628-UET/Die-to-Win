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
using namespace std;

// Window sizes
const int SCREEN_WIDTH=1280;
const int SCREEN_HEIGHT=720;

// Window to render to
SDL_Window *gWindow=nullptr;

// Renderer
SDL_Renderer *gRenderer=nullptr;

// Font (for menu + instructions, later)
TTF_Font *gFont=nullptr;

// Textures
LTexture cubeTexture;

LTexture yellowOrbTexture;
LTexture blueOrbTexture;
LTexture greenOrbTexture;

LTexture yellowPadTexture;
LTexture spiderPadTexture;
LTexture pinkPadTexture;

const int NUMBER_OF_BLOCKS=4;
const int NUMBER_OF_SPIKES=6;
SDL_Rect blockClips[NUMBER_OF_BLOCKS];
SDL_Rect spikeClips[NUMBER_OF_SPIKES];
LTexture blockSheetTexture;

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
            }
        }
    }

    return success;
}

// Load sprites
bool loadMedia() {
    bool success=true;
    if (!blockSheetTexture.loadFromFile("Sprites/Block and Spike.png")) {
        cout << "Failed to load block and spike texture." << endl;
        success=false;
    }
    else {
        // Corner block, top left corner by default
        blockClips[0].x=0;
        blockClips[0].y=0;
        blockClips[0].w=160;
        blockClips[0].h=160;

        // Wall block, horizontal by default
        blockClips[1].x=160;
        blockClips[1].y=0;
        blockClips[1].w=160;
        blockClips[1].h=160;

        // T-block, wall side facing left by default
        blockClips[2].x=0;
        blockClips[2].y=160;
        blockClips[2].w=160;
        blockClips[2].h=160;

        // Platform tip block, facing up by default
        blockClips[3].x=160;
        blockClips[3].y=160;
        blockClips[3].w=160;
        blockClips[3].h=160;

        // Platform tip[0] with spike on top[1], platform tip facing left, spike facing up by default
        spikeClips[0].x=320;
        spikeClips[0].y=160;
        spikeClips[0].w=160;
        spikeClips[0].h=160;

        spikeClips[1].x=320;
        spikeClips[1].y=0;
        spikeClips[1].w=160;
        spikeClips[1].h=160;

        // Platform[2] with spike on top[3], spike facing up by default
        spikeClips[2].x=480;
        spikeClips[2].y=160;
        spikeClips[2].w=160;
        spikeClips[2].h=160;

        spikeClips[3].x=480;
        spikeClips[3].y=0;
        spikeClips[3].w=160;
        spikeClips[3].h=160;

        // Platform[4] with big spike on top[5], spike facing up by default
        spikeClips[4].x=640;
        spikeClips[4].y=160;
        spikeClips[4].w=160;
        spikeClips[4].h=160;

        spikeClips[5].x=640;
        spikeClips[5].y=0;
        spikeClips[5].w=160;
        spikeClips[5].h=160;
    }
    if (!cubeTexture.loadFromFile("Sprites/Player.png")) {
        cout << "Failed to load cube texture." << endl;
        success=false;
    }
    if (!yellowOrbTexture.loadFromFile("Sprites/Yellow Orb.png") ||
        !blueOrbTexture.loadFromFile("Sprites/Blue Orb.png") ||
        !greenOrbTexture.loadFromFile("Sprites/Green Orb.png")) {

        cout << "Failed to load orb texture." << endl;
        success=false;
    }
    if (!yellowPadTexture.loadFromFile("Sprites/Yellow Pad.png") ||
        !spiderPadTexture.loadFromFile("Sprites/Spider Pad.png") ||
        !pinkPadTexture.loadFromFile("Sprites/Pink Pad.png")) {

        cout << "Failed to load pad texture." << endl;
        success=false;
    }
    return success;
}

// Cleanup
void close() {
    // Deal with textures
    blockSheetTexture.free();
    cubeTexture.free();
    yellowOrbTexture.free();
    blueOrbTexture.free();
    greenOrbTexture.free();
    yellowPadTexture.free();
    spiderPadTexture.free();
    pinkPadTexture.free();

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
};
unordered_map<string, BlockInfo> blockLookup={
    {"1C0", {0, 0}},     // Top left corner
    {"1C1", {0, 90}},    // Top right corner
    {"1C2", {0, 180}},   // Bottom right corner
    {"1C3", {0, 270}},   // Bottom left corner
    {"1WH", {1, 0}},     // Horizontal wall
    {"1WV", {1, 90}},    // Vertical wall
    {"1TL", {2, 0}},     // T-block left
    {"1TU", {2, 90}},    // T-block up
    {"1TR", {2, 180}},   // T-block right
    {"1TD", {2, 270}},   // T-block down
    {"1PU", {3, 0}},     // Platform tip up
    {"1PR", {3, 90}},    // Platform tip right
    {"1PD", {3, 180}},   // Platform tip down
    {"1PL", {3, 270}},   // Platform tip left
};

// Identify spike type
struct SpikeInfo {
    int clipIndex;
    double rotation;
    SDL_RendererFlip mirrored;
};
unordered_map<string, SpikeInfo> spikeLookup={
    // Platform tip with spike, facing up, mirrored or not
    {"2AU", {1, 0, SDL_FLIP_NONE}},
    {"2BU", {0, 0, SDL_FLIP_NONE}},
    {"2AUM", {1, 0, SDL_FLIP_HORIZONTAL}},
    {"2BUM", {0, 0, SDL_FLIP_HORIZONTAL}},

    // Platform tip with spike, facing right, mirrored or not
    {"2AR", {1, 90, SDL_FLIP_NONE}},
    {"2BR", {0, 90, SDL_FLIP_NONE}},
    {"2ARM", {1, 90, SDL_FLIP_HORIZONTAL}},
    {"2BRM", {0, 90, SDL_FLIP_HORIZONTAL}},

    // Platform tip with spike, facing down, mirrored or not
    {"2AD", {1, 180, SDL_FLIP_NONE}},
    {"2BD", {0, 180, SDL_FLIP_NONE}},
    {"2ADM", {1, 180, SDL_FLIP_HORIZONTAL}},
    {"2BDM", {0, 180, SDL_FLIP_HORIZONTAL}},

    // Platform tip with spike, facing left, mirrored or not
    {"2AL", {1, 270, SDL_FLIP_NONE}},
    {"2BL", {0, 270, SDL_FLIP_NONE}},
    {"2ALM", {1, 270, SDL_FLIP_HORIZONTAL}},
    {"2BLM", {0, 270, SDL_FLIP_HORIZONTAL}},

    // Normal platform with spike, facing up
    {"2CU", {3, 0, SDL_FLIP_NONE}},
    {"2DU", {2, 0, SDL_FLIP_NONE}},

    // Normal platform with spike, facing right
    {"2CR", {3, 90, SDL_FLIP_NONE}},
    {"2DR", {2, 90, SDL_FLIP_NONE}},

    // Normal platform with spike, facing down
    {"2CD", {3, 180, SDL_FLIP_NONE}},
    {"2DD", {2, 180, SDL_FLIP_NONE}},

    // Normal platform with spike, facing left
    {"2CL", {3, 270, SDL_FLIP_NONE}},
    {"2DL", {2, 270, SDL_FLIP_NONE}},

    // Normal platform with big spike, facing up
    {"2EU", {5, 0, SDL_FLIP_NONE}},
    {"2FU", {4, 0, SDL_FLIP_NONE}},

    // Normal platform with big spike, facing right
    {"2ER", {5, 90, SDL_FLIP_NONE}},
    {"2FR", {4, 90, SDL_FLIP_NONE}},

    // Normal platform with big spike, facing down
    {"2ED", {5, 180, SDL_FLIP_NONE}},
    {"2FD", {4, 180, SDL_FLIP_NONE}},

    // Normal platform with big spike, facing left
    {"2EL", {5, 270, SDL_FLIP_NONE}},
    {"2FL", {4, 270, SDL_FLIP_NONE}},
};

// Load level from a file
void loadLevel(const string &path, vector<Block> &blocks, vector<Spike> &spikes, vector<JumpOrb> &jumpOrbs, vector<JumpPad> &jumpPads) {
    ifstream file(path);
    if (!file.is_open()) {
        cout << "Failed to open level file." << endl;
        return;
    }

    // Read file and store objects
    string tile;
    int row=0;
    while (row<LEVEL_HEIGHT) {
        for (int col=0; col<LEVEL_WIDTH; col++) {
            int baseX=col*TILE_SIZE-TILE_SIZE*11/18;
            int baseY=row*TILE_SIZE-TILE_SIZE*9/18;

            file >> tile;

            if (blockLookup.find(tile)!=blockLookup.end()) {
                BlockInfo info=blockLookup[tile];
                blocks.emplace_back(baseX, baseY, TILE_SIZE, TILE_SIZE, info.rotation, tile);
            }
            else if (spikeLookup.find(tile)!=spikeLookup.end()) {
                SpikeInfo info=spikeLookup[tile];
                spikes.emplace_back(baseX+TILE_SIZE*3/8, baseY+TILE_SIZE/4, TILE_SIZE/4, TILE_SIZE/2, info.rotation, info.mirrored, tile);
            }
            else {
                switch (tile[0]) {
                case 'Y': // Yellow orb
                case 'B': // Blue orb
                case 'G': { // Green orb
                    jumpOrbs.emplace_back(baseX-TILE_SIZE/10, baseY-TILE_SIZE/10, TILE_SIZE*12/10, TILE_SIZE*12/10, tile[0]);
                    break;
                    }

                case 'J': // Yellow pad
                case 'P': { // Pink pad
                    jumpPads.emplace_back(baseX+TILE_SIZE/12, baseY+TILE_SIZE*13/15, TILE_SIZE*10/12, TILE_SIZE/6, tile[0]);
                    break;
                    }
                case 'S': // Spider pad
                    jumpPads.emplace_back(baseX+TILE_SIZE/30, baseY+TILE_SIZE*3/4, TILE_SIZE*14/15, TILE_SIZE*2/5, tile[0]);
                    break;
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
        SDL_FRect renderOrb={orb.getHitbox().x+TILE_SIZE/10, orb.getHitbox().y+TILE_SIZE/10, TILE_SIZE, TILE_SIZE};
        switch (orb.getType()) {
        case 'Y': // Yellow
            yellowOrbTexture.render(renderOrb);
            break;
        case 'B': // Blue
            blueOrbTexture.render(renderOrb);
            break;
        case 'G': // Green
            orb.updateRotation(deltaTime);
            greenOrbTexture.render(renderOrb, nullptr, orb.rotationAngle, nullptr, SDL_FLIP_NONE);
            break;
        }
    }

    // Render pads
    for (const auto &pad : jumpPads) {
        SDL_FRect renderPad=pad.getHitbox();
        switch (pad.getType()) {
        case 'J': // Yellow
            yellowPadTexture.render(renderPad);
            break;
        case 'S': // Spider
            spiderPadTexture.render(renderPad);
            break;
        case 'P': // Pink
            pinkPadTexture.render(renderPad);
            break;
        }
    }

    // Render platforms (blocks)
    for (const auto& block : blocks) {
        string type=block.getType();
        if (blockLookup.find(type)!=blockLookup.end()) {
            BlockInfo info=blockLookup[type];
            SDL_FRect renderBlock=block.getHitbox();
            blockSheetTexture.render(renderBlock, &blockClips[info.clipIndex], info.rotation, nullptr, SDL_FLIP_NONE);
        }
    }

    // Render spikes
    for (const auto& spike : spikes) {
        string type=spike.getType();
        if (spikeLookup.find(type)!=spikeLookup.end()) {
            SpikeInfo info=spikeLookup[type];
            SDL_FRect renderSpike={spike.getHitbox().x-TILE_SIZE*3/8, spike.getHitbox().y-TILE_SIZE/4, TILE_SIZE, TILE_SIZE};
            blockSheetTexture.render(renderSpike, &spikeClips[info.clipIndex], info.rotation, nullptr, info.mirrored);
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
            loadLevel("level2.txt", blocks, spikes, jumpOrbs, jumpPads);

            // Delta time, to keep physics consistent across all refresh rates
            Uint64 NOW=SDL_GetPerformanceCounter();
            Uint64 LAST=0;
            double deltaTime=0;

            bool quit=false;
            SDL_Event e;
            Player cube;

            // Running
            while (!quit) {
                // Calculate delta time
                LAST=NOW;
                NOW=SDL_GetPerformanceCounter();
                deltaTime=double((NOW-LAST)*1000)/SDL_GetPerformanceFrequency();
                deltaTime/=1000.0; // Convert to seconds

                // Check events
                while (SDL_PollEvent(&e)) {
                    if (e.type==SDL_QUIT || (e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_ESCAPE)) {
                        quit=true;
                    }
                    cube.handleEvent(e);
                }
                // Handle level interactions
                cube.move(blocks, jumpOrbs, deltaTime);
                cube.interact(blocks, spikes, jumpOrbs, jumpPads, quit);

                // Rendering
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x69, 0xB4, 0xFF);
                SDL_RenderClear(gRenderer);

                renderLevel(blocks, spikes, jumpOrbs, jumpPads, deltaTime);

                cube.render();

                SDL_RenderPresent(gRenderer);
            }
        }
        SDL_Delay(200);
    }
    close();
    cout << "You win";
    return 0;
}

