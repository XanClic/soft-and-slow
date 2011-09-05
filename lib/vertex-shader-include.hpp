#define gl_Color sas_current_color
#define gl_Vertex sas_current_vertex
#define gl_Position sas_current_position
#define gl_Normal sas_current_normal
#define gl_MultiTexCoord0 sas_multi_texcoord0


extern "C" vec4 gl_Position;
extern "C" const vec4 gl_Vertex, gl_MultiTexCoord0;

extern "C" const vec3 gl_Normal;

// Should be const, but cannot be, since gl_FrontColor is effectively the same
// variable without a const.
extern "C" vec4 gl_Color;


extern "C" void sas_vertex_transform(void);


static vec4 ftransform(void) __attribute__((unused));
static vec4 ftransform(void)
{
    return gl_ModelViewProjectionMatrix * gl_Vertex;
}
