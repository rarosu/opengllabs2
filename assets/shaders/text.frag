#version 440

in vec2 vs_texcoords;

uniform sampler2D texture;
uniform vec4 color;

void main()
{
    gl_FragColor = vec4(1.0f, 1.0f, 1.0, texture2D(texture, vs_texcoords).a) * color;
}