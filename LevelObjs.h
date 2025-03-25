#pragma once

#include <iostream>
#include <SDL.h>
#include <vector>

extern const int TILE_SIZE;

class Block {
public:
    Block(int x, int y, int w, int h);

    bool checkXCollision(double &playerX, double playerY, double &nextPlayerX,
                         double playerVelX, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    bool checkYCollision(int playerX, double &playerY, double &nextPlayerY,
                         double playerVelY, int PLAYER_WIDTH, int PLAYER_HEIGHT, bool &onPlatform) const;

    const SDL_Rect &getHitbox() const;

private:
    SDL_Rect hitbox;
};

class Spike {
public:
    Spike(int x, int y, int w, int h);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_Rect &getHitbox() const;

private:
    SDL_Rect hitbox;
};

class JumpOrb {
public:
    JumpOrb(int x, int y, int w, int h, char type);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_Rect &getHitbox() const;

    const int getType() const;

private:
    SDL_Rect hitbox;
    char orbType;
    bool orbUsed=false;
};

class JumpPad {
public:
    JumpPad(int x, int y, int w, int h, char type);

    bool checkCollision(double playerX, double playerY, int PLAYER_WIDTH, int PLAYER_HEIGHT) const;

    const SDL_Rect &getHitbox() const;

    const int getType() const;

    void markUsed();
    void resetUsed();

    bool canTrigger();

private:
    SDL_Rect hitbox;
    char padType;
    bool padUsed=false;
};
