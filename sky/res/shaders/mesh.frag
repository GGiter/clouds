#version 460
#extension GL_NV_shadow_samplers_cube : enable
out vec4 FragColor;

in vec2 TexCoords;
in vec3 ModelSpacePosition;
in vec3 normal;
in vec4 vertexColor;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform vec3 u_eyePosition;
uniform vec3 u_lightPosition;
uniform sampler2D texture_diffuse1;
uniform sampler2D RippleTexture;
uniform samplerCube cubemap;
uniform float u_useCubemap;
uniform float u_rainIntensity;
uniform float u_rainTimePassed;
uniform mat4 view;
uniform mat4 model;
uniform float snowAccumulation;
uniform int u_FrameCount;
uniform int u_useRipples;
uniform float shadowIntensity;
uniform int u_renderPuddles;
uniform int u_applyWetEffect; 
uniform int u_hasTexture; 


uniform sampler2D snowNormal1Texture;
uniform sampler2D snowNormal2Texture;
uniform sampler2D snowAlbedoTexture;
uniform sampler2D iceAlbedoTexture;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = max(0.01 * (1.0 - dot(normal, u_lightPosition)), 0.001);  

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    return shadow;
}

// https://github.com/ashima/webgl-noise noise

const float ImageSize = 256.;
const float FractionNoise = 0.3;
const float persistance =  0.6;
const float orders = 3.;

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i);
  vec4 p = permute( permute( permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
}


vec3 ScaleCoordinate(vec3 pos, float scale)
{
    return fract(pos) * scale;
}

// Function to generate noise
float GenerateNoise(vec3 posCoord) {
    float noiseValue = 0.0;
    for (float idx = 0.0; idx < orders; ++idx) {
        vec3 posScaled = idx * posCoord;
        float amplitude = pow(persistance, idx);
        posScaled = amplitude * ScaleCoordinate(posCoord, ImageSize);
        noiseValue += snoise(posScaled);
    }
    return noiseValue;
}

// Function to calculate specular lighting
vec3 CalculateSpecular(vec3 lightDir, vec3 viewDir, vec3 normal) {
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    return vec3(0.3) * spec;
}

//3D Value Noise generator by Morgan McGuire @morgan3d
//https://www.shadertoy.com/view/4dS3Wd
float hash(float n) { return fract(sin(n) * 1e4); }
float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float noise(vec3 x) {
	const vec3 step = vec3(110, 241, 171);

	vec3 i = floor(x);
	vec3 f = fract(x);

	// For performance, compute the base input to a 1D hash from the integer part of the argument and the
	// incremental change to the 1D based on the 3D -> 1D wrapping
    float n = dot(i, step);

	vec3 u = f * f * (3.0 - 2.0 * f);
	return mix(mix(mix( hash(n + dot(step, vec3(0, 0, 0))), hash(n + dot(step, vec3(1, 0, 0))), u.x),
		   mix( hash(n + dot(step, vec3(0, 1, 0))), hash(n + dot(step, vec3(1, 1, 0))), u.x), u.y),
	       mix(mix( hash(n + dot(step, vec3(0, 0, 1))), hash(n + dot(step, vec3(1, 0, 1))), u.x),
		   mix( hash(n + dot(step, vec3(0, 1, 1))), hash(n + dot(step, vec3(1, 1, 1))), u.x), u.y), u.z);
}

//Fractional Brownian Motion
#define NUM_OCTAVES 2

float fnoise(vec3 x) {
	float v = 0.0;
	float a = 0.5;
	vec3 shift = vec3(100);
	for (int i = 0; i < NUM_OCTAVES; ++i) {
		v += a * noise(x);
		x = x * 2.0 + shift;
		a *= 0.5;
	}
	return v;
}


vec4 ApplyWetSurfaceEffect(vec4 baseColor, vec3 viewDir, vec3 lightDir, float diffuseStrength) {
    // Calculate incident vector from camera to the surface
    vec3 I = normalize(ModelSpacePosition - vec3(0, 2, 0));
    
    // Enhance glossiness for a shiny, wet effect
    float wetGloss = 0.6 + 0.6 * u_rainIntensity; // Increase gloss intensity for shinier effect
    vec3 wetNormal = normal;
    
    // Calculate reflection direction based on the enhanced normal

    vec3 reflectionColor = baseColor.rgb;
    if(u_useCubemap > 0)
    {
        vec3 reflectDir = reflect(I, wetNormal);
        reflectionColor = texture(cubemap, reflectDir).rgb;
    }
   
    // Darken the reflection to simulate a wet, dampened look
    reflectionColor = mix(reflectionColor, vec3(0.0), 0.5); // Mix with black to darken reflection

    // Fresnel factor calculation
    float F0 = 0.04; // Base reflectivity for water or smooth surface
    float cosTheta = clamp(dot(viewDir, wetNormal), 0.0, 1.0);
    float fresnelFactor = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);

    // Generate FBM-based noise for randomized reflection application
    float reflectionNoise = fnoise(ModelSpacePosition * 100.0); // Adjust scale to control patchiness
    float reflectionThreshold = mix(0.1, 0.21, u_rainTimePassed); // Dynamically adjust threshold based on rain intensity

    // Determine reflection factor based on FBM noise and Fresnel effect
    float reflectionFactor = (reflectionNoise < reflectionThreshold) ? fresnelFactor * u_rainTimePassed : 0.0; // Adjust reflection factor
    
    // Adjust reflectionFactor to ensure it scales smoothly with u_rainTimePassed
    reflectionFactor = clamp(reflectionFactor, 0.0, 1.0); // Ensure it stays within [0, 1]

    // Calculate the wet surface color, using a darkened reflection color
    vec3 wetSurfaceColor = mix(baseColor.rgb, reflectionColor, reflectionFactor);

    // Adding a strong specular highlight to enhance shininess on reflective areas
    vec3 h = normalize(lightDir + viewDir); // Halfway vector for Phong specular highlight
    float specularStrength = pow(max(dot(wetNormal, h), 0.0), 100.0); // Increase exponent for sharper highlight
    vec3 specularHighlight = specularStrength * reflectionColor * u_rainTimePassed; // Modulate by rain intensity
    
    // **Additional gloss for non-reflective areas**: Calculate glossiness where reflection is minimal
    float nonReflectiveFactor = 1.0 - reflectionFactor; // Inverse of reflection factor
    float nonReflectiveGloss = pow(max(dot(wetNormal, h), 0.0), 50.0); // Lower gloss exponent for a broader sheen
    vec3 nonReflectiveHighlight = nonReflectiveGloss * vec3(1.0, 1.0, 1.0) * nonReflectiveFactor;

    // Combine base color with reflections, specular highlight, and additional non-reflective gloss effect
    vec3 finalColor = wetSurfaceColor + specularHighlight + nonReflectiveHighlight;
    return vec4(finalColor, 1.0);
}

