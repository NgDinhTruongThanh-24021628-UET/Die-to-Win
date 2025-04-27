#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <ctime>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "Texture.h"
#include "LevelObjs.h"
#include "Player.h"
#include "Enums.h"

extern SDL_Renderer *gRenderer;
extern LTexture instructionTexture[];
extern TTF_Font *gSmallFont;
extern SDL_Color textColor;

/// Block functions start

Block::Block(float x, float y, float w, float h, double a, SDL_RendererFlip m, const std::string &type) {
    hitbox={x, y, w, h};
    realX=x, realY=y;
    angle=a;
    blockType=type;
    mirror=m;
}

bool Block::checkXCollision(double &playerX, double playerY, double &nextPlayerX,
                            double playerVelX, int PLAYER_WIDTH, int PLAYER_HEIGHT) const {
    if (blockType[1]=='J') return false;

    bool collided=false;

    // From left side
    if (playerX+PLAYER_WIDTH<=hitbox.x &&
        nextPlayerX+PLAYER_WIDTH>=hitbox.x && // If player will go through platform
        playerY+PLAYER_HEIGHT>hitbox.y &&
        playerY<hitbox.y+hitbox.h) { // And will collide with platform

        nextPlayerX=hitbox.x-PLAYER_WIDTH;
        collided=true;
    }

    // From right side
    if (playerX>=hitbox.x+hitbox.w &&
        nextPlayerX<=hitbox.x+hitbox.w && // If player will go through platform
        playerY+PLAYER_HEIGHT>hitbox.y &&
        playerY<hitbox.y+hitbox.h) { // And will collide with platform

        nextPlayerX=hitbox.x+hitbox.w;
        collided=true;
    }

    return collided;
}

bool Block::checkYCollision(double playerX, double &playerY, double &nextPlayerY,
                            double playerVelY, int PLAYER_WIDTH, int PLAYER_HEIGHT,
                            bool &onPlatform, bool &hitCeiling, bool reverseGravity) const {
    bool collided=false;

    // Y-axis downward movement
    if (playerY+PLAYER_HEIGHT<=hitbox.y &&
        nextPlayerY+PLAYER_HEIGHT>=hitbox.y && // If player will go through platform
        playerX+PLAYER_WIDTH>hitbox.x &&
        playerX<hitbox.x+hitbox.w) { // And will collide with platform

        nextPlayerY=hitbox.y-PLAYER_HEIGHT;
        collided=true;
        if (!reverseGravity) { // Falling
            onPlatform=true;
        }
        else { // Jumping up
            hitCeiling=true;
        }
    }

    // Y-axis upward movement
    if (playerY>=hitbox.y+hitbox.h &&
        nextPlayerY<=hitbox.y+hitbox.h && // If player will go through platform
        playerX+PLAYER_WIDTH>hitbox.x &&
        playerX<hitbox.x+hitbox.w && // And will collide with platform
        blockType[1]!='J') { // Ignore jump-through blocks

        nextPlayerY=hitbox.y+hitbox.h;
        collided=true;
        if (!reverseGravity) { // Jumping up
            hitCeiling=true;
        }
        else { // Falling
            onPlatform=true;
        }
    }

    return collided;
}

const SDL_FRect &Block::getHitbox() const {
    return hitbox;
}

const std::string &Block::getType() const {
    return blockType;
}
void Block::switchType(std::string newType) {
    blockType=newType;
}

void Block::movingBlockX(double deltaTime) {
    if (unlocked) {
        hitbox.y=realY;
        float dx=realX-hitbox.x;
        float distance=fabs(dx);
        if (distance<1.0f) {
            hitbox.x=realX;
            unlocked=false;
        }
        else {
            float moveStep=speed*deltaTime;
            hitbox.x+=dx/distance*moveStep;
        }
    }
}
void Block::movingBlockY(double deltaTime) {
    if (unlocked) {
        hitbox.x=realX;
        float dy=realY-hitbox.y;
        float distance=fabs(dy);
        if (distance<1.0f) {
            hitbox.y=realY;
            unlocked=false;
        }
        else {
            float moveStep=speed*deltaTime;
            hitbox.y+=dy/distance*moveStep;
        }
    }
}
void Block::changeSpeed(float change) {
    speed*=change;
}
void Block::offsetPosition(float offsetX, float offsetY) {
    hitbox.x+=offsetX;
    hitbox.y+=offsetY;
}

