/*
 *
 * MIT License
 *
 * Copyright (c) 2018 Richard Knight
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef FAST_OBJ_HDR
#define FAST_OBJ_HDR

typedef struct
{
    /* Material name */
    const char*                 name;

    /* Parameters */
    float                       Ka[3];  /* Ambient */
    float                       Kd[3];  /* Diffuse */
    float                       Ks[3];  /* Specular */
    float                       Ke[3];  /* Emission */
    float                       Kt[3];  /* Transmittance */
    float                       Ns;     /* Shininess */
    float                       Ni;     /* Index of refraction */
    float                       Tr;     /* Transparency */

} fastObjMaterial;


typedef struct
{
    unsigned int                p;
    unsigned int                t;
    unsigned int                n;

} fastObjIndex;


typedef struct
{
    /* Group name */
    const char*                 name;

    /* Number of faces */
    unsigned int                face_count;

    /* Material index for each face */
    unsigned int*               materials;

    /* Vertex count for each face */
    unsigned int*               vertices;

    /* Array of indices */
    fastObjIndex*               indices;

} fastObjGroup;


typedef struct
{
    /* Vertex data */
    unsigned int                position_count;
    float*                      positions;

    unsigned int                texcoord_count;
    float*                      texcoords;

    unsigned int                normal_count;
    float*                      normals;

    /* Materials */
    unsigned int                material_count;
    fastObjMaterial*            materials;

    /* Mesh groups */
    unsigned int                group_count;
    fastObjGroup*               groups;

} fastObjMesh;


fastObjMesh*                    fast_obj_read(const char* path);
void                            fast_obj_destroy(fastObjMesh* mesh);

#endif


#ifdef FAST_OBJ_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Size of buffer to read into */
#define BUFFER_SIZE             65536

/* Max supported power when parsing float */
#define MAX_POWER               20

typedef struct
{
    /* Final mesh */
    fastObjMesh*                mesh;

    /* Current group */
    fastObjGroup                group;

    /* Current material index */
    unsigned int                material;

    /* Current line in file */
    unsigned int                line;

    /* Base path for materials/textures */
    const char*                 base;

} fastObjData;


static const
double POWER_10_POS[MAX_POWER] =
{
    1.0e0,  1.0e1,  1.0e2,  1.0e3,  1.0e4,  1.0e5,  1.0e6,  1.0e7,  1.0e8,  1.0e9,
    1.0e10, 1.0e11, 1.0e12, 1.0e13, 1.0e14, 1.0e15, 1.0e16, 1.0e17, 1.0e18, 1.0e19,
};

static const
double POWER_10_NEG[MAX_POWER] =
{
    1.0e0,   1.0e-1,  1.0e-2,  1.0e-3,  1.0e-4,  1.0e-5,  1.0e-6,  1.0e-7,  1.0e-8,  1.0e-9,
    1.0e-10, 1.0e-11, 1.0e-12, 1.0e-13, 1.0e-14, 1.0e-15, 1.0e-16, 1.0e-17, 1.0e-18, 1.0e-19,
};


static
void* memory_realloc(void* ptr, size_t bytes)
{
    return realloc(ptr, bytes);
}


static
void memory_dealloc(void* ptr)
{
    free(ptr);
}


#define array_clean(_arr)       ((_arr) ? memory_dealloc(_array_header(_arr)), 0 : 0)
#define array_push(_arr, _val)  (_array_mgrow(_arr, 1) ? ((_arr)[_array_size(_arr)++] = (_val), _array_size(_arr) - 1) : 0)
#define array_size(_arr)        ((_arr) ? _array_size(_arr) : 0)
#define array_capacity(_arr)    ((_arr) ? _array_capacity(_arr) : 0)
#define array_empty(_arr)       (array_size(_arr) == 0)

#define _array_header(_arr)     ((unsigned int*)(_arr) - 2)
#define _array_size(_arr)       (_array_header(_arr)[0])
#define _array_capacity(_arr)   (_array_header(_arr)[1])
#define _array_ngrow(_arr, _n)  ((_arr) == 0 || (_array_size(_arr) + (_n) >= _array_capacity(_arr)))
#define _array_mgrow(_arr, _n)  (_array_ngrow(_arr, _n) ? (_array_grow(_arr, _n) != 0) : 1)
#define _array_grow(_arr, _n)   (*((void**)&(_arr)) = array_realloc(_arr, _n, sizeof(*(_arr))))


