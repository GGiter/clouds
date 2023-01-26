#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 v_TexCoord;
out vec2 v_SkyTexCoord;
out vec3 v_Position;
uniform vec3 u_Translation;

/* Helper function used to remap values from one range to another */
float remap(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main()
{
	/* block zooming in to 4.0 */
	float scale = ((u_Translation.z + 4.0)/2.0);
	if(scale > 4.0)
	{
		scale = 4.0;
	}
	v_TexCoord = texCoord;

	/* sky position */
	vec2 skyTextureTranslation = vec2(u_Translation.x, u_Translation.y);
	skyTextureTranslation *= 0.5;
	skyTextureTranslation *= scale;
	v_SkyTexCoord.x = remap(texCoord.x, 0.0, 1.0, remap(scale, 1, 4, 0.5 + + skyTextureTranslation.x, 0.0), remap(scale, 1, 4, 0.75 +  skyTextureTranslation.x, 1.0)); 
	v_SkyTexCoord.y = remap(texCoord.y, 0.0, 1.0, remap(scale, 1, 4, 0.375 + skyTextureTranslation.y, 0.0), remap(scale, 1, 4, 0.625 + skyTextureTranslation.y, 1.0)); 

	/* cloud position */
	gl_Position =  vec4(position + vec3(u_Translation.x, u_Translation.y, 0), 1.) * vec4(scale, scale, scale, 1);
	v_Position = vec3(gl_Position);
};