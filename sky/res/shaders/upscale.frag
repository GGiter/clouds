#version 460

uniform sampler2D u_Fbo;
uniform vec2 u_Resolution;
uniform float u_DownscaleFactor;
out vec4 color;

void main()
{
	/* downscale  UV by given factor */
	vec2 uv = floor(gl_FragCoord.xy / u_DownscaleFactor);
	uv = uv / (u_Resolution / u_DownscaleFactor);
	vec4 col = texture(u_Fbo, uv.xy);
	color.rgba = col.rgba;
}