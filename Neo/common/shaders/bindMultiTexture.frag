#include "common/utility.frag"

uniform sampler2D slave;
out vec4 FragmentColor;

void main()
{
	FragmentColor = vec4(color.rgb+texture2D(slave, coord).rgb, 1.0);
}