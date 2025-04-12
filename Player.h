#pragma once

#include <vector>
#include <SDL.h>
#include "LevelObjs.h"

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern const int TILE_SIZE;

class Player {
public:
    // Player width
    const int PLAYER_WIDTH=TILE_SIZE;

    // Player height
    const int PLAYER_HEIGHT=TILE_SIZE;

    // Horizontal velocity
    static constexpr double X_VELOCITY=490.0;

    // Initial velocity when jump
    static constexpr double JUMP_VELOCITY=-1390.0;

    // Gravity
    static constexpr double GRAVITY=6000.0;

    // Fall speed limit
    static constexpr double TERMINAL_VELOCITY=4000.0;

    // Constructor
    Player();

    // Reset player status
    void reset();

    // Handle mouse + keyboard events
    void handleEvent(SDL_Event &e);

    // Allow player to enter 1-block-wide gap
    void forcePushIntoGap(std::vector<Block> &blocks);

    // Move player, platform physics included, deltaTime for consistent physics
    void move(std::vector<Block> &blocks, std::vector<JumpOrb> &jumpOrbs, double deltaTime);

    // Helper function for spider pad interactions
    void findClosestRectSPad(std::vector<Block> &blocks, std::vector<Spike> &spikes);

    // Jump orb and jump pad interactions
    void interact(std::vector<Block> &blocks, std::vector<Spike> &spikes,
                  std::vector<JumpOrb> &jumpOrbs, std::vector<JumpPad> &jumpPads, bool &quit);

    // Render player to window
    void render();

    // Get player hitbox, for spike collision
    SDL_FRect getHitbox();

    // Get player hitbox, for spider pad interactions
    SDL_FRect getSPadHitbox();

    // Get gravity status
    bool getGravity();

private:
    // Player X/Y positions
    double mPosX, mPosY;

    // Player X/Y velocities
    double mVelX, mVelY;

    // Check if key is being held, only allow jump once
    bool isJumpHeld, canJump;

    // Check if player is moving left or right
    bool moveLeft, moveRight;

    // Check if player is on a platform or hitting the ceiling
    bool onPlatform, hitCeiling;

    // Check if gravity is reversed
    bool reverseGravity;

    // Coyote time, allowing player to jump just after leaving platform
    double coyoteTimer;
    static constexpr double COYOTE_TIME=0.03;
};
