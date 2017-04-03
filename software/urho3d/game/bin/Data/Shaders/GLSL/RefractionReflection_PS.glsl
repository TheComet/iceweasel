#include "Uniforms.glsl"
#include "Samplers.glsl"

const float bumpyness = 0.03;
const float refractFactor = 0.9;

varying vec3 vPosition_worldSpace;
varying vec3 vNormal_worldSpace;
varying vec3 vTangent_worldSpace;
varying vec3 vEyeDirection_worldSpace;
varying vec2 vTexCoord;
varying vec3 vProjection;

void PS()
{
    vec3 eyeDirection_worldSpace = normalize(vEyeDirection_worldSpace);

    vec3 diffuse = texture2D(sDiffMap, vTexCoord).rgb;
    vec3 normal_textureSpace = normalize(texture2D(sNormalMap, vTexCoord).rgb * 2 - 1);

    vec3 distort = normal_textureSpace * bumpyness;
    vec2 nuv = vProjection.xy / vProjection.z + distort.xy;
    vec3 refractColor = texture2D(sEnvMap, nuv).rgb * cMatDiffColor.rgb * 1.5;

    mat3 invTBN = mat3(
        normalize(vTangent_worldSpace),
        normalize(cross(vTangent_worldSpace, vNormal_worldSpace)),
        normalize(vNormal_worldSpace)
    );
    vec3 normal_worldSpace = invTBN * normal_textureSpace;
    vec3 reflect_worldSpace = reflect(-eyeDirection_worldSpace, normal_worldSpace);
    vec3 reflectColor = textureCube(sEnvCubeMap, reflect_worldSpace).rgb;

    //gl_FragColor = vec4(mix(diffuse, refract, refractFactor), alpha);
    gl_FragColor = vec4(refractColor, 1);
}
