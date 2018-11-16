#ifndef HBKEYBOARD_H
#define HBKEYBOARD_H

#include "SDL/SDL.h"

struct Keyboard
{
    bool q = false;
    bool w = false;
    bool e = false;
    bool a = false;
    bool s = false;
    bool d = false;

    bool i = false;
    bool k = false;

    bool space = false;
    
    void handle_keyup(SDL_Keycode key);
    void handle_keydown(SDL_Keycode key);
};

#endif // include guard
