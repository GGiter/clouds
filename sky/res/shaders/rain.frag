#version 460
out vec4 FragColor;

in vec2 TexCoords;

uniform float u_Time;
uniform vec2 u_resolution;
uniform sampler2D texture_diffuse1;
uniform sampler2D u_depthTexture;

#define M_PI 3.1415926535897932384626433832795

void main()
{    
    vec2 movedTexCoords = TexCoords;
    movedTexCoords.y += u_Time * 1.0;
    vec2 speed = vec2(1., 1.);
    vec2 scale = vec2(1,1) * 0.001;
    vec2 SinT = sin(u_Time * 2.0f * M_PI / speed.xy) * scale.xy;
    // rotate and scale UV
    vec4 Cosines = vec4(cos(SinT), sin(SinT));
    vec2 CenteredUV = movedTexCoords.xy - vec2(0.5, 0.5);
    vec4 RotatedUV = vec4(dot(Cosines.xz*vec2(1,-1), CenteredUV)
                            , dot(Cosines.zx, CenteredUV)
                            , dot(Cosines.yw*vec2(1,-1), CenteredUV)
                             , dot(Cosines.wy, CenteredUV)) + vec4(0.5);


    vec4 Texture1 = texture(texture_diffuse1, RotatedUV.xy).rgba;
    vec4 Texture2 = texture(texture_diffuse1, RotatedUV.zw).rgba;
    vec4 Values = vec4(Texture1.a, Texture1.a, Texture2.a, Texture2.a);

    vec2 ScreenUV = (gl_FragCoord.xy / u_resolution) * vec2(1.0,0.56);

    float Depth = texture(u_depthTexture, ScreenUV).a;
    if(Depth <= 0.01)
        Depth = gl_FragCoord.z;
    vec4 RainOpacities = vec4(1.0);
    float RainDepthStart = 0.;
    float RainDepthRange = 0.7;
    vec4 Mask = RainOpacities * clamp(((Depth - RainDepthStart) / RainDepthRange), 0, 1);

    float RainColor = dot(Values, Mask);
    RainColor = dot(Values, Mask * 0.09);
    FragColor = vec4(RainColor);
    FragColor.rgb = vec3(1);
    if( texture(u_depthTexture, ScreenUV).r >= 0.3)
            FragColor. a = 0.0;
}