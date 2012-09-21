#include "common/shaders/utility.frag"

#define KERNEL_SIZE 8

out vec4 FragmentColor;

void main()
{
	vec4 sum = vec4(0);
	vec2 texcoord = coord;
	int j;
	int i;

	for( i= -KERNEL_SIZE;i < KERNEL_SIZE; i++)
	{
		for (j = -KERNEL_SIZE; j < KERNEL_SIZE; j++)
		{
			sum += texture2D(image, coord + vec2(j, i)*0.004) * (0.25/KERNEL_SIZE);
		}
	}
	if (color.r > 0.9){
		FragmentColor = sum*sum*0.012 + color;
	}else{
		if (color.r < 0.025){
			FragmentColor = sum*sum*0.0025 + color;
		}else{
			FragmentColor = sum*sum*0.0055 + color;
		}
	}
}
