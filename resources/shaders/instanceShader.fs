#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse;

struct Material {
    sampler2D diffuse;
    sampler2D specular;

    float shininess;
};

uniform Material material;

void main()
{
  FragColor = texture(texture_diffuse, TexCoords);
}