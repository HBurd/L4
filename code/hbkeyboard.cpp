#include "hbkeyboard.h"

void Keyboard::handle_keyup(SDL_Keycode key)
{
    switch (key)
    {
    case SDLK_q:
        q = false;
        break;
    case SDLK_w:
        w = false;
        break;
    case SDLK_e:
        e = false;
        break;
    case SDLK_a:
        a = false;
        break;
    case SDLK_s:
        s = false;
        break;
    case SDLK_d:
        d = false;
        break;
    case SDLK_i:
        i = false;
        break;
    case SDLK_k:
        k = false;
        break;
    case SDLK_SPACE:
        space = false;
        break;
    }
}

void Keyboard::handle_keydown(SDL_Keycode key)
{
    switch (key)
    {
    case SDLK_q:
        q = true;
        break;
    case SDLK_w:
        w = true;
        break;
    case SDLK_e:
        e = true;
        break;
    case SDLK_a:
        a = true;
        break;
    case SDLK_s:
        s = true;
        break;
    case SDLK_d:
        d = true;
        break;
    case SDLK_i:
        i = true;
        break;
    case SDLK_k:
        k = true;
        break;
    case SDLK_SPACE:
        space = true;
        break;
    }
}
