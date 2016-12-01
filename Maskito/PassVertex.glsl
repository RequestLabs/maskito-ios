attribute vec4 Position;
attribute vec2 TexSource;
attribute float InAlpha;
varying vec2 UV;
varying float Alpha;

void main(void) {
    UV = TexSource;
    Alpha = InAlpha;
    gl_Position = Position;
}