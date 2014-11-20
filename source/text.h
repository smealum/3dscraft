#pragma once

#include <stdio.h>

#include "font.h"

// #define print(...) sprintf(&superStr[strlen(superStr)], __VA_ARGS__); drawBottom()
#define print(...) 

void drawBottom();
extern char superStr[];

int drawCharacter(u8* fb, font_s* f, char c, s16 x, s16 y, u16 w, u16 h);
void drawString(u8* fb, font_s* f, char* str, s16 x, s16 y, u16 w, u16 h);
void gfxDrawText(gfxScreen_t screen, gfx3dSide_t side, font_s* f, char* str, s16 x, s16 y);
