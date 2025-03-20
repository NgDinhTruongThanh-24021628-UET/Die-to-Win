#include <iostream>
#include <SDL.h>
#include "Player.h"
#include "Texture.h"

extern SDL_Renderer *gRenderer;
extern LTexture cubeTexture;

// Constructor
Player::Player() {
    mPosX = 3 * TILE_SIZE - TILE_SIZE * 11 / 18;
    mPosY = SCREEN_HEIGHT - PLAYER_HEIGHT - TILE_SIZE * 9 / 18;
    mVelX=0;
    mVelY=0;
    isJumpHeld=false;
    moveLeft=false;
    moveRight=false;
    canJump=true;
    coyoteTimer=0.0;
}

// Handle mouse + keyboard events
void Player::handleEvent(SDL_Event &e) {
    // Press
    if (e.type==SDL_KEYDOWN && e.key.repeat==0) {
        switch (e.key.keysym.sym) {
        case SDLK_LEFT:
        case SDLK_a:
            moveLeft=true;
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            moveRight=true;
            break;
        case SDLK_SPACE:
        case SDLK_UP:
        case SDLK_w:
            isJumpHeld=true;
            break;
        }
    }
    else if (e.type==SDL_MOUSEBUTTONDOWN) {
        if (e.button.button==SDL_BUTTON_LEFT) {
            isJumpHeld=true;
        }
    }

    // Release
    else if (e.type==SDL_KEYUP && e.key.repeat==0) {
        switch (e.key.keysym.sym) {
        case SDLK_LEFT:
        case SDLK_a:
            moveLeft=false;
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            moveRight=false;
            break;
        case SDLK_SPACE:
        case SDLK_UP:
        case SDLK_w:
            isJumpHeld=false;
            break;
        }
    }
    else if (e.type==SDL_MOUSEBUTTONUP) {
        if (e.button.button==SDL_BUTTON_LEFT) {
            isJumpHeld=false;
        }
    }
}

// Move player, platform physics included, deltaTime for consistent physics
void Player::move(std::vector<SDL_Rect> &blocks, int blockCount, double deltaTime) {
    // Horizontal movement
    if (moveLeft && !moveRight) {
        mVelX = -X_VELOCITY;
    } else if (moveRight && !moveLeft) {
        mVelX = X_VELOCITY;
    } else {
        mVelX = 0.0;
    }

    double nextPosX = mPosX + mVelX * deltaTime;

    // X collision detection
    for (int i = 0; i < blockCount; ++i) {
        SDL_Rect platform = blocks[i];

        // From left side
        if (mVelX > 0 &&
            mPosX + PLAYER_WIDTH <= platform.x &&
            nextPosX + PLAYER_WIDTH >= platform.x &&
            mPosY + PLAYER_HEIGHT > platform.y &&
            mPosY < platform.y + platform.h) {

            nextPosX = platform.x - PLAYER_WIDTH;
            mVelX = 0.0;
        }

        // From right side
        if (mVelX < 0 &&
            mPosX >= platform.x + platform.w &&
            nextPosX <= platform.x + platform.w &&
            mPosY + PLAYER_HEIGHT > platform.y &&
            mPosY < platform.y + platform.h) {

            nextPosX = platform.x + platform.w;
            mVelX = 0.0;
        }
    }
    // Update position
    mPosX = nextPosX;

    // Prevent out of bounds
    if (mPosX < 0) mPosX = 0;
    if (mPosX + PLAYER_WIDTH > SCREEN_WIDTH) mPosX = SCREEN_WIDTH - PLAYER_WIDTH;

    // Gravity (scaled by deltaTime)
    mVelY += GRAVITY * deltaTime;
    if (mVelY > TERMINAL_VELOCITY) mVelY = TERMINAL_VELOCITY;
    double nextPosY = mPosY + mVelY * deltaTime;

    bool onPlatform = false;

    // Y collision detection
    for (int i = 0; i < blockCount; i++) {
        SDL_Rect platform = blocks[i];

        // Falling
        if (mVelY > 0 &&
            mPosY + PLAYER_HEIGHT <= platform.y &&
            nextPosY + PLAYER_HEIGHT >= platform.y &&
            mPosX + PLAYER_WIDTH > platform.x &&
            mPosX < platform.x + platform.w) {

            nextPosY = platform.y - PLAYER_HEIGHT;
            mVelY = 0.0;
            onPlatform = true;
        }

        // Jumping up, hitting bottom
        if (mVelY < 0 &&
            mPosY >= platform.y + platform.h &&
            nextPosY <= platform.y + platform.h &&
            mPosX + PLAYER_WIDTH > platform.x &&
            mPosX < platform.x + platform.w) {

            nextPosY = platform.y + platform.h;
            mVelY = 0.0;
        }
    }

    // On the ground
    if (nextPosY + PLAYER_HEIGHT >= SCREEN_HEIGHT) {
        nextPosY = SCREEN_HEIGHT - PLAYER_HEIGHT;
        mVelY = 0.0;
        onPlatform = true;
    }
    if (nextPosY <= 0) {
        nextPosY=0;
        mVelY=0.0;
    }

    // Calculate coyote time
    if (onPlatform) {
        coyoteTimer = COYOTE_TIME;  // Reset timer on ground
    }
    else {
        coyoteTimer -= deltaTime;   // Countdown in air
        if (coyoteTimer < 0) coyoteTimer = 0;
    }

    // Update position
    mPosY = nextPosY;

    // Allowing jump once
    if (isJumpHeld && canJump && (onPlatform || coyoteTimer > 0)) {
        mVelY = JUMP_VELOCITY;
        canJump = false;
        coyoteTimer = 0;  // Reset coyote timer after jump
    }
    if (!isJumpHeld) {
        canJump = true;
    }
}


// Render player to window
void Player::render() {
    cubeTexture.render(static_cast<int>(mPosX), static_cast<int>(mPosY));
}

// Get player hitbox, for spike collision
SDL_Rect Player::getHitbox() {
    return {static_cast<int>(mPosX), static_cast<int>(mPosY), PLAYER_WIDTH, PLAYER_HEIGHT};
}
