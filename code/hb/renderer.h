#ifndef HBRENDERER_H
#define HBRENDERER_H

#include <iostream>
#include <fstream>
#include <vector>

#include "GL/glew.h"

#include "hb/mesh_type.h"
#include "hb/math.h"

using std::vector;

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

typedef size_t ShaderProgramId;

struct Mesh
{
    Mesh(void *mesh_vertices, uint32_t num_vertices, uint16_t vertex_size, ShaderProgramId mesh_shader_program);
    Mesh(const char* filename, ShaderProgramId mesh_shader_program);

    ShaderProgramId shader_program;

    GLuint vbo;
    GLuint vao;
    vector<Vertex> vertices;
};

typedef size_t MeshId;

struct Renderer
{
    // an opengl context must already be available before constructing
    Renderer(unsigned int _width, unsigned int _height);
    //MeshId load_mesh(const char* filename, ShaderProgramId shader_prog_id);
    //void load_skybox(const char* filename, const char* texture_filename, ShaderProgramId shader_prog_id);
    ShaderProgramId load_shader(const char* vshader, const char* fshader);
    GLuint load_texture(const char* texture_filename);
    void set_screen_size(unsigned int w, unsigned int h);
    void draw_mesh(MeshId mesh_id, Vec3 position, Vec3 scale, Rotor orientation) const;
    void draw_skybox() const;
	void draw_crosshair(Vec3 position, float scale) const;
    void clear() const;

    unsigned int width;
    unsigned int height;

    Vec3 camera_pos;
    Rotor camera_orientation;

    MeshId skybox_mesh;
    GLuint skybox_texture;

    vector<Mesh> meshes;
    vector<ShaderProgram> shader_programs;
};

#ifdef FAST_BUILD
#include "renderer.cpp"
#endif

#endif // include guard
