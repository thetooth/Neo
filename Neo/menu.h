#pragma once
#include "UI.h"

namespace NeoPlatformer {
	class Menu : public klib::UI
	{
	public:
		Menu(void);
		~Menu(void);

		void draw();
	};
}

