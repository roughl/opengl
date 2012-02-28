#include <iostream>
#include <fstream>
#include <cstdlib>

#include <SDL/SDL.h>
#include <GL/glew.h>


using namespace std;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))


//Vertex, color
//
//SIZE : 4+4+4 +4 = 4*4 = 16 bytes
//It's better to make it multiple of 32
//32-16 = 16 bytes (of garbage should be added)
//16/4 = 4 floats should be added
struct TVertex_VC
{
	float	x, y, z;
	unsigned int	color;
	float	padding[4];
};

TVertex_VC	quadVertex[] = {
		{-0.8, -0.5, -0.9, 0xFFFFFFFF, {0,0,0,0}} ,
		{ 0.0, -0.5, -0.9, 0xFFFF0000, {0,0,0,0}} ,
		{-0.8,  0.5, -0.9, 0xFF00FF00, {0,0,0,0}} ,
		{ 0.0,  0.5, -0.9, 0xFF0000FF, {0,0,0,0}}
	};
GLushort	quadIndex[6] =  { 0,1,2,2,1,3 };

//1 VAO for the quad
//1 VAO for the triangle
GLuint VAOID[2];
//1 IBO for the quad (Index Buffer Object)
//1 IBO for the triangle
GLuint IBOID[2];
//1 IBO for the quad (Vertex Buffer Object)
//1 IBO for the triangle
GLuint VBOID[2];

GLuint	ShaderProgram[2];
GLuint	VertexShader[2];
GLuint	FragmentShader[2];

int ProjectionModelviewMatrix_Loc[2];		//The location of ProjectionModelviewMatrix in the shaders
int scale_Loc;
float scale;

void InitGLStates()
{
	glShadeModel(GL_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);
	glDisable(GL_STENCIL_TEST);
	glStencilMask(0xFFFFFFFF);
	glStencilFunc(GL_EQUAL, 0x00000000, 0x00000001);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glClearStencil(0);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DITHER);
	glActiveTexture(GL_TEXTURE0);
}

char* loadFile(const char *fname, GLint &fSize)
{
	ifstream::pos_type size;
	char *memblock;
	string text;
 
	// file read based on example in cplusplus.com tutorial
	ifstream file (fname, ios::in|ios::binary|ios::ate);
	if (file.is_open())
	{
		size = file.tellg();
		fSize = (GLuint) size;
		memblock = new char[size];
		file.seekg (0, ios::beg);
		file.read (memblock, size);
		file.close();
		cout << "file " << fname << " loaded" << endl;
		text.assign(memblock);
	}
	else
	{
		cout << "Unable to open file " << fname << endl;
		exit(1);
	}
 
	return memblock;
}

void printShaderInfoLog(GLint shader)
{
	int infoLogLen = 0;
	int charsWritten = 0;
	GLchar *infoLog;
 
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
 
	if (infoLogLen > 0)
	{
		infoLog = new GLchar[infoLogLen];
		// error check for fail to allocate memory omitted
		glGetShaderInfoLog(shader, infoLogLen, &charsWritten, infoLog);
		cout << "InfoLog : " << endl << infoLog << endl;
		delete [] infoLog;
	}
} 

int LoadShader(const char *pfilePath_vs, const char *pfilePath_fs, bool bindTexCoord0, bool bindNormal, bool bindColor, GLuint &shaderProgram, GLuint &vertexShader, GLuint &fragmentShader)
{
	shaderProgram=0;
	vertexShader=0;
	fragmentShader=0;
 
	char *vertexShaderString, *fragmentShaderString;
 
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);	
 
	// load shaders & get length of each
	int vlen;
	int flen;
	vertexShaderString = loadFile(pfilePath_vs, vlen);
	fragmentShaderString = loadFile(pfilePath_fs, flen);
 
	if(vertexShaderString==NULL)
	{
		glDeleteShader(vertexShader);
		vertexShader=0;
		glDeleteShader(fragmentShader);
		fragmentShader=0;
 
		return -1;
	}
 
	if(fragmentShaderString==NULL)
	{
		glDeleteShader(vertexShader);
		vertexShader=0;
		glDeleteShader(fragmentShader);
		fragmentShader=0;
 
		delete [] vertexShaderString;
 
		return -1;
	}
 
	glShaderSource(vertexShader, 1, (const GLchar **)&vertexShaderString, &vlen);
	glShaderSource(fragmentShader, 1, (const GLchar **)&fragmentShaderString, &flen);
 
	GLint compiled;
 
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
	if(compiled==false)
	{
		cout << "Vertex shader not compiled." << endl;
		printShaderInfoLog(vertexShader);
 
		glDeleteShader(vertexShader);
		vertexShader=0;
		glDeleteShader(fragmentShader);
		fragmentShader=0;
 
		delete [] vertexShaderString;
		delete [] fragmentShaderString;
 
		return -1;
	}
 
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
	if(compiled==false)
	{
		cout << "Fragment shader not compiled." << endl;
		printShaderInfoLog(fragmentShader);
 
		glDeleteShader(vertexShader);
		vertexShader=0;
		glDeleteShader(fragmentShader);
		fragmentShader=0;
 
		delete [] vertexShaderString;
		delete [] fragmentShaderString;
 
		return -1;
	}
 
	shaderProgram = glCreateProgram();
 
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
 
	glBindAttribLocation(shaderProgram, 0, "InVertex");
 
	if(bindTexCoord0)
		glBindAttribLocation(shaderProgram, 1, "InTexCoord0");
 
	if(bindNormal)
		glBindAttribLocation(shaderProgram, 2, "InNormal");
 
	if(bindColor)
		glBindAttribLocation(shaderProgram, 3, "InColor");
 
	glLinkProgram(shaderProgram);
 
	GLint IsLinked;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, (GLint *)&IsLinked);
	if(IsLinked==false)
	{
		cout << "Failed to link shader." << endl;
 
		GLint maxLength;
		char *pLinkInfoLog;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
		if(maxLength>0)
		{
			pLinkInfoLog=new char[maxLength];
			glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, pLinkInfoLog);
			cout<<pLinkInfoLog<<endl;
			delete [] pLinkInfoLog;
		}
 
		glDetachShader(shaderProgram, vertexShader);
		glDetachShader(shaderProgram, fragmentShader);
		glDeleteShader(vertexShader);
		vertexShader=0;
		glDeleteShader(fragmentShader);
		fragmentShader=0;
		glDeleteProgram(shaderProgram);
		shaderProgram=0;
 
		delete [] vertexShaderString;
		delete [] fragmentShaderString;
 
		return -1;
	}
 
	delete [] vertexShaderString;
	delete [] fragmentShaderString;
 
	return 1;		//Success
}

