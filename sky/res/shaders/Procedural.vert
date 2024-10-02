#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
out vec2 v_TexCoord;

uniform vec3 sunPosition;
const float rayleigh = 2.0;
const float turbidity = 10.0;
const float mieCoefficient = 0.005;

out vec3 vSunDirection;
out float vSunfade;
out vec3 vBetaR;
out vec3 vBetaM;
out float vSunE;
out vec3 vSunColor;
out vec3 vAmbient;

#define PI 3.14159265359


//based on https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky.html

// scattering coefficients at sea level (m)
const vec3 betaR = vec3(5.5e-6, 13.0e-6, 22.4e-6); // for Rayleigh 
const vec3 betaM = vec3(21e-6); // for Mie

// scale height (m) if thickness of the atmosphere if its density was uniform
const float hR = 7994.0; // for Rayleigh
const float hM = 1200.0; // for Mie

const float earthRadius = 6360e3; // (m)
const float atmosphereRadius = 6420e3; // (m)

const float sunPower = 3.0;

const int numSamples = 24;
const int numSamplesLight = 6;

struct ray_t {
	vec3 origin;
	vec3 direction;
};

bool raySphereIntersect(const in ray_t ray, inout float t0, inout float t1)
{
	vec3 rc = vec3(0.0) - ray.origin;
	float atmosphereRadius2 = atmosphereRadius * atmosphereRadius;
	float tca = dot(rc, ray.direction);
	float d2 = dot(rc, rc) - tca * tca;
	if (d2 > atmosphereRadius2) return false;
	float thc = sqrt(atmosphereRadius2 - d2);
	t0 = tca - thc;
	t1 = tca + thc;

	return true;
}

float computeRayleighPhaseFunc(float mu)
{
	return 3. * (1. + mu*mu)/(16. * PI);
}

float HG(float costheta, float g) {
	const float k = 0.0795774715459; 
	return k*(1.0-g*g)/(pow(1.0+g*g-2.0*g*costheta, 1.5));
}

bool computeSunLight(
	const in ray_t ray,
	inout float optical_depthR,
	inout float optical_depthM
){
	float t0 = 0.0;
	float t1 = 0.0;
	raySphereIntersect(ray, t0, t1);

	float marchPos = 0.;
	float marchStep = t1 / float(numSamplesLight);

	for (int i = 0; i < numSamplesLight; i++) {
		vec3 s = ray.origin + ray.direction * (marchPos + 0.5 * marchStep);
		float height = length(s) - earthRadius;
		if (height < 0.0)
			return false;

		optical_depthR += exp(-height / hR) * marchStep;
		optical_depthM += exp(-height / hM) * marchStep;

		marchPos += marchStep;
	}

	return true;
}

vec3 computeIncidentLight(const in ray_t ray)
{
	// "pierce" the atmosphere with the viewing ray
	float t0 = 0.0;
	float t1 = 0.0;
	if (!raySphereIntersect(
		ray, t0, t1)) {
		return vec3(0);
	}

	float marchStep = t1 / float(numSamples);

	vec3 sunDir = normalize(sunPosition);
	//mu in the paper which is the cosine of the angle between the sun direction and the ray direction 
	float mu = dot(ray.direction, sunDir);

	// Rayleigh and Mie phase functions
	float phaseR = computeRayleighPhaseFunc(mu);
	float phaseM = HG(mu, 0.96);

	// optical depth (or "average density"), represents the accumulated extinction coefficients, along the path, multiplied by the length of that path
	float opticalDepthR = 0.;
	float opticalDepthM = 0.;

	vec3 sumR = vec3(0); //sum of Rayleight
	vec3 sumM = vec3(0); //sum of Mie
	float marchPos = 0.;

	for (int i = 0; i < numSamples; i++) {
		vec3 samplePoint = ray.origin + ray.direction * (marchPos + 0.5 * marchStep);
		float height = length(samplePoint) - earthRadius;

		// compute optical depth for light
		float hr = exp(-height / hR) * marchStep;
		float hm = exp(-height / hM) * marchStep;
		opticalDepthR += hr;
		opticalDepthM += hm;

		// light optical depth
		ray_t lightRay = ray_t(samplePoint, sunDir);
		float opticalDepthLightR = 0.;
		float opticalDepthLightM = 0.;

		if (computeSunLight(lightRay, opticalDepthLightR, opticalDepthLightM)) 
		{
			vec3 tau = betaR * (opticalDepthR + opticalDepthLightR) + betaM * 1.1 * (opticalDepthM + opticalDepthLightM);
			vec3 attenuation = exp(-tau);
			sumR += hr * attenuation;
			sumM += hm * attenuation;
		}

		marchPos += marchStep;
	}

	return sunPower * (sumR * phaseR * betaR + sumM * phaseM * betaM);
}

