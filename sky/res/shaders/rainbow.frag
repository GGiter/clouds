/* RAINBOW */
#define RAINBOW_START_Y					0.0
const float RAINBOW_BRIGHTNESS  		= 1.5;
const float RAINBOW_INTENSITY   		= 0.50;
const vec3  RAINBOW_COLOR_RANGE 		= vec3(50.0, 51.0, 52);  // The angle for red, green and blue
vec3 RAINBOW_POS						= vec3(10004.5, -1800.0, 2.5);
vec3 RAINBOW_DIR 						= vec3(-0.2, -0.1, 0.0);

// Local variables
vec3 rainbow_pos;
vec3 rainbow_camera_dir;
vec3 rainbow_up;
vec3 rainbow_vertical;
vec3 rainbow_w;

// Smoothstep function
vec3 smoothstepColor(vec3 edge0, vec3 edge1, vec3 x) {
    return smoothstep(edge0, edge1, x);
}

// Rainbow color calculation
vec3 rainbowColor(in vec3 ray_dir) {
    vec3 normalized_dir = normalize(RAINBOW_DIR);
    float theta = degrees(acos(dot(normalized_dir, ray_dir)));
    vec3 nd = clamp(1.0 - abs((RAINBOW_COLOR_RANGE - theta) * 0.5), 0.0, 1.0);
    vec3 color = smoothstepColor(vec3(0.0), vec3(1.0), nd) * RAINBOW_INTENSITY;
    
    return color * max((RAINBOW_BRIGHTNESS - 0.75) * 1.5, 0.0);
}

// Setup rainbow parameters
void rainbowSetup() {
    rainbow_pos = RAINBOW_POS;
    rainbow_w = -normalize(-rainbow_pos);
    rainbow_up = normalize(cross(rainbow_w, vec3(0,1,0)));
    rainbow_vertical = normalize(cross(rainbow_up, rainbow_w));
}

// Function to check if a position is inside a bounding box
bool isInsideBox(vec3 position, vec3 minCorner, vec3 maxCorner) {
    return all(greaterThanEqual(position, minCorner)) && all(lessThanEqual(position, maxCorner));
}

// Main function to compute rainbow color
vec4 rainbow(vec2 fragCoord, vec3 worldPos, vec3 cameraPos) {
    // Initialize the color
    vec3 color = vec3(0.0);

    // Compute the ray direction from the camera to the fragment's world position
    vec3 ray_world = normalize(worldPos - cameraPos);  // Ray from the camera to the world position

    // Setup rainbow parameters (fixed in world space)
    rainbowSetup();

    // Calculate the direction for the rainbow, based on fixed rainbow orientation in world space
    vec3 wdDir = normalize(ray_world.x * rainbow_up + ray_world.y * rainbow_vertical - ray_world.z * rainbow_w);

    // Check if the fragment's world position is within the bounding box and above the rainbow's starting Y
    color += rainbowColor(wdDir);

     // Calculate alpha based on color intensity (you can adjust this formula)
    float alpha = max(color.r, max(color.g, color.b)); // Set alpha based on the maximum color intensity

    // Return the final color, clamped to [0, 1]
    return vec4(clamp(color, 0.0, 1.0), alpha);
}