#version 400

in vec4 in_coords;
out vec2 vs_texcoords;

void main()
{
    gl_Position = vec4(in_coords.xy, 0, 1);
    vs_texcoords = in_coords.zw;
}