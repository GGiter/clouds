#version 460
#pragma name FSR_RCAS

// FSR - [RCAS] ROBUST CONTRAST ADAPTIVE SHARPENING

const float FSR_SHARPENING = 0.2;
const float FSR_FILMGRAIN = 0.3;
const float FSR_GRAINCOLOR = 1.0;
const float FSR_GRAINPDF = 0.3;

in vec2 v_TexCoord;
layout(location = 0) out vec4 FragColor;
uniform sampler2D Source;
uniform vec2 OutputSize;
uniform int FrameCount;

#define A_GPU 1
#define A_GLSL 1
#include "include/ffx_a.h"


#define FSR_RCAS_F 1
AU4 con0;

AF4 FsrRcasLoadF(ASU2 p) { return AF4(texelFetch(Source, p, 0)); }
void FsrRcasInputF(inout AF1 r, inout AF1 g, inout AF1 b) {}

#include "include/ffx_fsr1.h"

// prng: A simple but effective pseudo-random number generator [0;1[
float prng(vec2 uv, float time) {
    return fract(sin(dot(uv + fract(time), vec2(12.9898, 78.233))) * 43758.5453);
}

// pdf: [-0.5;0.5[
// Removes noise modulation effect by reshaping the uniform/rectangular noise
// distribution (RPDF) into a Triangular (TPDF) or Gaussian Probability Density
// Function (GPDF).
// shape = 1.0: Rectangular
// shape = 0.5: Triangular
// shape < 0.5: Gaussian (0.2~0.4)
float pdf(float noise, float shape) {
    float orig = noise * 2.0 - 1.0;
    noise = pow(abs(orig), shape);
    noise *= sign(orig);
    noise -= sign(orig);
    return noise * 0.5;
}

void main() {
    FsrRcasCon(con0, FSR_SHARPENING);

    AU2 gxy = AU2(v_TexCoord.xy * OutputSize.xy); // Integer pixel position in output.
    AF3 Gamma2Color = AF3(0, 0, 0);
    FsrRcasF(Gamma2Color.r, Gamma2Color.g, Gamma2Color.b, gxy, con0);

    // FSR - [LFGA] LINEAR FILM GRAIN APPLICATOR
    if (FSR_FILMGRAIN > 0.0) {
        if (FSR_GRAINCOLOR == 0.0) {
            float noise = pdf(prng(v_TexCoord, FrameCount * 0.11), FSR_GRAINPDF);
            FsrLfgaF(Gamma2Color, vec3(noise), FSR_FILMGRAIN);
        } else {
            vec3 rgbNoise = vec3(
                pdf(prng(v_TexCoord, FrameCount * 0.11), FSR_GRAINPDF),
                pdf(prng(v_TexCoord, FrameCount * 0.13), FSR_GRAINPDF),
                pdf(prng(v_TexCoord, FrameCount * 0.17), FSR_GRAINPDF)
            );
            FsrLfgaF(Gamma2Color, rgbNoise, FSR_FILMGRAIN);
        }
    }

    FragColor = vec4(Gamma2Color, 1.0);
}
