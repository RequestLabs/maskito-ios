/* Gaussian Vertical*/
uniform float Scale;

attribute vec4 Position;
attribute vec2 TexSource;
attribute float InAlpha;
varying float Alpha;
varying vec2 UV;

varying vec2 v_blurTexCoords[2];

void main()
{
    gl_Position = Position;
    UV = TexSource;
    Alpha = InAlpha;
    v_blurTexCoords[ 0] = UV + vec2(0.0, -Scale);
    v_blurTexCoords[ 1] = UV + vec2(0.0,  Scale);
}