#pragma once

#include <string>
#include <SDL.h>

class LTexture {
public:
    // Constructor
    LTexture();

    // Destructor
    ~LTexture();

    // Load texture from image
    bool loadFromFile(std::string path);

    // Load texture from text
    bool loadFromRenderedText(std::string textureText, SDL_Color textColor);

    // Destroy texture
    void free();

    // Set color
    void setColor(Uint8 red, Uint8 green, Uint8 blue);

    // Set blend mode
    void setBlendMode(SDL_BlendMode blending);

    // Set alpha
    void setAlpha(Uint8 alpha);

    // Render texture
    void render(SDL_Rect renderQuad, SDL_Rect *clip=nullptr, double angle=0.0, SDL_Point *center=nullptr, SDL_RendererFlip flip=SDL_FLIP_NONE);

    // Get texture width + height
    int getWidth();
    int getHeight();

private:
    // Texture
    SDL_Texture *mTexture;

    // Texture width + height
    int mWidth;
    int mHeight;
};