bool Block::isInteractable() const {
    std::string type[16]={"1I1", "1I2", "1I3", "1I4", "1IP", "1S", "1P", "1C", "1BI", "1IN",
                        "1R", "1SA", "1ZA", "1XM", "1XI", "1WVI"};
    for (int i=0; i<16; i++) {
        if (blockType==type[i]) return true;
    }
    return false;
}
void Block::interact(unsigned long long &totalMoney, int &gainPerHit, int &passiveIncome, GameStatus &currentStatus,
                     std::vector<Block> &blocks, std::vector<PushableBlock> &pushableBlocks, std::vector<Spike> &spikes,
                     const std::string &levelName, double deltaTime, bool &timeStopped, double &timeStopTimer, int &powerPercent) {
    if (!isInteractable()) return;
    if (blockType=="1S") {
        currentStatus=SETTINGS;
    }
    else if (blockType=="1P") {
        currentStatus=START;
    }
    else if (blockType=="1C") {
        currentStatus=CREDITS;
    }
    else if (levelName=="Cookies") {
        interactClicker(totalMoney, gainPerHit, passiveIncome, blocks, spikes, deltaTime);
    }
    else if (levelName=="Enigma") {
        interactEnigma(blocks, spikes);
    }
    else if (levelName=="Move to Die" || levelName=="Illusion World") {
        interactMoveToDie(blocks, pushableBlocks, timeStopped, timeStopTimer);
    }
    else if (levelName=="Five Nights") {
        interactFiveNights(blocks, powerPercent);
    }
    else if (levelName=="Tic Tac Toe") {
        interactTicTacToe(blocks, spikes);
    }

}

// Helper function for level: Cookies
void Block::interactClicker(unsigned long long &totalMoney, int &gainPerHit, int &passiveIncome,
                            std::vector<Block> &blocks, std::vector<Spike> &spikes, double deltaTime) {
    // Spike to kill player (duh)
    if (spikes.empty()) {
        int baseX, baseY;
        for (const auto &block : blocks) {
            if (block.getType()=="1PD") {
                baseX=block.getHitbox().x;
                baseY=block.getHitbox().y;
            }
        }
        spikes.emplace_back(baseX+TILE_SIZE*2/5.0f, baseY+TILE_SIZE*3/10.0f, TILE_SIZE/5.0f, TILE_SIZE*2/5.0f, 0, SDL_FLIP_NONE, "2ED");
    }

    if (blockType=="1IP") { // Point block
        speed=50.0f;
        totalMoney+=gainPerHit;
    }

    else if (blockType=="1I2") { // Lower point block position
        if (counter>=5) counter=5;
        else {
            if (totalMoney>=(unsigned long long)value) {
                for (auto &block : blocks) {
                    if (block.getType()=="1IP") {
                        block.unlocked=true;
                        block.realY=block.getHitbox().y+TILE_SIZE/4;
                    }
                }
                totalMoney-=value;
                if (counter==0) value*=100;
                else value*=4;
                counter++;
            }
        }
    }

    else if (blockType=="1I3") { // Increase gain per hit
        if (counter>=25) counter=25;
        else {
            if (totalMoney>=(unsigned long long)value) {
                if (counter==0) gainPerHit*=5;
                else {
                    gainPerHit+=increment;
                    increment=value/counter;
                }
                totalMoney-=value;
                value*=2;
                counter++;
            }
        }
    }

    else if (blockType=="1I4") { // Increase passive income
        if (counter>=25) counter=25;
        else {
            if (totalMoney>=(unsigned long long)value) {
                if (counter==0) passiveIncome=1;
                else if (counter==1) passiveIncome=5;
                else {
                    passiveIncome+=increment;
                    increment=(value/counter)/2;
                }
                totalMoney-=value;
                if (counter<10) value*=3;
                else value*=2;
                counter++;
            }
        }
    }
}

