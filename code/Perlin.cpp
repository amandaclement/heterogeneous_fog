#include "Perlin.h"

// Constructor
Perlin::Perlin() {
    // Permutation table, as defined by Ken Perlin
    // It's an array of random values from 0 - 255 inclusive, will be used in a hash function to determine gradient vector
    int permutation[512] = {
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,190,
        6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,
        171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
        55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,
        188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,
        206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,
        43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,34,242,193,238,
        210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,204,176,
        115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };

    // Doubling permutation array to 512 to avoid having to perform modulo arithmetic when indexing the array
    // p[] will then be used in a hash function to determine gradient vector
    for (int i = 0; i < 512; i++)
        p[i] = permutation[i%256];
}

// Fade function, defined by Ken Perlin
// Used to create smooth transitions between the gradients of noise
// Parameter: t is a value between 0 and 1, and we return a smoothed version (curve) of that value that gradually steepens
double Perlin::fade(double t) {
    // t gets smoothened by gradually increasing the rate at which it's raised to a power
    return 6*t*t*t*t*t - 15*t*t*t*t + 10*t*t*t; // 6t^5 - 15t^4 + 10t^3
}

// Increment function 
// Helper function used to ensure that the noise function generates same value for a given coordinate regardless of its position within the tile, which helps to maintain a level of predictability to make the noise more natural-looking
// Parameters: val is the value we want to make sure falls within the repeating interval; repeat is # times the noise function is repeated in each dimension before wrapping around (tiling)
int Perlin::increment(int val, int repeat) {
    val++;

    // If repeat <= 0, the noise function doesn't repeat so there's no tiling to be done in that dimension hence we skip wrapping calculations
    // But if repeat > 0, there is tiling so need to ensure to wrap value back around if val goes beyond the range of repeat
    if (repeat > 0) 
        val %= repeat;
    
    return val;
}

// Gradient function
// Used for generating a gradient vector for calculating the noise value at a specific point in 3D space
// This gradient vector determines the direction and magnitude of the change in noise
// Parameters: hash is used to randomly select one of 16 possibile gradient vectors (each corresponding to a different edge of the cube), xyz are the 3D point's coordinates
double Perlin::gradient(int hash, double x, double y, double z) {
    // Selection of the gradient vector is based on hash value
    // Value of hash is always between 0 and 15 to allow the selection of one of the 16 gradient vectors
    switch(hash % 16)
    {
        case 0: return  x + y;
        case 1: return -x + y;
        case 2: return  x - y;  
        case 3: return -x - y;  
        case 4: return  x + z; 
        case 5: return -x + z;
        case 6: return  x - z;
        case 7: return -x - z;
        case 8: return  y + z;
        case 9: return -y + z;
        case 10: return  y - z; 
        case 11: return -y - z;
        case 12: return  y + x;
        case 13: return -y + z;
        case 14: return  y - x;
        case 15: return -y - z;
        default: return 0;       // Impossible case

        // Example: if we compare a gradient vector A that points in positive x-direction with large magnitude to gradient vector B that points in the negative x-direction with small magnitude, 
        // we'd see more change in A's noise value at a given point in that direction compared to B
    }    
}

// Lerp function
// Performs linear interpolation
// Parameters: a and b are the two values we want to return the linear interpolation (weighted average) for; x is the interpolation factor
double Perlin::lerp(double a, double b, double x) {
    return a + x * (b - a);
}

