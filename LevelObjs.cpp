#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "Texture.h"
#include "LevelObjs.h"
#include "Player.h"
#include "Enums.h"
using namespace std;

extern SDL_Renderer *gRenderer;
extern LTexture instructionTexture[];
extern TTF_Font *gSmallFont;
extern SDL_Color textColor;
extern bool uniqueDigits;

// Create random password
#include <algorithm>
#include <random>
#include <ctime>
std::vector<int> enigmaPassword;
void generateEnigmaPassword() {
    std::vector<int> digits={0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    std::mt19937 g(static_cast<unsigned int>(time(0)));
    std::shuffle(digits.begin(), digits.end(), g);

    enigmaPassword=std::vector<int>(digits.begin(), digits.begin()+4);
}

// Block functions start

Block::Block(float x, float y, float w, float h, double a, SDL_RendererFlip m, const std::string &type) {
    hitbox={x, y, w, h};
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
const string &Block::getType() const {
    return blockType;
}
void Block::movingBlock(double deltaTime) {
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
void Block::offsetPosition(float offsetX, float offsetY) {
    hitbox.x+=offsetX;
    hitbox.y+=offsetY;
}

bool Block::isInteractable() const {
    string type[12]={"1I1", "1I2", "1I3", "1I4", "1IP", "1S", "1P", "1C", "1BI", "1IN",
                     "1R", "1SA"};
    for (int i=0; i<12; i++) {
        if (blockType==type[i]) return true;
    }
    return false;
}
void Block::interact(unsigned long long &totalMoney, int &gainPerHit, int &passiveIncome, GameStatus &currentStatus,
                     vector<Block> &blocks, vector<PushableBlock> &pushableBlocks, vector<Spike> &spikes,
                     const string &levelName, double deltaTime, bool &timeStopped, double &timeStopTimer) {
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
        interactEnigma(blocks, spikes, deltaTime);
    }
    else if (levelName=="Move to Die" || levelName=="Illusion World") {
        interactMoveToDie(blocks, pushableBlocks, timeStopped, timeStopTimer);
    }
}

void Block::interactClicker(unsigned long long &totalMoney, int &gainPerHit, int &passiveIncome,
                            std::vector<Block> &blocks, std::vector<Spike> &spikes, double deltaTime) {
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
    else if (blockType=="1I2") {
        if (counter>=5) counter=5;
        else {
            if (totalMoney>=(unsigned long long)value) {
                for (auto &block : blocks) {
                    if (block.getType()=="1IP") {
                        block.unlocked=true;
                        block.realX=block.getHitbox().x;
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

void Block::interactEnigma(vector<Block> &blocks, vector<Spike> &spikes, double deltaTime) {
    if (spikes.empty()) spikes.emplace_back(800+TILE_SIZE*2/5.0f, 800+TILE_SIZE*3/10.0f, TILE_SIZE/5.0f, TILE_SIZE*2/5.0f, 0, SDL_FLIP_NONE, "2EU");
    if (enigmaPassword.empty()) generateEnigmaPassword();
    if (blockType=="1BI") {
        counter=(counter+1)%10;
    }
    else if (blockType=="1IN") {
        vector<Block*> digits;
        for (auto &block : blocks) {
            if (block.getType()=="1BI") {
                digits.push_back(&block);
            }
        }

        if (digits.size()!=enigmaPassword.size()) return;
        for (int i=0; i<int(digits.size())-1; i++) {
            for (int j=i+1; j<int(digits.size()); j++) {
                if (digits[i]->counter==digits[j]->counter) {
                    uniqueDigits=false;
                    return;
                }
            }
        }
        uniqueDigits=true;

        int correctPos=0, wrongPos=0;
        vector<bool> passwordUsed(enigmaPassword.size(), false);
        vector<bool> guessUsed(digits.size(), false);
        for (int i=0; i<int(digits.size()); i++) {
            if (digits[i]->counter==enigmaPassword[i]) {
                correctPos++;
                passwordUsed[i]=guessUsed[i]=true;
            }
        }
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
        for (auto &block : blocks) {
            if (block.getType()=="1BG") {
                block.counter=correctPos;
            }
            else if (block.getType()=="1BO") {
                block.counter=wrongPos;
            }
        }
        if (correctPos==4) {
            for (auto &spike : spikes) {
                spike.unlocked=true;
                spike.realX=SCREEN_WIDTH-TILE_SIZE*2-TILE_SIZE*7/18.0f+TILE_SIZE*2/5.0f;
                spike.realY=SCREEN_HEIGHT-TILE_SIZE*3/2.0f+TILE_SIZE*3/10.0f;
            }
        }
        digits.clear();
    }
}

void Block::interactMoveToDie(vector<Block> &blocks, vector<PushableBlock> &pushableBlocks, bool &timeStopped, double &timeStopTimer) {
    if (blockType=="1R") {
        for (auto &block : pushableBlocks) {
            if (timeStopped) {
                block.resetQueued=true;
            }
            else {
                block.resetPosition();
            }
        }
    }
    else if (blockType=="1SA") {
        if (!timeStopped) {
            timeStopped=true;
            timeStopTimer=5;
        }
    }
}

// Block functions end

// Pushable block functions start

PushableBlock::PushableBlock(float x, float y, float w, float h) {
    hitbox={x, y, w, h};
    originalX=x;
    originalY=y;
}

void PushableBlock::update(vector<Block> &platformBlocks, const SDL_FRect &playerHitbox,
                           bool moveLeft, bool moveRight, bool &dead, double deltaTime) {

    checkPush(platformBlocks, playerHitbox, moveLeft, moveRight, deltaTime);
    applyPhysics(platformBlocks, deltaTime);
    checkKill(playerHitbox, dead);
}

void PushableBlock::applyPhysics(vector<Block> &platformBlocks, double deltaTime) {
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

void PushableBlock::checkPush(vector<Block> &platformBlocks, const SDL_FRect &playerHitbox, bool moveLeft, bool moveRight, double deltaTime) {
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

// Spike functions start

Spike::Spike(float x, float y, float w, float h, double a, SDL_RendererFlip m, const string &type) {
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
const string &Spike::getType() const {
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

// Spike functions end

// Jump orb functions start

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

// Jump orb functions end

// Jump pad functions start

JumpPad::JumpPad(float x, float y, float w, float h, double a, const string &type) {
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

// Jump pad functions end
