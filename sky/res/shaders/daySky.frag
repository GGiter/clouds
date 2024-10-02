
//preetham variables
in vec3 vSunDirection;
in float vSunfade;
in vec3 vBetaR;
in vec3 vBetaM;
in float vSunE;

// optical length at zenith for molecules
const float rayleighZenithLength = 8.4E3;
const float mieZenithLength = 1.25E3;
const vec3 up = vec3( 0.0, 1.0, 0.0 );
// 66 arc seconds -> degrees, and the cosine of that
const float sunAngularDiameterCos = 0.999956676946448443553574619906976478926848692873900859324;

// constants for atmospheric scattering
const float pi = 3.141592653589793238462643383279502884197169;

// 3.0 / ( 16.0 * pi )
const float THREE_OVER_SIXTEENPI = 0.05968310365946075;

const float whiteScale = 1.0748724675633854; // 1.0 / Uncharted2Tonemap(1000.0)

const float mieDirectionalG = 0.8;

//used at the end of clouds calculations to improve colors a little
vec3 U2Tone(const vec3 x) {
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;

   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float HG( float sundotrd, float g) {
	float gg = g * g;
	return (1. - gg) / pow( 1. + gg - 2. * g * sundotrd, 1.5);
}

/* preetham model */
// Based on "A Practical Analytic Model for Daylight"
// aka The Preetham Model, the de facto standard analytic skydome model
// http://www.cs.utah.edu/~shirley/papers/sunsky/sunsky.pdf

vec3 preetham(const vec3 vWorldPosition, vec3 cameraPosition) {
    // cutoff angle at 90 to avoid singularity in next formula.
	float zenithAngle = acos( max( 0.0, dot( up, normalize( vWorldPosition ) ) ) );
	float inv = 1.0 / ( cos( zenithAngle ) + 0.15 * pow( 93.885 - ( ( zenithAngle * 180.0 ) / pi ), -1.253 ) );
	float sR = rayleighZenithLength * inv;
	float sM = mieZenithLength * inv;

	// combined extinction factor
	vec3 Fex = exp( -( vBetaR * sR + vBetaM * sM ) );

	// in scattering
	float cosTheta = dot( normalize( vWorldPosition ), vSunDirection );

	float rayleighPhase = THREE_OVER_SIXTEENPI * ( 1.0 + pow( cosTheta*0.5+0.5, 2.0 ) );
	vec3 betaRTheta = vBetaR * rayleighPhase;

	float miePhase = HG( cosTheta, mieDirectionalG );
	vec3 betaMTheta = vBetaM * miePhase;

	vec3 Lin = pow( vSunE * ( ( betaRTheta + betaMTheta ) / ( vBetaR + vBetaM ) ) * ( 1.0 - Fex ), vec3( 1.5 ) );
	Lin *= mix( vec3( 1.0 ), pow( vSunE * ( ( betaRTheta + betaMTheta ) / ( vBetaR + vBetaM ) ) * Fex, vec3( 1.0 / 2.0 ) ), clamp( pow( 1.0 - dot( up, vSunDirection ), 5.0 ), 0.0, 1.0 ) );

     // nightsky
	vec3 direction = normalize( vWorldPosition - cameraPosition );
    float theta = acos( direction.y ); // elevation --> y-axis, [-pi/2, pi/2]',
    float phi = atan( direction.z, direction.x ); // azimuth --> x-axis [-pi/2, pi/2]',
    vec2 uv = vec2( phi, theta ) / vec2( 2.0 * pi, pi ) + vec2( 0.5, 0.0 );
    vec3 L0 = vec3(0.1, 0.1, 0.1) * Fex;

	// composition + solar disc
	float sundisk = smoothstep( sunAngularDiameterCos, sunAngularDiameterCos + 0.00002, cosTheta );
	L0 += ( vSunE * 19000.0 * Fex ) * sundisk;

	vec3 texColor = ( Lin + L0 ) * 0.04 + vec3( 0.0, 0.0003, 0.00075 );

	vec3 curr = U2Tone( texColor );
	vec3 color = curr * whiteScale;

	vec3 retColor = pow( color, vec3( 1.0 / ( 1.2 + ( 1.2 * vSunfade ) ) ) );

	return retColor;
}
/*	end of atmospheric scattering */