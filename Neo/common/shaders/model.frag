#version 130
#extension GL_EXT_gpu_shader4 : enable

in vec3 vertex_light_position;
in vec3 vertex_normal;
in vec2 vTexCoord;

uniform sampler2D sceneTex;
uniform sampler2D AmbientOcclusion;
uniform sampler2D sceneDepthTex;
uniform float time;

vec2 rand2D(vec2 co){
	// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
	return
	vec2(fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453),
		fract(cos(dot(co.xy ,vec2(4.898,7.23))) * 23421.631));
}

float rand(vec2 co){
	return rand2D(co).x;
}

vec3 crosshatch(){
	float val = 0.0;
	if(int(gl_FragCoord.y)%2 == 0 && int(gl_FragCoord.x)%2 == 0){
		val = 1.0;
	}else if(int(gl_FragCoord.y)%2 != 0 && int(gl_FragCoord.x)%2 != 0){
		val = 1.0;
	}
	return vec3(0.0, val/1.4, val/1.2);
}

void main()
{
	vec2 uv = gl_TexCoord[0].xy;

	/*float diffuse_value = max(dot(vertex_normal, vertex_light_position), 0.0);
	float fDepth = 1.0 - (gl_FragCoord.z / gl_FragCoord.w) / 100.0;
    float ambientOcclusion = texture2D(AmbientOcclusion, vTexCoord).r;*/
	gl_FragColor = gl_Color+vec4(crosshatch(), 1.0);
}