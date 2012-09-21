#version 130

#define M_PI 3.14159265358979323846

uniform sampler2D tex0;
uniform float time;
uniform vec2 resolution;
uniform vec2 position;
uniform float radius;
uniform float blendingDivision;
const mat2 rotation = mat2( cos(M_PI/4.0), sin(M_PI/4.0),
                           -sin(M_PI/4.0), cos(M_PI/4.0));

vec4 circle(vec4 fragColor, vec2 pos, float radi, vec4 color){
	return mix(color, fragColor, smoothstep(0.25, 420.25, dot(pos/radi, pos)));
}

void main(void){
	vec2 pos = mod(gl_FragCoord.xy, vec2(resolution));
	vec4 color = texture2D(tex0, gl_TexCoord[0].xy);
    gl_FragColor = color+(circle(vec4(0.0), pos + vec2(position), radius, vec4(1.0, 1.0, 1.0, 1.0))/blendingDivision);
}