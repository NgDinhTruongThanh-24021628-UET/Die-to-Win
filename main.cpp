#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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
LTexture blockTexture;
LTexture spikeTexture;
LTexture cubeTexture;

LTexture yellowOrbTexture;
LTexture blueOrbTexture;
LTexture greenOrbTexture;

LTexture yellowPadTexture;
LTexture spiderPadTexture;
LTexture pinkPadTexture;

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
    if (!blockTexture.loadFromFile("Sprites/Block.png")) {
        cout << "Failed to load block texture." << endl;
        success=false;
    }
    if (!spikeTexture.loadFromFile("Sprites/Spike.png")) {
        cout << "Failed to load spike texture." << endl;
        success=false;
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
    blockTexture.free();
    spikeTexture.free();
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

// Load level from a file
void loadLevel(const string &path, vector<Block> &blocks, vector<Spike> &spikes, vector<JumpOrb> &jumpOrbs, vector<JumpPad> &jumpPads) {
    ifstream file(path);
    if (!file.is_open()) {
        cout << "Failed to open level file." << endl;
        return;
    }

    // Read file and store objects
    string line;
    int row=0;
    while (getline(file, line) && row<LEVEL_HEIGHT) {
        for (int col=0; col<int(line.length()) && col<LEVEL_WIDTH; col++) {
            char tile=line[col];
            int baseX=col*TILE_SIZE-TILE_SIZE*11/18;
            int baseY=row*TILE_SIZE-TILE_SIZE*9/18;

            switch (tile) {
            case '1': // Block
                blocks.emplace_back(baseX, baseY, TILE_SIZE, TILE_SIZE);
                break;

            case '2': // Spike
                spikes.emplace_back(baseX+TILE_SIZE*3/8, baseY+TILE_SIZE/4, TILE_SIZE/4, TILE_SIZE/2);
                break;

            case 'Y': // Yellow orb
            case 'B': // Blue orb
            case 'G': { // Green orb
                jumpOrbs.emplace_back(baseX-TILE_SIZE/10, baseY-TILE_SIZE/10, TILE_SIZE*12/10, TILE_SIZE*12/10, tile);
                break;
                }

            case 'J': // Yellow pad
            case 'P': { // Pink pad
                jumpPads.emplace_back(baseX+TILE_SIZE/12, baseY+TILE_SIZE*13/15, TILE_SIZE*10/12, TILE_SIZE/6, tile);
                break;
            }
            case 'S': // Spider pad
                jumpPads.emplace_back(baseX+TILE_SIZE/30, baseY+TILE_SIZE*3/4, TILE_SIZE*14/15, TILE_SIZE*2/5, tile);
                break;
            }
        }
        row++;
    }

    file.close();
}

void renderLevel(const vector<Block> &blocks, const vector<Spike> &spikes,
                 const vector<JumpOrb> &jumpOrbs, const vector<JumpPad> &jumpPads, double greenOrbAngle) {
    // Render orbs
    for (const auto &orb : jumpOrbs) {
        SDL_Rect renderOrb={orb.getHitbox().x+TILE_SIZE/10, orb.getHitbox().y+TILE_SIZE/10, TILE_SIZE, TILE_SIZE};
        switch (orb.getType()) {
        case 'Y': // Yellow
            yellowOrbTexture.render(renderOrb);
            break;
        case 'B': // Blue
            blueOrbTexture.render(renderOrb);
            break;
        case 'G': // Green
            greenOrbTexture.render(renderOrb, nullptr, greenOrbAngle, nullptr, SDL_FLIP_NONE);
            break;
        }
    }

    // Render pads
    for (const auto &pad : jumpPads) {
        SDL_Rect renderPad=pad.getHitbox();
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
        SDL_Rect renderBlock=block.getHitbox();
        blockTexture.render(renderBlock);
    }

    // Render spikes
    for (const auto& spike : spikes) {
        SDL_Rect renderSpike={spike.getHitbox().x-TILE_SIZE*3/8, spike.getHitbox().y-TILE_SIZE/4, TILE_SIZE, TILE_SIZE};
        spikeTexture.render(renderSpike);
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
            loadLevel("Level1.txt", blocks, spikes, jumpOrbs, jumpPads);

            // Delta time, to keep physics consistent across all refresh rates
            Uint64 NOW=SDL_GetPerformanceCounter();
            Uint64 LAST=0;
            double deltaTime=0;

            bool quit=false;
            SDL_Event e;
            Player cube;
            double greenOrbAngle=0.0;

            // Running
            while (!quit) {
                // Calculate delta time
                LAST=NOW;
                NOW=SDL_GetPerformanceCounter();
                deltaTime=double((NOW-LAST)*1000)/SDL_GetPerformanceFrequency();
                deltaTime/=1000.0; // Convert to seconds

                // Calculate green orb texture rotation
                greenOrbAngle+=180*deltaTime;
                if (greenOrbAngle>360) {
                    greenOrbAngle=0.0;
                }

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

                renderLevel(blocks, spikes, jumpOrbs, jumpPads, greenOrbAngle);

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

