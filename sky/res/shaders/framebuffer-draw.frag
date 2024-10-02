#version 460

uniform sampler2D u_Fbo;
out vec4 color;

void main()
{
	vec2 uv = floor(gl_FragCoord.xy);
	vec4 col = texture(u_Fbo, uv.xy);
	color.rgba = vec4(col.rgb, 1.0);
}