#version 460
out vec4 FragColor;

in vec2 TexCoords;
in vec3 position;
in vec3 ModelSpacePosition;
in vec3 normal;
in vec3 ModelSpaceNormal;
in vec4 vertexColor;
in vec3 FragPos;
in mat3 TBN;
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
uniform float snowAccumulation;
uniform int u_FrameCount;


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


vec3  ScaleCoordinate(vec3 pos, float scale)
{
    return fract(pos) * scale;
}

//TO-DO: snow occlusion, snow normal

// there are values in vertex color b and vertex color r
void main()
{    
    vec3 L = TBN * normalize(u_lightPosition - position);
    
    vec3 Normal = TBN * normal;
    vec3 RippleNormal = texture(RippleTexture, TexCoords * 10).rgb;
    vec3 N = mix(vec3(0,0,1), RippleNormal, u_rainIntensity * (1 - vertexColor.b * u_rainTimePassed));

    //vec3 N = mix(WaterNormal, normalize(normal), vertexColor.r * u_rainIntensity); //use different vertex color in the future
    vec4 BaseFragColor;
    float diff = max(dot(L, Normal), 0);
    vec3 viewDir = TBN * normalize(u_eyePosition - position);
    float shadow = ShadowCalculation(FragPosLightSpace);
    if(u_FrameCount > 0)
    {
        vec3 color = texture(texture_diffuse1, TexCoords).rgb;;
        vec3 diffuse = diff * color;
        vec3 ambient = 0.05 * color;
        vec3 reflectDir = reflect(-L, Normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
        vec3 specular = vec3(0.3) * spec;
        FragColor = vec4(ambient + (1.0 - shadow) * (diffuse + specular), texture(texture_diffuse1, TexCoords).a);
    }
    else
    {
        FragColor = texture(texture_diffuse1, TexCoords);
    }
    BaseFragColor = FragColor;

    if(u_useCubemap > 0.0 && u_rainIntensity > 0.0)
    {
        vec3 I = normalize(ModelSpacePosition - vec3(0,2.0,0));
        vec3 toCameraVector = normalize(u_eyePosition - ModelSpacePosition);
        float refractiveFactor = dot(toCameraVector, vec3(0.0, 1.0, 0.0));
        refractiveFactor = clamp(pow(refractiveFactor, 0.5), 0.0, 1.0);
        vec3 Refract = refract(I, normalize(ModelSpaceNormal), 1.00 / 1.2);
        vec3 Reflect = reflect(I, normalize(ModelSpaceNormal));
        vec3 reflection = textureCube(cubemap, Reflect).rgb;
        vec3 refraction = textureCube(cubemap, Refract).rgb;

        vec3 waterColor = vec3(0,0,0.5);

        vec3 diffuseColor = mix(reflection, refraction, refractiveFactor);
        vec3 diffuseWaterColor = texture(texture_diffuse1, TexCoords).rgb;
        diffuseWaterColor = mix(waterColor, diffuseWaterColor, 0.9);
        diffuseColor = mix(diffuseColor, diffuseWaterColor, 0.9);

        vec3 color = mix(reflection, refraction, refractiveFactor);
        color = mix(color, waterColor, 0.2);

        color = mix(diffuseColor, color, u_rainTimePassed);
        FragColor = vec4(mix(color, diffuseColor * clamp(1.0 - shadow, 0.05, 1.0), vertexColor.b), 1.0);

    }
    if(snowAccumulation > 0.0)
    {       
        vec3 snowNormal = mix(texture(snowNormal1Texture, TexCoords).rgb, texture(snowNormal2Texture, TexCoords).rgb, snowAccumulation);
        snowNormal = snowNormal * 2.0 - 1.0;
        snowNormal = normalize(TBN * snowNormal);

        float slope = acos(dot(snowNormal, vec3(0.0, -1.0, 0.0)));
        slope = clamp(slope / 3.14159, 0.0, 1.0);
        slope = pow(slope, 2.0);


        vec3 posCoord = vec3(gl_FragCoord.x, gl_FragCoord.y, 0.0);
        posCoord /= 10.0;
        float noiseval = 0.0;

        for (float idx = 0.0; idx < orders; ++idx) {
            vec3 posScaled = idx * posCoord;
            float amplitude = pow(persistance, idx);
            posScaled = amplitude * ScaleCoordinate(posCoord, ImageSize);
            noiseval += snoise(posScaled);
        }

        float alpha = slope;
        alpha += FractionNoise * noiseval;
        alpha *= snowAccumulation;

        vec3 SnowColor = texture2D(iceAlbedoTexture, TexCoords).rgb * diff;
        FragColor = vec4(mix(FragColor.rgb,SnowColor, (1 - vertexColor.b) * alpha * u_rainTimePassed) , 1.0);
        if(u_rainIntensity > 0.0)
            alpha *= vertexColor.b;

        diff = max(dot(L, Normal), 0);
        vec3 color = texture2D(snowAlbedoTexture, TexCoords).rgb;
        vec3 reflectDir = reflect(-L, snowNormal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
        vec3 specular = vec3(0.3) * spec;
        vec3 ambient = 0.5 * color;

        vec3 snowAlbedo = ambient + (1.0 - shadow) * (color * diff + specular);
        vec3 snowColor = snowAlbedo * alpha + vec3(FragColor) * (1 - alpha);
        FragColor = vec4(snowColor, 1.0);
    }
}