#pragma once

#include "common/math.h"
#include "hec.h"
#include "client/player_input.h"
#include "common/components.h"
#include "common/util.h"
#include <stdint.h>

#include "imgui/imgui.h"

const uint16_t DEFAULT_SERVER_PORT = 4444;
#define DEFAULT_IP "127.0.0.1"

struct MainMenu
{
    // draw the server create menu, or otherwise the connect menu
    bool is_server = true;

    uint16_t port = DEFAULT_SERVER_PORT;
    char ip[16] = DEFAULT_IP;

    void draw(bool *server, bool *client);
    bool draw_server_create_gui();
    bool draw_server_connect_gui();
};

struct SpawnMenu
{
    Vec3 coords;

    bool draw();
};

struct EntityInspectWindows
{
    EntityHandle selected_entity;
    int selected_add_component = 0;

    void draw(EntityManager &entity_manager);
};

struct Console
{
    Console(const char *name_);

    const char *name;

    // circular buffer
    char data[1024] = {};
    size_t mark = 0;

    void draw();
    void write(const char *string);
    // this one doesn't have to be null terminated
    void writen(const char *string, size_t string_len);
};

bool draw_guidance_menu(
    const EntityManager *entity_manager,
    EntityHandle player_handle,
    TrackingState *tracking);
