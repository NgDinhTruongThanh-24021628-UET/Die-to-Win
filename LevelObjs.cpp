#include <iostream>
#include <SDL.h>
#include <vector>
#include "Texture.h"
#include "LevelObjs.h"

extern SDL_Renderer *gRenderer;
extern LTexture blockTexture;

// Block functions start

Block::Block(int x, int y, int w, int h) {
    hitbox={x, y, w, h};
}

bool Block::checkXCollision(double &playerX, double playerY, double &nextPlayerX,
                         double playerVelX, int PLAYER_WIDTH, int PLAYER_HEIGHT) const {
    bool collided=false;

    // From left side
    if (playerVelX>0 &&
        playerX+PLAYER_WIDTH<=hitbox.x &&
        nextPlayerX+PLAYER_WIDTH>=hitbox.x && // If player will go through platform
        playerY+PLAYER_HEIGHT>hitbox.y &&
        playerY<hitbox.y+hitbox.h) { // And will collide with platform

        nextPlayerX=hitbox.x-PLAYER_WIDTH;
        collided=true;
    }

    // From right side
    if (playerVelX<0 &&
        playerX>=hitbox.x+hitbox.w &&
        nextPlayerX<=hitbox.x+hitbox.w && // If player will go through platform
        playerY+PLAYER_HEIGHT>hitbox.y &&
        playerY<hitbox.y+hitbox.h) { // And will collide with platform

        nextPlayerX=hitbox.x+hitbox.w;
        collided=true;
    }

    return collided;
}

bool Block::checkYCollision(int playerX, double &playerY, double &nextPlayerY,
                            double playerVelY, int PLAYER_WIDTH, int PLAYER_HEIGHT, bool &onPlatform) const {
    bool collided=false;

    // Falling
    if (playerVelY>0 &&
        playerY+PLAYER_HEIGHT<=hitbox.y &&
        nextPlayerY+PLAYER_HEIGHT>=hitbox.y && // If player will go through platform
        playerX+PLAYER_WIDTH>hitbox.x &&
        playerX<hitbox.x+hitbox.w) { // And will collide with platform

        nextPlayerY=hitbox.y-PLAYER_HEIGHT;
        collided=true;
        onPlatform=true;
    }

    // Jumping up, hitting block
    if (playerVelY<0 &&
        playerY>=hitbox.y+hitbox.h &&
        nextPlayerY<=hitbox.y+hitbox.h && // If player will go through platform
        playerX+PLAYER_WIDTH>hitbox.x &&
        playerX<hitbox.x+hitbox.w) { // And will collide with platform

        nextPlayerY=hitbox.y+hitbox.h;
        collided=true;
        onPlatform=true;
    }

    return collided;
}

const SDL_Rect &Block::getHitbox() const {
    return hitbox;
}

// Block functions end

// Spike functions start

Spike::Spike(int x, int y, int w, int h) {
    hitbox={x, y, w, h};
}

bool Spike::checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const {
    return playerX+PLAYER_WIDTH>=hitbox.x &&
           playerX<=hitbox.x+hitbox.w &&
           playerY+PLAYER_HEIGHT>=hitbox.y &&
           playerY<=hitbox.y+hitbox.h; // AABB collision
}

const SDL_Rect &Spike::getHitbox() const {
    return hitbox;
}

// Spike functions end

// Jump orb functions start

JumpOrb::JumpOrb(int x, int y, int w, int h, char type) {
    hitbox={x, y, w, h};
    orbType=type;
}

bool JumpOrb::checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const {
    return playerX+PLAYER_WIDTH>=hitbox.x &&
           playerX<=hitbox.x+hitbox.w &&
           playerY+PLAYER_HEIGHT>=hitbox.y &&
           playerY<=hitbox.y+hitbox.h; // AABB collision
}

const SDL_Rect &JumpOrb::getHitbox() const {
    return hitbox;
}

const int JumpOrb::getType() const {
    return orbType;
}

// Jump orb functions end

// Jump pad functions start

JumpPad::JumpPad(int x, int y, int w, int h, char type) {
    hitbox={x, y, w, h};
    padType=type;
}

bool JumpPad::checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const {
    return playerX+PLAYER_WIDTH>=hitbox.x &&
           playerX<=hitbox.x+hitbox.w &&
           playerY+PLAYER_HEIGHT>=hitbox.y &&
           playerY<=hitbox.y+hitbox.h; // AABB collision
}

const SDL_Rect &JumpPad::getHitbox() const {
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
