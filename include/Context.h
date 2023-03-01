#pragma once

#define MAX_INPUT_TEXT_SIZE 512
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct Context{
	char ** cmdStrings;
	size_t argsLen;

	SDL_Window * window;
	TTF_Font * font;
	SDL_Renderer * renderer;
	SDL_Texture * textTexture;

	char inputText[MAX_INPUT_TEXT_SIZE];
	size_t count;

	int initialized;
	int running;
}Context;
