#pragma once

#include <assert.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "Context.h"
#include "Utils.h"

#define FONT_TEXTURE_SIZE 512
#define FONT_SIZE 25
#define MAX_GLYPHS 128

typedef enum FontType{
	DEFAULT_FONT,
	/*More supported fonts here*/
	NUM_FONTS
}FontType;


void MateDb_InitFont(const FontType type, const char * filename);

void MateDb_DrawText(char * text, int x, int y, uint32_t color, int fontType);

SDL_Texture * MateDb_TextTexture(char * text, int fontType, const SDL_Color color, int * w, int * h);
