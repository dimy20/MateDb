#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include "MateDb.h"
#include "Context.h"
#include "Text.h"

void UI_Init();
void UI_Quit();
void UI_Run(const char * program);
