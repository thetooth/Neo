#include "common/shaders/utility.frag"

out vec4 FragmentColor;

void main(void)
{
	vec2 uv = coord;
	coord.x += sin(uv.y * 3.0 * 2.0 * 3.14159 + (time/500.0)) / 100.0;
	coord.y += sin(uv.x * 3.0 * 2.0 * 3.14159 + (time/500.0)) / 100.0;
	FragmentColor = texture2D(image, coord);
}
