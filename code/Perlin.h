#ifndef PERLIN_H
#define PERLIN_H
#include <cmath>
#include <vector>

// A class for generating Perlin noise
class Perlin {
    public:
        // Constructor
        Perlin();

        // Methods
        double fade(double t);
        int increment(int val, int repeat);
        double gradient(int hash, double x, double y, double z);
        double lerp(double a, double b, double x);
        double generatePerlinNoise(double x, double y, double z, int repeat);
        double generatePerlinOctaves(double x, double y, double z, int octaves, double persistence);

    private:
        // Instance variables
        int permutation[256];
        int p[512];
};
#endif