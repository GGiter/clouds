#version 460
out vec4 FragColor;

in vec2 TexCoords;
in vec3 position;
in vec3 normal;
in vec4 vertexColor;

uniform vec3 u_eyePosition;
uniform vec3 u_lightPosition;
uniform sampler2D texture_diffuse1;
uniform sampler2D RippleTexture;
uniform sampler2D glassNoiseTexture;
uniform samplerCube cubemap;
uniform float u_useCubemap;
uniform float u_rainIntensity;
uniform float u_Time;
uniform mat4 view;
uniform float snowAccumulation;

#define S(x, y, z) smoothstep(x, y, z)

vec3 N31(float p) {
    //  3 out, 1 in... DAVE HOSKINS
   vec3 p3 = fract(vec3(p) * vec3(.1031,.11369,.13787));
   p3 += dot(p3, p3.yzx + 19.19);
   return fract(vec3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

float SawTooth(float t) {
    return cos(t+cos(t))+sin(2.*t)*.2+sin(4.*t)*.02;
}

float DeltaSawTooth(float t) {
    return 0.4*cos(2.*t)+0.08*cos(4.*t) - (1.-sin(t))*sin(t+cos(t));
}  

vec2 GetDrops(vec2 uv, float seed, float m) {
    
    float t = u_Time+m*30.;
    vec2 o = vec2(0.);
    
    #ifndef DROP_DEBUG
    uv.y += t*.05;
    #endif
    
    uv *= vec2(10., 2.5)*2.;
    vec2 id = floor(uv);
    vec3 n = N31(id.x + (id.y+seed)*546.3524);
    vec2 bd = fract(uv);
    
    vec2 uv2 = bd;
    
    bd -= .5;
    
    bd.y*=4.;
    
    bd.x += (n.x-.5)*.6;
    
    t += n.z * 6.28;
    float slide = SawTooth(t);
    
    float ts = 1.5;
    vec2 trailPos = vec2(bd.x*ts, (fract(bd.y*ts*2.-t*2.)-.5)*.5);
    
    bd.y += slide*2.;								// make drops slide down
    
    #ifdef HIGH_QUALITY
    float dropShape = bd.x*bd.x;
    dropShape *= DeltaSawTooth(t);
    bd.y += dropShape;								// change shape of drop when it is falling
    #endif
    
    float d = length(bd);							// distance to main drop
    
    float trailMask = S(-.2, .2, bd.y);				// mask out drops that are below the main
    trailMask *= bd.y;								// fade dropsize
    float td = length(trailPos*max(.5, trailMask));	// distance to trail drops
    
    float mainDrop = S(.2, .1, d);
    float dropTrail = S(.1, .02, td);
    
    dropTrail *= trailMask;
    o = mix(bd*mainDrop, trailPos, dropTrail);		// mix main drop and drop trail
    
    #ifdef DROP_DEBUG
    if(uv2.x<.02 || uv2.y<.01) o = vec2(1.);
    #endif
    
    return o;
}

float stepfun(float x) {
	return (sign(x) + 1.0) / 2.0;
}

float square(vec2 pos) {
    return (stepfun(pos.x + 1.0) * stepfun(1.0 - pos.x)) *
        (stepfun(pos.y + 1.0) * stepfun(1.0 - pos.y));
}

vec3 FrostDist(vec3 pos)
{
    vec2 pos2D = pos.xy;
    
	return vec3(vec2(pos2D + square((pos2D) * 1.0) * 
        texture2D(glassNoiseTexture, (pos2D) * 1.0).xy * 0.2), pos.z);
}


void main()
{    
    float sunIntensity = 2;
    vec3 L = normalize(u_lightPosition - position);

    vec2 dropUv = TexCoords; 
    vec2 offs = GetDrops(dropUv * 5, 1., 0);
    if(u_rainIntensity == 0)
        offs = vec2(0,0);

    vec3 N = normalize(vec3(offs, 0.0));
    vec3 I = normalize(position);
    vec3 Reflect = reflect(I, normalize(N + normal));

    if(snowAccumulation > 0.0)
    {
        Reflect = FrostDist(Reflect);
    }

    vec3 reflection = texture(cubemap, Reflect).rgb;

    vec3 waterColor = vec3(0,0,0);
    vec3 color = mix(reflection, waterColor, 0.0);\

    FragColor = vec4(color, 1.0);
}