#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include "MateDb.h"
#include "Context.h"

#define MATEDEF inline static
Context ctx;

void UI_Init(){
	/* Initialize core*/
	MateDb_Init();

	/* Setup SDL */
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		DIE(SDL_GetError());
	}

	SDL_Window * window = SDL_CreateWindow("MateDb", 0, 0, 1024, 728, 0);
	if(!window)
		DIE(SDL_GetError());

	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if(!renderer)
		DIE(SDL_GetError());

	if(TTF_Init() < 0){
		DIE(TTF_GetError());
	};

	TTF_Font * font = TTF_OpenFont("assets/04B_11__.TTF", 25);

	if(!font){
		DIE(TTF_GetError());
	}

	ctx.window = window;
	ctx.renderer = renderer;
	ctx.initialized = 1;
	ctx.font = font;
	ctx.count = 0;
	memset(ctx.inputText, 0, MAX_INPUT_TEXT_SIZE);
}

void UI_Quit(){
	MateDb_Quit();

	SDL_DestroyTexture(ctx.textTexture);
	SDL_DestroyRenderer(ctx.renderer);
	SDL_DestroyWindow(ctx.window);
	TTF_CloseFont(ctx.font);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	TTF_Quit();
	SDL_Quit();
}

MATEDEF void UI_Update(){
	SDL_Event e;

	while(SDL_PollEvent(&e)){
		if(e.type == SDL_QUIT){
			ctx.running = 0;
		}

		if(e.type == SDL_KEYDOWN){
			switch(e.key.keysym.sym){
				case SDLK_BACKSPACE:
					if(ctx.count > 0){
						ctx.inputText[ctx.count - 1] = 0;
						ctx.count--;
					}
					break;
				case SDLK_RETURN:
					MateDb_ExecuteCmd();
					break;
			}

		}

		if(e.type == SDL_TEXTINPUT){
			if(ctx.count < 1024){
				ctx.inputText[ctx.count++] = *e.text.text;
			}
		}
	}

}

MATEDEF void UI_Render(){
	SDL_Color color = { 255, 255, 255 };
	SDL_Surface * surface = TTF_RenderText_Solid(ctx.font, ctx.inputText, color);
	SDL_Texture * textTexture = SDL_CreateTextureFromSurface(ctx.renderer, surface);

	SDL_FreeSurface(surface);

	int W, H;

	SDL_QueryTexture(textTexture, NULL, NULL, &W, &H);
	SDL_Rect dst = {0, 0, W, H};

	SDL_RenderCopy(ctx.renderer, textTexture, NULL, &dst);
}

MATEDEF void UI_RunInternal(){
	assert(ctx.initialized);
	SDL_StartTextInput();

	ctx.running = 1;

	while(ctx.running){
		SDL_RenderClear(ctx.renderer);

		UI_Update();

		if(ctx.count > 0){
			UI_Render();
		}

		SDL_RenderPresent(ctx.renderer);
	}

}

void UI_Run(const char * program){
	MateDb_StartSession(program, &UI_RunInternal);
}

