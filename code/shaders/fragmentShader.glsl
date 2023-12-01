/* INPUTS
   Vertex position in appropriate coordinate space (relative to camera)
   Perlin noise texture coordinates and noise textures 
   Base texture for geometry (if a base texture is used, and not just a plain color)
   Fog density, fog color, # of Perlin noise octaves used (based on step slider value), geometry color which user can modify
*/

/* OUTPUTS
    Final fragment color
*/ 

#version 330 core 

// Texture coordinates               
in vec2 texCoord;

// Distance between camera/viewer and objects in scene
in float distance; 

// Perlin noise texture coordinates
in vec3 noiseTexCoords0;
in vec3 noiseTexCoords1;
in vec3 noiseTexCoords2;
in vec3 noiseTexCoords3;

// Final fragment color
out vec4 fragColor; 

// Textures
uniform sampler2D baseTexture; 
uniform sampler3D noiseTexture0;             
uniform sampler3D noiseTexture1;        
uniform sampler3D noiseTexture2;        
uniform sampler3D noiseTexture3; 

// Other fog variables, which user can control
uniform float density;
uniform vec4 fogColor; 
uniform vec4 geoColor;               
uniform int numOctaves; 

void main()                                     
{   
    // Makes it possible to add a 2D texture as the base layer, however we use a vec4 color instead to allow user to choose their color
    vec4 baseColor = texture(baseTexture, texCoord);

    // Create the noise textures
    vec4 noiseOctave0 = texture(noiseTexture0, noiseTexCoords0); 
    vec4 noiseOctave1 = texture(noiseTexture1, noiseTexCoords1); 
    vec4 noiseOctave2 = texture(noiseTexture2, noiseTexCoords2); 
    vec4 noiseOctave3 = texture(noiseTexture3, noiseTexCoords3); 

    // For each texture, get the appropriate coordinate for turbulence calculations
    float k0 = noiseOctave0.x;
    float k1 = noiseOctave1.y;
    float k2 = noiseOctave2.z;
    float k3 = noiseOctave3.w;

    // Set up variables for fog formula
    float fogFactor = 0.0f;
    float turbulence = 0.0f; 

    // Number of noise octaves used (so the degree of turbulence) is based on user's choice so update the calculation accordingly
    // At 0 octaves, it's just regular exponential fog, with no turbulence
    if (numOctaves == 0) { 
        fogFactor = exp(-pow(distance*density, 2.0f));  
    }      
    else if (numOctaves == 4) { 
        turbulence = k0; fogFactor = exp(-pow(distance*density*turbulence, 2.0f)); 
    }               
    else if (numOctaves == 8) { 
        turbulence = k0 + k1/2.0f; 
        fogFactor = exp(-pow(distance*density*turbulence, 2.0f)); 
    }                     
    else if (numOctaves == 16) { 
        turbulence = k0 + k1/2.0f + k2/4.0f; 
        fogFactor = exp(-pow(distance*density*turbulence, 2.0f));  
    }           
    else if (numOctaves == 32) { 
        turbulence = k0 + k1/2.0f + k2/4.0f + k3/8.0f; 
        fogFactor = exp(-pow(distance*density*turbulence, 2.0f));  
    }  
    
    // Clamp the fog factor in range [0,1]
    fogFactor = clamp(fogFactor, 0.0f, 1.0f);

    // Output the final fragment color, which is based on the mixing of the fog color and geometry color, interpolated based on the fog factor
    fragColor = mix(fogColor, geoColor, fogFactor);             
}