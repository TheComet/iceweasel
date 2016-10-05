#include "Uniforms.glsl"
#include "Samplers.glsl"

const float bumpBias = -0.2;
const float bumpMagnitude = 0.02;

varying vec3 vPosition_worldSpace;
varying vec3 vNormal_worldSpace;
varying vec3 vTangent_worldSpace;
varying vec2 vTexCoord;

uniform sampler2D sBumpMap2;

void PS()
{
    // Construct TBN matrix
    vec3 normal_worldSpace = normalize(vNormal_worldSpace);
    vec3 tangent_WorldSpace = normalize(vTangent_worldSpace);
    vec3 bitangent_WorldSpace = normalize(cross(tangent_WorldSpace, normal_worldSpace));
    mat3 invMatTBN = mat3(
        tangent_WorldSpace,
        bitangent_WorldSpace,
        normal_worldSpace
    );
    mat3 matTBN = transpose(invMatTBN);

    // Height map influences UV coordinates
    vec3 eyeDirection_worldSpace = normalize(vPosition_worldSpace - cCameraPosPS);
    vec3 eyeDirection_tangentSpace = normalize(matTBN * eyeDirection_worldSpace);
    float heightOffset = texture2D(sBumpMap2, vTexCoord).r + bumpBias;
    vec2 newUV = vTexCoord - eyeDirection_tangentSpace.xy * heightOffset * bumpMagnitude;

    // Sample ambient diffuse component
    vec3 materialDiffuseColor = texture2D(sDiffMap, newUV).rgb;

        // Global ambient color
    vec3 ambientComponent = cAmbientColor.rgb * materialDiffuseColor;

#ifndef NO_NORMAL_MAP
    // Sample normal vector in tangent space
    vec3 normalMap_worldSpace = normalize(invMatTBN * (texture2D(sNormalMap, newUV).rgb * 2.0 - 1.0));
#else
    vec3 normalMap_worldSpace = normalize(vNormal_worldSpace);
#endif

    // Calculate light influence on diffuse
    vec3 lightVector_worldSpace = vPosition_worldSpace - cLightPosPS.xyz;
    vec3 lightDirection_worldSpace = normalize(lightVector_worldSpace);
    float cosTheta = clamp(dot(normalize(-lightVector_worldSpace), normalMap_worldSpace), 0.0, 1.0);
    float lightDistanceSquared = dot(lightVector_worldSpace, lightVector_worldSpace);
    vec3 diffuseComponent = materialDiffuseColor * cLightColor.rgb * cosTheta / lightDistanceSquared;

    // Calculate specular component
    float specularPower = cMatSpecColor.a;
    vec3 lightReflectDirection_worldSpace = reflect(-eyeDirection_worldSpace, normalMap_worldSpace);
    float cosAlpha = max(dot(lightDirection_worldSpace, lightReflectDirection_worldSpace), 0.0);
    vec3 specularComponent = cMatSpecColor.rgb * materialDiffuseColor * pow(cosAlpha, specularPower/4);

    gl_FragColor = vec4(ambientComponent + diffuseComponent + specularComponent, 1);
    //gl_FragColor = vec4(cosTheta);
}
