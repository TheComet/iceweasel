#include "Uniforms.glsl"
#include "Transform.glsl"

const float FUR_LENGTH = 0.002;

varying vec3 vProjection;

void VS()
{
    mat4 modelMatrix = iModelMatrix;

    vec3 position_worldSpace = GetWorldPos(modelMatrix);
    vec4 clipPos = GetClipPos(position_worldSpace);
    vProjection = vec3(
        clipPos.x*0.5 + clipPos.w*0.5,
        clipPos.y*0.5 + clipPos.w*0.5,
        clipPos.w
        // clipPos.w
    );

    gl_Position = clipPos;
}
