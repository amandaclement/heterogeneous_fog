#include "TextureGenerator.h"

// Generates the Perlin noise texture a Perlin noise texture based on specified # of octaves
unsigned char* TextureGenerator::generatePerlinTexture(const int textureWidth, const int textureHeight, const int octaves) {
    // To access Perlin noise methods
    Perlin* perlin = new Perlin;
    
    // Denotes the degree by which the amplitude changes between octaves
    double persistence = 1;  

    // Pointer to the texture data (texture size * 4 for alpha channel, in case we want transparency)
    unsigned char* textureData = new unsigned char[textureWidth * textureHeight * 4]; 

    // For incrementing textureData index
    int count = 0; 

    // Iterate over each pixel in texture map, generating a noise value for each point 
    for (int i = 0; i < textureWidth; i++) {
        for (int j = 0; j < textureHeight; j++) {
            // Compute x and y values of the point
            double x = (double)j / textureWidth * octaves;
            double y = (double)i / textureHeight * octaves;

            // Compute the Perlin turbulence value for this point
            // double noiseValue = perlin.generatePerlinTurbulence(x, y, 0.0, octaves);
            double noiseValue = perlin->generatePerlinNoise(x, y, 0.0, octaves);

            // Store the color of this pixel based on the noise value in the textureData array
            int colorValue = (int)(noiseValue * 255);

            textureData[count] = colorValue;
            textureData[count + 1] = colorValue;
            textureData[count + 2] = colorValue;
            textureData[count + 3] = 0.0;

            // Increment by 4 to account for r,g,b,a values
            count = count + 4;
        }
    }
    // Return the textureData array
    return textureData;   
}

// Generates a solid-colored texture
unsigned char* TextureGenerator::generateSolidTexture(const int textureWidth, const int textureHeight, const float r, const float g, const float b) {
    // Pointer to the texture data
    unsigned char* textureData = new unsigned char[textureWidth * textureHeight * 3]; 

    // For incrementing textureData index
    int count = 0;          

    // Iterate over each point in texture map, applying appropriate color values
    for (int i = 0; i < textureWidth; i++) {
        for (int j = 0; j < textureHeight; j++) {

            textureData[count] = r;
            textureData[count + 1] = g;
            textureData[count + 2] = b;

            // Increment by 3 to account for r,g,b values
            count = count + 3;
        }
    }   
    // Return the textureData array
    return textureData;
}