void createGeometry()
{
	scale = 1;
	glGenBuffers(1, &IBOID[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOID[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(GLushort), quadIndex, GL_STATIC_DRAW );

	glGenBuffers(1, &VBOID[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOID[0]);
	glBufferData(GL_ARRAY_BUFFER, 4*sizeof(TVertex_VC), quadVertex, GL_STATIC_DRAW);

	//VAO for the quad *********************
	glGenVertexArrays(1, &VAOID[0]);
	glBindVertexArray(VAOID[0]);

	//Bind the VBO and setup pointers for the VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBOID[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TVertex_VC), BUFFER_OFFSET(0));
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TVertex_VC), BUFFER_OFFSET(sizeof(float)*3));
	glEnableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
 
	//Bind the IBO for the VAO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOID[0]);
}

void display()
{
	static float speed = 0.0005;
	static float val = 1+speed;
	scale*=val;
	//scale = 1;
	if(scale > 10.0 || scale < 0.9) {
		speed = -speed;
		val += 2*speed;
	}
		
	float projectionModelviewMatrix[16];

	memset(projectionModelviewMatrix, 0, sizeof(float)*16);
	projectionModelviewMatrix[0] = 1;
	projectionModelviewMatrix[5] = 1;
	projectionModelviewMatrix[10] = 1.0;
	projectionModelviewMatrix[15] = 1.0;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	glUseProgram( ShaderProgram[0] );

	glUniformMatrix4fv( ProjectionModelviewMatrix_Loc[0], 1, false, projectionModelviewMatrix);
	glUniform1f( scale_Loc, scale);
	glBindVertexArray(VAOID[0]);
	// texture here

	glDrawRangeElements( GL_TRIANGLES, 0, 3, 6, GL_UNSIGNED_SHORT, NULL );

	SDL_GL_SwapBuffers();
}

void glCleanup()
{
	cout << "cleaning up...";
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0 );

	glUseProgram(0);

	glDeleteBuffers(1, &IBOID[0]);
	glDeleteBuffers(1, &VBOID[0]);
	glDeleteVertexArrays(1, &VAOID[0]);

	glDetachShader(ShaderProgram[0], VertexShader[0]);
	glDetachShader(ShaderProgram[0], FragmentShader[0]);
	glDeleteShader(VertexShader[0]);
	glDeleteShader(FragmentShader[0]);
	glDeleteProgram(ShaderProgram[0]);
	cout << " finished" << endl;
}

int main(void)
{
	atexit(SDL_Quit);

	if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		cerr << "SDL_Init failed: " << SDL_GetError() << endl;
		exit(1);
	}
	Uint32 flags = 0;
	flags = SDL_OPENGL | SDL_HWSURFACE | SDL_RESIZABLE;

	if( !SDL_SetVideoMode(800, 600, 0, flags) ) {
		cerr << "SDL_SetVideoMode failed: " << SDL_GetError() << endl;
		exit(2);
	}
	glewExperimental = true;
	if( glewInit() != GLEW_OK) {
		cerr << "glewInit failed" << endl;
		exit(3);
	}

	int openGLVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &openGLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &openGLVersion[1]);
	cout<<"OpenGL major version = "<<openGLVersion[0]<<endl;
	cout<<"OpenGL minor version = "<<openGLVersion[1]<<endl<<endl;

	int numberOfExtensions;
	glGetIntegerv( GL_NUM_EXTENSIONS, &numberOfExtensions );
	for( int i=0; i<numberOfExtensions; i++ ) {
		cout << glGetStringi( GL_EXTENSIONS, i ) << " ";
	}
	cout << endl;
	
	InitGLStates();
	atexit(glCleanup);

	if( LoadShader("shader1.vert", "shader1.frag", false, false, true, ShaderProgram[0], VertexShader[0], FragmentShader[0] ) < 0 ) {
		exit(4);
	}
	ProjectionModelviewMatrix_Loc[0] = glGetUniformLocation( ShaderProgram[0], "ProjectionModelviewMatrix");
	scale_Loc = glGetUniformLocation( ShaderProgram[0], "scale");
	
	createGeometry();
	SDL_Event event;
	Uint32 OldTime = SDL_GetTicks();
	Uint32 frames = 0;
	for(bool done; !done;) {
		if(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					done = 1;
					break;
			}
		}

		display();
		Uint32 Time = SDL_GetTicks();
		Uint32 diff = Time-OldTime;
		frames++;
		if(diff > 5000) {
			cout << frames << " frames in " << (double)diff/1000.0 << " seconds FPS: " << (double)frames*1000/(double)(diff) << endl;
			OldTime = Time;
			frames=0;
		}
	};
	return 0;
}
