#include "hbplayer_control.h"
#include "hbnet.h"
#include "hbpackets.h"
#include "hbutil.h"
#include "imgui/imgui.h"
#include <cmath>

const float MAX_THRUST = 1.0f;
const float MAX_TORQUE = 1.0f;

PlayerControlState player_control_get_state(
    const Keyboard kb,
    bool stabilize,
    Physics player,
    bool track,
    Physics target)
{
    // build the control packet and send it to the server

    PlayerControlState control_state;
    if (kb.held.space)
    {
        control_state.thrust = MAX_THRUST;
    }

    float roll_torque = 0.0f;
    float pitch_torque = 0.0f;
    float yaw_torque = 0.0f;

    if (track)
    {
        Vec3 target_direction = target.velocity - player.velocity;
        if (target_direction.norm() >= 0.0001f)
        {
            // find the rotor representing the rotation to the thrust vector
            Rotor err = Rotor(player.orientation.to_matrix() * Vec3(0.0f, 0.0f, 1.0f),
                              target_direction.normalize());
            

        }
    }
    else if (stabilize)  // the two do not work together
    {
        float roll = player.angular_velocity.xy;
        float pitch = player.angular_velocity.yz;
        float yaw = player.angular_velocity.zx;

        roll_torque = -roll;
        pitch_torque = -pitch;
        yaw_torque = -yaw;
    }
 
    if (kb.held.q) roll_torque = -MAX_TORQUE;
    if (kb.held.e) roll_torque = MAX_TORQUE;
    if (kb.held.w) pitch_torque = MAX_TORQUE;
    if (kb.held.s) pitch_torque = -MAX_TORQUE;
    if (kb.held.a) yaw_torque = -MAX_TORQUE;
    if (kb.held.d) yaw_torque = MAX_TORQUE;

    // clamp torque

    float factor = 1.0f;
    if (fabs(roll_torque) > MAX_TORQUE)
    {
        factor = MAX_TORQUE / fabs(roll_torque);
    }
    roll_torque *= factor;
    pitch_torque *= factor;
    yaw_torque *= factor;

    factor = 1.0f;
    if (fabs(pitch_torque) > MAX_TORQUE)
    {
        factor = MAX_TORQUE / fabs(pitch_torque);
    }
    roll_torque *= factor;
    pitch_torque *= factor;
    yaw_torque *= factor;

    factor = 1.0f;
    if (fabs(yaw_torque) > MAX_TORQUE)
    {
        factor = MAX_TORQUE / fabs(yaw_torque);
    }
    roll_torque *= factor;
    pitch_torque *= factor;
    yaw_torque *= factor;
 
    control_state.torque = Rotor::roll(roll_torque) * Rotor::pitch(pitch_torque) * Rotor::yaw(yaw_torque);

    return control_state;
}

void player_control_update(Physics *physics, PlayerControlState control_state, float dt)
{
    Vec3 thrust = physics->orientation.to_matrix() * Vec3(0.0f, 0.0f, -1.0f);
    thrust = thrust * control_state.thrust * dt;
    
    physics->velocity += thrust;
    physics->angular_velocity = physics->angular_velocity * Rotor::lerp(Rotor(), control_state.torque, dt);
}

void PlayerInputBuffer::save_input(PlayerControlState control_state, float dt)
{
    inputs[next_seq_num % ARRAY_LENGTH(inputs)] = {control_state, dt, next_seq_num};
    next_seq_num++;
}

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
