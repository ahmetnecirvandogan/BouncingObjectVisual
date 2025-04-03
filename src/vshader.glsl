#version 330 core

in vec4 vPosition;
in vec4 vColor;
in vec3 vNormal; // Added for lighting

out vec4 color;
out vec3 Normal; // Pass to fragment shader
out vec3 FragPos; // For lighting calculations

uniform mat4 ModelView;
uniform mat4 Projection;

void main()
{
    gl_Position = Projection * ModelView * vPosition;
    FragPos = vec3(ModelView * vPosition);
    Normal = mat3(transpose(inverse(ModelView))) * vNormal; // Transform normals correctly
    color = vColor;
}

