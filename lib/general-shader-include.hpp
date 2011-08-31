#include <cmath>


extern "C" void sas_multiply_matrix(float *d, const float *s);
extern "C" void sas_matrix_dot_vector(const float *matrix, float *vector);
extern "C" void sas_multiply_matrix_3x3(float *d, const float *s);
extern "C" void sas_matrix_dot_vector_3x3(const float *matrix, float *vector);


union vec2_raw
{
    float v[2];
    struct { float x, y; } __attribute__((packed));
    struct { float r, g; } __attribute__((packed));
    struct { float s, t; } __attribute__((packed));
};

union vec3_raw
{
    float v[3];
    struct { float x, y, z; } __attribute__((packed));
    struct { float r, g, b; } __attribute__((packed));
    struct { float s, t, p; } __attribute__((packed));
    union vec2_raw xy, rg, st;
};

union vec4_raw
{
    float v[4];
    struct { float x, y, z, w; } __attribute__((packed));
    struct { float r, g, b, a; } __attribute__((packed));
    struct { float s, t, p, q; } __attribute__((packed));
    struct { union vec2_raw xy, zw; } __attribute__((packed));
    struct { union vec2_raw rg, ba; } __attribute__((packed));
    struct { union vec2_raw st, pq; } __attribute__((packed));
    union vec3_raw xyz, rgb, stp;
    struct { float v4_x1; union vec2_raw yz; } __attribute__((packed));
    struct { float v4_r1; union vec2_raw gb; } __attribute__((packed));
    struct { float v4_s1; union vec2_raw tp; } __attribute__((packed));
    struct { float v4_x2; union vec3_raw yzw; } __attribute__((packed));
    struct { float v4_r2; union vec3_raw gba; } __attribute__((packed));
    struct { float v4_s2; union vec3_raw tpq; } __attribute__((packed));
};


class vec3;
class vec4;

class vec2
{
    public:
        vec2(void) { }
        vec2(const union vec2_raw &xy) { v[0] = xy.v[0]; v[1] = xy.v[1]; }
        vec2(float x, float y) { v[0] = x; v[1] = y; }
        vec2(const vec3 &xyz);
        vec2(const vec4 &xyzw);

        float operator[](int i) { return v[i]; }
        vec2 operator*(const vec2 &v) { return vec2(x * v.x, y * v.y); }
        vec2 operator*(float v) { return vec2(x * v, y * v); }
        vec2 operator/(float v) { return vec2(x / v, y / v); }
        vec2 operator+(const vec2 &v) { return vec2(x + v.x, y + v.y); }
        vec2 operator-(const vec2 &v) { return vec2(x - v.x, y - v.y); }

        union
        {
            float v[2];
            struct { float x, y; } __attribute__((packed));
            struct { float r, g; } __attribute__((packed));
            struct { float s, t; } __attribute__((packed));
        };
};

static inline vec2 operator*(float scale, const vec2 &v) { return vec2(v.x * scale, v.y * scale); }

class vec3
{
    public:
        vec3(void) { }
        vec3(const union vec3_raw &xyz) { v[0] = xyz.v[0]; v[1] = xyz.v[1]; v[2] = xyz.v[2]; }
        vec3(float x, float y, float z) { v[0] = x; v[1] = y; v[2] = z; }
        vec3(const vec4 &xyzw);

        float operator[](int i) { return v[i]; }
        vec3 operator*(const vec3 &v) { return vec3(x * v.x, y * v.y, z * v.z); }
        vec3 operator*(float v) { return vec3(x * v, y * v, z * v); }
        vec3 operator/(float v) { return vec3(x / v, y / v, z / v); }
        vec3 operator+(const vec3 &v) { return vec3(x + v.x, y + v.y, z + v.z); }
        vec3 operator-(const vec3 &v) { return vec3(x - v.x, y - v.y, z - v.z); }

        union
        {
            float v[3];
            struct { float x, y, z; } __attribute__((packed));
            struct { float r, g, b; } __attribute__((packed));
            struct { float s, t, p; } __attribute__((packed));
            union vec2_raw xy, rg, st;
            struct { float v3_x1; union vec2_raw yz; } __attribute__((packed));
            struct { float v3_r1; union vec2_raw gb; } __attribute__((packed));
            struct { float v3_s1; union vec2_raw tp; } __attribute__((packed));
        };
};

static inline vec3 operator*(float scale, const vec3 &v) { return vec3(v.x * scale, v.y * scale, v.z * scale); }

class vec4
{
    public:
        vec4(void) { }
        vec4(const union vec4_raw &xyzw) { v[0] = xyzw.v[0]; v[1] = xyzw.v[1]; v[2] = xyzw.v[2]; v[3] = xyzw.v[3]; }
        vec4(float x, float y, float z, float w) { v[0] = x; v[1] = y; v[2] = z; v[3] = w; }
        vec4(const vec2 &xy, float z, float w) { v[0] = xy.v[0]; v[1] = xy.v[1]; v[2] = z; v[3] = w; }
        vec4(const vec3 &xyz) { v[0] = xyz.v[0]; v[1] = xyz.v[1]; v[2] = xyz.v[2]; v[3] = 0.f; }
        vec4(const vec3 &xyz, float w) { v[0] = xyz.v[0]; v[1] = xyz.v[1]; v[2] = xyz.v[2]; v[3] = w; }

