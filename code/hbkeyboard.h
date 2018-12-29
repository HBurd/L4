#ifndef HBKEYBOARD_H
#define HBKEYBOARD_H

#include "SDL/SDL.h"

struct KeyboardData
{
    bool q = false;
    bool w = false;
    bool e = false;
    bool a = false;
    bool s = false;
    bool d = false;

    bool i = false;
    bool k = false;

    bool tilde = false;

    bool space = false;
    bool enter = false;
};

struct Keyboard
{
    KeyboardData held;
    KeyboardData down;
    void handle_keyup(SDL_Keycode key);
    void handle_keydown(SDL_Keycode key);
    void clear_keydowns();
};

#endif // include guard
