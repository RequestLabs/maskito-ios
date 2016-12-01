/* Gaussian Horizontal*/
uniform float Scale;

attribute vec4 Position;
attribute vec2 TexSource;
attribute float InAlpha;
varying float Alpha;
varying vec2 UV;

varying vec2 v_blurTexCoords[14];

void main()
{
    gl_Position = Position;
    UV = TexSource;
    Alpha = InAlpha;
    v_blurTexCoords[ 0] = UV + vec2(-0.028 * Scale, 0.0);
    v_blurTexCoords[ 1] = UV + vec2(-0.024 * Scale, 0.0);
    v_blurTexCoords[ 2] = UV + vec2(-0.020 * Scale, 0.0);
    v_blurTexCoords[ 3] = UV + vec2(-0.016 * Scale, 0.0);
    v_blurTexCoords[ 4] = UV + vec2(-0.012 * Scale, 0.0);
    v_blurTexCoords[ 5] = UV + vec2(-0.008 * Scale, 0.0);
    v_blurTexCoords[ 6] = UV + vec2(-0.004 * Scale, 0.0);
    v_blurTexCoords[ 7] = UV + vec2( 0.004 * Scale, 0.0);
    v_blurTexCoords[ 8] = UV + vec2( 0.008 * Scale, 0.0);
    v_blurTexCoords[ 9] = UV + vec2( 0.012 * Scale, 0.0);
    v_blurTexCoords[10] = UV + vec2( 0.016 * Scale, 0.0);
    v_blurTexCoords[11] = UV + vec2( 0.020 * Scale, 0.0);
    v_blurTexCoords[12] = UV + vec2( 0.024 * Scale, 0.0);
    v_blurTexCoords[13] = UV + vec2( 0.028 * Scale, 0.0);
}