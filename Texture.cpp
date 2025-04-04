#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "Texture.h"
using namespace std;

extern SDL_Renderer *gRenderer;
extern TTF_Font *gFont;

// Constructor
LTexture::LTexture() {
    mTexture=nullptr;
    mWidth=0;
    mHeight=0;
}

// Destructor
LTexture::~LTexture() {
    free();
}

// Load texture from image
bool LTexture::loadFromFile(string path) {
    free();
    SDL_Texture *newTexture=nullptr;
    SDL_Surface *loadedSurface=IMG_Load(path.c_str());
    if (loadedSurface==nullptr) {
        cout << "Unable to render image. " << IMG_GetError() << endl;
    }
    else {
        // SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0xFF, 0xFF, 0xFF));
        newTexture=SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture==nullptr) {
            cout << "Unable to create texture from image. " << SDL_GetError() << endl;
        }
        else {
            mWidth=loadedSurface->w;
            mHeight=loadedSurface->h;
        }
        SDL_FreeSurface(loadedSurface);
    }
    mTexture=newTexture;
    return mTexture!=nullptr;
}

// Load texture from text
bool LTexture::loadFromRenderedText(string textureText, SDL_Color textColor) {
    free();
    SDL_Surface *textSurface=TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
    if (textSurface==nullptr) {
        cout << "Unable to render text surface. " << TTF_GetError() << endl;
    }
    else {
        mTexture=SDL_CreateTextureFromSurface(gRenderer, textSurface);
        if (mTexture==nullptr) {
            cout << "Unable to create texture from rendered text. " << SDL_GetError() << endl;
        }
        else {
            mWidth=textSurface->w;
            mHeight=textSurface->h;
        }
        SDL_FreeSurface(textSurface);
    }
    return mTexture!=nullptr;
}

// Destroy texture
void LTexture::free() {
    if (mTexture!=nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture=nullptr;
        mWidth=0;
        mHeight=0;
    }
}

// Set color
void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue) {
    SDL_SetTextureColorMod(mTexture, red, green, blue);
}

// Set blend mode
void LTexture::setBlendMode(SDL_BlendMode blending) {
    SDL_SetTextureBlendMode(mTexture, blending);
}

// Set alpha
void LTexture::setAlpha(Uint8 alpha) {
    SDL_SetTextureAlphaMod(mTexture, alpha);
}

// Render texture
void LTexture::render(SDL_FRect renderQuad, SDL_Rect *clip, double angle, SDL_FPoint *center, SDL_RendererFlip flip) {
    if (clip!=nullptr) {
        float scaleX=renderQuad.w/clip->w;
        float scaleY=renderQuad.h/clip->h;

        renderQuad.w=clip->w*scaleX;
        renderQuad.h=clip->h*scaleY;
    }
    SDL_RenderCopyExF(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

// Get texture width + height
int LTexture::getWidth() {
    return mWidth;
}
int LTexture::getHeight() {
    return mHeight;
}
