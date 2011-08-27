extern "C" void sas_multiply_matrix(float *d, float *s);
extern "C" void sas_matrix_dot_vector(float *matrix, float *vector);

class vec2
{
    public:
        float operator[](int i) { return v[i]; }

        union
        {
            float v[2];
            struct { float x, y; } __attribute__((packed));
            struct { float r, g; } __attribute__((packed));
            struct { float s, t; } __attribute__((packed));
        };
};

class vec4
{
    public:
        vec4(float x, float y, float z, float w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
        vec4(vec2 xy, float z, float w) { v[0] = xy.v[0]; v[1] = xy.v[1]; v[2] = z; v[3] = w; }

        float operator[](int i) { return v[i]; }
        vec4 operator*(vec4 &v) { return vec4(x * v.x, y * v.y, z * v.z, w * v.w); }

        union
        {
            float v[4];
            struct { float x, y, z, w; } __attribute__((packed));
            struct { vec2 xy, zw; } __attribute__((packed));
            struct { float r, g, b, a; } __attribute__((packed));
            struct { float s, t, p, q; } __attribute__((packed));
            struct { vec2 st, pq; } __attribute__((packed));
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

typedef int sampler2D;

#define gl_ModelViewMatrix sas_modelview
#define gl_ProjectionMatrix sas_projection
#define gl_ModelViewProjectionMatrix sas_modelviewprojection
#define gl_TexCoord sas_current_texcoord
#define gl_FragCoord sas_current_position
#define gl_FrontColor sas_current_color
#define gl_FragColor sas_current_color

extern "C" vec4 gl_Vertex, gl_Position;
extern "C" vec4 gl_TexCoord[8], gl_MultiTexCoord0;

extern "C" mat4 gl_ModelViewProjectionMatrix;
extern "C" mat4 gl_ModelViewMatrix, gl_ProjectionMatrix;

extern "C" vec4 gl_FragColor, gl_FragCoord;

extern "C" void sas_fragment_transform(void);

static vec4 texture2D(sampler2D texture, vec2 coords)
{
    (void)texture;
    (void)coords;

    return vec4(1.f, 1.f, 1.f, 1.f);
}
