#include <iostream>
#include <SDL.h>
#include "Player.h"
#include "Texture.h"
#include "LevelObjs.h"

extern SDL_Renderer *gRenderer;
extern LTexture cubeTexture;

// Constructor
Player::Player() {
    mPosX=TILE_SIZE-TILE_SIZE*11/18;
    mPosY=SCREEN_HEIGHT-PLAYER_HEIGHT-TILE_SIZE*9/18;
    mVelX=0;
    mVelY=0;
    isJumpHeld=false;
    canJump=true;
    moveLeft=false;
    moveRight=false;
    reverseGravity=false;
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
void Player::move(std::vector<Block> &blocks, std::vector<JumpOrb> &jumpOrbs, double deltaTime) {

    // Horizontal movement
    if (moveLeft && !moveRight) {
        mVelX=-X_VELOCITY;
    }
    else if (moveRight && !moveLeft) {
        mVelX=X_VELOCITY;
    }
    else {
        mVelX=0.0;
    }

    double nextPosX=mPosX+mVelX*deltaTime;

    // Block collision detection (X axis)
    for (const auto &block : blocks) {
        if (block.checkXCollision(mPosX, mPosY, nextPosX, mVelX, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            mVelX=0.0;
        }
    }

    /* Prevent out of bounds
    if (nextPosX<=0) {
        nextPosX=0;
        mVelX=0.0;
    }
    if (nextPosX+PLAYER_WIDTH>=SCREEN_WIDTH) {
        nextPosX=SCREEN_WIDTH-PLAYER_WIDTH;
        mVelX=0.0;
    } */

    // Update position
    mPosX=nextPosX;

    // Gravity (scaled by deltaTime)
    if (!reverseGravity) {
        mVelY+=GRAVITY*deltaTime;
        if (mVelY>TERMINAL_VELOCITY) mVelY=TERMINAL_VELOCITY;
    }
    else {
        mVelY-=GRAVITY*deltaTime;
        if (mVelY<-TERMINAL_VELOCITY) mVelY=-TERMINAL_VELOCITY;
    }

    double nextPosY=mPosY+mVelY*deltaTime;

    bool onPlatform=false;

    // Block collision detection (Y axis)
    for (const auto &block : blocks) {
        if (block.checkYCollision(mPosX, mPosY, nextPosY, mVelY, PLAYER_WIDTH, PLAYER_HEIGHT, onPlatform)) {
            mVelY=0.0;
        }
    }

    /* Prevent out of bounds
    if (nextPosY+PLAYER_HEIGHT>=SCREEN_HEIGHT) {
        nextPosY=SCREEN_HEIGHT-PLAYER_HEIGHT;
        mVelY=0.0;
        onPlatform=true;
    }
    if (nextPosY<=0) {
        nextPosY=0;
        mVelY=0.0;
    } */

    // Calculate coyote time
    if (onPlatform) {
        coyoteTimer=COYOTE_TIME; // Reset timer on ground
    }
    else {
        coyoteTimer-=deltaTime; // Countdown in air
        if (coyoteTimer<0) coyoteTimer=0;
    }

    // If player touches both orb and platform, prioritize orb
    bool touchingOrb=false;
    for (auto &orb : jumpOrbs) {
        if (orb.checkCollision(mPosX, mPosY, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            touchingOrb=true;
        }
    }

    // Allowing jump once
    if (isJumpHeld && canJump && (onPlatform || coyoteTimer>0) && !touchingOrb) {
        if (!reverseGravity) {
            mVelY=JUMP_VELOCITY;
        }
        else {
            mVelY=-JUMP_VELOCITY;
        }
        canJump=false;
        coyoteTimer=0; // Reset coyote timer after jump
    }
    if (!isJumpHeld) {
        canJump=true;
    }

    // Update position
    mPosY=nextPosY;
}

// Helper function for spider pad interactions
void Player::findClosestRectSPad(std::vector<Block> &blocks, std::vector<Spike> &spikes) {

    // Player hitboxes
    SDL_Rect normalHitbox=getHitbox();
    SDL_Rect SPadHitbox=getSPadHitbox();

    // Touch spider pad in normal gravity
    if (reverseGravity) {
        // Set up position to teleport to
        int closestPosY=0;

        for (const auto &block : blocks) {
            SDL_Rect blockHitbox=block.getHitbox();
            if (blockHitbox.x<normalHitbox.x+normalHitbox.w &&
                blockHitbox.x+blockHitbox.w>normalHitbox.x && // If player hitbox inside platform
                blockHitbox.y+blockHitbox.h<=normalHitbox.y) { // And below platform

                if (blockHitbox.y+blockHitbox.h>=closestPosY) {
                    closestPosY=blockHitbox.y+blockHitbox.h;
                }
            }
        }

        for (const auto &spike : spikes) {
            SDL_Rect spikeHitbox=spike.getHitbox();
            if (spikeHitbox.x<=SPadHitbox.x+SPadHitbox.w &&
                spikeHitbox.x+spikeHitbox.w>=SPadHitbox.x && // If player hitbox inside spike
                spikeHitbox.y+spikeHitbox.h<=SPadHitbox.y) { // And below spike

                if (spikeHitbox.y+spikeHitbox.h>=closestPosY) {
                    closestPosY=spikeHitbox.y+spikeHitbox.h;
                }
            }
        }

        mPosY=closestPosY;
    }

    // Touch spider pad in reverse gravity
    else {
        // Set up position to teleport to
        int closestPosY=SCREEN_HEIGHT;

        for (const auto &block : blocks) {
            SDL_Rect blockHitbox=block.getHitbox();
            if (blockHitbox.x<normalHitbox.x+normalHitbox.w &&
                blockHitbox.x+blockHitbox.w>normalHitbox.x && // If player hitbox inside platform
                blockHitbox.y>=normalHitbox.y+normalHitbox.h) { // And above platform

                if (blockHitbox.y<=closestPosY+normalHitbox.h) {
                    closestPosY=blockHitbox.y-normalHitbox.h;
                }
            }
        }

        for (const auto &spike : spikes) {
            SDL_Rect spikeHitbox=spike.getHitbox();
            if (spikeHitbox.x<=SPadHitbox.x+SPadHitbox.w &&
                spikeHitbox.x+spikeHitbox.w>=SPadHitbox.x && // If player hitbox inside spike
                spikeHitbox.y>=SPadHitbox.y+SPadHitbox.h) { // And above spike

                if (spikeHitbox.y<=closestPosY+SPadHitbox.h) {
                    closestPosY=spikeHitbox.y-SPadHitbox.h;
                }
            }
        }

        mPosY=closestPosY;
    }
}

// Jump orb and jump pad interactions
void Player::interact(std::vector<Block> &blocks, std::vector<Spike> &spikes,
                      std::vector<JumpOrb> &jumpOrbs, std::vector<JumpPad> &jumpPads, bool &quit) {

    // Orb interactions
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
                    if (mVelY>0) mVelY=0;
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
                    if (mVelY<0) mVelY=0;
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

    // Pad interactions
    for (auto &pad : jumpPads) {
        if (pad.checkCollision(mPosX, mPosY, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            if (pad.canTrigger()) {
                char padType=pad.getType();
                if (!reverseGravity) {
                    switch (padType) {
                    case 'J': // Yellow pad
                        mVelY=JUMP_VELOCITY*1.37;
                        break;
                    case 'S': // Spider pad
                        reverseGravity=true;
                        findClosestRectSPad(blocks, spikes);
                        mVelY=0.0;
                        break;
                    case 'P': // Pink pad
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
                        findClosestRectSPad(blocks, spikes);
                        mVelY=0.0;
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

    // Spike collision
    for (const auto &spike : spikes) {
        if (spike.checkCollision(mPosX, mPosY, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            quit=true;
        }
    }
}

// Render player to window
void Player::render() {
    cubeTexture.render(static_cast<int>(mPosX), static_cast<int>(mPosY), nullptr, 0.0, nullptr, (reverseGravity ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE));
}


// Get player hitbox, for spike collision
SDL_Rect Player::getHitbox() {
    return {static_cast<int>(mPosX), static_cast<int>(mPosY), PLAYER_WIDTH, PLAYER_HEIGHT};
}

// Get player hitbox, for spider pad interactions
SDL_Rect Player::getSPadHitbox() {
    return {static_cast<int>(mPosX)+TILE_SIZE*7/20, static_cast<int>(mPosY)+TILE_SIZE*7/20, PLAYER_WIDTH*3/10, PLAYER_HEIGHT*3/10};
}

// Get gravity status
bool Player::getGravity() {
    return reverseGravity;
}
