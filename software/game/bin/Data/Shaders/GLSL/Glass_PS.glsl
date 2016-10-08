#include "Uniforms.glsl"
#include "Samplers.glsl"

const float bumpyness = 0.03;
const float refractFactor = 0.9;
const float alpha = 1;

varying vec3 vPosition_worldSpace;
varying vec2 vTexCoord;
varying vec4 vProjection;

void PS()
{
    vec3 distort = texture2D(sNormalMap, vTexCoord).rgb * 2 - 1;
    distort = normalize(distort) * bumpyness;
    vec3 diffuse = texture2D(sDiffMap, vTexCoord).rgb;
    vec2 nuv = vProjection.xy / vProjection.z + distort.xy;
    vec3 refract = texture2D(sEnvMap, nuv).rgb * cMatDiffColor.rgb;

    gl_FragColor = vec4(mix(diffuse, refract, refractFactor), alpha);
}
