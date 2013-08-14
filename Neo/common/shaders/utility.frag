#version 330

uniform float time;
uniform vec2 resolution = vec2(1024.0f, 1024.0f);
uniform sampler2D image;

vec2 coord = vec2(gl_FragCoord.x/resolution.x, (-gl_FragCoord.y+resolution.y)/resolution.y);
vec4 color = texture2D(image, coord);

vec2 rand2D(vec2 co){
	// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
	return
	vec2(fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453),
		fract(cos(dot(co.xy ,vec2(4.898,7.23))) * 23421.631));
}

float rand(vec2 co){
	return rand2D(co).x;
}