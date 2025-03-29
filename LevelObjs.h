#pragma once

#include <iostream>
#include <SDL.h>
#include <vector>

extern const int TILE_SIZE;

class Block {
public:
    Block(int x, int y, int w, int h, double a, const std::string &type);

    bool checkXCollision(double &playerX, double playerY, double &nextPlayerX,
                         double playerVelX, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    bool checkYCollision(int playerX, double &playerY, double &nextPlayerY,
                         double playerVelY, int PLAYER_WIDTH, int PLAYER_HEIGHT, bool &onPlatform) const;

    const SDL_Rect &getHitbox() const;
    const std::string &getType() const;

    double angle;

private:
    SDL_Rect hitbox;
    std::string blockType;
};

class Spike {
public:
    Spike(int x, int y, int w, int h, double a, SDL_RendererFlip m, const std::string &type);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_Rect &getHitbox() const;
    const std::string &getType() const;

    double angle;
    SDL_RendererFlip mirror;

private:
    SDL_Rect hitbox;
    std::string spikeType;
};

class JumpOrb {
public:
    JumpOrb(int x, int y, int w, int h, char type);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_Rect &getHitbox() const;

    const int getType() const;

    mutable double rotationAngle=0.0;
    void updateRotation(double deltaTime) const;

private:
    SDL_Rect hitbox;
    char orbType;
};

class JumpPad {
public:
    JumpPad(int x, int y, int w, int h, char type);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_Rect &getHitbox() const;

    const int getType() const;

    // Only trigger pad once
    void markUsed();
    void resetUsed();
    bool canTrigger();

private:
    SDL_Rect hitbox;
    char padType;
    bool padUsed=false;
};
