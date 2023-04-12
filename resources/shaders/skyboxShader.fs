#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    vec4 result = 0.8 * texture(skybox, TexCoords);
    FragColor = result;

}