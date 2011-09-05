#version 110

void main()
{
    gl_FragColor = texture2D(0, gl_TexCoord[0].st) * gl_FragColor;
}
