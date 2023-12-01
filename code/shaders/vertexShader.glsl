/* INPUTS
   Vertex position, texture coordinates (used if we aren't simply applying a base color to the geometry),
   Model, view, projection, and transform matrices for placement of geometry in appropriate coordinate space (relative to camera)  
   Animation vector for animation the noise texture coordinates 
   Animation flag for user to turn on/off animation
*/

/* OUTPUTS
    Noise texture coordinates, calculated based on camera position and animation vector 
    Distance between camera and geometry 
    Updated vertex position
*/ 

#version 330 core  

layout (location = 0) in vec3 aPos;            
layout (location = 1) in vec2 aTexCoord;  

// Texture coordinates for base texture (if one is used) and Perlin noise textures
out vec2 texCoord;                                                  
out vec3 noiseTexCoords0;
out vec3 noiseTexCoords1; 
out vec3 noiseTexCoords2; 
out vec3 noiseTexCoords3; 

// Distance between camera/viewer and objects in scene
out float distance; 

// Matrices
uniform mat4 view;                              
uniform mat4 projection;                       
uniform mat4 model;
uniform mat4 transform;

// Animation variables
uniform vec4 animation;
uniform bool animationFlag;

// To know if we drawing the background, which has slightly different values than other geometry in the scene
uniform bool background;

// User control
uniform float fogSize; 

void main()
{
    // Compute the position of the geometry relative to the camera/viewer
    vec4 positionRelativeToCam = view * model * vec4(aPos, 1.0);

    // This position gets assigned as the base coordinates for the noise textures 
    // This is essentially done so that the fog can be applied over the entire scene to give a much more realistic effect than applying it to each object individually
    // After experimenting with different techniques, this gave the best result
    vec3 coords = positionRelativeToCam.xyz;

    // Distance between camera/viewer and geometry in scene
    distance = length(positionRelativeToCam.xyz);

    // Calculations for animating the fog
    if (animationFlag == true) {
        noiseTexCoords0 = coords * ((animation.x + 0.13) + fogSize * 0.3);
        noiseTexCoords1 = coords * ((animation.y + 0.13) + fogSize * 0.3);
        noiseTexCoords2 = coords * ((animation.z + 0.13) + fogSize * 0.3);
        noiseTexCoords3 = coords * ((animation.w + 0.13) + fogSize * 0.3);
    } 

    // Calculations for static fog
    else {
        // Multiply by constant to "stretch" texture to better fit geometry surface
        noiseTexCoords0 = coords * (fogSize + 0.1);
        noiseTexCoords1 = coords * (fogSize + 0.1);
        noiseTexCoords2 = coords * (fogSize + 0.1);
        noiseTexCoords3 = coords * (fogSize + 0.1);
    }

    // If we're drawing the background, we want slightly different values for proper sizing (a less intense fog effect)
    if (background == true) {
        noiseTexCoords0 = coords * 0.03;
        noiseTexCoords1 = coords * 0.03;
        noiseTexCoords2 = coords * 0.03;
        noiseTexCoords3 = coords * 0.03;
    }

    // Output the final vertex position
    gl_Position = projection * view * model * vec4(aPos, 1.0); 

    // Output the based texture coordinates
    texCoord = aTexCoord;  
}