const vec3 up = vec3( 0.0, 1.0, 0.0 ); // up vector

// constants for atmospheric scattering
const float e = 2.71828182845904523536028747135266249775724709369995957;
const float pi = 3.141592653589793238462643383279502884197169;

// wavelength of used primaries, according to preetham
const vec3 lambda = vec3( 680E-9, 550E-9, 450E-9 );
// (8.0 * pow(pi, 3.0) * pow(pow(n, 2.0) - 1.0, 2.0) * (6.0 + 3.0 * pn)) / (3.0 * N * pow(lambda, vec3(4.0)) * (6.0 - 7.0 * pn))
const vec3 totalRayleigh = vec3( 5.804542996261093E-6, 1.3562911419845635E-5, 3.0265902468824876E-5 );

// mie stuff
// K coefficient for the primaries
const float v = 4.0;
const vec3 K = vec3( 0.686, 0.678, 0.666 );
const vec3 MieConst = vec3( 1.8399918514433978E14, 2.7798023919660528E14, 4.0790479543861094E14 ); // MieConst = pi * pow( ( 2.0 * pi ) / lambda, vec3( v - 2.0 ) ) * K

// earth shadow
const float cutoffAngle = 1.6110731556870734; // cutoffAngle = pi / 1.95;
const float steepness = 1.5;
const float EE = 1000.0;

float calculateSunIntensity( float zenithAngleCos ) {
	zenithAngleCos = clamp( zenithAngleCos, -1.0, 1.0 );
	return EE * max( 0.0, 1.0 - pow( e, -( ( cutoffAngle - acos( zenithAngleCos ) ) / steepness ) ) );
}

vec3 calculateTotalMie( float T ) {
	float c = ( 0.2 * T ) * 10E-18;
	return 0.434 * c * MieConst;
}

void main()
{
	gl_Position  = vec4(position, 1.0);
	v_TexCoord = texCoord;

	vSunDirection = normalize( sunPosition );

	vSunE = calculateSunIntensity( dot( vSunDirection, up ) );

	vSunfade = 1.0 - clamp( 1.0 - exp( ( sunPosition.y / 450000.0 ) ), 0.0, 1.0 );

	float rayleighCoefficient = rayleigh - ( 1.0 * ( 1.0 - vSunfade ) );

	// extinction (absorbtion + out scattering)
	// rayleigh coefficients
	vBetaR = totalRayleigh * rayleighCoefficient;
	// mie coefficients
	vBetaM = calculateTotalMie( turbidity ) * mieCoefficient;

	ray_t ray = ray_t(vec3(0.0, earthRadius+1.0, 0.0), normalize(vSunDirection + vec3(0.01, 0.01, 0.0)));
	vSunColor = computeIncidentLight(ray); //caluclate sun color

	ray = ray_t(vec3(0.0, earthRadius+1.0, 0.0), normalize(vec3(0.4, 0.1, 0.0)));
	vAmbient = computeIncidentLight(ray); // calculate ambient (used for clouds)
};