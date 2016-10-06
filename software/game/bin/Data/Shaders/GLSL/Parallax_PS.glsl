#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "Lighting.glsl"
#include "Fog.glsl"

const float bumpBias = -0.2;
const float bumpMagnitude = 0.02;
float attenuationFallOff = 0.3;

varying vec4 vPosition_worldSpace;
varying vec3 vNormal_worldSpace;
varying vec3 vTangent_worldSpace;
varying vec2 vTexCoord;

#ifdef SHADOW
varying vec4 vShadowPosition[NUMCASCADES];
#endif

void PS()
{
    vec3 eyeDirection_worldSpace = normalize(vPosition_worldSpace.xyz - cCameraPosPS);

    /*
     * If normal maps or bump maps are enabled, we need the tangent/binormal/
     * normal (TBN) matrix and its inverse, so the normal vector from the
     * normal map can be transformed into world space, and so that the eye
     * vector can be transformed into texture space for the parallax
     * calculation.
     */
#if !defined(NO_NORMAL_MAP) || !defined(NO_BUMP_MAP)
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
#endif

#if !defined(NO_BUMP_MAP)
    // Height map influences UV coordinates
    vec3 eyeDirection_tangentSpace = normalize(matTBN * eyeDirection_worldSpace);
    float heightOffset = texture2D(sNormalMap, vTexCoord).a;
    vec2 newUV = vTexCoord - eyeDirection_tangentSpace.xy * heightOffset * bumpMagnitude;
#else
    vec2 newUV = vTexCoord;
#endif

    vec3 finalColor = vec3(0);

    // Sample ambient diffuse component
    vec3 materialDiffuseColor = texture2D(sDiffMap, newUV).rgb;

    /*
     * If we have a normal map, sample that and transform it from texture space
     * into world space. If we don't have a normal map, just use the vertex
     * normal.
     */
#if !defined(NO_NORMAL_MAP)
    // Sample normal vector in tangent space
    vec3 normalMap_worldSpace = normalize(invMatTBN * (texture2D(sNormalMap, newUV).rgb * 2.0 - 1.0));
#else
    vec3 normalMap_worldSpace = normalize(vNormal_worldSpace);
#endif

    /*
     * The ambient component is applied once during the base pass, but omitted
     * during additional light passes.
     */
#if !defined(PERPIXEL)
    vec3 ambientComponent = cAmbientColor.rgb * materialDiffuseColor;
    finalColor += ambientComponent;
#else
    // Calculate light influence on diffuse
    vec3 lightVector_worldSpace = vPosition_worldSpace.xyz - cLightPosPS.xyz;
    vec3 lightDirection_worldSpace = normalize(lightVector_worldSpace);
    float cosTheta = max(dot(normalize(-lightVector_worldSpace), normalMap_worldSpace), 0.0);
    float lightDistanceSquared = pow(dot(lightVector_worldSpace, lightVector_worldSpace), attenuationFallOff);
    vec3 diffuseComponent = cLightColor.rgb * cMatDiffColor.rgb * materialDiffuseColor * cosTheta / lightDistanceSquared;

    finalColor += diffuseComponent;

#   if defined(SPECULAR)
    // Calculate specular component
    float specularPower = cMatSpecColor.a;
    float specularIntensity = cLightColor.a;
    vec3 lightReflectDirection_worldSpace = reflect(-eyeDirection_worldSpace, normalMap_worldSpace);
    float cosAlpha = max(dot(lightDirection_worldSpace, lightReflectDirection_worldSpace), 0.0);
    vec3 specularComponent = cLightColor.rgb * cMatSpecColor.rgb * materialDiffuseColor * pow(cosAlpha, specularPower);

    finalColor += specularComponent;
#   endif

#   ifdef SHADOW
    float shadow = GetShadow(vShadowPosition, vPosition_worldSpace.w);
    finalColor *= shadow;
#   endif

#endif

    gl_FragColor = vec4(finalColor, 1);
}
