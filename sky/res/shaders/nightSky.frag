/*moon calculation */

#define MOONLIGHTCOLOR vec3(.8,0.8,0.8)

bool intersectSphere(in vec3 rayOrigin, in vec3 rayDirection, in vec4 sphere, out float distance, out vec3 normal) {
    // Calculate the vector from the ray origin to the sphere center
    vec3 toSphereCenter = rayOrigin - sphere.xyz;
    
    // Compute coefficients for the quadratic equation
    float b = dot(rayDirection, toSphereCenter);
    float c = dot(toSphereCenter, toSphereCenter) - sphere.w * sphere.w;
    float discriminant = b * b - c;

    // Check for intersections
    if (discriminant > 0.0) {
        float t = -b - sqrt(discriminant);
        
        // Ensure the intersection is in front of the ray origin
        if (t > 0.0) {
            // Calculate the normal at the intersection point
            normal = normalize((rayOrigin + t * rayDirection - sphere.xyz) / sphere.w);
            distance = t;
            return true;
        }
    }

    // No intersection found
    return false;
}

vec3 moon(vec3 rayDirection, vec3 skyColor, float time) {
    // Calculate the moon's directional vector
    float moonTime = time * 0.5;
    vec3 moonDirection = normalize(vec3(
        cos(moonTime * 0.1),
        0.8 * (0.6 + 0.5 * sin(-moonTime * 0.1)),
        sin(moonTime * 0.1)
    ));

    // Compute the moon's glow based on its direction
    float moonGlow = clamp(1.0782 * dot(moonDirection, rayDirection), 0.0, 2.0);
    vec3 color = skyColor + 0.43 * MOONLIGHTCOLOR * pow(moonGlow, 3.0);

    // Variables for intersection calculations
    float distance;
    vec3 normal;
    bool moonHit = false;

    // Check for intersection with the moon sphere
    if (intersectSphere(vec3(0.0), rayDirection, vec4(moonDirection, 0.03), distance, normal)) {
        // Calculate the lighting based on the normal at the intersection point
        float lightIntensity = dot(normalize(vec3(-moonDirection.x, 0.0, -moonDirection.z) + vec3(2.2, -1.6, 0.0)), normal);
        color += 3.0 * MOONLIGHTCOLOR * clamp(lightIntensity, 0.0, 1.0);
        moonHit = true;
    }

    // Blend the moonlight color with the sky color
    return mix(color, skyColor, (skyColor.x + skyColor.y + skyColor.z) / 1.5);
}


/*stars calculation*/
//
//	SimplexPolkaDot3D
//	polkadots over a simplex (tetrahedron) grid
//	Return value range of 0.0->1.0
//	

