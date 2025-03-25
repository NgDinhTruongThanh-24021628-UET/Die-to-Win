#include <iostream>
#include <SDL.h>
#include "Player.h"
#include "Texture.h"
#include "LevelObjs.h"

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
void Player::move(std::vector<Block> &blocks, bool reverseGravity, double deltaTime) {
    // Horizontal movement
    if (moveLeft && !moveRight) {
        mVelX = -X_VELOCITY;
    } else if (moveRight && !moveLeft) {
        mVelX = X_VELOCITY;
    } else {
        mVelX = 0.0;
    }

    double nextPosX = mPosX + mVelX * deltaTime;

    // Block collision detection (X axis)
    for (const auto &block : blocks) {
        if (block.checkXCollision(mPosX, mPosY, nextPosX, mVelX, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            mVelX=0.0;
        }
    }

    /* Prevent out of bounds
    if (nextPosX <= 0) {
        nextPosX = 0;
        mVelX=0.0;
    }
    if (nextPosX + PLAYER_WIDTH >= SCREEN_WIDTH) {
        nextPosX = SCREEN_WIDTH - PLAYER_WIDTH;
        mVelX=0.0;
    } */

    // Update position
    mPosX = nextPosX;

    // Gravity (scaled by deltaTime)
    if (!reverseGravity) {
        mVelY += GRAVITY * deltaTime;
        if (mVelY > TERMINAL_VELOCITY) mVelY = TERMINAL_VELOCITY;
    }
    else {
        mVelY-=GRAVITY*deltaTime;
        if (mVelY<-TERMINAL_VELOCITY) mVelY=-TERMINAL_VELOCITY;
    }

    double nextPosY = mPosY + mVelY * deltaTime;

    bool onPlatform = false;

    // Block collision detection (Y axis)
    for (const auto &block : blocks) {
        if (block.checkYCollision(mPosX, mPosY, nextPosY, mVelY, PLAYER_WIDTH, PLAYER_HEIGHT, onPlatform)) {
            mVelY=0.0;
        }
    }

    /* Prevent out of bounds
    if (nextPosY + PLAYER_HEIGHT >= SCREEN_HEIGHT) {
        nextPosY = SCREEN_HEIGHT - PLAYER_HEIGHT;
        mVelY = 0.0;
        onPlatform = true;
    }
    if (nextPosY <= 0) {
        nextPosY=0;
        mVelY=0.0;
    } */

    // Calculate coyote time
    if (onPlatform) {
        coyoteTimer = COYOTE_TIME;  // Reset timer on ground
    }
    else {
        coyoteTimer -= deltaTime;   // Countdown in air
        if (coyoteTimer < 0) coyoteTimer = 0;
    }

    // Allowing jump once
    if (isJumpHeld && canJump && (onPlatform || coyoteTimer > 0)) {
        if (!reverseGravity) {
            mVelY = JUMP_VELOCITY;
        }
        else {
            mVelY=-JUMP_VELOCITY;
        }
        canJump = false;
        coyoteTimer = 0;  // Reset coyote timer after jump
    }
    if (!isJumpHeld) {
        canJump = true;
    }

    // Update position
    mPosY = nextPosY;
}

// Jump orb and jump pad interactions
void Player::interact(std::vector<JumpOrb> &jumpOrbs, std::vector<JumpPad> &jumpPads, double deltaTime) {
    for (auto &orb : jumpOrbs) {
        if (orb.checkCollision(mPosX, mPosY, PLAYER_WIDTH, PLAYER_HEIGHT) && isJumpHeld && canJump) {
            char orbType=orb.getType();
            if (!reverseGravity) {
                switch (orbType) {
                case 'Y': // Yellow orb
                    mVelY=JUMP_VELOCITY;
                    break;
                case 'B': // Blue orb
                    reverseGravity=true;
                    mVelY=0;
                    break;
                case 'G': // Green orb
                    reverseGravity=true;
                    mVelY=-JUMP_VELOCITY;
                    break;
                }
            }
            else {
                switch (orbType) {
                case 'Y':
                    mVelY=-JUMP_VELOCITY;
                    break;
                case 'B':
                    reverseGravity=false;
                    mVelY=0;
                    break;
                case 'G':
                    reverseGravity=false;
                    mVelY=JUMP_VELOCITY;
                    break;
                }
            }
            canJump=false;
        }
    }
    for (auto &pad : jumpPads) {
        if (pad.checkCollision(mPosX, mPosY, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            if (pad.canTrigger()) {
                char padType=pad.getType();
                if (!reverseGravity) {
                    switch (padType) {
                    case 'J':
                        mVelY=JUMP_VELOCITY*1.37;
                        break;
                    case 'S':
                        reverseGravity=true;
                        break;
                    case 'P':
                        mVelY=JUMP_VELOCITY;
                        break;
                    }
                }
                else {
                    switch (padType) {
                    case 'J':
                        mVelY=-JUMP_VELOCITY*1.37;
                        break;
                    case 'S':
                        reverseGravity=false;
                        break;
                    case 'P':
                        mVelY=-JUMP_VELOCITY;
                        break;
                    }
                }
                pad.markUsed();
            }
        }
        else {
            pad.resetUsed();
        }
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

// Get gravity status
bool Player::getGravity() {
    return reverseGravity;
}