static
void* array_realloc(void* ptr, unsigned int n, unsigned int b)
{
    unsigned int  sz   = array_size(ptr);
    unsigned int  nsz  = sz + n;
    unsigned int  cap  = array_capacity(ptr);
    unsigned int  ncap = 3 * cap / 2;
    unsigned int* r;


    if (ncap < nsz)
        ncap = nsz;
    ncap = (ncap + 15) & ~15u;

    r = (unsigned int*)(memory_realloc(ptr ? _array_header(ptr) : 0, b * ncap + 2 * sizeof(unsigned int)));
    if (!r)
        return 0;

    r[0] = sz;
    r[1] = ncap;

    return (r + 2);
}


static
void* file_open(const char* path)
{
    return fopen(path, "rb");
}


static
void file_close(void* file)
{
    FILE* f;
    
    f = (FILE*)(file);
    fclose(f);
}


static
unsigned int file_read(void* file, void* dst, unsigned int bytes)
{
    FILE* f;
    
    f = (FILE*)(file);
    return fread(dst, 1, bytes, f);
}


static
unsigned int file_size(void* file)
{
    FILE* f;
    off_t p;
    off_t n;
    
    f = (FILE*)(file);

    p = ftello(f);
    fseeko(f, 0, SEEK_END);
    n = ftello(f);
    fseeko(f, p, SEEK_SET);

    return (unsigned int)(n);
}


static
const char* string_copy(const char* s, const char* e)
{
    unsigned int n;
    char*        p;
        
    n = e - s; 
    p = (char*)(memory_realloc(0, n + 1));
    if (p)
    {
        memcpy(p, s, n);
        p[n] = '\0';
    }

    return p;
}


static
const char* string_substr(const char* s, unsigned int a, unsigned int b)
{
    return string_copy(s + a, s + b);
}


static
const char* string_concat(const char* a, const char* s, const char* e)
{
    unsigned int an;
    unsigned int sn;
    char*        p;
        
    an = a ? strlen(a) : 0;
    sn = e - s; 
    p = (char*)(memory_realloc(0, an + sn + 1));
    if (p)
    {
        if (a)
            memcpy(p, a, an);
        memcpy(p + an, s, sn);
        p[an + sn] = '\0';
    }

    return p;
}


static
int string_equal(const char* a, const char* s, const char* e)
{
    while (*a++ == *s++ && s != e)
        ;

    return (*a == '\0' && s == e);
}


static
int string_find_last(const char* s, char c, unsigned int* p)
{
    const char* e;

    e = s + strlen(s);
    while (e > s)
    {
        e--;

        if (*e == c)
        {
            *p = e - s;
            return 1;
        }
    }

    return 0;
}


static
int is_whitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\r');
}


static
int is_newline(char c)
{
    return (c == '\n');
}


static
int is_digit(char c)
{
    return (c >= '0' && c <= '9');
}


static
int is_exponent(char c)
{
    return (c == 'e' || c == 'E');
}


static
const char* skip_whitespace(const char* ptr)
{
    while (is_whitespace(*ptr))
        ptr++;

    return ptr;
}


static
const char* skip_line(const char* ptr)
{
    while (!is_newline(*ptr++))
        ;

    return ptr;
}


static
fastObjGroup group_default(void)
{
    fastObjGroup group;

    group.name       = 0;
    group.face_count = 0;
    group.materials  = 0;
    group.vertices   = 0;
    group.indices    = 0;

    return group;
}


static
void group_clean(fastObjGroup* group)
{
    memory_dealloc((void*)(group->name));

    array_clean(group->materials);
    array_clean(group->vertices);
    array_clean(group->indices);
}


static
void flush_output(fastObjData* data)
{
    /* Add group if not empty */
    if (data->group.face_count > 0)
        array_push(data->mesh->groups, data->group);
    else
        group_clean(&data->group);

    /* Reset for more data */
    data->group = group_default();
}


static
const char* parse_int(const char* ptr, int* val)
{
    int sign;
    int num;


    if (*ptr == '-')
    {
        sign = -1;
        ptr++;
    }
    else
    {
        sign = +1;
    }

    num = 0;
    while (is_digit(*ptr))
        num = 10 * num + (*ptr++ - '0');

    *val = sign * num;

    return ptr;
}