        float operator[](int i) { return v[i]; }
        vec4 operator*(const vec4 &v) { return vec4(x * v.x, y * v.y, z * v.z, w * v.w); }
        vec4 operator*(float v) { return vec4(x * v, y * v, z * v, w * v); }
        vec4 operator/(float v) { return vec4(x / v, y / v, z / v, w / v); }
        vec4 operator+(const vec4 &v) { return vec4(x + v.x, y + v.y, z + v.z, w + v.w); }
        vec4 operator-(const vec4 &v) { return vec4(x - v.x, y - v.y, z - v.z, w - v.w); }

        union
        {
            float v[4];
            struct { float x, y, z, w; } __attribute__((packed));
            struct { float r, g, b, a; } __attribute__((packed));
            struct { float s, t, p, q; } __attribute__((packed));
            struct { union vec2_raw xy, zw; } __attribute__((packed));
            struct { union vec2_raw rg, ba; } __attribute__((packed));
            struct { union vec2_raw st, pq; } __attribute__((packed));
            union vec3_raw xyz, rgb, stp;
            struct { float v4_x1; union vec2_raw yz; } __attribute__((packed));
            struct { float v4_r1; union vec2_raw gb; } __attribute__((packed));
            struct { float v4_s1; union vec2_raw tp; } __attribute__((packed));
            struct { float v4_x2; union vec3_raw yzw; } __attribute__((packed));
            struct { float v4_r2; union vec3_raw gba; } __attribute__((packed));
            struct { float v4_s2; union vec3_raw tpq; } __attribute__((packed));
        };
};

static inline vec4 operator*(float scale, const vec4 &v) { return vec4(v.x * scale, v.y * scale, v.z * scale, v.w * scale); }

class mat3;

class mat4
{
    public:
        mat4(const mat3 &m);

        float operator[](int i) { return v[i]; }
        mat4 operator*(const mat4 &m)
        {
            mat4 ret(*this);
            sas_multiply_matrix(ret.v, m.v);
            return ret;
        }
        vec4 operator*(const vec4 &vec)
        {
            vec4 ret(vec);
            sas_matrix_dot_vector(v, ret.v);
            return ret;
        }

        float v[16];
};

class mat3
{
    public:
        mat3(const mat4 &m) { for (int y = 0; y < 3; y++) for (int x = 0; x < 3; x++) v[x * 3 + y] = m.v[x * 4 + y]; }

        float operator[](int i) { return v[i]; }
        mat3 operator*(const mat3 &m)
        {
            mat3 ret(*this);
            sas_multiply_matrix_3x3(ret.v, m.v);
            return ret;
        }
        vec3 operator*(const vec3 &vec)
        {
            vec3 ret(vec);
            sas_matrix_dot_vector_3x3(v, ret.v);
            return ret;
        }

        float v[9];
};

typedef int sampler2D;


#define gl_ModelViewMatrix sas_modelview
#define gl_ProjectionMatrix sas_projection
#define gl_ModelViewProjectionMatrix sas_modelviewprojection
#define gl_NormalMatrix sas_normal_matrix

#define gl_TexCoord sas_current_texcoord
#define gl_FrontColor sas_current_color


extern "C" vec4 gl_TexCoord[8];

extern "C" mat4 gl_ModelViewProjectionMatrix;
extern "C" mat3 gl_NormalMatrix;
extern "C" mat4 gl_ModelViewMatrix, gl_ProjectionMatrix;

extern "C" vec4 gl_FrontColor;


// I do love casts without involving the compiler (this function actually
// returns sas_color_t).
extern "C" vec4 sas_texture_get(int unit, float u, float v);


static inline vec4 texture2D(sampler2D texture, vec2 coords) __attribute__((unused));
static inline vec4 texture2D(sampler2D texture, vec2 coords)
{
    return sas_texture_get(texture, coords.s, coords.t);
}

template<typename T> static inline T mod(T x, T y);
static inline float mod(float x, float y) { return fmodf(x, y); }
static inline int mod(int x, int y) { return x % y; }

template<typename T> static inline T abs(T x);
static inline float abs(float x) { return fabsf(x); }
static inline int abs(int x) { return (x < 0) ? -x : x; }

template<typename T> static inline T max(T x, T y) { return (x > y) ? x : y; }
template<typename T> static inline T min(T x, T y) { return (x < y) ? x : y; }

static inline vec2 mix(vec2 x, vec2 y, float a) { return x * (1.f - a) + y * a; }
static inline vec3 mix(vec3 x, vec3 y, float a) { return x * (1.f - a) + y * a; }
static inline vec4 mix(vec4 x, vec4 y, float a) { return x * (1.f - a) + y * a; }
static inline float mix(float x, float y, float a) { return x * (1.f - a) + y * a; }


template<typename T> static inline T normalize(T vec);

static inline vec2 normalize(vec2 vec)
{
    float len = sqrtf(vec.x * vec.x + vec.y * vec.y);
    return vec / len;
}

static inline vec3 normalize(vec3 vec)
{
    float len = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    return vec / len;
}

static inline vec4 normalize(vec4 vec)
{
    float len = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
    return vec / len;
}


template<typename T> static inline float dot(T x, T y);
static inline float dot(vec2 x, vec2 y) { return x.x * y.x + x.y * y.y; }
static inline float dot(vec3 x, vec3 y) { return x.x * y.x + x.y * y.y + x.z * y.z; }
static inline float dot(vec4 x, vec4 y) { return x.x * y.x + x.y * y.y + x.z * y.z + x.w * y.w; }


static inline vec2 reflect(vec2 in, vec2 normal) { return in - 2 * dot(normal, in) * normal; }
static inline vec3 reflect(vec3 in, vec3 normal) { return in - 2 * dot(normal, in) * normal; }
static inline vec4 reflect(vec4 in, vec4 normal) { return in - 2 * dot(normal, in) * normal; }
