#version 110

uniform sampler2D blend_text;
uniform sampler2D text1;
uniform sampler2D text2;
uniform sampler2D text3;

void main(void)
{
    vec2 tex_coord = vec2(gl_TexCoord[0]);
    vec4 RGB       = texture2D(blend_text, tex_coord);

    gl_FragColor   = texture2D(text1, tex_coord) * RGB.r +
                     texture2D(text2, tex_coord) * RGB.g +
                     texture2D(text3, tex_coord) * RGB.b;
}
