#include "common/shaders/utility.frag"

#ifdef GL_ES
precision mediump float;
#endif

// quantum mechanical wave function or whatever...

uniform vec2 mouse;

vec3 rgbgrain(vec2 seed, float division, float offset){
	return vec3(
					(rand(seed)/division)+offset,
					(rand(seed*rand(seed))/division)+offset,
					(rand(seed*rand(seed*rand(seed)))/division)+offset
	);
}

void main( void ) {

	vec2 posScale = vec2(resolution.y,resolution.x)/sqrt(resolution.x*resolution.y);
	vec2 position = (( gl_FragCoord.xy / resolution.xy ) );
	
	vec3 ssum = vec3(0.), csum = vec3(0.);
	
	float t = time * 0.37;
	
	for (float i = 0.; i < 10.; i++) {
		float i2 = i*.03;
		float c0 = cos(t+i2);
		float s0 = sin(t+i2);
		float c1 = cos(t*.9+i2*.9);
		float s1 = sin(t*.9+i2*.9);
		float c2 = cos(t*1.3+i2*1.3);
		float s2 = sin(t*1.3+i2*1.3);
		
		float x2 = ((c0+1.)*(s1+1.)+c2+5.)*.06;
		float y2 = ((s0+1.)*(-c1+1.)+s2+5.)*.06;
		float dx2 = (-s0*(s1+1.)+(c0+1.)*.9*c1-1.3*s2)*.06;
		float dy2 = (c0*(-c1+1.)+(s0+1.)*.9*s1+1.3*c2)*.06;

		vec2 p = position-vec2(x2,y2);
		float a = (p.x*dx2+p.y*dy2)*300.;
		vec3 av = a*(vec3(1.,1.1,1.2));
		csum += cos(av)*exp(-a*a*.01);
		ssum += sin(av)*exp(-a*a*.01);

	}
	
	gl_FragColor = vec4(vec3(24/255.0, 20/255.0, 26/255.0)+(sqrt(csum*csum+ssum*ssum)*0.01)*rgbgrain(gl_FragCoord.xy*time, 10.5, 1.0), 1.0 );
}