// Perlin noise function
// Returns the final color value of the 3D point, after going through Perlin noise calculations
// Parameters: xyz are coordinates of point in 3D space; repeat is # times the noise function is repeated in each dimension before wrapping around (tiling)
double Perlin::generatePerlinNoise(double x, double y, double z, int repeat) {
    // If repeat <= 0, we don't repeat the noise function, and if repeat > 0, apply the repeat value to the point coordinates
    // Take the mod of each coordinate with the repeat value to wrap the noise function in each dimension
    if (repeat > 0) {
        x = fmod(x,repeat);  // x ranges from [0,repeat)
        y = fmod(y,repeat);  // y ranges from [0,repeat)
        z = fmod(z,repeat);  // z ranges from [0,repeat)
    }

    // Calculating the unit cube the the x,y,z point will be located in

    // Compute int part of point coordinates, which are used for permutation table lookup, so we map the coordinates 
    // to the range [0,255] to match the permutation table range which can be achieved by taking coordinate mod 256
    int xi = (int)fmod(x,256.0);  // xi ranges from [0,1]
    int yi = (int)fmod(y,256.0);  // yi ranges from [0,1]
    int zi = (int)fmod(z,256.0);  // zi ranges from [0,1]

    // Compute decimal part of point coordinates, which are used for interpolation and should be in the range [0,1]
    double xd = x-(int)x;  // xd ranges from [0,1]
    double yd = y-(int)y;  // yd ranges from [0,1]
    double zd = z-(int)z;  // zd ranges from [0,1]

    // Example: point(1.8, 2.6, 3.4)
    // Integer parts -> 1,2,3   so we look up indices 1, 2 and 3 in permutation table
    // Fractional parts -> 0.8, 0.6, 0.4   so we use these for interpolation

    // Use these decimal parts of point coordinates to interpolate between values, essentially to smoothly blend between values on the grid
    // These values are used for interpolation
    double xf = fade(xd);  // xd ranges from [0,1] so passing xd to fade() creates a value xf that varies smoothly still ranging from [0,1]
    double yf = fade(yd);  // yd ranges from [0,1] so passing yd to fade() creates a value yf that varies smoothly still ranging from [0,1]
    double zf = fade(zd);  // zd ranges from [0,1] so passing zd to fade() creates a value zf that varies smoothly still ranging from [0,1]

    // Hash function for Perlin Noise that uses permutation (p[]) table
    // The following lines select random gradient vectors for each of the 8 corner points of a cube in 3D space
    // These 8 corner points form the unit cube that the point of interest is located in
    // For each of the 8 corners of the unit cube, 3 ints are computed based on the point coordinates, which are used to index into p[] to get an int value that maps to a gradient vector
    int x0y0z0 = p[p[p[xi]+yi]+zi];
    int x0y1z0 = p[p[p[xi]+increment(yi,repeat)]+zi];
    int x0y0z1 = p[p[p[xi]+yi]+increment(zi,repeat)];
    int x0y1z1 = p[p[p[xi]+increment(yi,repeat)]+increment(zi,repeat)];
    int x1y0z0 = p[p[p[increment(xi,repeat)]+yi]+zi];
    int x1y1z0 = p[p[p[increment(xi,repeat)]+increment(yi,repeat)]+zi];
    int x1y0z1 = p[p[p[increment(xi,repeat)]+yi]+increment(zi,repeat)];
    int x1y1z1 = p[p[p[increment(xi,repeat)]+increment(yi,repeat)]+increment(zi,repeat)];

    // Example: int x0y0z0 = p[p[p[xi]+yi]+zi]
    // 1. We get int value at this index xi in p[]
    // 2. Add yi to the int value from step 1, and then get the int value at this new index in p[]
    // 3. Add zi to the int value from step 2, and then get the int value at this new index in p[]
    // Resulting int is a value that maps to a gradient vector in 3D space that' randomly chosen from the permutation table

    // Use linear interpolation to blend together the gradient values for the 8 corners of the cube surrounding the input point to generate a smooth value for that point
    // We're essentially measuring how much the gradient vector and the input vector point in the same direction, and use this to calculate a weighted contribution for the final noise value

    // x1 = linear interpolation between gradient vectors at corners (xd,yd,zd) and (xd-1,yd,zd) to the final value at the point being evaluated. The contribution is linearly interpolated using xf as the weight, so it ranges from value at (xd,yd,zd) when xf = 0 to value at (xd-1,yd,zd) when xf = 1
    double x1 = lerp(gradient(x0y0z0,xd,yd,zd), gradient(x1y0z0,xd-1,yd,zd), xf);
    // x2 = linear interpolation between gradient vectors at corners (xd,yd-1,zd) and (xd-1,yd-1,zd) to the final value at the point being evaluated. The contribution is linearly interpolated using xf as the weight, so it ranges from value at (xd,yd-1,zd) when xf = 0 to value at (xd-1,yd-1,zd) when xf = 1
    double x2 = lerp(gradient(x0y1z0,xd,yd-1,zd), gradient(x1y1z0,xd-1,yd-1,zd), xf);
    // y1 = interpolates between x1 and y1 based on weight yf. Resulting value ranges from x1 when yf = 0 to y1 when yf = 1
    double y1 = lerp(x1,x2,yf);   
    // x1 = linear interpolation between gradient vectors at corners (xd,yd,zd-1) and (xd-1,yd,zd-1) to the final value at the point being evaluated. The contribution is linearly interpolated using xf as the weight, so it ranges from value at (xd,yd,zd-1) when xf = 0 to value at (xd-1,yd,zd-1) when xf = 1                                                   
    x1 = lerp(gradient(x0y0z1,xd,yd,zd-1), gradient(x1y0z1,xd-1,yd,zd-1), xf);
    // x2 = linear interpolation between gradient vectors at corners (xd,yd-1,zd-1) and (xd-1,yd-1,zd-1) to the final value at the point being evaluated. The contribution is linearly interpolated using xf as the weight, so it ranges from value at (xd,yd-1,zd-1) when xf = 0 to value at (xd-1,yd-1,zd-1) when xf = 1  
    x2 = lerp(gradient(x0y1z1,xd,yd-1,zd-1), gradient(x1y1z1,xd-1,yd-1,zd-1), xf);
    // y2 = final Perlin noise value for the given point, mapped to range [0,1] instead of [-1,1] for convenience
    double y2 = (lerp(y1,y2,zf)+1)/2;
    // cout << y2 << endl;
    return (lerp(y1,y2,zf)+1)/2;
}

