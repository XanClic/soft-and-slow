#define gl_FragCoord sas_current_position
#define gl_FragColor sas_current_color


extern "C" vec4 gl_FragColor, gl_FragCoord;


extern "C" void sas_fragment_transform(void);
