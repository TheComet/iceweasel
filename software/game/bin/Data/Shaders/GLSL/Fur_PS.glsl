#include "Uniforms.glsl"
#include "Samplers.glsl"

#if defined(FUR_BASE)
varying vec3 vProjection;
#endif

void PS()
{
#if defined(FUR_BASE)
    gl_FragColor = vec4(texture2D(sEnvMap, vProjection.xy / vProjection.z).rgb, 1);
#else
    gl_FragColor = vec4(vec3(1), 0.5);
#endif
}
