#version 460
#pragma name FSR_EASU
// FSR - [EASU] EDGE ADAPTIVE SPATIAL UPSAMPLING

in vec2 v_TexCoord;
layout(location = 0) out vec4 FragColor;
uniform sampler2D Source;
uniform vec2 SourceSize;
uniform vec2 OutputSize;

#define A_GPU 1
#define A_GLSL 1
#include "include/ffx_a.h"

#define FSR_EASU_F 1
AU4 con0, con1, con2, con3;

AF4 FsrEasuRF(AF2 p) { return textureGather(Source, p, 0); }
AF4 FsrEasuGF(AF2 p) { return textureGather(Source, p, 1); }
AF4 FsrEasuBF(AF2 p) { return textureGather(Source, p, 2); }

#include "include/ffx_fsr1.h"

void main() {
    FsrEasuCon(con0, con1, con2, con3,
        SourceSize.x, SourceSize.y,  // Viewport size (top left aligned) in the input image which is to be scaled.
        SourceSize.x, SourceSize.y,  // The size of the input image.
        OutputSize.x, OutputSize.y); // The output resolution.

    AU2 gxy = AU2(v_TexCoord.xy * OutputSize.xy); // Integer pixel position in output.
    AF3 Gamma2Color = AF3(0, 0, 0);
    FsrEasuF(Gamma2Color, gxy, con0, con1, con2, con3);

    FragColor = vec4(Gamma2Color, 1.0);
}
