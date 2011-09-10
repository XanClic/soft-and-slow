#define gl_FragCoord __dti->current_position
#define gl_FragColor __dti->current_color

#define gl_TexCoord __dti->current_texcoord
#define gl_FrontColor __dti->current_color


struct __draw_thread_info
{
    vec4 current_position;
    vec4 current_color;
    vec4 current_texcoord[8];
};

extern "C" void sas_fragment_transform(__draw_thread_info *__dti);
