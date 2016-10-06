#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "Lighting.glsl"
#include "Fog.glsl"

varying vec4 vPosition_worldSpace;
varying vec3 vNormal_worldSpace;
varying vec3 vTangent_worldSpace;
varying vec2 vTexCoord;

#ifdef PERPIXEL
#   ifdef SHADOW
varying vec4 vShadowPosition[NUMCASCADES];
#   endif
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;


    // Forward and interpolate normals to pixel shader
    vec3 worldPosition = GetWorldPos(modelMatrix);
    vNormal_worldSpace = GetWorldNormal(modelMatrix);
    vTangent_worldSpace = GetWorldTangent(modelMatrix);
    vTexCoord = GetTexCoord(iTexCoord);

    // Transform vertex to projection space
    gl_Position = GetClipPos(worldPosition);

#ifdef SHADOW
    // Shadow projection: transform from world space to shadow space
    vec4 projWorldPos = vec4(worldPosition, 1.0);
    for (int i = 0; i < NUMCASCADES; i++)
        vShadowPosition[i] = GetShadowPos(i, vNormal_worldSpace, projWorldPos);

    vPosition_worldSpace = vec4(worldPosition, GetDepth(gl_Position));
#else
    vPosition_worldSpace = vec4(worldPosition, 1.0);
#endif

}
