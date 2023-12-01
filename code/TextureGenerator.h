#ifndef TEXTUREGENERATOR_H
#define TEXTUREGENERATOR_H
#include "Perlin.h"

// A class for generating textures
class TextureGenerator {
    public:
        // Constructor
        TextureGenerator() {}

        // Methods
        unsigned char* generatePerlinTexture(const int textureWidth, const int textureHeight, const int octaves);
        unsigned char* generateSolidTexture(const int textureWidth, const int textureHeight, const float r, const float g, const float b);
};

#endif