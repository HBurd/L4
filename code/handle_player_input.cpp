#include "hb/handle_player_input.h"

// TODO: this function does too many things
void handle_player_input(
    PlayerControlState input,
    float dt,
    Physics *player_physics,
    PlayerInputBuffer *player_input_buffer,
    ClientData *client)
{
    ControlUpdatePacket control_update(input, player_input_buffer->next_seq_num);
    client->send_to_server(*(GamePacket*)&control_update);

    // save this input
    player_input_buffer->save_input(input, dt);

    // client side prediction:
    player_control_update(
        player_physics,
        input,
        dt);
}