static
const char* parse_float(const char* ptr, float* val)
{
    double        sign;
    double        num;
    double        fra;
    double        div;
    int           eval;
    const double* powers;


    ptr = skip_whitespace(ptr);

    switch (*ptr)
    {
    case '+':
        sign = 1.0;
        ptr++;
        break;

    case '-':
        sign = -1.0;
        ptr++;
        break;

    default:
        sign = 1.0;
        break;
    }


    num = 0.0;
    while (is_digit(*ptr))
        num = 10.0 * num + (double)(*ptr++ - '0');

    if (*ptr == '.')
        ptr++;

    fra = 0.0;
    div = 1.0;

    while (is_digit(*ptr))
    {
        fra  = 10.0 * fra + (double)(*ptr++ - '0');
        div *= 10.0;
    }

    num += fra / div;

    if (is_exponent(*ptr))
    {
        ptr++;

        switch (*ptr)
        {
        case '+':
            powers = POWER_10_POS;
            ptr++;
            break;

        case '-':
            powers = POWER_10_NEG;
            ptr++;
            break;

        default:
            powers = POWER_10_POS;
            break;
        }

        eval = 0;
        while (is_digit(*ptr))
            eval = 10 * eval + (*ptr++ - '0');

        num *= (eval >= MAX_POWER) ? 0.0 : powers[eval];
    }

    *val = (float)(sign * num);

    return ptr;
}


static
const char* parse_vertex(fastObjData* data, const char* ptr)
{
    unsigned int ii;
    float        v;


    for (ii = 0; ii < 3; ii++)
    {
        ptr = parse_float(ptr, &v);
        array_push(data->mesh->positions, v);
    }

    return ptr;
}


static
const char* parse_texcoord(fastObjData* data, const char* ptr)
{
    unsigned int ii;
    float        v;


    for (ii = 0; ii < 2; ii++)
    {
        ptr = parse_float(ptr, &v);
        array_push(data->mesh->texcoords, v);
    }

    return ptr;
}


static
const char* parse_normal(fastObjData* data, const char* ptr)
{
    unsigned int ii;
    float        v;


    for (ii = 0; ii < 3; ii++)
    {
        ptr = parse_float(ptr, &v);
        array_push(data->mesh->normals, v);
    }

    return ptr;
}


static
const char* parse_face(fastObjData* data, const char* ptr)
{
    unsigned int count;
    fastObjIndex vn;
    int          v;
    int          t;
    int          n;


    ptr = skip_whitespace(ptr);

    count = 0;
    while (!is_newline(*ptr))
    {
        v = 0;
        t = 0;
        n = 0;

        ptr = parse_int(ptr, &v);
        if (*ptr == '/')
        {
            ptr++;
            if (*ptr != '/')
                ptr = parse_int(ptr, &t);

            if (*ptr == '/')
            {
                ptr++;
                ptr = parse_int(ptr, &n);
            }
        }

        if (v < 0)
            vn.p = array_size(data->mesh->positions) - (unsigned int)(-v);
        else
            vn.p = (unsigned int)(v);

        if (t < 0)
            vn.t = array_size(data->mesh->texcoords) - (unsigned int)(-t);
        else if (t > 0)
            vn.t = (unsigned int)(t);
        else
            vn.t = 0;

        if (n < 0)
            vn.n = array_size(data->mesh->normals) - (unsigned int)(-n);
        else if (n > 0)
            vn.n = (unsigned int)(n);
        else
            vn.n = 0;

        array_push(data->group.indices, vn);
        count++;

        ptr = skip_whitespace(ptr);
    }

    array_push(data->group.vertices, count);
    array_push(data->group.materials, data->material);

    data->group.face_count++;

    return ptr;
}


static
const char* parse_group(fastObjData* data, const char* ptr)
{
    const char* s;
    const char* e;


    ptr = skip_whitespace(ptr);

    s = ptr;
    while (!is_whitespace(*ptr) && !is_newline(*ptr))
        ptr++;

    e = ptr;

    flush_output(data);
    data->group.name = string_copy(s, e);

    return ptr;
}


static
fastObjMaterial mtl_default(void)
{
    fastObjMaterial mtl;

    mtl.name = 0;

    mtl.Ka[0] = 0.0;
    mtl.Ka[1] = 0.0;
    mtl.Ka[2] = 0.0;
    mtl.Kd[0] = 1.0;
    mtl.Kd[1] = 1.0;
    mtl.Kd[2] = 1.0;
    mtl.Ks[0] = 0.0;
    mtl.Ks[1] = 0.0;
    mtl.Ks[2] = 0.0;
    mtl.Ke[0] = 0.0;
    mtl.Ke[1] = 0.0;
    mtl.Ke[2] = 0.0;
    mtl.Kt[0] = 0.0;
    mtl.Kt[1] = 0.0;
    mtl.Kt[2] = 0.0;
    mtl.Ns    = 1.0;
    mtl.Ni    = 1.0;
    mtl.Tr    = 0.0;

    return mtl;
}