// Function to apply cubemap effects
vec4 ApplyCubemapEffects(vec4 baseColor, vec3 lightViewDir, vec3 viewDir, float diffuseStrength) {
    vec3 I = normalize(ModelSpacePosition - vec3(0, 2, 0));
    float refractiveFactor = 0.5;
    vec3 rippleNormal = texture(RippleTexture, TexCoords).rgb;
    float rippleAlpha = texture(RippleTexture, TexCoords).a * u_useRipples;

    // Blending factor for ripple normal and model space normal
    float rippleBlendFactor = clamp(rippleAlpha * u_rainIntensity * (1.0 - vertexColor.b * u_rainTimePassed), 0.0, 1.0);
    vec3 blendedNormal = normalize(mix(normal, rippleNormal, rippleBlendFactor));

    vec3 refractDir = refract(I, normalize(blendedNormal), 1.00 / 1.2);
    vec3 reflectDir = reflect(I, normalize(blendedNormal));
    vec3 reflectionColor = textureCube(cubemap, reflectDir).rgb;
    vec3 refractionColor = textureCube(cubemap, refractDir).rgb;

    // Darken the reflection and refractionColor to simulate a wet, dampened look
    reflectionColor = mix(reflectionColor, vec3(0.0), 0.5); // Mix with black to darken reflection
    refractionColor = mix(refractionColor, vec3(0.0), 0.5); // Mix with black to darken refractionColor

    vec3 waterColor = diffuseStrength * vec3(0, 0, 0.5);
    vec3 diffuseColor = diffuseStrength * mix(reflectionColor, refractionColor, refractiveFactor);
    vec3 diffuseWaterColor = diffuseStrength * texture(texture_diffuse1, TexCoords).rgb;
    diffuseWaterColor = mix(waterColor, diffuseWaterColor, 0.9);
    diffuseColor = mix(diffuseColor, diffuseWaterColor, 0.9);

    vec3 finalColor = mix(baseColor.rgb, mix(reflectionColor, refractionColor, refractiveFactor), u_rainTimePassed);
    return vec4(mix(finalColor, baseColor.rgb, vertexColor.b), 1.0);
}