// Helper function for level: Enigma
// Generate random password
std::vector<int> enigmaPassword;
void generateEnigmaPassword() {
    std::vector<int> digits={0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    std::mt19937 g(static_cast<unsigned int>(time(0)));
    std::shuffle(digits.begin(), digits.end(), g);

    enigmaPassword=std::vector<int>(digits.begin(), digits.begin()+4);
}

bool uniqueDigitsInPassword=true;

void Block::interactEnigma(std::vector<Block> &blocks, std::vector<Spike> &spikes) {
    // Spikes to kill player (duh)
    if (spikes.empty()) {
        for (int i=0; i<3; i++) {
            spikes.emplace_back(800+TILE_SIZE*2/5.0f, 800+TILE_SIZE*3/10.0f, TILE_SIZE/5.0f, TILE_SIZE*2/5.0f, 0, SDL_FLIP_NONE, "2EU");
        }
    }

    // Generate password
    if (enigmaPassword.empty()) generateEnigmaPassword();

    if (blockType=="1BI") { // Password digit block
        counter=(counter+1)%10;
    }

    else if (blockType=="1IN") { // Check solution block
        // Pointer vector to digit blocks
        std::vector<Block*> digits(4, nullptr);
        int n=0;
        for (auto &block : blocks) {
            if (block.getType()=="1BI") {
                digits[n]=&block;
                n++;
            }
        }

        // Check if all digits in solution are unique
        if (digits.size()!=enigmaPassword.size()) return;
        for (int i=0; i<int(digits.size())-1; i++) {
            for (int j=i+1; j<int(digits.size()); j++) {
                if (digits[i]->counter==digits[j]->counter) {
                    uniqueDigitsInPassword=false;
                    return;
                }
            }
        }
        uniqueDigitsInPassword=true;

        // Setup for solution check
        int correctPos=0, wrongPos=0;
        std::vector<bool> passwordUsed(enigmaPassword.size(), false);
        std::vector<bool> guessUsed(digits.size(), false);

        // Check for digits in correct position
        for (int i=0; i<int(digits.size()); i++) {
            if (digits[i]->counter==enigmaPassword[i]) {
                correctPos++;
                passwordUsed[i]=guessUsed[i]=true;
            }
        }

        // Check for digits in wrong position but is in password
        for (int i=0; i<int(digits.size()); i++) {
            if (guessUsed[i]) continue;
            for (int j=0; j<int(enigmaPassword.size()); j++) {
                if (!passwordUsed[j] && digits[i]->counter==enigmaPassword[j]) {
                    wrongPos++;
                    passwordUsed[j]=true;
                    break;
                }
            }
        }

        // Render to screen
        for (auto &block : blocks) {
            if (block.getType()=="1BG") {
                block.counter=correctPos;
            }
            else if (block.getType()=="1BO") {
                block.counter=wrongPos;
            }
        }

        // Move spikes if player wins
        if (correctPos==4) {
            for (int i=0; i<3; i++) {
                spikes[i].unlocked=true;
                spikes[i].realX=SCREEN_WIDTH-(i+1)*TILE_SIZE-TILE_SIZE*7/18.0f+TILE_SIZE*2/5.0f;
                spikes[i].realY=SCREEN_HEIGHT-TILE_SIZE*3/2.0f+TILE_SIZE*3/10.0f;
            }
            enigmaPassword.clear();
        }
    }
}

// Helper function for level: Move to Die + Illusion World
void Block::interactMoveToDie(std::vector<Block> &blocks, std::vector<PushableBlock> &pushableBlocks, bool &timeStopped, double &timeStopTimer) {
    if (blockType=="1R") { // Reset pushable block position
        for (auto &block : pushableBlocks) {
            if (timeStopped) {
                block.resetQueued=true;
            }
            else {
                block.resetPosition();
            }
        }
    }

    else if (blockType=="1SA") { // Time stop
        if (!timeStopped) {
            timeStopped=true;
            timeStopTimer=5;
        }
    }
}

// Helper function for level: Five Nights
void Block::interactFiveNights(std::vector<Block> &blocks, int &powerPercent) {
    if (blockType=="1ZA") { // Lose power
        if (powerPercent>=5) {
            powerPercent-=5;
        }
        else powerPercent=0;

        for (auto &block : blocks) { // Move the 2 power blocks
            if (block.getType()=="1ZA") {
                if (powerPercent>0) block.unlocked=true;
                if (block.realY<SCREEN_HEIGHT-4*TILE_SIZE) {
                    block.realY+=2*TILE_SIZE;
                }
                else {
                    block.realY-=2*TILE_SIZE;
                }
            }
        }
    }
}

// Helper function for level: Tic Tac Toe (simple AI)
bool botWins=false;
bool playerWins=false;
bool stalemate=false; // Set outcome

// Check game status, set outcome
void checkGameOver (std::vector<std::vector<Block*>> tttBoard, const int &filledTiles, bool &gameOver, bool &playerWins, bool &botWins, bool &stalemate) {
    for (int r=0; r<3 && !gameOver; r++) { // Row filled with X/O
        if (tttBoard[r][0]->getType()==tttBoard[r][1]->getType() &&
            tttBoard[r][0]->getType()==tttBoard[r][2]->getType() &&
            (tttBoard[r][0]->getType()=="1X" || tttBoard[r][0]->getType()=="1O")) {

            gameOver=true;
            if (tttBoard[r][0]->getType()=="1X") playerWins=true;
            else if (tttBoard[r][0]->getType()=="1O") botWins=true;
        }
    }
    for (int c=0; c<3 && !gameOver; c++) { // Column filled with X/O
        if (tttBoard[0][c]->getType()==tttBoard[1][c]->getType() &&
            tttBoard[0][c]->getType()==tttBoard[2][c]->getType() &&
            (tttBoard[0][c]->getType()=="1X" || tttBoard[0][c]->getType()=="1O")) {

            gameOver=true;
            if (tttBoard[0][c]->getType()=="1X") playerWins=true;
            else if (tttBoard[0][c]->getType()=="1O") botWins=true;
        }
    }

    // Diagonal filled with X/O
    if (tttBoard[0][0]->getType()==tttBoard[1][1]->getType() &&
        tttBoard[0][0]->getType()==tttBoard[2][2]->getType() &&
        (tttBoard[0][0]->getType()=="1X" || tttBoard[0][0]->getType()=="1O")) {

        gameOver=true;
        if (tttBoard[0][0]->getType()=="1X") playerWins=true;
        else if (tttBoard[0][0]->getType()=="1O") botWins=true;
    }
    else if (tttBoard[0][2]->getType()==tttBoard[1][1]->getType() &&
             tttBoard[0][2]->getType()==tttBoard[2][0]->getType() &&
             (tttBoard[0][2]->getType()=="1X" || tttBoard[0][2]->getType()=="1O")) {

        gameOver=true;
        if (tttBoard[0][2]->getType()=="1X") playerWins=true;
        else if (tttBoard[0][2]->getType()=="1O") botWins=true;
    }

    // Entire board is filled
    else if (filledTiles==9) {
        gameOver=true;
        stalemate=true;
    }
}

void Block::interactTicTacToe(std::vector<Block> &blocks, std::vector<Spike> &spikes) {
    // Spikes to kill player (duh)
    if (spikes.empty()) {
        for (int i=0; i<3; i++) {
            spikes.emplace_back(800+TILE_SIZE*2/5.0f, 800+TILE_SIZE*3/10.0f, TILE_SIZE/5.0f, TILE_SIZE*2/5.0f, 0, SDL_FLIP_NONE, "2EU");
        }
    }

    // Current player position tracker
    static int currentRow=0;
    static int currentCol=0;

    // Check game status
    static int filledTiles=0;
    static bool gameOver=false;

    // Pointer vector to tic tac toe board
    std::vector<std::vector<Block*>> tttBoard(3, std::vector<Block*>(3, nullptr));
    int row=0, col=0;
    for (auto &block : blocks) {
        if (block.getType()=="1E" || block.getType()=="1B" || block.getType()=="1X" || block.getType()=="1O") {
            tttBoard[row][col]=&block;
            col++;
            if (col>=3) {
                col=0;
                row++;
            }
        }
    }

    // Move player position
    if (blockType=="1XM" && !gameOver) {
        // Revert current tile to empty
        if (tttBoard[currentRow][currentCol]->getType()=="1B") {
            tttBoard[currentRow][currentCol]->switchType("1E");
        }

        // Skip tiles with X or O block
        int tries=0;
        do {
            currentCol++;
            if (currentCol>=3) {
                currentCol=0;
                currentRow++;
                if (currentRow>=3) {
                    currentRow=0;
                }
            }
            tries++;
        } while ((tttBoard[currentRow][currentCol]->getType()=="1X" || tttBoard[currentRow][currentCol]->getType()=="1O") && tries<9);

        // Change next tile to lined
        if (tttBoard[currentRow][currentCol]->getType()=="1E") {
            tttBoard[currentRow][currentCol]->switchType("1B");
        }
    }

    // Place X on board
    else if (blockType=="1XI" && !gameOver) {
        // Change current tile to X
        if (tttBoard[currentRow][currentCol]->getType()=="1B") {
            tttBoard[currentRow][currentCol]->switchType("1X");
            filledTiles++;

            checkGameOver(tttBoard, filledTiles, gameOver, playerWins, botWins, stalemate);

            // Only allows O move if game is not over
            if (!gameOver) {
                // Find empty tiles
                std::vector<std::pair<int, int>> possibleOMoves;
                for (int r=0; r<3; r++) {
                    for (int c=0; c<3; c++) {
                        if (tttBoard[r][c]->getType()=="1E" || tttBoard[r][c]->getType()=="1B") {
                            possibleOMoves.push_back({r, c});
                        }
                    }
                }

                // Random O placement
                if (!possibleOMoves.empty()) {
                    int pick=rand()%int(possibleOMoves.size());
                    int oRow=possibleOMoves[pick].first;
                    int oCol=possibleOMoves[pick].second;
                    tttBoard[oRow][oCol]->switchType("1O");
                    filledTiles++;
                }

                checkGameOver(tttBoard, filledTiles, gameOver, playerWins, botWins, stalemate);

                // If AI took player's current position, find the next empty tile
                if (!gameOver) {
                    int tries=0;
                    do {
                        currentCol++;
                        if (currentCol>=3) {
                            currentCol=0;
                            currentRow++;
                            if (currentRow>=3) {
                                currentRow=0;
                            }
                        }
                        tries++;
                    } while ((tttBoard[currentRow][currentCol]->getType()=="1X" || tttBoard[currentRow][currentCol]->getType()=="1O") && tries<9);

                    if (tttBoard[currentRow][currentCol]->getType()=="1E") {
                        tttBoard[currentRow][currentCol]->switchType("1B");
                    }
                }
            }
        }
    }

    // Move spikes if player wins
    if (playerWins) {
        for (int i=0; i<3; i++) {
            spikes[i].unlocked=true;
            spikes[i].realX=(i+1)*TILE_SIZE+TILE_SIZE*7/18.0f+TILE_SIZE*2/5.0f;
            spikes[i].realY=SCREEN_HEIGHT-TILE_SIZE*3/2.0f+TILE_SIZE*3/10.0f;
        }
    }

    // Reset game
    else if (blockType == "1R") {
        for (int r=0; r<3; r++) {
            for (int c=0; c<3; c++) {
                tttBoard[r][c]->switchType("1E");
                currentCol=0;
                currentRow=0;
                if (tttBoard[currentRow][currentCol]->getType()=="1E") {
                    tttBoard[currentRow][currentCol]->switchType("1B");
                }
                playerWins=false;
                botWins=false;
                stalemate=false;
                gameOver=false;
                filledTiles=0;
            }
        }
    }
}

/// Block functions end

/// Pushable block functions start

PushableBlock::PushableBlock(float x, float y, float w, float h) {
    hitbox={x, y, w, h};
    originalX=x;
    originalY=y;
}

void PushableBlock::update(std::vector<Block> &platformBlocks, const SDL_FRect &playerHitbox,
                           bool moveLeft, bool moveRight, bool &dead, double deltaTime) {

    checkPush(platformBlocks, playerHitbox, moveLeft, moveRight, deltaTime);
    applyPhysics(platformBlocks, deltaTime);
    checkKill(playerHitbox, dead);
}

void PushableBlock::applyPhysics(std::vector<Block> &platformBlocks, double deltaTime) {
    velY+=GRAVITY*deltaTime;
    if (velY>TERMINAL_VELOCITY) velY=TERMINAL_VELOCITY;

    SDL_FRect nextPos=hitbox;
    nextPos.y+=velY*deltaTime;
    grounded=false;

    for (const auto &block : platformBlocks) {
        if (block.getType()!="1J" && block.getType()!="1JL" && block.getType()!="1JR") { // Ignore jump-through platforms
            SDL_FRect b=block.getHitbox();
            if (hitbox.y+hitbox.h<=b.y &&
                nextPos.y+hitbox.h>=b.y &&
                hitbox.x+hitbox.w>b.x &&
                hitbox.x<b.x+b.w) {

                nextPos.y=b.y-hitbox.h;
                velY=0.0;
                grounded=true;
                break;
            }
        }
    }

    hitbox.y=nextPos.y;
}

bool PushableBlock::checkYCollision(double playerX, double &playerY, double &nextPlayerY,
                                    double playerVelY, int PLAYER_WIDTH, int PLAYER_HEIGHT, bool &onPlatform) {
    bool collided=false;

    // Y-axis downward movement
    if (playerY+PLAYER_HEIGHT<=hitbox.y &&
        nextPlayerY+PLAYER_HEIGHT>=hitbox.y && // If player will go through platform
        playerX+PLAYER_WIDTH>hitbox.x &&
        playerX<hitbox.x+hitbox.w) { // And will collide with platform

        nextPlayerY=hitbox.y-PLAYER_HEIGHT;
        collided=true;
        onPlatform=true;
    }

    // Y-axis upward movement
    if (playerY>=hitbox.y+hitbox.h &&
        nextPlayerY<=hitbox.y+hitbox.h && // If player will go through platform
        playerX+PLAYER_WIDTH>hitbox.x &&
        playerX<hitbox.x+hitbox.w) { // And will collide with platform

        nextPlayerY=hitbox.y+hitbox.h;
        collided=true;
        onPlatform=true;
    }

    return collided;
}

void PushableBlock::checkPush(std::vector<Block> &platformBlocks, const SDL_FRect &playerHitbox, bool moveLeft, bool moveRight, double deltaTime) {
    touchingLeft=(playerHitbox.x+playerHitbox.w>hitbox.x &&
                  playerHitbox.x<hitbox.x &&
                  playerHitbox.y+playerHitbox.h>hitbox.y &&
                  playerHitbox.y<hitbox.y+hitbox.h);

    touchingRight=(playerHitbox.x<hitbox.x+hitbox.w &&
                   playerHitbox.x+playerHitbox.w>hitbox.x+hitbox.w &&
                   playerHitbox.y+playerHitbox.h>hitbox.y &&
                   playerHitbox.y<hitbox.y+hitbox.h);

    float moveStep=0.0;
    if (touchingLeft && moveRight) {
        moveStep=PUSH_SPEED*deltaTime;
    }
    else if (touchingRight && moveLeft) {
        moveStep=-PUSH_SPEED*deltaTime;
    }

    SDL_FRect nextPos=hitbox;
    nextPos.x+=moveStep;

    for (const auto &block : platformBlocks) {
        if (block.getType()!="1J" && block.getType()!="1JL" && block.getType()!="1JR") { // Ignore jump-through platforms
            SDL_FRect b=block.getHitbox();
            if (hitbox.x+hitbox.w<=b.x &&
                nextPos.x+hitbox.w>=b.x &&
                hitbox.y+hitbox.h>b.y &&
                hitbox.y<b.y+b.h) {

                nextPos.x=b.x-hitbox.w;
            }
            if (hitbox.x>=b.x+b.w &&
                nextPos.x<=b.x+b.w &&
                hitbox.y+hitbox.h>b.y &&
                hitbox.y<b.y+b.h) {

                nextPos.x=b.x+b.w;
            }
        }
    }

    hitbox.x=nextPos.x;
}

void PushableBlock::checkKill(const SDL_FRect &playerHitbox, bool &dead) {
    if (velY>1000.0 && SDL_HasIntersectionF(&hitbox, &playerHitbox)) {
        dead=true;
    }
}

void PushableBlock::resetPosition() {
    hitbox.x=originalX;
    hitbox.y=originalY;
}

SDL_FRect PushableBlock::getHitbox() const {
    return hitbox;
}

/// Pushable block functions end

/// Spike functions start

Spike::Spike(float x, float y, float w, float h, double a, SDL_RendererFlip m, const std::string &type) {
    hitbox={x, y, w, h};
    angle=a;
    mirror=m;
    spikeType=type;
}

bool Spike::checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const {
    return playerX+PLAYER_WIDTH>=hitbox.x &&
           playerX<=hitbox.x+hitbox.w &&
           playerY+PLAYER_HEIGHT>=hitbox.y &&
           playerY<=hitbox.y+hitbox.h; // AABB collision
}

const SDL_FRect &Spike::getHitbox() const {
    return hitbox;
}
const std::string &Spike::getType() const {
    return spikeType;
}
void Spike::movingSpike(double deltaTime) {
    if (unlocked) {
        hitbox.x=realX;
        float dy=realY-hitbox.y;
        float distance=fabs(dy);
        if (distance<1.0f) {
            hitbox.y=realY;
            unlocked=false;
        }
        else {
            float moveStep=speed*deltaTime;
            hitbox.y+=dy/distance*moveStep;
        }
    }
}

/// Spike functions end

/// Jump orb functions start

JumpOrb::JumpOrb(float x, float y, float w, float h, char type) {
    hitbox={x, y, w, h};
    orbType=type;
}

bool JumpOrb::checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const {
    return playerX+PLAYER_WIDTH>=hitbox.x &&
           playerX<=hitbox.x+hitbox.w &&
           playerY+PLAYER_HEIGHT>=hitbox.y &&
           playerY<=hitbox.y+hitbox.h; // AABB collision
}

const SDL_FRect &JumpOrb::getHitbox() const {
    return hitbox;
}

const char JumpOrb::getType() const {
    return orbType;
}

void JumpOrb::updateRotation(double deltaTime) const {
    rotationAngle+=180*deltaTime;
    if (rotationAngle>=360) rotationAngle-=360;
}

/// Jump orb functions end

/// Jump pad functions start

JumpPad::JumpPad(float x, float y, float w, float h, double a, const std::string &type) {
    hitbox={x, y, w, h};
    angle=a;
    padType=type;
}

bool JumpPad::checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const {
    return playerX+PLAYER_WIDTH>=hitbox.x &&
           playerX<=hitbox.x+hitbox.w &&
           playerY+PLAYER_HEIGHT>=hitbox.y &&
           playerY<=hitbox.y+hitbox.h; // AABB collision
}

const SDL_FRect &JumpPad::getHitbox() const {
    return hitbox;
}

const std::string &JumpPad::getType() const {
    return padType;
}

void JumpPad::markUsed() {
    padUsed=true;
}
void JumpPad::resetUsed() {
    padUsed=false;
}

bool JumpPad::canTrigger() {
    return !padUsed;
}

/// Jump pad functions end
