#ifndef HBGUI_H
#define HBGUI_H

#include "hbmath.h"
#include "hbentities.h"
#include <stdint.h>

const uint16_t DEFAULT_SERVER_PORT = 4444;
const uint32_t DEFAULT_CONNECTION_IP = 0x7f000001;

struct MainMenu
{
    bool draw_main_menu = true;

    // draw the server create menu, or otherwise the connect menu
    bool is_server = true;

    uint16_t port = DEFAULT_SERVER_PORT;
    uint32_t ip = DEFAULT_CONNECTION_IP;

    void draw(bool* server, bool* client);
    bool draw_server_create_gui();
    bool draw_server_connect_gui();
};

struct SpawnMenu
{
    bool draw_spawn_menu = false;
    Vec3 coords;

    bool draw(bool can_spawn);
};

bool draw_guidance_menu(
    const EntityManager* entity_manager,
    EntityHandle player_handle,
    EntityHandle* selected_entity,
    bool* track,
    bool* stabilize);

#endif // include guard
