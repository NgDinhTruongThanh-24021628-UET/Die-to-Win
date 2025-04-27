#pragma once

#include <iostream>
#include <SDL.h>
#include <vector>
#include "Enums.h"

extern const float TILE_SIZE;

class Block;
class PushableBlock;
class Spike;
class JumpOrb;
class JumpPad;

extern bool uniqueDigitsInPassword;
extern bool botWins;
extern bool playerWins;
extern bool stalemate;

class Block {
public:
    // Constructor
    Block(float x, float y, float w, float h, double a, SDL_RendererFlip m, const std::string &type);

    // Collision detection
    bool checkXCollision(double &playerX, double playerY, double &nextPlayerX,
                         double playerVelX, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    bool checkYCollision(double playerX, double &playerY, double &nextPlayerY,
                         double playerVelY, int PLAYER_WIDTH, int PLAYER_HEIGHT,
                         bool &onPlatform, bool &hitCeiling, bool reverseGravity) const;

    // Get block hitbox
    const SDL_FRect &getHitbox() const;

    // Get + change block type
    const std::string &getType() const;
    void switchType(std::string newType);

    // Functions to change block's position
    void movingBlockX(double deltaTime);
    void movingBlockY(double deltaTime);
    void changeSpeed(float change);
    void offsetPosition(float offsetX, float offsetY);

    // Interactable blocks
    bool isInteractable() const;
    void interact(unsigned long long &totalMoney, int &gainPerHit, int &passiveIncome, GameStatus &currentStatus,
                  std::vector<Block> &blocks, std::vector<PushableBlock> &pushableBlocks, std::vector<Spike> &spikes,
                  const std::string &levelName, double deltaTime, bool &timeStopped, double &timeStopTimer, int &powerPercent, bool &cutscenePlaying);

    // Helper functions for each level
    void interactClicker(unsigned long long &totalMoney, int &gainPerHit, int &passiveIncome,
                         std::vector<Block> &blocks, std::vector<Spike> &spikes, double deltaTime);
    void interactEnigma(std::vector<Block> &blocks, std::vector<Spike> &spikes);
    void interactMoveToDie(std::vector<Block> &blocks, std::vector<PushableBlock> &pushableBlocks, bool &timeStopped, double &timeStopTimer);
    void interactFiveNights(std::vector<Block> &blocks, int &powerPercent);
    void interactTicTacToe(std::vector<Block> &blocks, std::vector<Spike> &spikes);
    void interactJojo(std::vector<Block> &blocks, std::vector<Spike> &spikes, bool &cutscenePlaying);

    // For rendering blocks
    double angle;
    SDL_RendererFlip mirror;

    // For moving blocks
    bool unlocked=false;
    float realX, realY;
    float speed=300.0f;

    // Internal values
    int counter=0;
    int value=5;
    int increment=5;

private:
    SDL_FRect hitbox;
    std::string blockType;
};

class PushableBlock {
public:
    // Constructor
    PushableBlock(float x, float y, float w, float h);

    // Update pushable block every frame
    void update(std::vector<Block> &platformBlocks, const SDL_FRect &playerHitbox,
                bool moveLeft, bool moveRight, bool &dead, double deltaTime);

    // Functions in update() : apply gravity, check if being pushed, check if falling on player
    void applyPhysics(std::vector<Block> &platformBlocks, double deltaTime);
    void checkPush(std::vector<Block> &platformBlocks, const SDL_FRect &playerHitbox, bool moveLeft, bool moveRight, double deltaTime);
    void checkKill(const SDL_FRect &playerHitbox, bool &dead);

    // Reset position
    void resetPosition();

    // Get block hitbox
    SDL_FRect getHitbox() const;

    // Check Y collision (only for landing on block)
    bool checkYCollision(double playerX, double &playerY, double &nextPlayerY,
                         double playerVelY, int PLAYER_WIDTH, int PLAYER_HEIGHT, bool &onPlatform);

    // Block physics
    double velX=0.0;
    double velY=0.0;
    double GRAVITY=6000.0;
    double TERMINAL_VELOCITY=5000.0;
    double PUSH_SPEED=300.0;
    bool grounded=false;
    bool touchingLeft, touchingRight;

    // For time stop level
    bool resetQueued=false;

    // Save original position
    float originalX, originalY;

private:
    SDL_FRect hitbox;
};

class Spike {
public:
    Spike(float x, float y, float w, float h, double a, SDL_RendererFlip m, const std::string &type);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_FRect &getHitbox() const;
    const std::string &getType() const;

    void movingSpike(double deltaTime);
    void changeSpeed(float change);

    double angle;
    SDL_RendererFlip mirror;

    bool unlocked=false;
    float realX, realY;
    float speed=300.0f;

private:
    SDL_FRect hitbox;
    std::string spikeType;
};

class JumpOrb {
public:
    JumpOrb(float x, float y, float w, float h, char type);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_FRect &getHitbox() const;

    const char getType() const;

    mutable double rotationAngle=0.0;
    void updateRotation(double deltaTime) const;

private:
    SDL_FRect hitbox;
    char orbType;
};

class JumpPad {
public:
    JumpPad(float x, float y, float w, float h, double a, const std::string &type);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_FRect &getHitbox() const;
    const std::string &getType() const;

    // Only trigger pad once
    void markUsed();
    void resetUsed();
    bool canTrigger();

    double angle;

private:
    SDL_FRect hitbox;
    std::string padType;
    bool padUsed=false;
};
