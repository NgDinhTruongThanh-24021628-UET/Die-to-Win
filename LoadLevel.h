#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "LevelObjs.h"

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

// Split the level into tiles to place objects
extern const float TILE_SIZE;
extern const int LEVEL_WIDTH;
extern const int LEVEL_HEIGHT;

// Vector to store objects
extern std::vector<Block> blocks;
extern std::vector<Spike> spikes;
extern std::vector<JumpOrb> jumpOrbs;
extern std::vector<JumpPad> jumpPads;
extern std::vector<PushableBlock> pushableBlocks;

// Identify block type
struct BlockInfo {
    int clipIndex;
    double rotation;
    SDL_RendererFlip mirrored;
};
extern std::unordered_map<std::string, BlockInfo> blockLookup;

// Identify spike type
struct SpikeInfo {
    int clipIndex;
    double rotation;
    SDL_RendererFlip mirrored;
};
extern std::unordered_map<std::string, SpikeInfo> spikeLookup;

// Identify jump orb type
struct JumpOrbInfo {
    int clipIndex;
    float offsetX;
    float offsetY;
};
extern std::unordered_map<std::string, JumpOrbInfo> jumpOrbLookup;

// Identify jump pad type
struct JumpPadInfo {
    int clipIndex;
    double rotation;
};
extern std::unordered_map<std::string, JumpPadInfo> jumpPadLookup;

// Load level from a file
void loadLevel(const std::string &path, std::vector<Block> &blocks, std::vector<PushableBlock> &pushableBlocks,
               std::vector<Spike> &spikes, std::vector<JumpOrb> &jumpOrbs, std::vector<JumpPad> &jumpPads);

