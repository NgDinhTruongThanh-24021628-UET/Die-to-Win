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
#include "LoadLevel.h"
#include "Rendering.h"
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

// Texture clipping
const int NUMBER_OF_BLOCKS=30;
const int NUMBER_OF_SPIKES=3;
const int NUMBER_OF_ORBS=4;
const int NUMBER_OF_PADS=3;
SDL_Rect blockClips[NUMBER_OF_BLOCKS];
SDL_Rect spikeClips[NUMBER_OF_SPIKES];
SDL_Rect orbClips[NUMBER_OF_ORBS];
SDL_Rect padClips[NUMBER_OF_PADS];

// Music + SFX
Mix_Music *gameThemeSong=nullptr;
Mix_Music *fnafSong=nullptr;
Mix_Chunk *deathSound=nullptr;

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

                if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096)<0) {
                    cout << "SDL_mixer could not initialize. " << Mix_GetError() << endl;
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
        if (!instructionTexture[0].setTextOnce("Press any key to start", textColor, gMediumFont) ||
            !instructionTexture[1].setTextOnce("Press left/right to customize background", textColor, gMediumFont) ||
            !instructionTexture[2].setTextOnce("Press left/right to customize color", textColor, gMediumFont) ||
            !instructionTexture[3].setTextOnce("Press up/down to select settings", textColor, gMediumFont) ||
            !instructionTexture[4].setTextOnce("Press Enter to finish", textColor, gMediumFont) ||

            !instructionTexture[10].setTextOnce("Press R to restart, ESC to exit", textColor, gMediumFont) ||

            !gameTitleTexture.setTextOnce("Die to Win", textColor, gXtraFont) ||
            !winMsgTexture.setTextOnce("Congratulations", textColor, gLargeFont)) {
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

        // Jump-through platforms
        blockClips[28]={1440, 160, 160, 160};
        blockClips[29]={1600, 160, 160, 160};
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
        orbClips[3]={480, 0, 160, 160};     // Dash orb

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

    gameThemeSong=Mix_LoadMUS("Resources/Game Theme.ogg");
    if (gameThemeSong==nullptr) {
        cout << "Failed to load game theme song. " << Mix_GetError() << endl;
        success=false;
    }
    fnafSong=Mix_LoadMUS("Resources/FNAF Song.mp3");
    if (fnafSong==nullptr) {
        cout << "Failed to load level song. " << Mix_GetError() << endl;
        success=false;
    }
    deathSound=Mix_LoadWAV("Resources/Death Sound.mp3");
    if (deathSound==nullptr) {
        cout << "Failed to load death sound effect. " << Mix_GetError() << endl;
        success=false;
    }
    return success;
}

// Cleanup
void close() {
    // Deal with music + SFX
    Mix_FreeMusic(gameThemeSong);
    gameThemeSong=nullptr;
    Mix_FreeMusic(fnafSong);
    fnafSong=nullptr;
    Mix_FreeChunk(deathSound);
    deathSound=nullptr;

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

void offsetPosition(vector<Block> &blocks, const string &levelName) {
    if (levelName=="Tic Tac Toe") {
        for (auto &block : blocks) {
            if (block.getType()=="1XM") block.offsetPosition(TILE_SIZE/2, 0);
        }
    }
}

void displayTextInLevel(Player cube, vector<Block> &blocks, GameStatus currentStatus, GameSetting currentSetting,
                        const string &levelName, const int &levelIndex) {

    if (currentStatus==MENU) {
        instructionTexture[14].setTextOnce("Menu", textColor, gTinyFont);
        instructionTexture[14].render(4, -4);
        instructionTexture[14].render(SCREEN_WIDTH-instructionTexture[14].getWidth()-4, -4);

        for (const Block &block : blocks) {
            if (block.getType()=="1S") {
                instructionTexture[30].setTextOnce("Settings", textColor, gMediumFont);
                instructionTexture[30].render(block.getHitbox().x+(block.getHitbox().w-instructionTexture[30].getWidth())/2,
                                               block.getHitbox().y-instructionTexture[30].getHeight());
            }
            if (block.getType()=="1P") {
                instructionTexture[30].setTextOnce("Play", textColor, gMediumFont);
                instructionTexture[30].render(block.getHitbox().x+(block.getHitbox().w-instructionTexture[30].getWidth())/2,
                                               block.getHitbox().y-instructionTexture[30].getHeight());
            }
            if (block.getType()=="1C") {
                instructionTexture[30].setTextOnce("Credits", textColor, gMediumFont);
                instructionTexture[30].render(block.getHitbox().x+(block.getHitbox().w-instructionTexture[30].getWidth())/2,
                                               block.getHitbox().y-instructionTexture[30].getHeight());
            }
            if (block.getType()=="1K0") {
                gameTitleTexture.render(block.getHitbox().x+(9*TILE_SIZE-gameTitleTexture.getWidth())/2, block.getHitbox().y);
            }
            if (block.getType()=="1K2") {
                instructionTexture[30].setTextOnce("v0.5 ", textColor, gSmallFont);
                instructionTexture[30].render(block.getHitbox().x+block.getHitbox().w-instructionTexture[30].getWidth(),
                                               block.getHitbox().y+block.getHitbox().h-instructionTexture[30].getHeight());
            }
        }
    }

    else if (currentStatus==PLAYING) {
        instructionTexture[15].setTextOnce("Level "+to_string(levelIndex), textColor, gTinyFont);
        instructionTexture[15].render(4, -4);
        instructionTexture[16].setTextOnce(levelName, textColor, gTinyFont);
        instructionTexture[16].render(SCREEN_WIDTH-instructionTexture[16].getWidth()-4, -4);

        if (levelName=="Cookies") {
            vector<Block*> textPlat;
            for (Block &block : blocks) {
                if (block.getType()=="1IP") {
                    instructionTexture[20].setTextOnce(to_string(cube.getGainPerHit()), textColor, gSmallFont);
                    instructionTexture[20].render(block.getHitbox().x+(block.getHitbox().w-instructionTexture[20].getWidth())/2,
                                                  block.getHitbox().y-instructionTexture[20].getHeight()+6);
                }
                if (block.getType()=="1I2") {
                    instructionTexture[20].setTextOnce((block.counter<5 ? to_string(block.value) : "MAX"), textColor, gSmallFont);
                    instructionTexture[20].render(block.getHitbox().x+(block.getHitbox().w-instructionTexture[20].getWidth())/2,
                                                  block.getHitbox().y-instructionTexture[20].getHeight()+6);
                }
                if (block.getType()=="1I3" || block.getType()=="1I4") {
                    instructionTexture[20].setTextOnce((block.counter<25 ? to_string(block.value) : "MAX"), textColor, gSmallFont);
                    instructionTexture[20].render(block.getHitbox().x+(block.getHitbox().w-instructionTexture[20].getWidth())/2,
                                                  block.getHitbox().y-instructionTexture[20].getHeight()+6);
                }
                if (block.getType()=="1I2" || block.getType()=="1I3" || block.getType()=="1I4") {
                    instructionTexture[25].setTextOnce(to_string(block.counter), textColor, gTinyFont);
                    instructionTexture[25].render(block.getHitbox().x+TILE_SIZE/12, block.getHitbox().y);
                }
                if (block.getType()=="1PL") {
                    textPlat.push_back(&block);
                }
            }
            instructionTexture[21].loadFromRenderedText(to_string(cube.getTotalMoney()), textColor, gMediumFont);
            instructionTexture[21].render(textPlat[0]->getHitbox().x+(TILE_SIZE*5-instructionTexture[21].getWidth())/2,
                                          textPlat[0]->getHitbox().y+(TILE_SIZE-instructionTexture[21].getHeight())/2);

            instructionTexture[22].setTextOnce(to_string(cube.getPassiveIncome())+" /sec", textColor, gMediumFont);
            instructionTexture[22].render(textPlat[1]->getHitbox().x+(TILE_SIZE*5-instructionTexture[22].getWidth())/2,
                                          textPlat[1]->getHitbox().y+(TILE_SIZE-instructionTexture[22].getHeight())/2);

            textPlat.clear();
        }

        else if (levelName=="Enigma") {
            for (Block &block : blocks) {
                if (block.getType()=="1BG" || block.getType()=="1BO" || block.getType()=="1BI") {
                    instructionTexture[40].loadFromRenderedText(to_string(block.counter), textColor, gMediumFont);
                    instructionTexture[40].render(block.getHitbox().x+(block.getHitbox().w-instructionTexture[40].getWidth())/2,
                                                  block.getHitbox().y+(block.getHitbox().h-instructionTexture[40].getHeight())/2);
                }
                if (!uniqueDigitsInPassword) {
                    instructionTexture[41].setTextOnce("Password should contain 4 different digits", textColor, gMediumFont);
                    instructionTexture[41].render((SCREEN_WIDTH-instructionTexture[41].getWidth())/2, SCREEN_HEIGHT/2);
                }
            }
        }

        else if (levelName=="Illusion World") {
            if (cube.timeStopped) {
                instructionTexture[45].loadFromRenderedText(to_string(int(cube.timeStopTimer)+1), textColor, gXtraFont);
                instructionTexture[45].setAlpha(100);
                instructionTexture[45].render((SCREEN_WIDTH-instructionTexture[45].getWidth())/2, (SCREEN_HEIGHT-instructionTexture[45].getHeight())/2);
            }
        }

        else if (levelName=="Five Nights") {
            for (const Block &block : blocks) {
                if (block.getType()=="1PL") {
                    instructionTexture[46].loadFromRenderedText(to_string(cube.powerPercent)+" %", textColor, gMediumFont);
                    instructionTexture[46].render(TILE_SIZE/2+block.getHitbox().x+(block.getHitbox().w-instructionTexture[46].getWidth())/2,
                                                  block.getHitbox().y+(block.getHitbox().h-instructionTexture[46].getHeight())/2);
                }
            }
        }
    }
}

const int ALL_LEVELS=15;
string levelName[ALL_LEVELS]={"The Hub",
                              "Die to Win", "Getting Over It", "Geometry Jump", "VVVVVV", "Trial and Error", "Dash",
                              "Labyrinth", "Enigma", "Move to Die", "Cookies", "Illusion World", "Five Nights",
                              "Tic Tac Toe" ,"Vertigo"};
static int levelIndex=12;

int main(int argc, char *argv[]) {
    if (!init()) {
        cout << "Failed to initialize." << endl;
    }
    else {
        if (!loadMedia()) {
            cout << "Failed to load media." << endl;
        }
        else {
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
                    else if (currentStatus==CREDITS && e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_RETURN) {
                        currentStatus=MENU;
                    }
                    else if (currentStatus==WIN && e.type==SDL_KEYDOWN) {
                        if (e.key.keysym.sym==SDLK_r) {
                            currentStatus=RESTART;
                        }
                        else if (e.key.keysym.sym==SDLK_ESCAPE) {
                            quit=true;
                        }
                    }
                    else if (currentStatus==PLAYING || currentStatus==MENU) {
                        cube.handleEvent(e);
                    }
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

                renderLevel(blocks, pushableBlocks, spikes, jumpOrbs, jumpPads, deltaTime);
                cube.render();
                displayTextInLevel(cube, blocks, currentStatus, currentSetting, levelName[levelIndex], levelIndex);

                if (currentStatus==START) {
                    Mix_PlayMusic(gameThemeSong, -1);
                    cube.reset();
                    loadLevel("Resources/Levels/"+levelName[levelIndex]+".txt", blocks, pushableBlocks, spikes, jumpOrbs, jumpPads);
                    offsetPosition(blocks, levelName[levelIndex]);
                    fadeAlpha=0;
                    currentStatus=PLAYING;
                }

                // Playing
                if (currentStatus==PLAYING) {
                    // Handle player interactions
                    cube.move(blocks, pushableBlocks, spikes, jumpOrbs, currentStatus, levelName[levelIndex], deltaTime);
                    cube.interact(blocks, pushableBlocks, spikes, jumpOrbs, jumpPads, levelName[levelIndex], deltaTime, dead);
                    for (auto &block : pushableBlocks) {
                        if (!cube.timeStopped) block.update(blocks, cube.getHitbox(), cube.moveLeft, cube.moveRight, dead, deltaTime);
                    }
                    if (dead) {
                        Mix_PlayChannel(-1, deathSound, 0);
                        levelIndex++;
                        if (levelIndex<ALL_LEVELS) {
                            dead=false;
                            currentStatus=START;
                            SDL_Delay(1000);
                        }
                        else {
                            currentStatus=WIN;
                        }
                    }
                }

                // Menu screen
                if (currentStatus==MENU) {
                    loadLevel("Resources/Levels/Menu.txt", blocks, pushableBlocks, spikes, jumpOrbs, jumpPads);
                    if (!Mix_PlayingMusic()) Mix_PlayMusic(gameThemeSong, -1);
                    cube.move(blocks, pushableBlocks, spikes, jumpOrbs, currentStatus, levelName[levelIndex], deltaTime);
                }

                // Test level
                if (currentStatus==TEST) {
                    loadLevel("Resources/Levels/test.txt", blocks, pushableBlocks, spikes, jumpOrbs, jumpPads);
                    currentStatus=PLAYING;
                }

                // Restart after win
                if (currentStatus==RESTART) {
                    dead=false;
                    fadeAlpha=0;
                    Mix_PlayMusic(gameThemeSong, -1);
                    cube.reset();
                    levelIndex=1;
                    loadLevel("Resources/Levels/"+levelName[levelIndex]+".txt", blocks, pushableBlocks, spikes, jumpOrbs, jumpPads);
                    currentStatus=PLAYING;
                    continue;
                }

                // Credits
                if (currentStatus==CREDITS) {
                    cube.resetBool();

                    float textPosY=TILE_SIZE*9/18;
                    fadeAlpha=220;
                    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, static_cast<Uint8>(fadeAlpha));
                    SDL_FRect dimOverlay={TILE_SIZE*7/18, textPosY, SCREEN_WIDTH-TILE_SIZE*14/18, SCREEN_HEIGHT-TILE_SIZE};
                    SDL_RenderFillRectF(gRenderer, &dimOverlay);

                    instructionTexture[90].setTextOnce("Special thanks to", textColor, gLargeFont);
                    instructionTexture[90].render((SCREEN_WIDTH-instructionTexture[90].getWidth())/2, textPosY);
                    textPosY+=instructionTexture[90].getHeight();
                    instructionTexture[91].setTextOnce("RobTop Games, creator of Geometry Dash", textColor, gMediumFont);
                    instructionTexture[91].render((SCREEN_WIDTH-instructionTexture[91].getWidth())/2, textPosY);
                    textPosY+=instructionTexture[91].getHeight();
                    instructionTexture[92].setTextOnce("Lazy Foo Productions", textColor, gMediumFont);
                    instructionTexture[92].render((SCREEN_WIDTH-instructionTexture[92].getWidth())/2, textPosY);
                    textPosY+=instructionTexture[92].getHeight();
                    instructionTexture[93].setTextOnce("GDColon.com", textColor, gMediumFont);
                    instructionTexture[93].render((SCREEN_WIDTH-instructionTexture[93].getWidth())/2, textPosY);
                    textPosY+=instructionTexture[93].getHeight();
                    instructionTexture[94].setTextOnce("ChatGPT", textColor, gMediumFont);
                    instructionTexture[94].render((SCREEN_WIDTH-instructionTexture[94].getWidth())/2, textPosY);
                    textPosY+=instructionTexture[94].getHeight()+TILE_SIZE/2;
                    instructionTexture[4].render((SCREEN_WIDTH-instructionTexture[4].getWidth())/2, textPosY);
                }

                // Settings screen
                if (currentStatus==SETTINGS) {
                    cube.resetBool();
                    loadLevel("Resources/Levels/Settings.txt", blocks, pushableBlocks, spikes, jumpOrbs, jumpPads);

                    float textPosY=TILE_SIZE*9/18;
                    fadeAlpha=200;
                    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, static_cast<Uint8>(fadeAlpha));
                    SDL_FRect dimOverlay={TILE_SIZE*7/18, textPosY, SCREEN_WIDTH-TILE_SIZE*14/18, 3*instructionTexture[3].getHeight()};
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
