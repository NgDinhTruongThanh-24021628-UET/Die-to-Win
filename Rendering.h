#pragma once

#include <vector>
extern const float TILE_SIZE;

void renderLevel(const std::vector<Block> &blocks, const std::vector<PushableBlock> &pushableBlocks, const std::vector<Spike> &spikes,
                 const std::vector<JumpOrb> &jumpOrbs, const std::vector<JumpPad> &jumpPads, double deltaTime);
