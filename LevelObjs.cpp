#include <iostream>
#include <string>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "Texture.h"
#include "LevelObjs.h"
#include "Player.h"
#include "Enums.h"
using namespace std;

extern SDL_Renderer *gRenderer;
extern LTexture instructionTexture[1000];
extern TTF_Font *gSmallFont;
extern SDL_Color textColor;

// Block functions start

Block::Block(float x, float y, float w, float h, double a, const std::string &type) {
    hitbox={x, y, w, h};
    angle=a;
    blockType=type;
}

bool Block::checkXCollision(double &playerX, double playerY, double &nextPlayerX,
                            double playerVelX, int PLAYER_WIDTH, int PLAYER_HEIGHT) const {
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
        playerX<hitbox.x+hitbox.w) { // And will collide with platform

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
void Block::changePosition(float x, float y) {
    hitbox.x=x;
    hitbox.y=y;
}

bool Block::isInteractable() const {
    string type[8]={"1I1", "1I2", "1I3", "1I4", "1IP", "1S", "1P", "1C"};
    for (int i=0; i<8; i++) {
        if (blockType==type[i]) return true;
    }
    return false;
}
void Block::interact(unsigned long long &totalMoney, int &gainPerHit, int &passiveIncome, GameStatus &currentStatus, vector<Block> &blocks) {
    if (!isInteractable()) return;
    if (blockType=="1IP") {
        totalMoney+=gainPerHit;
    }
    else if (blockType=="1I3") {
        if (counter>=25) counter=25;
        else {
            if (totalMoney>=(unsigned long long)value) {
                if (counter==0) gainPerHit*=5;
                else gainPerHit+=increment;
                totalMoney-=value;
                increment*=2;
                value*=2;
                counter++;
            }
        }
    }
    else if (blockType=="1I4") {
        if (counter>=25) counter=25;
        else {
            if (totalMoney>=(unsigned long long)value) {
                passiveIncome+=increment;
                totalMoney-=value;
                increment*=2;
                value*=2;
                counter++;
            }
        }
    }
    else if (blockType=="1I2") {
        if (counter>=5) counter=5;
        else {
            if (counter==0) value=100;
            if (totalMoney>=(unsigned long long)value) {
                for (auto &block : blocks) {
                    if (block.getType()=="1IP") {
                        block.changePosition(block.getHitbox().x, block.getHitbox().y+TILE_SIZE/3);
                    }
                }
                totalMoney-=value;
                value*=20;
                counter++;
            }
        }
    }
    else if (blockType=="1S") {
        currentStatus=SETTINGS;
    }
    else if (blockType=="1P") {
        currentStatus=START;
    }
    else if (blockType=="1C") {
        currentStatus=CREDITS;
    }
}

// Block functions end

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

const int JumpOrb::getType() const {
    return orbType;
}

void JumpOrb::updateRotation(double deltaTime) const {
    rotationAngle+=180*deltaTime;
    if (rotationAngle>=360) rotationAngle-=360;
}

// Jump orb functions end

// Jump pad functions start

JumpPad::JumpPad(float x, float y, float w, float h, char type) {
    hitbox={x, y, w, h};
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

const int JumpPad::getType() const {
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