static
const char* parse_usemtl(fastObjData* data, const char* ptr)
{
    const char*      s;
    const char*      e;
    unsigned int     idx;
    fastObjMaterial* mtl;


    ptr = skip_whitespace(ptr);

    /* Parse the material name */
    s = ptr;
    while (!is_whitespace(*ptr) && !is_newline(*ptr))
        ptr++;

    e = ptr;


    /* If there are no materials yet, add a dummy invalid material at index 0 */
    if (array_empty(data->mesh->materials))
        array_push(data->mesh->materials, mtl_default());


    /* Find an existing material with the same name */
    idx = 0;
    while (idx < array_size(data->mesh->materials))
    {
        mtl = &data->mesh->materials[idx];
        if (string_equal(mtl->name, s, e))
            break;

        idx++;
    }

    if (idx == array_size(data->mesh->materials))
        idx = 0;

    data->material = idx;

    return ptr;
}


static
void mtl_clean(fastObjMaterial* mtl)
{
    memory_dealloc((void*)(mtl->name));
}


static
const char* read_mtl_single(const char* p, float* v)
{
    return parse_float(p, v);
}


static
const char* read_mtl_triple(const char* p, float v[3])
{
    p = read_mtl_single(p, &v[0]);
    p = read_mtl_single(p, &v[1]);
    p = read_mtl_single(p, &v[2]);

    return p;
}


static
int read_mtllib(fastObjData* data, void* file)
{
    unsigned int    n;
    const char*     s;
    char*           contents;
    unsigned int    l;
    const char*     p;
    const char*     e;
    fastObjMaterial mtl;


    /* Read entire file */
    n = file_size(file);

    contents = (char*)(memory_realloc(0, n + 1));
    if (!contents)
        return 0;

    l = file_read(file, contents, n);
    contents[l] = '\n';

    mtl = mtl_default();

    p = contents;
    e = contents + l;
    while (p < e)
    {
        p = skip_whitespace(p);

        switch (*p)
        {
        case 'n':
            p++;
            if (p[0] == 'e' &&
                p[1] == 'w' &&
                p[2] == 'm' &&
                p[3] == 't' &&
                p[4] == 'l' &&
                is_whitespace(p[5]))
            {
                /* Push previous material (if there is one) */
                if (mtl.name)
                {
                    array_push(data->mesh->materials, mtl);
                    mtl = mtl_default();
                }


                /* Read name */
                p += 5;

                while (is_whitespace(*p))
                    p++;

                s = p;
                while (!is_whitespace(*p) && !is_newline(*p))
                    p++;

                mtl.name = string_copy(s, p);
            }
            break;

        case 'K':
            if (p[1] == 'a')
                p = read_mtl_triple(p + 2, mtl.Ka);
            else if (p[1] == 'd')
                p = read_mtl_triple(p + 2, mtl.Kd);
            else if (p[1] == 's')
                p = read_mtl_triple(p + 2, mtl.Ks);
            else if (p[1] == 'e')
                p = read_mtl_triple(p + 2, mtl.Ke);
            else if (p[1] == 't')
                p = read_mtl_triple(p + 2, mtl.Kt);
            break;

        case 'N':
            if (p[1] == 's')
                p = read_mtl_single(p + 2, &mtl.Ns);
            else if (p[1] == 'i')
                p = read_mtl_single(p + 2, &mtl.Ni);
            break;

        case 'T':
            if (p[1] == 'r')
                p = read_mtl_single(p + 2, &mtl.Tr);
            break;

        case 'd':
            if (is_whitespace(p[1]))
            {
                float d = 1.0f;;
                p = read_mtl_single(p + 1, &d);
                if (d >= 0.0f && d <= 1.0f)
                    mtl.Tr = 1.0f - d;
            }
            break;

        case '#':
            break;
        }

        p = skip_line(p);
    }

    /* Push final material */
    if (mtl.name)
        array_push(data->mesh->materials, mtl);

    memory_dealloc(contents);

    return 1;
}


static
const char* parse_mtllib(fastObjData* data, const char* ptr)
{
    const char* s;
    const char* e;
    const char* lib;
    void*       file;


    ptr = skip_whitespace(ptr);

    s = ptr;
    while (!is_whitespace(*ptr) && !is_newline(*ptr))
        ptr++;

    e = ptr;

    lib = string_concat(data->base, s, e);
    if (lib)
    {
        file = file_open(lib);
        if (file)
        {
            read_mtllib(data, file);
            file_close(file);
        }

        memory_dealloc((void*)(lib));
    }

    return ptr;
}


