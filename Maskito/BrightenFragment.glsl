// Interpolated values from the vertex shaders
varying lowp vec2 UV;
varying lowp float Brighten;

// Values that stay constant for the whole mesh.
uniform sampler2D TextureSampler;
uniform lowp float minimum;
uniform lowp float maximum;
uniform highp float threshold;

void main(){
    lowp float dist = maximum - minimum;
    lowp float cutoff = minimum + dist * threshold;
    
    // Output color = color of the texture at the specified UV
    highp vec4 color = texture2D( TextureSampler, UV );
    
    highp float m = min(color.r, min(color.b, color.g));
    highp float M = max(color.r, max(color.b, color.g));
    highp float C = M - m;
    if (C > 0.0) {
        highp float v = (color.r + color.b + color.g) / 3.0;
        highp float S = C / v;
        highp float k = 0.0;
        highp float amount = min(1.0, max(0.0, (0.7 - S)));
        highp vec3 greyScaleFace =  color.rgb * (1.0 - Brighten) + vec3(1.0) * v * Brighten; // Fades from color at outermouth to greyscale by inner mouth
        color.rgb = (1.0 - amount) * color.rgb + amount * greyScaleFace; // the colourest thing is all colour, the least coloury thing is most greyscale
        color.rgb += 0.2 * Brighten * amount * color.rgb; // brighten by 10% when in a white pixel in the mouth.
    }
    gl_FragColor = color;
}