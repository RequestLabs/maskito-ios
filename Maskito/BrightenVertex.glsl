attribute vec4 Position;
attribute vec2 TexSource;
attribute float InBrighten;
varying float Brighten;
varying vec2 UV;


void main(void) {
    UV = TexSource;
    Brighten = InBrighten;
    gl_Position = Position;
}