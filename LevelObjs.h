#pragma once

#include <iostream>
#include <SDL.h>
#include <vector>
#include "Enums.h"

extern const int TILE_SIZE;

class Block {
public:
    Block(float x, float y, float w, float h, double a, const std::string &type);

    bool checkXCollision(double &playerX, double playerY, double &nextPlayerX,
                         double playerVelX, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    bool checkYCollision(double playerX, double &playerY, double &nextPlayerY,
                         double playerVelY, int PLAYER_WIDTH, int PLAYER_HEIGHT,
                         bool &onPlatform, bool &hitCeiling, bool reverseGravity) const;

    const SDL_FRect &getHitbox() const;
    const std::string &getType() const;
    void changePosition(float x, float y);

    bool isInteractable() const;
    void interact(unsigned long long &totalMoney, int &gainPerHit, int &passiveIncome, GameStatus &currentStatus, std::vector<Block> &blocks);

    double angle;

    int counter=0;
    int value=5;

private:
    SDL_FRect hitbox;
    std::string blockType;
    int increment=5;
};

class Spike {
public:
    Spike(float x, float y, float w, float h, double a, SDL_RendererFlip m, const std::string &type);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_FRect &getHitbox() const;
    const std::string &getType() const;

    double angle;
    SDL_RendererFlip mirror;

private:
    SDL_FRect hitbox;
    std::string spikeType;
};

class JumpOrb {
public:
    JumpOrb(float x, float y, float w, float h, char type);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_FRect &getHitbox() const;

    const int getType() const;

    mutable double rotationAngle=0.0;
    void updateRotation(double deltaTime) const;

private:
    SDL_FRect hitbox;
    char orbType;
};

class JumpPad {
public:
    JumpPad(float x, float y, float w, float h, char type);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_FRect &getHitbox() const;

    const int getType() const;

    // Only trigger pad once
    void markUsed();
    void resetUsed();
    bool canTrigger();

private:
    SDL_FRect hitbox;
    char padType;
    bool padUsed=false;
};
