#include <string>
#include <vector>
#include <SDL.h>
#include "Texture.h"
#include "LevelObjs.h"
#include "LoadLevel.h"

extern SDL_Renderer *gRenderer;

extern LTexture blockSheetTexture;
extern LTexture orbPadSheetTexture;

extern SDL_Rect blockClips[];
extern SDL_Rect spikeClips[];
extern SDL_Rect orbClips[];
extern SDL_Rect padClips[];

void renderLevel(const std::vector<Block> &blocks, const std::vector<PushableBlock> &pushableBlocks, const std::vector<Spike> &spikes,
                 const std::vector<JumpOrb> &jumpOrbs, const std::vector<JumpPad> &jumpPads, double deltaTime) {
    // Render orbs
    for (const auto &orb : jumpOrbs) {
        SDL_FRect renderOrb={orb.getHitbox().x+TILE_SIZE/10, orb.getHitbox().y+TILE_SIZE/10, TILE_SIZE, TILE_SIZE};
        switch (orb.getType()) {
        case 'Y': // Yellow
            orbPadSheetTexture.render(renderOrb, &orbClips[0], 0, nullptr, SDL_FLIP_NONE);
            break;
        case 'B': // Blue
            orbPadSheetTexture.render(renderOrb, &orbClips[1], 0, nullptr, SDL_FLIP_NONE);
            break;
        case 'G': // Green
            orb.updateRotation(deltaTime);
            orbPadSheetTexture.render(renderOrb, &orbClips[2], orb.rotationAngle, nullptr, SDL_FLIP_NONE);
            break;
        case 'D': // Dash
            orbPadSheetTexture.render(renderOrb, &orbClips[3], 0, nullptr, SDL_FLIP_NONE);
            break;
        }
    }

    // Render pads
    for (const auto &pad : jumpPads) {
        std::string type=pad.getType();
        if (jumpPadLookup.find(type)!=jumpPadLookup.end()) {
            JumpPadInfo info=jumpPadLookup[type];
            SDL_FRect renderPad;
            if (type[0]=='J' || type[0]=='P') {
                if (info.rotation==0) {
                    renderPad={pad.getHitbox().x-TILE_SIZE/12, pad.getHitbox().y-TILE_SIZE*13/15, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==180) {
                    renderPad={pad.getHitbox().x-TILE_SIZE/12, pad.getHitbox().y+TILE_SIZE/30, TILE_SIZE, TILE_SIZE};
                }
            }
            else if (type[0]=='S') { // Spider
                if (info.rotation==0) {
                    renderPad={pad.getHitbox().x-TILE_SIZE/30, pad.getHitbox().y-TILE_SIZE*3/4, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==90) {
                    renderPad={pad.getHitbox().x+TILE_SIZE*3/20, pad.getHitbox().y-TILE_SIZE/30, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==180) {
                    renderPad={pad.getHitbox().x-TILE_SIZE/30, pad.getHitbox().y+TILE_SIZE*3/20, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==270) {
                    renderPad={pad.getHitbox().x-TILE_SIZE*3/4, pad.getHitbox().y-TILE_SIZE/30, TILE_SIZE, TILE_SIZE};
                }
            }
            orbPadSheetTexture.render(renderPad, &padClips[info.clipIndex], info.rotation, nullptr, SDL_FLIP_NONE);
        }
    }

    // Render spikes
    for (const auto &spike : spikes) {
        std::string type=spike.getType();
        if (spikeLookup.find(type)!=spikeLookup.end()) {
            SpikeInfo info=spikeLookup[type];
            SDL_FRect renderSpike;
            if (type[1]=='A' || type[1]=='C') { // Small spike
                if (info.rotation==0) {
                    renderSpike={spike.getHitbox().x-TILE_SIZE*2/5, spike.getHitbox().y-TILE_SIZE*7/10, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==90) {
                    renderSpike={spike.getHitbox().x-TILE_SIZE/10, spike.getHitbox().y-TILE_SIZE*2/5, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==180) {
                    renderSpike={spike.getHitbox().x-TILE_SIZE*2/5, spike.getHitbox().y-TILE_SIZE/10, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==270) {
                    renderSpike={spike.getHitbox().x-TILE_SIZE*7/10, spike.getHitbox().y-TILE_SIZE*2/5, TILE_SIZE, TILE_SIZE};
                }
            }
            else if (type[1]=='E') { // Big spike
                if (info.rotation==0 || info.rotation==180) {
                    renderSpike={spike.getHitbox().x-TILE_SIZE*2/5, spike.getHitbox().y-TILE_SIZE*3/10, TILE_SIZE, TILE_SIZE};
                }
                else if (info.rotation==90 || info.rotation==270) {
                    renderSpike={spike.getHitbox().x-TILE_SIZE*3/10, spike.getHitbox().y-TILE_SIZE*2/5, TILE_SIZE, TILE_SIZE};
                }
            }
            blockSheetTexture.render(renderSpike, &spikeClips[info.clipIndex], info.rotation, nullptr, info.mirrored);
        }
    }

    // Render platforms (blocks)
    for (const auto &block : blocks) {
        std::string type=block.getType();
        if (blockLookup.find(type)!=blockLookup.end()) {
            BlockInfo info=blockLookup[type];
            SDL_FRect renderBlock=block.getHitbox();
            blockSheetTexture.render(renderBlock, &blockClips[info.clipIndex], info.rotation, nullptr, info.mirrored);
            if (type=="1BG") {
                SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 160);
                SDL_RenderFillRectF(gRenderer, &block.getHitbox());
            }
            if (type=="1BO") {
                SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(gRenderer, 255, 102, 0, 160);
                SDL_RenderFillRectF(gRenderer, &block.getHitbox());
            }
        }
    }

    for (const auto &block : pushableBlocks) {
        SDL_FRect renderBlock=block.getHitbox();
        blockSheetTexture.render(renderBlock, &blockClips[17], 0, nullptr, SDL_FLIP_NONE);
    }
}
