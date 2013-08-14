// Copyright 2005-2011 The Department of Redundancy Department.

#include "masternoodles.h"
#include "neo.h"
#include "platformer.h"
#include "console.h"

//#include "miniz.c"

#ifdef _WIN32
#ifdef KLGLENV64
#pragma comment(lib,"kami64.lib")
#pragma comment(lib,"fmodex64_vc.lib")
#else
#pragma comment(lib,"kami.lib")
#pragma comment(lib,"fmodex_vc.lib")
#pragma comment(lib,"python3.lib")
#endif
#endif

using namespace klib;
using namespace NeoPlatformer;

void Loading(KLGL *gc, KLGLTexture *loading);
void Warning(KLGL *gc);

int main(int argc, char **argv){
	int quit = 1;
	char inputBuffer[256] = {};
	char textBuffer[4096] = {};

	enum GameMode {_MENU, _MAPLOAD, _MAPDESTROY, _INGAME, _CREDITS};

	System sys;
	Neo neo;

	neo.consoleInput =	0;
	neo.mode =			GameMode::_MENU;
	neo.paused =		0;

	bool internalTimer = false, mapScroll = false;
	int frame = 0,fps = 0,cycle = 0, th_id = 0, nthreads = 0, qualityPreset = 0, shaderAlliterations = 0, scrollPos = INT_MAX;
	float tweenX = 0, tweenY = 0, titleFade = 0;
	Point<int> mouseXY, mouseXY_prev;
	clock_t t0 = clock(),t1 = 0,t2 = 0;
	char wTitle[256] = {};

	Point<int> *stars[1000] = {};

	tween::Tweener tweener;
	tween::TweenerParam titleFaderTween(2000, tween::EXPO, tween::EASE_OUT);

	t0 = clock();

	try {
		// Init display
		sys.gc = new KLGL("Neo", APP_SCREEN_W, APP_SCREEN_H, 60, false);

		// Init filesystem
		/*memset(&zip_archive, 0, sizeof(zip_archive));
		status = mz_zip_reader_init_file(&zip_archive, "common.zip", 0);
		if (!status){
		cl("mz_zip_reader_init_file() failed!\n");
		}else{
		cl("Initialized compressed filesystem(Miniz %s), using it...\n", MZ_VERSION);
		// Try to extract to the heap.
		char* fname = "common/sounds/ambient.mp3";
		p = mz_zip_reader_extract_file_to_heap(&zip_archive, fname, &uncomp_size, 0);
		if (!p){
		printf("mz_zip_reader_extract_file_to_heap() failed!\n");
		mz_zip_reader_end(&zip_archive);
		return EXIT_FAILURE;
		}
		cl("Successfully extracted file \"%s\", size %u\n", fname, (unsigned int)uncomp_size);
		// We're done.
		free(p);
		mz_zip_reader_end(&zip_archive);
		}*/
		/*unsigned char *testIn = new unsigned char[4096];
		unsigned char *testOut = new unsigned char[4096];
		unsigned char *testOut2 = new unsigned char[4096];
		unsigned long destLen;
		unsigned long destLen2;
		strncpy((char*)testIn, JSONData_dump(sys.gc->config->data).c_str(), 4096);
		cl("Source: %s\n", testIn);
		mz_compress(testOut, &destLen, testIn, strlen((char*)testIn)+1);
		cl("Compressed: %s\n", testOut);
		mz_uncompress(testOut2, &destLen2, testOut, destLen);
		cl("Decompressed: %s\n", testOut2);*/

		// Python
		PyImport_AppendInittab("emb", NeoPython::PyInit_main);
		PyImport_AppendInittab("neo", PyInit_neo);
		Py_Initialize();
		PyImport_ImportModule("emb");
		NeoPython::stdout_write_type write = [&] (std::string s) { cl(s.c_str()); };
		NeoPython::set_stdout(write);
		PyRun_SimpleString("import platform; import neo; neo.cl(\"Initialized Python {0}\\n\".format(platform.python_version()))");

		//GetCursorPos(&mouseXY);
		//ScreenToClient(sys.gc->windowManager->wm->hWnd, &mouseXY);
		//mouseXY_prev = mouseXY;

		// Loading screen and menu buffer
		sys.warningTexture = new KLGLTexture("common/textures/warning.png");
		sys.startupTexture = new KLGLTexture("common/textures/startup.png");
		sys.creditsTexture = new KLGLTexture("common/textures/credits.png");

		// Init sound
		sys.audio = new Audio();
		sys.audio->loadSound("common/sounds/startup.ogg", 10, FMOD_LOOP_OFF);
		sys.audio->loadSound("common/sounds/8bp123-01-jellica-dennye.mp3", 0, FMOD_LOOP_NORMAL);

		// :3
		Loading(sys.gc, sys.startupTexture);
		sys.audio->system->playSound(FMOD_CHANNEL_FREE, sys.audio->sound[10], false, &sys.audio->channel[0]);

		// Configuration values
		internalTimer		= sys.gc->config->GetBoolean("neo", "useInternalTimer", true);
		qualityPreset		= sys.gc->config->GetInteger("neo", "qualityPreset", 1);
		shaderAlliterations	= sys.gc->config->GetInteger("neo", "blurAlliterations", 4);

		// Textures
		sys.charmapTexture  = new KLGLTexture("common/textures/internalfont.png");
		sys.charmapTexture2  = new KLGLTexture("common/textures/numbers.png");
		sys.testTexture = new KLGLTexture("common/textures/test.png");
		sys.loadingTexture = new KLGLTexture("common/textures/loading.png");
		sys.titleTexture = new KLGLTexture("common/textures/neo-logo.png");

		// Sprites
		//loaderSprite = new KLGLSprite(testTexture, 64, 64);

		// UI
		neo.dialog = new UIDialog(sys.gc, "common/textures/UI_Interface.png");
		neo.dialog->bgColor = new KLGLColor(51, 51, 115, 255);
		neo.dialog->pos.width = 512;
		neo.dialog->pos.height = 256;
		neo.dialog->pos.x = (sys.gc->buffer.width/2)-(neo.dialog->pos.width/2)-8;
		neo.dialog->pos.y = (sys.gc->buffer.height/2)-(neo.dialog->pos.height/2)-8;

		// Fonts
		sys.font = new KLGLFont(sys.charmapTexture->gltexture, sys.charmapTexture->width, sys.charmapTexture->height, 8, 8, -1);
		sys.numberFont = new KLGLFont(sys.charmapTexture2->gltexture, sys.charmapTexture2->width, sys.charmapTexture2->height, 5, 5, '0');

		// Game Console
		neo.console = new Console(4096);

		// Default shaders
		/*gc->InitShaders(0, 0, 
		"common/shaders/postDefaultV.glsl",
		"common/shaders/test.frag"
		);*/
		sys.gc->InitShaders(5, 0, 
			"common/shaders/postDefaultV.glsl",
			"common/shaders/waves.frag"
			);
		sys.gc->InitShaders(6, 0, 
			"common/shaders/model.vert",
			"common/shaders/model.frag"
			);
		sys.gc->InitShaders(7, 0, 
			"common/shaders/postDefaultV.glsl",
			"common/shaders/cubes.frag"
			);

	}catch(KLGLException e){
		quit = 1;
		Warning(sys.gc);
		if (KLGLDebug){
		}
	}

	cl("\nNeo %s R%d\n", NEO_VERSION, NEO_BUILD_VERSION);
	if (neo.consoleInput)
	{
		neoCallbackList["mode"] = &neo.mode;
		cl("Python Console Enabled!\n\n");
	}

	if (!quit){
		cl("An important resource is missing or failed to load, check the console window _now_ for more details.");
		if(KLGLDebug){
			quit = 1;
		}
	}

	while (clock()-t0 < 2500 && !KLGLDebug){
		// Ain't no thing
		#ifdef _WIN32
		Sleep(10);
		#else
		sleep(10);
		#endif
	}

	sys.audio->system->playSound(FMOD_CHANNEL_FREE, sys.audio->sound[0], false, &sys.audio->channel[1]);
	//audio->channel[1]->setVolume(0.0f);

	tweener.step(clock());
	titleFaderTween.addProperty(&titleFade, 255);
	tweener.addTween(&titleFaderTween);

	while (sys.gc->ProcessEvent(&quit))
	{
		t0 = clock();

		sys.gc->HandleEvent([&] (std::vector<klib::KLGLKeyEvent>::iterator key) {
			if (key->isDown()){
				if (KLGLDebug){
					cl("Key: %x, %c\n", key->getCode(), key->getChar());
				}
				// Application wide events
				switch (key->getCode()){
				case klib::KLGLKeyEvent::KEY_ESCAPE:
					quit = 0;
					break;
				case klib::KLGLKeyEvent::KEY_d:
					KLGLDebug = !KLGLDebug;
					break;
				}

				// Context sensitive
				switch(neo.mode){
				case GameMode::_MENU:
					/*if(neo.consoleInput){
					if ((sys.gc->windowManager->wm->msg.wParam>=32) && (sys.gc->windowManager->wm->msg.wParam<=255) && (sys.gc->windowManager->wm->msg.wParam != '|')) {
					if (strlen(inputBuffer) < 255) {
					//cl("key: 0x%x\n", graphicsContext->msg.wParam);
					char buf[3];
					sprintf(buf, "%c\0", vktochar(sys.gc->windowManager->wm->msg.wParam));
					strcat(inputBuffer, buf);

					}
					} else if (sys.gc->windowManager->wm->msg.wParam == klib::KLGLKeyEvent::KEY_BACK) {
					if(strlen(inputBuffer) > 0){
					inputBuffer[strlen(inputBuffer)-1]='\0';
					}
					} else if (sys.gc->windowManager->wm->msg.wParam == klib::KLGLKeyEvent::KEY_TAB) {
					inputBuffer[strlen(inputBuffer)]='\t';
					} else if (sys.gc->windowManager->wm->msg.wParam == klib::KLGLKeyEvent::KEY_RETURN) {
					if(strlen(inputBuffer) > 0){
					if (strcmp(inputBuffer, "exit") == 0){
					PostQuitMessage(0);
					quit = true;
					}else{
					neo.console->input(inputBuffer);
					}
					}else{
					cl("-- RETURN\n", 13);
					neo.mode = GameMode::_MAPLOAD;
					}
					fill_n(inputBuffer, 256, '\0');
					}
					} else */
					if (key->getCode() == klib::KLGLKeyEvent::KEY_RETURN) {
						neo.mode = GameMode::_MAPLOAD;
					} else if (key->getCode() == klib::KLGLKeyEvent::KEY_e) {
						neo.mode = GameMode::_CREDITS;
					}
					break;
				case GameMode::_INGAME:
					switch (key->getCode()){
					case klib::KLGLKeyEvent::KEY_RETURN:
						neo.mode = GameMode::_MAPDESTROY;
						break;
					case klib::KLGLKeyEvent::KEY_t:
						internalTimer = !internalTimer;
						break;
					case klib::KLGLKeyEvent::KEY_v:
						sys.gc->vsync = !sys.gc->vsync;
						//SetVSync(sys.gc->vsync);
						break;
					case klib::KLGLKeyEvent::KEY_g:
						if (qualityPreset < 3)
						{
							qualityPreset++;
						}else{
							qualityPreset = 0;
						}
						break;
					case klib::KLGLKeyEvent::KEY_r:
						neo.mode = GameMode::_MAPLOAD;
						break;
					case klib::KLGLKeyEvent::KEY_m:
						mapScroll = !mapScroll;
						break;
					case klib::KLGLKeyEvent::KEY_p:
						if (!neo.paused){
							titleFade = 0.0f;
							titleFaderTween.addProperty(&titleFade, 255);
							tweener.addTween(&titleFaderTween);
						}else{
							titleFade = 0.0f;
							tweener.removeTween(&titleFaderTween);
						}
						neo.paused = !neo.paused;
						break;
					case klib::KLGLKeyEvent::KEY_LEFT:
						neo.gameEnv->character->left();
						break;
					case klib::KLGLKeyEvent::KEY_RIGHT:
						neo.gameEnv->character->right();
						break;
					case klib::KLGLKeyEvent::KEY_UP:
						neo.gameEnv->character->jump();
						break;
					}
					break;
				}
			}else{
				if (key->getCode() == klib::KLGLKeyEvent::KEY_ESCAPE){
					quit = 0;
				}
				switch(neo.mode){
				case GameMode::_INGAME:
					switch (key->getCode()){
					case klib::KLGLKeyEvent::KEY_LEFT:
						neo.gameEnv->character->leftUp();
						break;
					case klib::KLGLKeyEvent::KEY_RIGHT:
						neo.gameEnv->character->rightUp();
						break;
					case klib::KLGLKeyEvent::KEY_UP:
						neo.gameEnv->character->jumpUp();
						break;
					}
					break;
				}
			}
			key->finished();
		});

		if (t0-t2 >= CLOCKS_PER_SEC){
			fps = frame;
			frame = 0;
			t2 = clock();

			// Title
			sprintf(wTitle, "Neo - FPS: %d", fps);
			//SetWindowText(sys.gc->windowManager->wm->hWnd, wTitle);

			// Update graphics quality
			sys.gc->BindShaders(1);
			glUniform1i(glGetUniformLocation(sys.gc->GetShaderID(1), "preset"), qualityPreset);
			sys.gc->UnbindShaders();
		}else if (t0-t1 >= CLOCKS_PER_SEC/sys.gc->fps || !internalTimer){

			//GetCursorPos(&mouseXY);
			//ScreenToClient(sys.gc->windowManager->wm->hWnd, &mouseXY);

			sys.audio->system->update();

			switch(neo.mode){
			case GameMode::_MENU:											// ! Start menu

				sprintf(textBuffer, "@CFFFFFF@D%s\n\n%s\n>>> %s%c", clBuffer, neo.console->buffer, inputBuffer, (frame%16 <= 4 ? '_' : ' '));

				// Update clock
				tweener.step(t0);
				t1 = clock();

				sys.gc->OpenFBO(45, 0.0, 0.0, 0.0);
				sys.gc->OrthogonalStart();
				{
					// Reset pallet
					KLGLColor(255, 255, 255, 255).Set();

					sys.gc->OrthogonalStart();

					/*gc->BindShaders(5);
					glUniform1f(glGetUniformLocation(gc->GetShaderID(5), "time"), cycle/40.0f);
					glUniform2f(glGetUniformLocation(gc->GetShaderID(5), "resolution"), gc->buffer.width, gc->buffer.height);
					gc->BindMultiPassShader(5, 1, false);
					gc->UnbindShaders();
					gc->BindShaders(7);
					glUniform1f(glGetUniformLocation(gc->GetShaderID(7), "time"), cycle/100.0f);
					glUniform2f(glGetUniformLocation(gc->GetShaderID(7), "resolution"), gc->buffer.width, gc->buffer.height);
					gc->BindMultiPassShader(7, 1, false);
					gc->UnbindShaders();*/

					sys.gc->Blit2D(sys.titleTexture, 0, 0);

					// Draw console
					if(neo.consoleInput){
						sys.font->Draw(0, 8, textBuffer);
					}

					// Fader
					if (titleFade < 254){
						//sys.gc->Rectangle2D(0, 0, sys.gc->buffer.width, sys.gc->buffer.height, KLGLColor(0, 0, 0, int(255-titleFade)));
					}
				}
				sys.gc->OrthogonalEnd();
				sys.gc->Swap();
				break;
			case GameMode::_MAPLOAD:										// ! Threaded map loader :D

				cl("\nNew game...\n");
				try{
					// Shutdown any sound the menu mite be playing
					sys.audio->channel[0]->stop();
					sys.audio->channel[1]->stop();
					// Initialize the game engine and pass-through our systems
					neo.gameEnv = new Environment("common/map01");
					neo.gameEnv->gcProxy = sys.gc;
					neo.gameEnv->audioProxy = sys.audio;
					neo.gameEnv->hudFont = sys.font;
					neo.gameEnv->numberFont = sys.numberFont;

					#ifdef _WIN32
					cl("Performing async loading...\n");
					std::chrono::milliseconds span (10);
					std::future<int> loaderStatus = std::async(launch::async, &LoadEnv, neo.gameEnv);
					while (loaderStatus.wait_for(span) == std::future_status::timeout){
						// Read latest console buffer
						sprintf(textBuffer, "@CFFFFFF%s", clBuffer);

						sys.gc->OpenFBO();
						sys.gc->OrthogonalStart();
						glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
						KLGLColor(255,255,255,255).Set();
						sys.gc->Blit2D(sys.loadingTexture, 0, 0);
						neo.gameEnv->drawLoader(sys.gc, 0, sys.gc->buffer.height-24, sys.gc->buffer.width, 16, 50, 1);
						if (KLGLDebug){
							sys.font->Draw(8, 14, textBuffer);
						}
						sys.gc->OrthogonalEnd();
						sys.gc->Swap();
					}
					#else
					sys.gc->OpenFBO();
					sys.gc->OrthogonalStart();
					glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
					sys.gc->Blit2D(sys.loadingTexture, 0, 0);
					sys.gc->OrthogonalEnd();
					sys.gc->Swap();
					LoadEnv(neo.gameEnv);
					#endif

					// Setup initial post quality
					sys.gc->BindShaders(1);
					glUniform1i(glGetUniformLocation(sys.gc->GetShaderID(1), "preset"), qualityPreset);
					sys.gc->UnbindShaders();

					// Misc
					for (int i = 0; i < 100; i++)
					{
						stars[i] = new Point<int>;
						stars[i]->x = rand()%(neo.gameEnv->map.width*16+1);
						stars[i]->y = rand()%(neo.gameEnv->map.height*16+1);
					}

					sys.audio->system->playSound(FMOD_CHANNEL_FREE, sys.audio->sound[1], false, &sys.audio->channel[0]);
					sys.audio->system->playSound(FMOD_CHANNEL_FREE, sys.audio->sound[4], false, &sys.audio->channel[1]);
					sys.audio->channel[0]->setVolume(0.5f);
					sys.audio->channel[1]->setVolume(0.15f);

					neo.mode = GameMode::_INGAME;
				}catch(KLGLException e){
					cl("KLGLException: %s\n", e.getMessage());
					//MessageBox(NULL, e.getMessage(), "KLGLException", MB_OK | MB_ICONERROR);
					neo.mode = GameMode::_MENU;
				}

				cl("\nGame created successfully.\n");

				break;
			case GameMode::_MAPDESTROY:

				cl("Unloading game...");
				try{
					// Audio
					sys.audio->channel[0]->stop();
					sys.audio->channel[1]->stop();
					// Destroy the game
					delete neo.gameEnv;
					sys.gc->UnloadShaders(1);
					sys.gc->UnloadShaders(2);
					sys.gc->UnloadShaders(3);

					sys.audio->system->playSound(FMOD_CHANNEL_FREE, sys.audio->sound[0], false, &sys.audio->channel[0]);
					sys.audio->channel[0]->setVolume(0.5f);
				}catch(KLGLException e){
					cl("KLGLException: %s\n", e.getMessage());
					neo.mode = GameMode::_MENU;
				}

				neo.mode = GameMode::_MENU;
				cl(" [OK]\n");

				break;
			case GameMode::_INGAME:											// ! Main game test environment

				// Update clock
				neo.gameEnv->dt = (t0 - t1)/float(CLOCKS_PER_SEC);
				tweener.step(t0);
				t1 = clock();

				// BEGIN DRAWING!
				sys.gc->OpenFBO(50, 0.0, 0.0, 80.0);
				sys.gc->OrthogonalStart();
				{
					if (neo.paused){
						if (titleFade < 255){
							glBindTexture(GL_TEXTURE_2D, sys.gc->fbo_texture[1]);
							glBegin(GL_QUADS);
							glTexCoord2d(0.0,0.0); glVertex2i(0,				0);
							glTexCoord2d(1.0,0.0); glVertex2i(sys.gc->buffer.width,	0);
							glTexCoord2d(1.0,1.0); glVertex2i(sys.gc->buffer.width,	sys.gc->buffer.height);
							glTexCoord2d(0.0,1.0); glVertex2i(0,				sys.gc->buffer.height);
							glEnd();
							glBindTexture(GL_TEXTURE_2D, 0);
						}
						/*gc->OrthogonalStart(gc->overSampleFactor/2.0f);
						static int scrollOffset;
						if (scrollOffset == NULL || scrollOffset > 16){
						scrollOffset = 0;
						}
						glColor4ub(255, 255, 255, titleFade);
						for (int y = 0; y-1 <= gc->buffer.height/32; y++)
						{
						for (int x = 0; x-1 <= gc->buffer.width/32; x++)
						{
						gc->BlitSprite2D(gameEnv->hudSpriteSheet16, (x*16)-scrollOffset, (y*16)-scrollOffset, 0);
						}
						}
						scrollOffset++;
						glColor4ub(255, 255, 255, 255);*/

						sys.gc->OrthogonalStart();

						// Pause menu
						neo.dialog->comp();
						neo.dialog->draw();


					}else{
						// Compute game physics and update events
						neo.gameEnv->comp(sys.gc, (mapScroll ? mouseXY.x - sys.gc->buffer.width / 2 : 0), (mapScroll ? mouseXY.y - sys.gc->buffer.height / 2 : 0));

						neo.gameEnv->debugflags.x = mouseXY.x;
						neo.gameEnv->debugflags.y = mouseXY.y;

						// Debug info
						if (KLGLDebug)
						{
							sprintf(textBuffer, 
								"@CF8F8F8@DNeo\n-------------------------\n"\
								"Render FPS: %d\n"\
								"       Syn: V%dT%d\n"\
								"       Buf: %dx%d\n"\
								"       Acu: %f (less is more)\n\n"\
								"Player Pos: %f,%f\n"\
								"       Vel: %f,%f\n\n"\
								"Map    Pos: %d,%d\n"\
								"       Rel: %d,%d >> %d\n"\
								"       Tar: %d,%d >> %d\n\n"\
								"Debug  Act: %d\n"\
								"       DB0: %d\n"\
								"       DB1: %d\n", 
								fps,
								sys.gc->vsync,
								internalTimer,
								APP_SCREEN_W,
								APP_SCREEN_H,
								neo.gameEnv->dt,
								neo.gameEnv->character->pos.x, 
								neo.gameEnv->character->pos.y, 
								neo.gameEnv->character->vel.x,
								neo.gameEnv->character->vel.y,
								neo.gameEnv->scroll.x, 
								neo.gameEnv->scroll.y,
								neo.gameEnv->target.x,
								neo.gameEnv->target.y, FPM,
								neo.gameEnv->scroll_real.x,
								neo.gameEnv->scroll_real.y, FPM,
								KLGLDebug,
								neo.gameEnv->debugflags.x,
								neo.gameEnv->debugflags.y
								);
						}

						// Scaling
						//glScalef(float(gc->window.width)/float(APP_SCREEN_W), float(gc->window.width)/float(APP_SCREEN_W), 0.0);

						// Background
						int horizon = max(sys.gc->buffer.width, sys.gc->buffer.height+(-neo.gameEnv->scroll.y));
						int stratosphere = min(0, -neo.gameEnv->scroll.y);
						glBegin(GL_QUADS);
						glColor3ub(71,84,93);
						glVertex2i(0, stratosphere); glVertex2i(sys.gc->buffer.width, stratosphere);
						glColor3ub(214,220,214);
						glVertex2i(sys.gc->buffer.width, horizon); glVertex2i(0, horizon);
						glEnd();

						// Barell rotation
						//glTranslatef(gc->window.width/gc->scaleFactor, gc->window.height/gc->scaleFactor, 0.0f);
						//glRotatef(mouseXY.x/10.0f, 0.0f, 0.0f, 1.0f );
						//glTranslatef(-gc->window.width/gc->scaleFactor, -gc->window.height/gc->scaleFactor, 0.0f);

						// Stars
						for (int i = 0; i < 100; i++)
						{
							int x = (stars[i]->x-240)-(neo.gameEnv->scroll.x/6);
							int y = (stars[i]->y-240)-(neo.gameEnv->scroll.y/6);
							if(x < 0 || x > APP_SCREEN_W || y < 0 || y > APP_SCREEN_H){
								continue;
							}
							sys.gc->Rectangle2D(x, y, 1, 1, KLGLColor(255, 255, 255, 80+rand()%47));
						}

						// Reset pallet
						KLGLColor(255, 255, 255, 255).Set();

						// Cloud
						sys.gc->Blit2D(neo.gameEnv->backdropTexture, (APP_SCREEN_W/2)-(neo.gameEnv->scroll.x/4), (APP_SCREEN_H/3)-(neo.gameEnv->scroll.y/4));

						/*glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, neo.gameEnv->mapSpriteSheet->texturePtr->gltexture);
						glActiveTexture(GL_TEXTURE2);
						glBindTexture(GL_TEXTURE_2D, sys.testTexture->gltexture);
						sys.gc->BindShaders(5);
						glUniform1i(glGetUniformLocation(sys.gc->GetShaderID(5), "tileTexture"), 1);
						glUniform1i(glGetUniformLocation(sys.gc->GetShaderID(5), "maskTexture"), 2);
						glUniform1f(glGetUniformLocation(sys.gc->GetShaderID(5), "time"), sys.gc->shaderClock);
						glUniform2f(glGetUniformLocation(sys.gc->GetShaderID(5), "resolution"), sys.gc->buffer.width, sys.gc->buffer.height);
						glUniform4f(glGetUniformLocation(sys.gc->GetShaderID(5), "tileRect"), 64, 64, neo.gameEnv->mapSpriteSheet->texturePtr->width, neo.gameEnv->mapSpriteSheet->texturePtr->height);
						sys.gc->BindMultiPassShader(5, 1, false);
						sys.gc->UnbindShaders();*/

						// Sea
						sys.gc->BindShaders(6);
						glUniform1f(glGetUniformLocation(sys.gc->GetShaderID(6), "time"), sys.gc->shaderClock/1000.0f);
						glUniform2f(glGetUniformLocation(sys.gc->GetShaderID(6), "resolution"), sys.gc->buffer.width/sys.gc->overSampleFactor, sys.gc->buffer.height/sys.gc->overSampleFactor);
						glUniform2f(glGetUniformLocation(sys.gc->GetShaderID(6), "offset"), -neo.gameEnv->scroll.x, neo.gameEnv->scroll.y-144);
						sys.gc->BindMultiPassShader(6, 1, false);
						sys.gc->UnbindShaders();

						// Draw Map
						neo.gameEnv->drawMap(sys.gc);

						// Draw Player & Enemys
						neo.gameEnv->character->draw(neo.gameEnv, sys.gc, neo.gameEnv->mapSpriteSheet, frame);
						for (auto enemy = neo.gameEnv->enemys.begin(); enemy != neo.gameEnv->enemys.end(); enemy++){
							(*enemy)->draw(sys.gc, neo.gameEnv->mapSpriteSheet, frame);
						}

						// Lighting effects
						sys.gc->BindShaders(4);
						glUniform1f(glGetUniformLocation(sys.gc->GetShaderID(4), "time"), t0);
						glUniform2f(glGetUniformLocation(sys.gc->GetShaderID(4), "resolution"), sys.gc->buffer.width/sys.gc->overSampleFactor, sys.gc->buffer.height/sys.gc->overSampleFactor);
						glUniform2f(glGetUniformLocation(sys.gc->GetShaderID(4), "position"), (neo.gameEnv->scroll.x-(56*16))/1.0f, (neo.gameEnv->scroll.y-(27*16))/1.0f);
						glUniform1f(glGetUniformLocation(sys.gc->GetShaderID(4), "radius"), 140.0f);
						glUniform1f(glGetUniformLocation(sys.gc->GetShaderID(4), "blendingDivision"), 5.0f+sin(cycle/16.0f));
						glUniform2f(glGetUniformLocation(sys.gc->GetShaderID(4), "mouse"), mouseXY.x, mouseXY.y);
						sys.gc->BindMultiPassShader(4, 1, false);
						sys.gc->UnbindShaders();

						// Adv Bloom
						/*gc->BindShaders(3);
						glUniform1i(glGetUniformLocation(gc->GetShaderID(3), "image"), 0);
						glUniform1i(glGetUniformLocation(gc->GetShaderID(3), "slave"), 1);
						glUniform1f(glGetUniformLocation(gc->GetShaderID(3), "time"), gc->shaderClock);
						glUniform2f(glGetUniformLocation(gc->GetShaderID(3), "resolution"), gc->buffer.width/gc->overSampleFactor, gc->buffer.height/gc->overSampleFactor);
						gc->BindMultiPassShader(3, 1, false);
						gc->UnbindShaders();*/

						/*gc->BindShaders(2);
						glUniform1i(glGetUniformLocation(gc->GetShaderID(2), "image"), 0);
						glUniform1i(glGetUniformLocation(gc->GetShaderID(2), "slave"), 1);
						glUniform1f(glGetUniformLocation(gc->GetShaderID(2), "time"), gc->shaderClock);
						glUniform2f(glGetUniformLocation(gc->GetShaderID(2), "resolution"), gc->buffer.width/gc->overSampleFactor, gc->buffer.height/gc->overSampleFactor);
						gc->BindMultiPassShader(2, 1, false);
						gc->UnbindShaders();*/

						/*glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, neo.gameEnv->lvlupTexture->gltexture);
						glActiveTexture(GL_TEXTURE2);
						glBindTexture(GL_TEXTURE_2D, sys.testTexture->gltexture);
						sys.gc->BindShaders(5);
						glUniform1i(glGetUniformLocation(sys.gc->GetShaderID(5), "tileTexture"), 1);
						glUniform1i(glGetUniformLocation(sys.gc->GetShaderID(5), "maskTexture"), 2);
						glUniform1f(glGetUniformLocation(sys.gc->GetShaderID(5), "time"), sys.gc->shaderClock);
						glUniform2f(glGetUniformLocation(sys.gc->GetShaderID(5), "resolution"), sys.gc->buffer.width, sys.gc->buffer.height);
						glUniform4f(glGetUniformLocation(sys.gc->GetShaderID(5), "tileRect"), 0, 0, 32, 32);
						sys.gc->BindMultiPassShader(5, 1, false);
						sys.gc->UnbindShaders();*/

						// Master post shader data
						sys.gc->BindShaders(1);
						glUniform1f(glGetUniformLocation(sys.gc->GetShaderID(1), "time"), t0);
						glUniform2f(glGetUniformLocation(sys.gc->GetShaderID(1), "resolution"), sys.gc->buffer.width/sys.gc->overSampleFactor, sys.gc->buffer.height/sys.gc->overSampleFactor);
						sys.gc->BindMultiPassShader(1, 1, false);
						sys.gc->UnbindShaders();

						// HUD, Score, Health, etc
						neo.gameEnv->drawHUD(sys.gc);

						if (KLGLDebug){
							sys.gc->OrthogonalStart(sys.gc->overSampleFactor);
							sys.font->Draw(0, 8, textBuffer);
						}
					}
				}
				sys.gc->OrthogonalEnd();
				sys.gc->Swap();
				break;
			case GameMode::_CREDITS:
				t1 = clock();
				sys.gc->OpenFBO(50, 0.0, 0.0, 80.0);
				sys.gc->OrthogonalStart();
				if (scrollPos == INT_MAX){
					scrollPos = sys.gc->buffer.height;
				}else if(scrollPos < -2400){
					neo.mode = -1;
				}
				scrollPos -= (frame%3 == 2);
				sys.gc->Blit2D(sys.creditsTexture, 0, scrollPos);
				sys.gc->OrthogonalEnd();
				sys.gc->Swap();
				break;
			default:
				Warning(sys.gc);
				//Sleep(1000);
			}
			frame++;
			cycle++;
		}

	}
	delete sys.gc;
	return 0;
}

void Loading(KLGL *gc, KLGLTexture *loading)
{
	gc->OpenFBO();
	gc->OrthogonalStart();
	{
		// Draw loading screen
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gc->fbo[0]);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		gc->Blit2D(loading, 0, 0);
	}
	gc->OrthogonalEnd();
	gc->Swap();
}

void Warning(KLGL *gc){
	static KLGLFont *font;
	if (font == NULL){
		font = new KLGLFont();
	}
	gc->OpenFBO();
	gc->OrthogonalStart();
	{
		glClearColor(0.8f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		font->Draw(0, 8, clBuffer);
	}
	gc->OrthogonalEnd();
	gc->Swap();
}
