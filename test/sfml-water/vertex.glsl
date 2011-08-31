#version 110

varying vec3 vertex_pos;

void main(void)
{
    vertex_pos = vec3(gl_Vertex);

    gl_Position = ftransform();
    gl_FrontColor = gl_Color;
    gl_TexCoord[0] = gl_MultiTexCoord0;
}
