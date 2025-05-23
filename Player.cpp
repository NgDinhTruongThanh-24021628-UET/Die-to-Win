#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#include <cmath>
#include "Player.h"
#include "Texture.h"
#include "LevelObjs.h"
#include "Enums.h"

extern SDL_Renderer *gRenderer;
extern LTexture cubeTexture;
extern LTexture toBeContinued;
extern Mix_Music *fnafSong;
extern Mix_Music *jojoSong;

// Constructor
Player::Player() {
    mPosX=2*TILE_SIZE-TILE_SIZE*11/18;
    mPosY=SCREEN_HEIGHT-3*PLAYER_HEIGHT-TILE_SIZE*9/18;
    mVelX=0;
    mVelY=0;
    isJumpHeld=false;
    canJump=true;
    moveLeft=false;
    moveRight=false;
    onPlatform=false;
    hitCeiling=false;
    reverseGravity=false;
    touchingOrb=false;
    isDashing=false;
    coyoteTimer=0.0;
    totalMoney=0;
    gainPerHit=1;
    passiveIncome=0;
    income=0;
}

// Reset player status
void Player::reset() {
    mPosX=2*TILE_SIZE-TILE_SIZE*11/18;
    mPosY=SCREEN_HEIGHT-3*PLAYER_HEIGHT-TILE_SIZE*9/18;
    mVelX=0;
    mVelY=0;
    isJumpHeld=false;
    canJump=true;
    moveLeft=false;
    moveRight=false;
    onPlatform=false;
    hitCeiling=false;
    reverseGravity=false;
    touchingOrb=false;
    isDashing=false;
    coyoteTimer=0.0;
    totalMoney=0;
    gainPerHit=1;
    passiveIncome=0;
    income=0;
}
void Player::resetBool() {
    isJumpHeld=false;
    canJump=true;
    moveLeft=false;
    moveRight=false;
    onPlatform=false;
    hitCeiling=false;
    touchingOrb=false;
    isDashing=false;
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

// Allow player to enter 1-block-wide gap
void Player::forcePushIntoGap(std::vector<Block> &blocks) {
    int limit=ceil(TILE_SIZE/double(15.0));
    for (size_t i=0; i<blocks.size()-1; i++) {
        const SDL_FRect leftBlock=blocks[i].getHitbox();
        const SDL_FRect rightBlock=blocks[i+1].getHitbox();
        {
            // Ensure blocks are on the same height
            if (leftBlock.y!=rightBlock.y) continue;
            // Ensure gap is close to the player
            if (mPosX<leftBlock.x+leftBlock.w-limit || mPosX+PLAYER_WIDTH>rightBlock.x+limit) continue; // X position check
            if (mPosY-limit>leftBlock.y+leftBlock.h || mPosY+PLAYER_HEIGHT+limit<leftBlock.y) continue; // Y position check
            // Ensure exactly 1-block-wide gap
            if (rightBlock.x-(leftBlock.x+leftBlock.w)!=TILE_SIZE) continue;
        }
        // Normal gravity
        if (!reverseGravity) {
            // Check if player is falling into the gap
            if (mPosY+PLAYER_HEIGHT==leftBlock.y && mVelX==0) {
                // Force player inside the gap
                mPosX=leftBlock.x+leftBlock.w;
            }
            // Check if player is jumping into the gap
            else if (mVelY<0) {
                mPosX=leftBlock.x+leftBlock.w;
            }
            // Edge case: Player is squeezed between floor and gap
            else if (mPosY==leftBlock.y+leftBlock.h && isJumpHeld) {
                mPosX=leftBlock.x+leftBlock.w;
            }
        }
        // Reversed gravity
        else {
            // Check if player is falling into the gap
            if (mPosY==leftBlock.y+leftBlock.h && mVelX==0) {
                mPosX=leftBlock.x+leftBlock.w;
            }
            // Check if player is jumping into the gap
            else if (mVelY>0) {
                mPosX=leftBlock.x+leftBlock.w;
            }
            // Edge case: Player is squeezed between floor and gap
            else if (mPosY+PLAYER_HEIGHT==leftBlock.y && isJumpHeld) {
                mPosX=leftBlock.x+leftBlock.w;
            }
        }
    }
}

// Move player, platform physics included, deltaTime for consistent physics
void Player::move(std::vector<Block> &blocks, std::vector<PushableBlock> &pushableBlocks, std::vector<Spike> &spikes,
                  std::vector<JumpOrb> &jumpOrbs, GameStatus &currentStatus, const std::string &levelName, double deltaTime) {

    // If player touches both orb and platform, prioritize orb
    touchingOrb=false;
    if (!isJumpHeld || hitCeiling) isDashing=false;

    for (auto &orb : jumpOrbs) {
        if (orb.checkCollision(mPosX, mPosY, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            touchingOrb=true;
        }
    }

    // Flip input
    if (levelName=="Vertigo") {
        flippedInput=true;
    }
    else flippedInput=false;

    // Horizontal movement
    if (!isDashing || !isJumpHeld) {
        if (moveLeft && !moveRight) {
            mVelX=(flippedInput ? X_VELOCITY : -X_VELOCITY);
        }
        else if (moveRight && !moveLeft) {
            mVelX=(flippedInput ? -X_VELOCITY : X_VELOCITY);
        }
        else {
            mVelX=0.0;
        }
    }
    else mVelX=0.0;

    double nextPosX=mPosX+mVelX*deltaTime;

    // Block collision detection (X axis)
    for (auto &block : blocks) {
        if (block.checkXCollision(mPosX, mPosY, nextPosX, mVelX, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            mVelX=0.0;
            if (block.getType()=="1WVI") block.interact(totalMoney, gainPerHit, passiveIncome, currentStatus,
                                                  blocks, pushableBlocks, spikes, levelName, deltaTime,
                                                  timeStopped, timeStopTimer, powerPercent, cutscenePlaying);
        }
    }
    for (auto &block : pushableBlocks) {
        if (block.touchingLeft) {
            nextPosX=block.getHitbox().x-PLAYER_WIDTH;
        }
        else if (block.touchingRight) {
            nextPosX=block.getHitbox().x+block.getHitbox().w;
        }
    }

    // Update position
    mPosX=nextPosX;

    // Vertical movement
    // Gravity (scaled by deltaTime)
    if (!isDashing) { // Dash orb ignores gravity
        if (!reverseGravity) {
            mVelY+=GRAVITY*deltaTime;
            if (mVelY>TERMINAL_VELOCITY) mVelY=TERMINAL_VELOCITY;
        }
        else {
            mVelY-=GRAVITY*deltaTime;
            if (mVelY<-TERMINAL_VELOCITY) mVelY=-TERMINAL_VELOCITY;
        }
    }

    double nextPosY=mPosY+mVelY*deltaTime;
    onPlatform=false;
    hitCeiling=false;

    // Block collision detection (Y axis)
    for (auto &block : blocks) {
        if (block.checkYCollision(mPosX, mPosY, nextPosY, mVelY, PLAYER_WIDTH, PLAYER_HEIGHT,
                                  onPlatform, hitCeiling, reverseGravity)) {
            if (!isDashing) mVelY=0.0;
            if (onPlatform==false) block.interact(totalMoney, gainPerHit, passiveIncome, currentStatus,
                                                  blocks, pushableBlocks, spikes, levelName, deltaTime,
                                                  timeStopped, timeStopTimer, powerPercent, cutscenePlaying);
        }
    }
    forcePushIntoGap(blocks);
    for (auto &block : pushableBlocks) {
        if (block.checkYCollision(mPosX, mPosY, nextPosY, mVelY, PLAYER_WIDTH, PLAYER_HEIGHT, onPlatform)) {
            mVelY=0.0;
        }
    }

    // Calculate coyote time
    if (onPlatform) {
        coyoteTimer=COYOTE_TIME; // Reset timer on ground
    }
    else {
        coyoteTimer-=deltaTime; // Countdown in air
        if (coyoteTimer<0) coyoteTimer=0;
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
    if (cutscenePlaying) canJump=false;

    // Update position
    mPosY=nextPosY;
}

// Helper function for spider pad interactions
void Player::findClosestRectSPad(JumpPad pad, std::vector<Block> &blocks, std::vector<Spike> &spikes) {

    // Player hitboxes
    SDL_FRect normalHitbox=getHitbox();
    SDL_FRect SPadHitbox=getSPadHitbox();

    // Teleport to ceiling
    if (pad.angle==0) {
        // Set up position to teleport to
        float closestPosY=0;

        for (auto &block : blocks) {
            if (block.getType()=="1J") continue;
            SDL_FRect blockHitbox=block.getHitbox();
            if (blockHitbox.x<normalHitbox.x+normalHitbox.w &&
                blockHitbox.x+blockHitbox.w>normalHitbox.x && // If player hitbox inside platform
                blockHitbox.y+blockHitbox.h<=normalHitbox.y) { // And below platform

                if (blockHitbox.y+blockHitbox.h>=closestPosY) {
                    closestPosY=blockHitbox.y+blockHitbox.h;
                }
            }
        }

        for (const auto &spike : spikes) {
            SDL_FRect spikeHitbox=spike.getHitbox();
            if (spikeHitbox.x<=SPadHitbox.x+SPadHitbox.w &&
                spikeHitbox.x+spikeHitbox.w>=SPadHitbox.x && // If player hitbox inside spike
                spikeHitbox.y+spikeHitbox.h<=SPadHitbox.y) { // And below spike

                if (spikeHitbox.y+spikeHitbox.h>=closestPosY) {
                    closestPosY=spikeHitbox.y+spikeHitbox.h;
                }
            }
        }

        reverseGravity=true;
        mPosY=closestPosY;
    }

    // Teleport to floor
    else if (pad.angle==90 || pad.angle==180 || pad.angle==270) {
        // Set up position to teleport to
        float closestPosY=SCREEN_HEIGHT;

        for (auto &block : blocks) {
            if (block.getType()=="1J") continue;
            SDL_FRect blockHitbox=block.getHitbox();
            if (blockHitbox.x<normalHitbox.x+normalHitbox.w &&
                blockHitbox.x+blockHitbox.w>normalHitbox.x && // If player hitbox inside platform
                blockHitbox.y>=normalHitbox.y+normalHitbox.h) { // And above platform

                if (blockHitbox.y<=closestPosY+normalHitbox.h) {
                    closestPosY=blockHitbox.y-normalHitbox.h;
                }
            }
        }

        for (const auto &spike : spikes) {
            SDL_FRect spikeHitbox=spike.getHitbox();
            if (spikeHitbox.x<=SPadHitbox.x+SPadHitbox.w &&
                spikeHitbox.x+spikeHitbox.w>=SPadHitbox.x && // If player hitbox inside spike
                spikeHitbox.y>=SPadHitbox.y+SPadHitbox.h) { // And above spike

                if (spikeHitbox.y<=closestPosY+SPadHitbox.h) {
                    closestPosY=spikeHitbox.y-SPadHitbox.h;
                }
            }
        }

        reverseGravity=false;
        mPosY=closestPosY;
    }
}

// Jump orb and jump pad interactions
void Player::interact(std::vector<Block> &blocks, std::vector<PushableBlock> &pushableBlocks, std::vector<Spike> &spikes,
                      std::vector<JumpOrb> &jumpOrbs, std::vector<JumpPad> &jumpPads, const std::string &levelName, double deltaTime, bool &dead) {

    // Idle tycoon
    if (levelName=="Cookies") {
        income+=passiveIncome*deltaTime;
        if (income>=passiveIncome) {
            totalMoney+=passiveIncome;
            income=0;
        }
        if (totalMoney>1000000) {
            for (auto &spike : spikes) {
                if (!spike.unlocked) {
                    spike.unlocked=true;
                    spike.realX=SCREEN_WIDTH-TILE_SIZE*8-TILE_SIZE*7/18.0f+TILE_SIZE*2/5.0f;
                    spike.realY=SCREEN_HEIGHT-TILE_SIZE*3-TILE_SIZE/2.0f+TILE_SIZE*3/10.0f;
                }
            }
        }
    }

    // Time stop
    else if (levelName=="Illusion World") {
        if (timeStopped) {
            SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 80);
            SDL_FRect timeStopOverlay={TILE_SIZE*7/18, TILE_SIZE/2, SCREEN_WIDTH-TILE_SIZE*14/18, SCREEN_HEIGHT-TILE_SIZE};
            SDL_RenderFillRectF(gRenderer, &timeStopOverlay);

            timeStopTimer-=deltaTime;
            if (timeStopTimer<0) {
                timeStopped=false;
                timeStopTimer=0;
            }
        }
        else {
            for (auto &block : pushableBlocks) {
                if (block.resetQueued) {
                    block.resetQueued=false;
                    block.resetPosition();
                }
            }
        }
    }

    // Fnaf puzzle
    else if (levelName=="Five Nights") {
        if (!powerOut) {
            drain+=drainRate*deltaTime;
            if (drain>=drainRate) {
                powerPercent-=drainRate;
                drain=0;
            }
        }
        if (powerPercent==0) {
            powerOut=true;

            SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 240);
            SDL_FRect fnafOverlay={0, 0, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT)};
            SDL_RenderFillRectF(gRenderer, &fnafOverlay);
        }
        if (powerOut && !diedFromPowerOut) {
            Mix_PlayMusic(fnafSong, 0);
            diedFromPowerOut=true;
        }
        if (!Mix_PlayingMusic()) dead=true;
    }

    // Jojo reference
    else if (levelName=="Star on Shoulder") {
        if (cutscenePlaying && !roundaboutPlaying) {
            Mix_PlayMusic(jojoSong, 0);
            roundaboutPlaying=true;
        }
        levelFreeze=false;
        for (const auto &block : blocks) {
            if (block.getType()=="3ADM" && block.getHitbox().y>SCREEN_HEIGHT-3*TILE_SIZE) {
                if (Mix_PlayingMusic()) levelFreeze=true;
            }
        }
        if (levelFreeze) {
            SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(gRenderer, 211, 211, 211, 80);
            SDL_FRect cutsceneOverlay={TILE_SIZE*7/18, TILE_SIZE/2, SCREEN_WIDTH-TILE_SIZE*14/18, SCREEN_HEIGHT-TILE_SIZE};
            SDL_RenderFillRectF(gRenderer, &cutsceneOverlay);

            toBeContinued.render(0, SCREEN_HEIGHT-toBeContinued.getHeight());
        }
        if (!Mix_PlayingMusic()) {
            cutscenePlaying=false;
            levelFreeze=false;
        }
    }

    // Orb interactions
    for (auto &orb : jumpOrbs) {
        if (orb.checkCollision(mPosX, mPosY, PLAYER_WIDTH, PLAYER_HEIGHT) && isJumpHeld && canJump) {
            char type=orb.getType();
            switch (type) {
            case 'Y': // Yellow orb
                if (!reverseGravity) mVelY=JUMP_VELOCITY;
                else mVelY=-JUMP_VELOCITY;
                break;
            case 'B': // Blue orb
                if (!reverseGravity) {
                    reverseGravity=true;
                    mVelY=-GRAVITY/8;
                }
                else {
                    reverseGravity=false;
                    mVelY=GRAVITY/8;
                }
                break;
            case 'G': // Green orb
                if (!reverseGravity) {
                    reverseGravity=true;
                    mVelY=-JUMP_VELOCITY;
                }
                else {
                    reverseGravity=false;
                    mVelY=JUMP_VELOCITY;
                }
                break;
            case 'D': // Dash orb
                if (isJumpHeld) {
                    mVelY=-GRAVITY/8;
                    isDashing=true;
                }
                break;
            }
            canJump=false;
        }
    }

    // Pad interactions
    for (auto &pad : jumpPads) {
        if (pad.checkCollision(mPosX, mPosY, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            if (pad.canTrigger()) {
                std::string type=pad.getType();
                switch (type[0]) {
                case 'J': // Yellow pad
                    if (!reverseGravity) mVelY=JUMP_VELOCITY*1.37;
                    else mVelY=-JUMP_VELOCITY*1.37;
                    break;
                case 'P': // Pink pad
                    if (!reverseGravity) mVelY=JUMP_VELOCITY;
                    else mVelY=-JUMP_VELOCITY;
                    break;
                case 'S': // Spider pad
                    findClosestRectSPad(pad, blocks, spikes);
                    mVelY=0;
                    break;
                }
                pad.markUsed();
            }
        }
        else {
            pad.resetUsed();
        }
    }

    // Spike collision
    for (auto &spike : spikes) {
        if (!levelFreeze) spike.movingSpike(deltaTime);
        if (spike.checkCollision(mPosX, mPosY, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            dead=true;
        }
    }

    for (auto &block : blocks) {
        if (!levelFreeze) {
            if (block.getType()=="1Y") block.movingBlockX(deltaTime);
            else block.movingBlockY(deltaTime);
        }
    }
}

// Render player to window
void Player::render() {
    SDL_FRect cube=getHitbox();
    cubeTexture.render(cube, nullptr, 0.0, nullptr, (reverseGravity ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE));
}


// Get player hitbox, for spike collision
SDL_FRect Player::getHitbox() {
    return {static_cast<float>(mPosX), static_cast<float>(mPosY), static_cast<float>(PLAYER_WIDTH), static_cast<float>(PLAYER_HEIGHT)};
}

// Get player hitbox, for spider pad interactions
SDL_FRect Player::getSPadHitbox() {
    return {static_cast<float>(mPosX)+TILE_SIZE*7/20, static_cast<float>(mPosY)+TILE_SIZE*7/20, static_cast<float>(PLAYER_WIDTH)*3/10, static_cast<float>(PLAYER_HEIGHT)*3/10};
}

// Get gravity status
bool Player::getGravity() {
    return reverseGravity;
}

// Just for idle tycoon
int Player::getGainPerHit() {
    return gainPerHit;
}
int Player::getPassiveIncome() {
    return passiveIncome;
}
unsigned long long Player::getTotalMoney() {
    return totalMoney;
}
