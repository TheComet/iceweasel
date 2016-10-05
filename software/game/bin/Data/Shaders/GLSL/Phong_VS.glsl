#include "Uniforms.glsl"
#include "Transform.glsl"
#include "Lighting.glsl"

varying vec3 vPosition_worldSpace;
varying vec3 vNormal_worldSpace;
varying vec3 vTangent_worldSpace;
varying vec2 vTexCoord;

void VS()
{
    mat4 modelMatrix = iModelMatrix;

    // Forward and interpolate normals to pixel shader
    vPosition_worldSpace = GetWorldPos(modelMatrix);
    vNormal_worldSpace = GetWorldNormal(modelMatrix);
    vTangent_worldSpace = GetWorldTangent(modelMatrix);
    vTexCoord = GetTexCoord(iTexCoord);

    // Transform vertex to projection space
    gl_Position = GetClipPos(vPosition_worldSpace);
}