static
void parse_buffer(fastObjData* data, const char* ptr, const char* end)
{
    const char* p;
    
    
    p = ptr;
    while (p != end)
    {
        p = skip_whitespace(p);

        switch (*p)
        {
        case 'v':
            p++;

            switch (*p++)
            {
            case ' ':
            case '\t':
                p = parse_vertex(data, p);
                break;

            case 't':
                p = parse_texcoord(data, p);
                break;

            case 'n':
                p = parse_normal(data, p);
                break;
            }
            break;

        case 'f':
            p++;

            switch (*p++)
            {
            case ' ':
            case '\t':
                p = parse_face(data, p);
                break;
            }
            break;

        case 'g':
            p++;

            switch (*p++)
            {
            case ' ':
            case '\t':
                p = parse_group(data, p);
                break;
            }
            break;

        case 'm':
            p++;
            if (p[0] == 't' &&
                p[1] == 'l' &&
                p[2] == 'l' &&
                p[3] == 'i' &&
                p[4] == 'b' &&
                is_whitespace(p[5]))
                p = parse_mtllib(data, p + 5);
            break;

        case 'u':
            p++;
            if (p[0] == 's' &&
                p[1] == 'e' &&
                p[2] == 'm' &&
                p[3] == 't' &&
                p[4] == 'l' &&
                is_whitespace(p[5]))
                p = parse_usemtl(data, p + 5);
            break;

        case '#':
            break;
        }

        p = skip_line(p);

        data->line++;
    }
}


void fast_obj_destroy(fastObjMesh* m)
{
    unsigned int ii;


    for (ii = 0; ii < array_size(m->groups); ii++)
        group_clean(&m->groups[ii]);

    for (ii = 0; ii < array_size(m->materials); ii++)
        mtl_clean(&m->materials[ii]);

    array_clean(m->positions);
    array_clean(m->texcoords);
    array_clean(m->normals);
    array_clean(m->groups);
    array_clean(m->materials);

    memory_dealloc(m);
}


fastObjMesh* fast_obj_read(const char* path)
{
    fastObjData  data;
    fastObjMesh* m;
    void*        file;
    char*        buffer;
    char*        start;
    char*        end;
    char*        last;
    unsigned int read;
    unsigned int sep;
    unsigned int bytes;


    /* Open file */
    file = file_open(path);
    if (!file)
        return 0;


    /* Empty mesh */
    m = (fastObjMesh*)(memory_realloc(0, sizeof(fastObjMesh)));
    if (!m)
        return 0;

    m->positions = 0;
    m->texcoords = 0;
    m->normals   = 0;
    m->materials = 0;
    m->groups    = 0;


    /* Add dummy position/texcoord/normal */
    array_push(m->positions, 0.0f);
    array_push(m->positions, 0.0f);
    array_push(m->positions, 0.0f);

    array_push(m->texcoords, 0.0f);
    array_push(m->texcoords, 0.0f);

    array_push(m->normals, 0.0f);
    array_push(m->normals, 0.0f);
    array_push(m->normals, 1.0f);


    /* Data needed during parsing */
    data.mesh     = m;
    data.group    = group_default();
    data.material = 0;
    data.line     = 1;
    data.base     = 0;


    /* Find base path for materials/textures */
    if (string_find_last(path, '/', &sep))
        data.base = string_substr(path, 0, sep + 1);


    /* Create buffer for reading file */
    buffer = (char*)(memory_realloc(0, 2 * BUFFER_SIZE * sizeof(char)));
    if (!buffer)
        return 0;

    start = buffer;
    for (;;)
    {
        /* Read another buffer's worth from file */
        read = (unsigned int)(file_read(file, start, BUFFER_SIZE));
        if (read == 0 && start == buffer)
            break;


        /* Ensure buffer ends in a newline */
        if (read < BUFFER_SIZE)
        {
            if (read == 0 || start[read - 1] != '\n')
                start[read++] = '\n';
        }

        end = start + read;
        if (end == buffer)
            break;


        /* Find last new line */
        last = end;
        while (last > buffer)
        {
            last--;
            if (*last == '\n')
                break;
        }


        /* Check there actually is a new line */
        if (*last != '\n')
            break;

        last++;


        /* Process buffer */
        parse_buffer(&data, buffer, last);


        /* Copy overflow for next buffer */
        bytes = (unsigned int)(end - last);
        memcpy(buffer, last, bytes);
        start = buffer + bytes;
    }


    /* Flush final group */
    flush_output(&data);
    group_clean(&data.group);

    m->position_count = array_size(m->positions) / 3;
    m->texcoord_count = array_size(m->texcoords) / 2;
    m->normal_count   = array_size(m->normals) / 3;
    m->material_count = array_size(m->materials);
    m->group_count    = array_size(m->groups);


    /* Clean up */
    memory_dealloc(buffer);
    memory_dealloc((void*)(data.base));

    file_close(file);

    return m;
}

#endif
