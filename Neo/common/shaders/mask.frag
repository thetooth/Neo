#include "common/shaders/utility.frag"

out vec4 FragmentColor;

uniform sampler2D tileTexture;
uniform sampler2D maskTexture;
uniform vec4 tileRect;

void main(){
	float alpha = 0.0;
	vec2 tileCoord;
	tileCoord.x = mod(gl_FragCoord.x+(time/24.0), 32.0)/32.0;
	tileCoord.y = mod(gl_FragCoord.y+(time/24.0), 32.0)/32.0;
	vec4 tileColor = texture2D(tileTexture, tileCoord+(tileRect.xy/resolution));
	vec4 maskColor = texture2D(maskTexture, vec2(gl_FragCoord.x/resolution.x, (gl_FragCoord.y)/resolution.y));
	alpha = (maskColor.r+maskColor.g+maskColor.b)/3.0;
	FragmentColor = mix(color, tileColor, alpha);
}