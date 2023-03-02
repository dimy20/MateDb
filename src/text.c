#include "Text.h"

extern Context ctx;

static TTF_Font * fonts[NUM_FONTS];
static SDL_Rect glyphs[NUM_FONTS][MAX_GLYPHS];
static SDL_Texture * fontAtlas[NUM_FONTS];

inline static void unpackColor24(uint32_t color, uint8_t * r, uint8_t * g, uint8_t * b){
	// 0xRRGGBBAA
	*r = (uint8_t)(color >> 24);
	*g = (uint8_t)(color >> 16);
	*b = (uint8_t)(color >> 8);
}

void MateDb_InitFont(const FontType type, const char * filename){
	SDL_Surface * surface;
	SDL_Surface * currGlyph;
	char c[2];
	SDL_Rect *g;

	memset(&glyphs[type], 0, sizeof(SDL_Rect) * MAX_GLYPHS);

	TTF_Font * font = TTF_OpenFont(filename, FONT_SIZE);

	if(!font){
		DIE(TTF_GetError());
	};

	// save the font
	fonts[type] = font;

	int w, h;
	w = h = FONT_TEXTURE_SIZE;

	surface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0xff);
	if(!surface){
		DIE(TTF_GetError());
	}

	uint32_t key = SDL_MapRGBA(surface->format, 0, 0, 0, 0);
	if(SDL_SetColorKey(surface, SDL_TRUE, key) < 0){
		DIE(SDL_GetError());
	}

	int x = 0, y = 0;
	w = 0, h = 0;

	for(char i = ' '; i <= 'z'; i++){
		c[0] = i;
		c[1] = '\0';

		SDL_Color white = {0xff, 0xff, 0xff};
		currGlyph = TTF_RenderUTF8_Blended(fonts[type], c, white);

		// size of current glyph?
		if(TTF_SizeText(fonts[type], c, &w, &h) < 0){
			DIE(TTF_GetError());
		}


		if(x + w >= FONT_TEXTURE_SIZE){
			x = 0;

			y += h + 1;

			if(y + h >= FONT_TEXTURE_SIZE){
				DIE("Out of space in glyph atlas texture map")
			}
		}

		SDL_Rect dest = {x, y, w, h};
		if(SDL_BlitSurface(currGlyph, NULL, surface, &dest) < 0){
			DIE(SDL_GetError());
		}

		g = &glyphs[type][(int)i];
		*g = dest;

		SDL_FreeSurface(currGlyph);
		x += w;
	}


	SDL_Texture * atlas = SDL_CreateTextureFromSurface(ctx.renderer, surface);
	if(!atlas){
		DIE(SDL_GetError());
	}

	fontAtlas[type] = atlas;

}

void MateDb_DrawText(char * text, int x, int y, uint32_t color, int fontType){
	SDL_Rect *glyph;
	SDL_Texture * atlas = fontAtlas[fontType];

	uint8_t r, g, b;
	unpackColor24(color, &r, &g, &b);

	SDL_SetTextureColorMod(atlas, r, g, b);

	size_t len = strlen(text);

	for(size_t i = 0; i < len; i++){
		int c = text[i];

		assert(c >= 0 && c < 128);

		glyph = &glyphs[fontType][c];

		SDL_Rect dest = {x, y, glyph->w, glyph->h};

		SDL_RenderCopy(ctx.renderer, atlas, glyph, &dest);

		x += glyph->w;

	}
}

SDL_Texture * MateDb_TextTexture(char * text, int fontType, const SDL_Color color, int * w, int * h){
	SDL_Surface * s = TTF_RenderText_Solid(fonts[fontType], text, color);

	if(!s){
		DIE(SDL_GetError());
	}

	SDL_Texture * texture = SDL_CreateTextureFromSurface(ctx.renderer, s);

	if(!texture){
		DIE(SDL_GetError());
	}

	SDL_QueryTexture(texture, NULL, NULL, w, h);

	return texture;
}
