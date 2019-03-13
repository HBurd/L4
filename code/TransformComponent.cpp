#include "hb/TransformComponent.h"

using std::vector;

Transform::Transform(Vec3 transform_position)
:position(transform_position) {}

void update_transform_components(vector<Transform> *transform_components, float dt)
{
    for (size_t i = 0; i < transform_components->size(); i++)
    {
        Transform &transform = (*transform_components)[i];

        transform.position += transform.velocity * dt;

        Rotor delta_rotor = Rotor::angle_axis(dt * transform.angular_velocity);
        transform.orientation = transform.orientation * delta_rotor;
        
        // we need to normalize orientation and angular velocity every frame,
        // or we get accumulating errors
        transform.orientation = transform.orientation.normalize();
    }
}
