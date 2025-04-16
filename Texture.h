#pragma once

#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

class LTexture {
public:
    // Constructor
    LTexture();

    // Destructor
    ~LTexture();

    // Load texture from image
    bool loadFromFile(std::string path);

    // Load texture from text
    bool loadFromRenderedText(std::string textureText, SDL_Color textColor, TTF_Font *font);

    // Destroy texture
    void free();

    // Set color
    void setColor(SDL_Color color);

    // Set blend mode
    void setBlendMode(SDL_BlendMode blending);

    // Set alpha
    void setAlpha(Uint8 alpha);

    // Render texture
    void render(SDL_FRect renderQuad, SDL_Rect *clip=nullptr, double angle=0.0, SDL_FPoint *center=nullptr, SDL_RendererFlip flip=SDL_FLIP_NONE);
    void render(float x, float y, SDL_Rect *clip=nullptr, double angle=0.0, SDL_FPoint *center=nullptr, SDL_RendererFlip flip=SDL_FLIP_NONE);

    // Get texture width + height
    float getWidth();
    float getHeight();

private:
    // Texture
    SDL_Texture *mTexture;

    // Texture width + height
    float mWidth;
    float mHeight;
};
