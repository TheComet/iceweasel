#include "Uniforms.glsl"
#include "Transform.glsl"

const float FUR_LENGTH = 0.002;

varying vec3 vNormal_worldSpace;

void VS()
{
    mat4 modelMatrix = iModelMatrix;

    vNormal_worldSpace = GetWorldNormal(modelMatrix);
    vec3 position_worldSpace = GetWorldPos(modelMatrix) + vNormal_worldSpace * FUR_HEIGHT * FUR_LENGTH;

    gl_Position = GetClipPos(position_worldSpace);
}