// Function to apply snow effects
vec4 ApplySnowEffects(vec4 baseColor, vec3 lightViewDir, float diffuseStrength, vec3 viewDir, float shadowFactor, vec3 specularColor) {
    vec3 snowNormal = mix(texture(snowNormal1Texture, TexCoords).rgb, texture(snowNormal2Texture, TexCoords).rgb, snowAccumulation);
    snowNormal = normalize((snowNormal * 2.0 - 1.0));

    // Calculate slope
    float slope = acos(dot(snowNormal, vec3(0.0, -1.0, 0.0)));
    slope = clamp(slope / 3.14159, 0.0, 1.0);
    //slope = pow(slope, 2.0);

    // Noise generation
    vec3 posCoord = vec3(TexCoords.x, TexCoords.y, 0.0) * 200;
    float noiseVal = GenerateNoise(posCoord);

    // Calculate snow alpha
    float alpha = slope + FractionNoise * smoothstep(0.0, 1.0, noiseVal);
    alpha *= snowAccumulation;
    if(vertexColor == vec4(0.5))
        alpha *= 1.2;

    // Snow color blending
    vec3 snowColor = texture2D(iceAlbedoTexture, TexCoords).rgb;
    vec4 snowFragColor = vec4(mix(baseColor.rgb, snowColor, (1.0 - vertexColor.b) * alpha * u_rainTimePassed), 1.0);

    // Specular highlight for snow
    if (u_rainIntensity > 0.0) {
        alpha *= vertexColor.b;
    }
    vec3 color = texture2D(snowAlbedoTexture, TexCoords).rgb;
    vec3 reflectDir = reflect(-lightViewDir, snowNormal);
    vec3 ambient = 0.9 * color;

    // Final snow color
    vec3 snowAlbedo = ambient + (1.0 - shadowFactor) * (color * diffuseStrength + specularColor);
    vec3 finalSnowColor = snowAlbedo * alpha + vec3(snowFragColor) * (1.0 - alpha);
    return vec4(finalSnowColor, 1.0);
}

// Main function for fragment shader
void main() 
{    
    vec3 lightViewDir = normalize(u_lightPosition - ModelSpacePosition);
    vec4 baseFragColor = vec4(1,1,1,1);
    float diffuseStrength = max(dot(lightViewDir, normal), 0);
    vec3 viewDir = normalize(ModelSpacePosition - u_eyePosition);
    float shadowFactor = 0.0;
    if(shadowIntensity > 0.0)
        shadowFactor = shadowIntensity * ShadowCalculation(FragPosLightSpace);

    vec3 specularColor = vec3(0);

    // Lighting calculations
    if (u_FrameCount > 0) {
        vec4 color = vec4(0.23, 0.156, 0.055, 1.0);
        if(u_hasTexture > 0)
            color = texture(texture_diffuse1, TexCoords).rgba;
        vec3 ambientColor = 0.1 * color.rgb;
        vec3 diffuseColor = diffuseStrength * color.rgb;
        specularColor = CalculateSpecular(lightViewDir, viewDir, normal);
        
        baseFragColor = vec4(ambientColor + (1.0 - shadowFactor) * (diffuseColor + specularColor), color.a);
    } else {
        baseFragColor = texture(texture_diffuse1, TexCoords);
    }

    // Apply cubemap reflections/refractions
    // Conditionally apply cubemap reflections/refractions
    if (u_rainIntensity > 0.0 && u_useCubemap > 0.0) {
        if (vertexColor.b > 0.99 && u_applyWetEffect > 0) {
            baseFragColor = ApplyWetSurfaceEffect(baseFragColor, viewDir, lightViewDir, diffuseStrength);
        } else if (u_renderPuddles > 0) {
            baseFragColor = ApplyCubemapEffects(baseFragColor, lightViewDir, viewDir, diffuseStrength);
        }
    }

    // Handle snow accumulation effects
    if (snowAccumulation > 0.0) {       
        baseFragColor = ApplySnowEffects(baseFragColor, lightViewDir, diffuseStrength, viewDir, shadowFactor, specularColor);
    }

    FragColor = baseFragColor;
}