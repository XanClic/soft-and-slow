extern "C" void sas_multiply_matrix(float *d, float *s);
extern "C" void sas_matrix_dot_vector(float *matrix, float *vector);

class vec4
{
    public:
        float operator[](int i) { return v[i]; }

        union
        {
            float v[4];
            struct { float x, y, z, w; } __attribute__((packed));
            struct { float r, g, b, a; } __attribute__((packed));
            struct { float s, t, p, q; } __attribute__((packed));
        };
};

class mat4
{
    public:
        float operator[](int i) { return v[i]; }
        mat4 operator*(mat4 &m)
        {
            mat4 ret(*this);
            sas_multiply_matrix(ret.v, m.v);
            return ret;
        }
        vec4 operator*(vec4 &vec)
        {
            vec4 ret(vec);
            sas_matrix_dot_vector(v, ret.v);
            return ret;
        }

        float v[16];
};

#define gl_ModelViewMatrix sas_modelview
#define gl_ProjectionMatrix sas_projection
#define gl_ModelViewProjectionMatrix sas_modelviewprojection
#define gl_Color sas_current_color
#define gl_Vertex sas_current_vertex
#define gl_Position sas_current_position
#define gl_TexCoord sas_current_texcoord
#define gl_MultiTexCoord0 sas_multi_texcoord0

extern "C" vec4 gl_Vertex, gl_Position;
extern "C" vec4 gl_TexCoord[8], gl_MultiTexCoord0;

extern "C" mat4 gl_ModelViewProjectionMatrix;
extern "C" mat4 gl_ModelViewMatrix, gl_ProjectionMatrix;

extern "C" vec4 gl_Color;

static vec4 ftransform(void)
{
    return gl_ModelViewProjectionMatrix * gl_Vertex;
}

extern "C" void sas_vertex_transform(void);
