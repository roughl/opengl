//[VERTEX SHADER]
#version 130
 
in vec3 InVertex;
in vec4 InColor;
 
 
smooth out vec4 Color;
 
uniform mat4 ProjectionModelviewMatrix;
uniform float scale;
 
 
void main()
{
	gl_Position = ProjectionModelviewMatrix * vec4(InVertex, scale);
	Color = InColor;
}

