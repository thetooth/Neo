uniform float time;
uniform sampler2D bgl_RenderedTexture;
float scale = 2.0;

float rand(vec2 co){
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float find_closest(int x, int y, float c0)
{
vec4 dither[4];

dither[0] = vec4( 1.0, 33.0, 9.0, 41.0);
dither[1] = vec4(49.0, 17.0, 57.0, 25.0);
dither[2] = vec4(13.0, 45.0, 5.0, 37.0);
dither[3] = vec4(61.0, 29.0, 53.0, 21.0);

float limit = 0.0;
if(x < 4)
{
limit = (dither[x][y]+1.0)/64.0;
}

if(c0 < limit)
{
return 0.0;

}else{

return 1.0;
}

}


void main(void)
{
float grain = (rand(texture2D(bgl_RenderedTexture, gl_TexCoord[0].xy).gb)/10.0)+1.0;
vec4 lum = vec4(0.299, 0.587, 0.114, 0.0);
float grayscale = dot(texture2D(bgl_RenderedTexture, gl_TexCoord[0].xy), lum);
vec3 rgb = grain*texture2D(bgl_RenderedTexture, gl_TexCoord[0].xy).rgb;

vec2 xy = gl_FragCoord.xy * scale;
int x = int(mod(xy.x, 4.0));
int y = int(mod(xy.y, 4.0));

vec3 finalRGB;

finalRGB.r = find_closest(x, y, rgb.r/2.0);
finalRGB.g = find_closest(x, y, rgb.g/2.0);
finalRGB.b = find_closest(x, y, rgb.b);

float final = find_closest(x, y, grayscale);

gl_FragColor = vec4(finalRGB, 1.0);
}