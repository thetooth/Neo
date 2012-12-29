#pragma once

#include "masternoodles.h"
#include "platformer.h"
#include "console.h"

using namespace klib;
namespace NeoPlatformer {
	struct System
	{
		// Graphics
		KLGL *gc;
		KLGLTexture *charmapTexture, *charmapTexture2, *titleTexture, *loadingTexture, *testTexture, *startupTexture, *warningTexture, *creditsTexture;
		KLGLSprite *loaderSprite;
		KLGLFont *font, *numberFont;
		// Audio
		Audio *audio;
	};
	struct Neo
	{
		// Neo System
		int mode, paused, consoleInput;
		Environment *gameEnv;
		Console *console;
		UIDialog *dialog;
	};
	/*struct FS
	{
		// Miniz
		mz_bool status;
		size_t uncomp_size;
		mz_zip_archive zip_archive;
		void *p;
	};*/
	int initMain(KLGL* gc);
}