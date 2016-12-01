// Interpolated values from the vertex shaders
varying lowp vec2 UV;
varying lowp float Alpha;

// Values that stay constant for the whole mesh.
uniform sampler2D TextureSampler;
uniform lowp float scale;

void main(){
    
    // Output color = color of the texture at the specified UV
    highp vec4 color = texture2D( TextureSampler, UV );
    
    highp float m = min(color.r, min(color.b, color.g));
    highp float M = max(color.r, max(color.b, color.g));
    highp float C = M - m;
    if (C > 0.0) {
        
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(color.bg, K.wz), vec4(color.gb, K.xy), step(color.b, color.g));
        highp vec4 q = mix(vec4(p.xyw, color.r), vec4(color.r, p.yzx), step(p.x, color.r));
        
        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        highp vec3 hsv = vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        
        if ((hsv.r > (45.0 / 360.0)) && (hsv.r < (75.0 / 360.0))) {
            highp float v = (color.r + color.b + color.g) / 3.0;
            highp float S = C / v;
            highp float k = 0.0;
            highp float amount = min(1.0, max(0.0, 1.0 - S));
            highp float amount_exp = exp( - 0.5*(amount - 1.0)*(amount - 1.0) / 0.01);
            color.rgb += amount_exp * scale * v;
        }
    }
    gl_FragColor = color;
}