#version 330 core

in vec4 vPosition;

uniform mat4 ModelView;
uniform mat4 Projection;
//uniform vec4 disp;

void main()
{
    gl_Position = Projection * ModelView * vPosition;
    //gl_Position = Projection * (vPosition + disp);
    
}
