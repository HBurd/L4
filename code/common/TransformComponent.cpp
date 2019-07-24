#include "common/TransformComponent.h"
#include <cmath>

const float WORLD_SECTOR_DIM = 100.0f;

Transform::Transform(Vec3 transform_position)
:position(transform_position) {}

void update_transform_components(
    WorldSector *world_sectors,
    Transform *transforms,
    size_t num_components,
    float dt)
{
    for (size_t i = 0; i < num_components; i++)
    {
        WorldSector &sector = world_sectors[i];
        Transform &transform = transforms[i];

        transform.position += transform.velocity * dt;

        float y_sector_offset = roundf(transform.position.y / WORLD_SECTOR_DIM);
        float x_sector_offset = roundf(transform.position.x / WORLD_SECTOR_DIM);
        float z_sector_offset = roundf(transform.position.z / WORLD_SECTOR_DIM);

        transform.position -= Vec3(x_sector_offset, y_sector_offset, z_sector_offset) * WORLD_SECTOR_DIM;

        sector.x += (int64_t)x_sector_offset;
        sector.y += (int64_t)y_sector_offset;
        sector.z += (int64_t)z_sector_offset;

        Rotor delta_rotor = Rotor::angle_axis(dt * transform.angular_velocity);
        transform.orientation = transform.orientation * delta_rotor;
        
        // we need to normalize orientation and angular velocity every frame,
        // or we get accumulating errors
        transform.orientation = transform.orientation.normalize();
    }
}

void update_transform(WorldSector *position_rf, Transform *transform, float dt)
{
    update_transform_components(position_rf, transform, 1, dt);
}

Vec3 mesh_to_sector(Transform transform, Vec3 position)
{
    return transform.orientation.to_matrix() * position + transform.position;
}

Vec3 relative_to_sector(const WorldSector &from, const WorldSector &sector, const Vec3 &position)
{
    float x_offset = WORLD_SECTOR_DIM * (sector.x - from.x);
    float y_offset = WORLD_SECTOR_DIM * (sector.y - from.y);
    float z_offset = WORLD_SECTOR_DIM * (sector.z - from.z);

    return Vec3(x_offset, y_offset, z_offset) + position;
}
