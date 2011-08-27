#version 110

uniform sampler2D display_texture;

void main()
{
    gl_FragColor = texture2D(display_texture, gl_TexCoord[0].st) * gl_FragColor;
}
