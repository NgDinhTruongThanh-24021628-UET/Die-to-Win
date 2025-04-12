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

// Font
TTF_Font *gSmallFont=nullptr;
TTF_Font *gMediumFont=nullptr;
TTF_Font *gLargeFont=nullptr;

// Textures
LTexture winMsgTexture;
LTexture gameTitleTexture;
LTexture instructionTexture[2];

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

    gSmallFont=TTF_OpenFont("AmaticSC-Bold.ttf", 40);
    gMediumFont=TTF_OpenFont("AmaticSC-Bold.ttf", 72);
    gLargeFont=TTF_OpenFont("AmaticSC-Bold.ttf", 120);
    if (gSmallFont==nullptr || gMediumFont==nullptr || gLargeFont==nullptr) {
        cout << "Failed to load font. " << TTF_GetError() << endl;
        success=false;
    }
    else {
        SDL_Color textColor={255, 255, 255};
        if (!instructionTexture[0].loadFromRenderedText("Press anywhere to play", textColor, gMediumFont) ||
            !instructionTexture[1].loadFromRenderedText("Press R to restart, ESC to exit", textColor, gMediumFont) ||
            !gameTitleTexture.loadFromRenderedText("Die to Win", textColor, gLargeFont) ||
            !winMsgTexture.loadFromRenderedText("Congratulations", textColor, gLargeFont)) {
            cout << "Failed to render text texture." << endl;
            success=false;
        }
    }

    if (!blockSheetTexture.loadFromFile("Sprites/Block and Spike.png")) {
        cout << "Failed to load block and spike texture." << endl;
        success=false;
    }
    else {
        // Corner block, top left corner by default
        blockClips[0]={0, 0, 160, 160};

        // Wall block, horizontal by default
        blockClips[1]={160, 0, 160, 160};

        // T-block, wall side facing left by default
        blockClips[2]={0, 160, 160, 160};

        // Platform tip block, facing up by default
        blockClips[3]={160, 160, 160, 160};

        // Platform tip[0] with spike on top[1], platform tip facing left, spike facing up by default
        spikeClips[0]={320, 160, 160, 160};
        spikeClips[1]={320, 0, 160, 160};

        // Platform[2] with spike on top[3], spike facing up by default
        spikeClips[2]={480, 160, 160, 160};
        spikeClips[3]={480, 0, 160, 160};

        // Platform[4] with big spike on top[5], spike facing up by default
        spikeClips[4]={640, 160, 160, 160};
        spikeClips[5]={640, 0, 160, 160};
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

    for (int i=0; i<2; i++) {
        instructionTexture[i].free();
    }
    gameTitleTexture.free();
    winMsgTexture.free();

    // Deal with fonts
    TTF_CloseFont(gSmallFont);
    gSmallFont=nullptr;
    TTF_CloseFont(gMediumFont);
    gMediumFont=nullptr;
    TTF_CloseFont(gLargeFont);
    gLargeFont=nullptr;

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

    blocks.clear(); spikes.clear(); jumpOrbs.clear(); jumpPads.clear();

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

// Game status
enum GameStatus {
    PLAYING,
    START,
    WIN,
    RESTART
};

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

            Player cube;
            GameStatus currentStatus=START;
            static double fadeAlpha=180;
            bool dead=false;
            SDL_FRect instructionRect[2];

            bool quit=false;
            SDL_Event e;

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
                    else if (currentStatus==START && (e.type==SDL_KEYDOWN || e.type==SDL_MOUSEBUTTONDOWN)) {
                        currentStatus=PLAYING;
                        fadeAlpha=0;
                    }
                    else if (currentStatus==WIN && e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_r) {
                        currentStatus=RESTART;
                    }
                    else if (currentStatus==PLAYING) {
                        cube.handleEvent(e);
                    }
                }

                // Render game
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x69, 0xB4, 0xFF);
                SDL_RenderClear(gRenderer);
                renderLevel(blocks, spikes, jumpOrbs, jumpPads, deltaTime);
                cube.render();

                // Start
                if (currentStatus==START) {
                    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, static_cast<Uint8>(fadeAlpha));
                    SDL_FRect dimOverlay={0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                    SDL_RenderFillRectF(gRenderer, &dimOverlay);

                    SDL_FRect gameTitleRect={(SCREEN_WIDTH-gameTitleTexture.getWidth())/2,
                                             (SCREEN_HEIGHT-gameTitleTexture.getHeight()-instructionTexture[0].getHeight())/2,
                                             gameTitleTexture.getWidth(),
                                             gameTitleTexture.getHeight()};
                    gameTitleTexture.render(gameTitleRect);

                    instructionRect[0]={(SCREEN_WIDTH-instructionTexture[0].getWidth())/2,
                                        gameTitleRect.y+gameTitleRect.h,
                                        instructionTexture[0].getWidth(),
                                        instructionTexture[0].getHeight()};
                    instructionTexture[0].render(instructionRect[0]);
                }

                // Restart after win
                else if (currentStatus==RESTART) {
                    dead=false;
                    fadeAlpha=0;
                    cube.reset();
                    loadLevel("level2.txt", blocks, spikes, jumpOrbs, jumpPads);
                    currentStatus=PLAYING;
                    continue;
                }

                // Playing
                else if (currentStatus==PLAYING) {
                    // Handle player interactions
                    cube.move(blocks, jumpOrbs, deltaTime);
                    cube.interact(blocks, spikes, jumpOrbs, jumpPads, dead);
                    if (dead) {
                        currentStatus=WIN;
                    }
                }

                // Win
                else if (currentStatus==WIN) {
                    if (fadeAlpha<180) {
                        fadeAlpha+=400*deltaTime;
                        if (fadeAlpha>180) {
                            fadeAlpha=180;
                        }
                    }

                    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, static_cast<Uint8>(fadeAlpha));
                    SDL_FRect dimOverlay={0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                    SDL_RenderFillRectF(gRenderer, &dimOverlay);

                    SDL_FRect winMsgRect={(SCREEN_WIDTH-winMsgTexture.getWidth())/2,
                                          (SCREEN_HEIGHT-winMsgTexture.getHeight()-instructionTexture[1].getHeight())/2,
                                          winMsgTexture.getWidth(),
                                          winMsgTexture.getHeight()};
                    winMsgTexture.setAlpha(static_cast<Uint8>(fadeAlpha)*255/180);
                    winMsgTexture.render(winMsgRect);

                    instructionRect[1]={(SCREEN_WIDTH-instructionTexture[1].getWidth())/2,
                                        winMsgRect.y+winMsgRect.h,
                                        instructionTexture[1].getWidth(),
                                        instructionTexture[1].getHeight()};
                    instructionTexture[1].setAlpha(static_cast<Uint8>(fadeAlpha)*255/180);
                    instructionTexture[1].render(instructionRect[1]);
                }

                SDL_RenderPresent(gRenderer);
            }
        }
    }
    close();
    return 0;
}