float SimplexPolkaDot3D(vec3 P, float density )
{
    //	calculate the simplex vector and index math
	vec3 Pn = P;
    //	simplex math constants
    float SKEWFACTOR = 1.0/3.0;
    float UNSKEWFACTOR = 1.0/6.0;
    float SIMPLEX_CORNER_POS = 0.5;
    float SIMPLEX_PYRAMID_HEIGHT = 0.70710678118654752440084436210485;	// sqrt( 0.5 )	height of simplex pyramid.

    Pn *= SIMPLEX_PYRAMID_HEIGHT;		// scale space so we can have an approx feature size of 1.0  ( optional )

    //	Find the vectors to the corners of our simplex pyramid
    vec3 Pi = floor( Pn + vec3(dot( Pn, vec3( SKEWFACTOR) ) ));
    vec3 x0 = Pn - Pi + vec3(dot(Pi, vec3( UNSKEWFACTOR ) ));
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = vec3(1.0) - g;
    vec3 Pi_1 = min( g.xyz, l.zxy );
    vec3 Pi_2 = max( g.xyz, l.zxy );
    vec3 x1 = x0 - Pi_1 + vec3(UNSKEWFACTOR);
    vec3 x2 = x0 - Pi_2 + vec3(SKEWFACTOR);
    vec3 x3 = x0 - vec3(SIMPLEX_CORNER_POS);

    //	pack them into a parallel-friendly arrangement
    vec4 v1234_x = vec4( x0.x, x1.x, x2.x, x3.x );
    vec4 v1234_y = vec4( x0.y, x1.y, x2.y, x3.y );
    vec4 v1234_z = vec4( x0.z, x1.z, x2.z, x3.z );

	vec3 gridcell = Pi;
	vec3 v1_mask = Pi_1;
	vec3 v2_mask = Pi_2;

    vec2 OFFSET = vec2( 50.0, 161.0 );
    float DOMAIN = 69.0;
    float SOMELARGEFLOAT = 6351.29681;
    float ZINC = 487.500388;

    //	truncate the domain
    gridcell.xyz = gridcell.xyz - floor(gridcell.xyz * ( 1.0 / DOMAIN )) * DOMAIN;
    vec3 gridcell_inc1 = step( gridcell, vec3( DOMAIN - 1.5 ) ) * ( gridcell + vec3(1.0) );

    //	compute x*x*y*y for the 4 corners
    vec4 Pp = vec4( gridcell.xy, gridcell_inc1.xy ) + vec4(OFFSET.xy, OFFSET.xy);
    Pp *= Pp;
    vec4 V1xy_V2xy = mix( vec4(Pp.xy, Pp.xy), vec4(Pp.zw, Pp.zw), vec4( v1_mask.xy, v2_mask.xy ) );		//	apply mask for v1 and v2
    Pp = vec4( Pp.x, V1xy_V2xy.x, V1xy_V2xy.z, Pp.z ) * vec4( Pp.y, V1xy_V2xy.y, V1xy_V2xy.w, Pp.w );

    vec2 V1z_V2z = vec2(gridcell_inc1.z);
	if (v1_mask.z <0.5) 
		V1z_V2z.x = gridcell.z;

	if (v2_mask.z <0.5) 
		V1z_V2z.y = gridcell.z;

	vec4 temp = vec4(SOMELARGEFLOAT) + vec4( gridcell.z, V1z_V2z.x, V1z_V2z.y, gridcell_inc1.z ) * ZINC;
    vec4 mod_vals =  vec4(1.0) / ( temp );

    //	compute the final hash
    vec4 hash = fract( Pp * mod_vals );

    //	apply user controls
    float INV_SIMPLEX_TRI_HALF_EDGELEN = 2.3094010767585030580365951220078;	// scale to a 0.0->1.0 range.  2.0 / sqrt( 0.75 )
    float radius = INV_SIMPLEX_TRI_HALF_EDGELEN;///(1.15-density);
    v1234_x *= radius;
    v1234_y *= radius;
    v1234_z *= radius;

    //	return a smooth falloff from the closest point.  ( we use a f(x)=(1.0-x*x)^3 falloff )
    vec4 pointDistance = max( vec4( 0.0 ), vec4(1.0) - ( v1234_x*v1234_x + v1234_y*v1234_y + v1234_z*v1234_z ) );
    pointDistance = pointDistance*pointDistance*pointDistance;
	vec4 b = (vec4(density)-hash)*(1.0/density);
	b = max(vec4(0.0), b);
	b = min(vec4(1.0), b);
	b = pow(b, vec4(1.0/density));
    return dot(b, pointDistance);
}

vec3 stars(vec3 normalizedDirection) {
    // Initialize color
    vec3 color = vec3(0.0);
    
    // Generate star intensity using Simplex noise
    float starIntensity = SimplexPolkaDot3D(normalizedDirection * 100.0, 0.15) 
                        + SimplexPolkaDot3D(normalizedDirection * 150.0, 0.25) * 0.7;
    
    // Base star color
    vec3 baseColor = vec3(0.05, 0.07, 0.1);
    
    // Adjust color based on star intensity and vertical direction
    color += baseColor + max(0.0, (starIntensity - smoothstep(0.2, 0.95, 0.5 - 0.5 * normalizedDirection.y)));
    color += baseColor * (1.0 - smoothstep(-0.1, 0.45, normalizedDirection.y));
    
    return color;
}
