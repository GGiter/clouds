#version 460

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in vec2 v_SkyTexCoord;
in vec3 v_Position;

uniform sampler3D u_Texture3D;
uniform float u_RotationX;
uniform float u_RotationY;
uniform float u_RotationZ;
uniform vec3 u_Translation;
uniform float u_Jitter;
uniform int u_UseMipMaps;

/* Configuration properties */
#define MAX_STEPS 256
#define VOLUME_LENGTH 5
#define SHADOW_STEPS 12
#define SHADOW_LENGTH 2.

vec3 skyColor = vec3(0.0,0.0,0.0);
vec3 rayOrigin = vec3(0.0,0.0,0.0);

/* Helper functions used to rotate objects */
mat3 rotationX( in float angle ) {
	return mat3(	1.0,		0,			0,
			 		0, 	cos(angle),	-sin(angle),
					0, 	sin(angle),	 cos(angle));
}

mat3 rotationY( in float angle ) {
	return mat3(	cos(angle),		0,		sin(angle),
			 				0,		1.0,			 0,
					-sin(angle),	0,		cos(angle));
}

mat3 rotationZ( in float angle ) {
	return mat3(	cos(angle),		-sin(angle),	0,
			 		sin(angle),		cos(angle),		0,
							0,				0,		1);
}

/* helper function used to remap values from one range to another */
float remap(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

/* function which samples the 3D texture with cloud data */
float map(vec3 p, float LOD) {
    p = ((p - vec3(0.5)) * rotationY(radians(u_RotationY)) * rotationX(radians(u_RotationX)) * rotationZ(radians(u_RotationZ)) + vec3(0.5));
    return textureLod(u_Texture3D, p, LOD).r;
}

/* lighting functions */
float beer(float d){
	return exp(-d);
}

float powder(float d){
	return (1. - exp(-2.*d));
}

float HG( float sundotrd, float g) {
	float gg = g * g;
	return (1. - gg) / pow( 1. + gg - 2. * g * sundotrd, 1.5);
}

/* raymarching function used to calculate lighting for each step from main raymarching function */
void lightRayMarch(in vec3 ray, in vec3 lightVector, in float opacitySample, inout vec3 lightEnergy, inout float transmittance)
{
    /* configure variables used to calculate lighting */
    float lightDotEye = dot(normalize(lightVector), normalize(ray));
    vec3 sunColor = vec3(1.0, 1.0, .95);
    vec3 ambientLight = (vec3(65., 70., 80.)*(1.5/255.));
    float density = 0.8;
    float shadowStepLength = SHADOW_LENGTH/float(SHADOW_STEPS);
    float densityMultiplier = -SHADOW_LENGTH*density;
    float light_density = 1.0;
    vec3 lightRay = ray;

    /* raymarch to light source and calculate light density */
    for(int s = 0; s < SHADOW_STEPS; ++s)
    {
        lightRay += lightVector;
        float lightSample = map(lightRay, u_UseMipMaps > 0 ? float(s) : 0);
        if(lightSample > 0.0 )
        {
            float Ti = exp(lightSample*densityMultiplier);
            light_density *= Ti;
        }
    }

    /* use calculated density to calculate lighting */
    float dTransmittance = exp(opacitySample*densityMultiplier);

    /* beer and powder */
    float beerPowderTerm = powder(opacitySample);

    /* calculate scattering using henyey-greenstein function */
    float scattering = mix(HG(lightDotEye, -0.08), HG(lightDotEye, 0.08), clamp(lightDotEye*0.5 + 0.5, 0.0, 1.0));
    scattering = max(scattering, 1.0); //clamp scattering

    /* calculate light color using beer law, powder and henyey-greenstein function */
    vec3 lightColor = (mix(mix(ambientLight*1.8, skyColor, 0.2) * scattering, scattering * sunColor, beerPowderTerm * light_density)) * opacitySample;

    /* calculate color intensity */
    vec3 lightColorInt = (lightColor - lightColor * dTransmittance) * (1.0 / opacitySample);
    lightEnergy += transmittance * lightColor; 
    transmittance *= dTransmittance;
}

void main()
{
    vec2 position = v_TexCoord * 2 - vec2(0.5,0.5);

    /* scale size of the cloud and block to it to 4 */
    if(u_Translation.z > 4.0) 
    {
        float zValueClamped = clamp(u_Translation.z - 4.0, 0, 1.0);
        float texMul = remap(zValueClamped, 0, 1, 2.0, 1.0);
        float texPosition = remap(zValueClamped, 0, 1, 0.5, 0.0);
        position = v_TexCoord * texMul - texPosition;
    }

    vec3 rayOrigin =  vec3(position, 0);
    //simulate going through cloud if the z is bigger than 5
    //the logic is to scale the cloud first before going through it
    if(u_Translation.z > 5.0) 
       rayOrigin.z = u_Translation.z - 5.0; 

    /* configure ray/camera direction */
    vec3 rayDirection =  normalize(vec3(0, 0, 1));

    float stepLength = VOLUME_LENGTH/float(MAX_STEPS);
    float shadowStepLength = SHADOW_LENGTH/float(SHADOW_STEPS);

    /* set light sources and their vectors from camera origin */
    vec3 lightPos = vec3(1, 2, 10) + vec3(u_Translation.x, u_Translation.y, 0.0);
    vec3 lightPos2 = vec3(-1, 2, 10) + vec3(u_Translation.x, u_Translation.y, 0.0);
    vec3 lightVector = normalize(rayOrigin - lightPos2);
    vec3 lightVector2 = normalize(rayOrigin - lightPos2);

    /* configure variables used in raymarching */
    vec3 lightEnergy = vec3(0, 0, 0);
    float transmittance = 1;
    float opacity = 0;
    vec3 ray = rayOrigin;

    for(int i = 0; i < MAX_STEPS; ++i)
    {
        /* small optimization no point in increasing opacity after it reaches 1.0 */
        if(opacity >= 1)
            break;

        /* sample opacity from texture data */
        float opacitySample = map(ray, 0);
        if(opacitySample > 0.001)
        {       
            // calculate lighting for each light source (in these case there are two sources */
            lightRayMarch(ray, lightVector, opacitySample, lightEnergy, transmittance);
            lightRayMarch(ray, lightVector2, opacitySample, lightEnergy, transmittance);
            opacity += opacitySample;        
        }

        ray += rayDirection * stepLength;
    }
    
    /* calculate final color */
    vec4 cloudOutput = vec4(vec3(lightEnergy), transmittance);
    color = vec4((cloudOutput.rgb + mix(vec3(0), skyColor, cloudOutput.a)), 1);
};