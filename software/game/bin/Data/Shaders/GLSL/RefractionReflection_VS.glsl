#include "Uniforms.glsl"
#include "Transform.glsl"

varying vec3 vPosition_worldSpace;
varying vec3 vNormal_worldSpace;
varying vec3 vTangent_worldSpace;
varying vec3 vEyeDirection_worldSpace;
varying vec2 vTexCoord;
varying vec3 vProjection;

void VS()
{
    mat4 modelMatrix = iModelMatrix;

    vPosition_worldSpace = GetWorldPos(modelMatrix);
    vNormal_worldSpace = GetWorldNormal(modelMatrix);
    vTangent_worldSpace = GetWorldTangent(modelMatrix);
    vTexCoord = GetTexCoord(iTexCoord);
    vec4 clipPos = GetClipPos(vPosition_worldSpace);
    vProjection = vec3(
        clipPos.x*0.5 + clipPos.w*0.5,
        clipPos.y*0.5 + clipPos.w*0.5,
        clipPos.w
        // clipPos.w
    );

    vEyeDirection_worldSpace = cCameraPos - vPosition_worldSpace;

    gl_Position = clipPos;
}
