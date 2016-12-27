#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"

varying vec2 vTexCoord;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetTexCoord(iTexCoord);
}

void PS()
{
    vec4 sample = texture2D(sDiffMap, vTexCoord);
    vec3 emissive = sample.rgb * sample.a;
    gl_FragColor = vec4(emissive, 1);
}
