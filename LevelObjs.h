#pragma once

#include <iostream>
#include <SDL.h>
#include <vector>
#include "Enums.h"

extern const int TILE_SIZE;

class Block;
class Spike;
class JumpOrb;
class JumpPad;

class Block {
public:
    Block(float x, float y, float w, float h, double a, SDL_RendererFlip m, const std::string &type);

    bool checkXCollision(double &playerX, double playerY, double &nextPlayerX,
                         double playerVelX, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    bool checkYCollision(double playerX, double &playerY, double &nextPlayerY,
                         double playerVelY, int PLAYER_WIDTH, int PLAYER_HEIGHT,
                         bool &onPlatform, bool &hitCeiling, bool reverseGravity) const;

    const SDL_FRect &getHitbox() const;
    const std::string &getType() const;
    void movingBlock(double deltaTime);
    void offsetPosition(float offsetX, float offsetY);

    bool isInteractable() const;
    void interact(unsigned long long &totalMoney, int &gainPerHit, int &passiveIncome, GameStatus &currentStatus,
                  std::vector<Block> &blocks, std::vector<Spike> &spikes, const std::string &levelName, double deltaTime);
    void interactClicker(unsigned long long &totalMoney, int &gainPerHit, int &passiveIncome,
                         std::vector<Block> &blocks, std::vector<Spike> &spikes, double deltaTime);
    void interactEnigma(std::vector<Block> &blocks, std::vector<Spike> &spikes, double deltaTime);
    void interactTicTacToe();

    double angle;
    SDL_RendererFlip mirror;

    bool unlocked=false;
    float realX, realY;
    float speed=300.0f;

    int counter=0;
    int value=5;
    int increment=5;

private:
    SDL_FRect hitbox;
    std::string blockType;
};

class Spike {
public:
    Spike(float x, float y, float w, float h, double a, SDL_RendererFlip m, const std::string &type);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_FRect &getHitbox() const;
    const std::string &getType() const;
    void movingSpike(double deltaTime);

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
