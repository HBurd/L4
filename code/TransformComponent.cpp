#include "hb/TransformComponent.h"

using std::vector;

const float WORLD_SECTOR_DIM = 100.0f;

Transform::Transform(Vec3 transform_position)
:position(transform_position) {}

void update_transform_components(
    WorldSector *world_sector_components,
    Transform *transform_components,
    size_t num_components,
    float dt)
{
    for (size_t i = 0; i < num_components; i++)
    {
        WorldSector &sector = world_sector_components[i];
        Transform &transform = transform_components[i];

        transform.position += transform.velocity * dt;

        int64_t x_sector_offset = roundf(transform.position.x / WORLD_SECTOR_DIM);
        int64_t y_sector_offset = roundf(transform.position.y / WORLD_SECTOR_DIM);
        int64_t z_sector_offset = roundf(transform.position.z / WORLD_SECTOR_DIM);

        transform.position -= Vec3(x_sector_offset, y_sector_offset, z_sector_offset) * WORLD_SECTOR_DIM;

        sector.x += x_sector_offset;
        sector.y += y_sector_offset;
        sector.z += z_sector_offset;

        Rotor delta_rotor = Rotor::angle_axis(dt * transform.angular_velocity);
        transform.orientation = transform.orientation * delta_rotor;
        
        // we need to normalize orientation and angular velocity every frame,
        // or we get accumulating errors
        transform.orientation = transform.orientation.normalize();
    }
}

Vec3 to_world_position(WorldSector sector, Vec3 position)
{
    
    return position + WORLD_SECTOR_DIM * Vec3(sector.x, sector.y, sector.z);
}