// Perlin Octaves function - NOTE: this is not used finally, as we calculate the Perlin turbulence in the fragment shader instead
// Used to produce a more complex/detailed noise pattern by generating layers of Perlin noise at different frequencies and amplitudes, and then combining them
// Parameters: xyz are point coordinates in 3D space; octaves denotes the # of octaves to use in generating the noise; persistence denotes the degree by which the amplitude changes between octaves
double Perlin::generatePerlinOctaves(double x, double y, double z, int octaves, double persistence) {
    double total = 0.0;      // Used to store the accumulated noise values
    double frequency = 1.0;  // Used to adjust the frequency of each noise octave. Frequency determines how quickly the noise values change with respect to changes in input coordinates. Higher frequency = finer-grained/more detail, lower frequency = coarser/more general
    double amplitude = 1.0;  // Used to adjust the amplitude of each noise octave. Amplitude determines the degree of variation between the highest and lowest noise values. Higher amplitude = more variation in noise, lower amplitude = smoother noise pattern
    double max = 0.0;        // Used for normalizing the total
    
    // Iterate over each noise octave to accumulate the noise values
    for (int i = 0; i < octaves; i++) {
        total += generatePerlinNoise(x * frequency, y * frequency, z * frequency, 0) * amplitude; // Adds noise value for the current octave to the total
        max += amplitude;                                                                         // Used for normalizing the result once loop exits
        amplitude *= persistence;                                                                 // Reduces amplitude to decrease the contribution of each subsequent octave to the final noise value
        frequency *= 2;                                                                           // Increases the frequency of each subsequent octave to make the noise more fine-grained
    }
    
    return total/max;     // Normalize total, resulting in a noise value in range [0,1]
}