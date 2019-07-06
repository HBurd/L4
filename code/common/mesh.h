#pragma once

#include <vector>
#include "GL/glew.h"

typedef size_t ShaderProgramId;
typedef size_t MeshId;


namespace MeshType
{
    enum MeshType
    {
        SHIP,
        PROJECTILE,
        SKYBOX,
        PLANET,
        CROSSHAIR,
        BOX,

        NUM_MESH_TYPES,
    };
}

#pragma pack(push, 1)
struct Vertex
{
    Vec3 position;
    Vec3 normal;
    Vec2 uv;

    Vertex(Vec3 _position, Vec3 _normal, Vec2 _uv);
};
#pragma pack(pop)

struct ShaderProgram
{
    ShaderProgram(const char* vshader_filename,
                  const char* fshader_filename);
    GLint vshader;
    GLint fshader;
    GLuint program;
};

struct Mesh
{
    Mesh(void *mesh_vertices, uint32_t num_vertices, uint16_t vertex_size, ShaderProgramId mesh_shader_program);
    Mesh(const char* filename, ShaderProgramId mesh_shader_program);

    ShaderProgramId shader_program;

    GLuint vbo;
    GLuint vao;
    std::vector<Vertex> vertices;
};
