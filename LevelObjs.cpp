#include <iostream>
#include <SDL.h>
#include <vector>
#include "Texture.h"
#include "LevelObjs.h"

extern SDL_Renderer *gRenderer;

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
const std::string &Block::getType() const {
    return blockType;
}

// Block functions end

// Spike functions start

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
