#include "Uniforms.glsl"
#include "Transform.glsl"

varying vec3 vPosition_worldSpace;
varying vec2 vTexCoord;
varying vec4 vProjection;

void VS()
{
    mat4 modelMatrix = iModelMatrix;

    vPosition_worldSpace = GetWorldPos(modelMatrix);
    vTexCoord = GetTexCoord(iTexCoord);
    vec4 clipPos = GetClipPos(vPosition_worldSpace);
    vProjection = vec4(
        clipPos.x*0.5 + clipPos.w*0.5,
        clipPos.w*0.5 + clipPos.y*0.5,
        clipPos.w,
        clipPos.w
    );

    gl_Position = clipPos;
}
