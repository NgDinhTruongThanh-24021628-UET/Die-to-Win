#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "Texture.h"
using namespace std;

extern SDL_Renderer *gRenderer;

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
bool LTexture::loadFromRenderedText(string textureText, SDL_Color textColor, TTF_Font *font) {
    free();
    SDL_Surface *textSurface=TTF_RenderText_Solid(font, textureText.c_str(), textColor);
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
void LTexture::setColor(SDL_Color color) {
    SDL_SetTextureColorMod(mTexture, color.r, color.g, color.b);
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
    SDL_RenderCopyExF(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}
void LTexture::render(float x, float y, SDL_Rect *clip, double angle, SDL_FPoint *center, SDL_RendererFlip flip) {
    SDL_FRect renderQuad={x, y, mWidth, mHeight};
    SDL_RenderCopyExF(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

// Get texture width + height
float LTexture::getWidth() {
    return mWidth;
}
float LTexture::getHeight() {
    return mHeight;
}
