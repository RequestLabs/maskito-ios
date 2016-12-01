attribute vec4 Position;
attribute vec2 TexSource;
attribute float InAlpha;
varying float Alpha;
varying vec2 UV;


void main(void) {
    UV = TexSource;
    Alpha = InAlpha;
    gl_Position = Position;
}