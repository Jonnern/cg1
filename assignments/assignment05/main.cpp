////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013, Computer Graphics Group RWTH Aachen University         //
// All rights reserved.                                                       //
////////////////////////////////////////////////////////////////////////////////
/*
 * Basics of Computer Graphics Exercise
 *
 * DO NOT EDIT THIS FILE!
 */

#include <gl_core_32.hh>
#include <GLFW/glfw3.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <limits>

#include "assignment.h"

// OpenGL Math:
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// some variables we'll need:
//
const unsigned int windowWidth  = 512;
const unsigned int windowHeight = 512;
float nearPlane =   0.1f;
float farPlane  = 100.0f;
GLFWwindow* g_window;

// assigment specific includes:

// assignment specific variables:
GLuint vs, fs, prog;
const char *vsrc  = "#version 150\n in vec4 aPosition; \n out vec2 texCoord; \n void main() { texCoord = aPosition.xy/2.0; texCoord += 0.5; gl_Position = aPosition; }\n";
const char *fsrc  = "#version 150\n out vec4 oColor; \n in vec2 texCoord; \n uniform sampler2D tex; \n void main() { oColor = texture(tex, texCoord); }\n";
GLuint arrayBuffer;
GLuint vao;
GLuint texture;
unsigned char textureData[windowWidth*windowHeight*4];


//
// Can be usefull if your hardware supports ARB_debug_output. If you have compile problems with this,
// just comment it out.
//
void debugCallback(GLenum source, GLenum type, GLuint id,
                   GLenum severity, GLsizei length,
                   const GLchar *message, const void *userParam)
{
     cout << "Note: ";
     if (source == GL_DEBUG_SOURCE_API_ARB)
            cout << "OpenGL";
     else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
            cout << "your OS";
     else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
            cout << "the Shader Compiler";
     else if (source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
            cout << "a third party component";
     else if (source == GL_DEBUG_SOURCE_APPLICATION_ARB)
            cout << "your application";
     else if (source == GL_DEBUG_SOURCE_OTHER_ARB)
            cout << "someone";

     cout << " reported a problem - it's a";
     if (type == GL_DEBUG_TYPE_ERROR_ARB)
            cout << "n error";
     else if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
            cout << " deprecated behavior";
     else if(type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
            cout << "n undefined behavior";
     else if(type == GL_DEBUG_TYPE_PORTABILITY_ARB)
            cout << " portability issue";
     else if(type == GL_DEBUG_TYPE_PERFORMANCE_ARB)
            cout << " performance issue";
     else if(type == GL_DEBUG_TYPE_OTHER_ARB)
            cout << " something";

     cout << endl;
     cout << "The message was: " << message << endl << endl;
}



/**********************************************************************************************************************
 * Returns true if a window with the desired context could get created.
 * Requested OpenGL version gets set by ACGL defines.
 */
bool createWindow( bool forceOpenGL32 )
{
    /////////////////////////////////////////////////////////////////////////////////////
    // Configure OpenGL context
    //
	if (forceOpenGL32) {
		// request OpenGL 3.2, will return a 4.1 context on Mavericks as well
		glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
		glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	}

    // activate multisampling (second parameter is the number of samples):
    //glfwWindowHint( GLFW_SAMPLES, 8 );

    // request an OpenGL debug context:
    glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, true );

    // define whether the window can get resized:
    glfwWindowHint( GLFW_RESIZABLE, false );

    /////////////////////////////////////////////////////////////////////////////////////
    // try to create an OpenGL context in a window and check the supported OpenGL version:
    //                                                  R,G,B,A, Depth,Stencil
    g_window = glfwCreateWindow( windowWidth, windowHeight, "Basic Techniques in Computer Graphics", NULL, NULL);
    if (!g_window) {
        cerr << "Failed to open a GLFW window" << endl;
        return false;
    }
    glfwMakeContextCurrent(g_window);

    return true;
}

void initializeOpenGL() 
{
	int glInit = ogl_LoadFunctionsForDebug( GL_TRUE, GL_TRUE );
	
	if (glInit != ogl_LOAD_SUCCEEDED) {
		cerr << "could not initialize OpenGL" << endl;
		exit(1);
	}
	
	if (ogl_ext_KHR_debug) {
		glDebugMessageCallback( debugCallback, NULL );
	}
    
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glEnable( GL_DEPTH_TEST );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void checkForShaderCompileErrors( GLuint _shader )
{
    GLint shaderError;
    glGetShaderiv(_shader, GL_COMPILE_STATUS, &shaderError);
    if(shaderError != GL_TRUE)
    {
        // yes, errors
        cerr << "Shader compile issue: " << endl;
    }

    // a log gets always printed (could be warnings)
    GLsizei length = 0;
    glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &length);
    if(length > 1)
    {
        // a compile log can get produced even if there were no errors, but warnings!
        GLchar* pInfo = new char[length + 1];
        glGetShaderInfoLog(_shader,  length, &length, pInfo);
        cerr << "Compile log: " << (char*) pInfo << endl;
        delete[] pInfo;
    }
}

void checkForShaderLinkErrors( GLint _shaderProgram )
{
    GLint programError;
    glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &programError);

    if (programError != GL_TRUE)
    {
        // yes, errors :-(
        cerr << "Program could not get linked:" << endl;
    }

    GLsizei length = 0;
    glGetProgramiv(_shaderProgram, GL_INFO_LOG_LENGTH, &length);
    if (length > 1)
    {
        // error log or warnings:
        GLchar* pInfo = new char[length + 1];
        glGetProgramInfoLog(_shaderProgram,  length, &length, pInfo);
        cerr << "Linker log: " << (char*)pInfo << endl;
        delete[] pInfo;
    }
}

bool prepareExercise()
{
    // prepare the shaders:
    vs = glCreateShader( GL_VERTEX_SHADER );
    fs = glCreateShader( GL_FRAGMENT_SHADER );
    prog = glCreateProgram();
    
    // at least OpenGL 3.2
    glShaderSource( vs, 1, &vsrc, NULL );
    glShaderSource( fs, 1, &fsrc, NULL );
    
    glCompileShader(vs);
    checkForShaderCompileErrors( vs );
    glCompileShader(fs);
    checkForShaderCompileErrors( fs );
    
    glAttachShader( prog, vs );
    glAttachShader( prog, fs );
    
    glLinkProgram( prog );
    checkForShaderLinkErrors( prog );

    glUseProgram( prog );
    
    if (glGetError() != GL_NO_ERROR) {
        cerr << "could not prepare shaders" << endl;
        return false;
    }
    
    GLint attributeLocation;
    attributeLocation = glGetAttribLocation( prog, "aPosition" );
    
    // prepare the geometry:
    const int componentsPerVertex = 4;
    float quad[ componentsPerVertex * 4 ] = {
	-1.0, -1.0, -0.5, 1.0,
	-1.0,  1.0, -0.5, 1.0,
	 1.0, -1.0, -0.5, 1.0,
	 1.0,  1.0, -0.5, 1.0
    };
    
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    
    glGenBuffers( 1, &arrayBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, arrayBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW );
    
    glVertexAttribPointer( attributeLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, componentsPerVertex*sizeof(float), 0 );
    glEnableVertexAttribArray( attributeLocation );

    if (glGetError() != GL_NO_ERROR) {
        cerr << "could not prepare geometry" << endl;
        return false;
    }
    
    // prepare texture:
    glGenTextures( 1, &texture );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    
    if (glGetError() != GL_NO_ERROR) {
        cerr << "could not prepare texture" << endl;
        return false;
    }
    
    return true;
}

void cleanupExercise()
{
	glDeleteBuffers( 1, &arrayBuffer );
    glDeleteVertexArrays( 1, &vao );
    glDeleteProgram( prog );
    glDeleteShader( vs );
    glDeleteShader( fs );
    glDeleteTextures( 1, &texture );
}

void drawQuad()
{
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, windowWidth, windowHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    
    GLint uniformLocation = glGetUniformLocation( prog, "tex" );
    glUniform1i( uniformLocation, 0 );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}

/*
 * Use this function to update the texture:
 * x,y are pixel coordinates and should be in range of [0..windowWidth-1] [0..windowHeight-1]
 * color is a r,g,b color with each channel from [0..1]
 */
void setPixel( int x, int y, glm::vec3 color )
{
    color = glm::clamp( color, glm::vec3( 0.0, 0.0, 0.0 ), glm::vec3( 1.0, 1.0, 1.0 ) );
    textureData[(y*windowWidth + x)*4 +0] = (unsigned char) (color.r * 255.0);
    textureData[(y*windowWidth + x)*4 +1] = (unsigned char) (color.g * 255.0);
    textureData[(y*windowWidth + x)*4 +2] = (unsigned char) (color.b * 255.0);
    textureData[(y*windowWidth + x)*4 +3] = 255;
}

void callStudentCode( int sceneToDraw, double runTime )
{
	drawScene( sceneToDraw, runTime );
	drawQuad();
}

void resizeCallback( GLFWwindow* p, int newWidth, int newHeight )
{
    // define the part of the screen OpenGL should draw to (in pixels):
    glViewport( 0, 0, newWidth, newHeight );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




int main( int argc, char *argv[] )
{
    // Initialise GLFW
    if ( !glfwInit() )
    {
        cerr << "Failed to initialize GLFW" << endl;
        exit( -1 );
    }
    
    // get a 3.2 context:
    bool windowOK = createWindow( true );
	
	if ( !windowOK ) {
		cerr << "failed to create window" << endl;
		glfwTerminate();
		exit( -1 );
	}
	initializeOpenGL();
    
    glfwSetWindowTitle( g_window, "ACG - Introduction to CG - assignment" );
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode( g_window, GLFW_STICKY_KEYS, 1 );
	glfwSetWindowSizeCallback(  g_window, resizeCallback );

    // Enable vertical sync (on cards that support it)
    // vertical sync
    int vSync = 1;
    glfwSwapInterval( vSync );

    bool exitProgram = false;
    bool holdDownAKey = false;
    bool holdDownBKey = false;
    bool holdDownCKey = false;
    bool holdDownDKey = false;
	bool holdDownVKey = false;
    int sceneToDraw = 1;
    glGetError(); // clear errors
	
	if(!prepareExercise())
        exit(-1);
	
	initCustomResources();
	
    double startTimeInSeconds = glfwGetTime();
    do {
	    glfwPollEvents();
		
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        double runTime = glfwGetTime() - startTimeInSeconds;
		
        callStudentCode( sceneToDraw, runTime );

        // Swap buffers
        glfwSwapBuffers( g_window );
        
        if ((glfwGetKey(g_window, 'A') == GLFW_PRESS) && (!holdDownAKey)) {
            holdDownAKey = true;
            sceneToDraw  = 1;
        }
        if ((glfwGetKey(g_window, 'A') == GLFW_RELEASE) && (holdDownAKey)) holdDownAKey = false;
        
        if ((glfwGetKey(g_window, 'B') == GLFW_PRESS) && (!holdDownBKey)) {
            holdDownBKey = true;
            sceneToDraw  = 2;
        }
        if ((glfwGetKey(g_window, 'B') == GLFW_RELEASE) && (holdDownBKey)) holdDownBKey = false;
        
        if ((glfwGetKey(g_window, 'C') == GLFW_PRESS) && (!holdDownCKey)) {
            holdDownCKey = true;
            sceneToDraw  = 3;
        }
        if ((glfwGetKey(g_window, 'C') == GLFW_RELEASE) && (holdDownCKey)) holdDownCKey = false;

        if ((glfwGetKey(g_window, 'D') == GLFW_PRESS) && (!holdDownDKey)) {
            holdDownDKey = true;
            sceneToDraw  = 4;
        }
        if ((glfwGetKey(g_window, 'D') == GLFW_RELEASE) && (holdDownDKey)) holdDownDKey = false;
        
		
		if ((glfwGetKey(g_window, 'V') == GLFW_PRESS) && (!holdDownVKey)) {
        	holdDownVKey = true;
        	vSync = (vSync+1)%2;
        	cout << "vsync is ";
        	if (vSync) {cout << "on";} else {cout << "off";}
        	cout << endl;
        	glfwSwapInterval(vSync);
        }
        if((glfwGetKey(g_window, 'V') == GLFW_RELEASE) && (holdDownVKey))
            holdDownVKey = false;
		
        
        if (glfwGetKey(g_window, GLFW_KEY_ESCAPE ) == GLFW_PRESS) exitProgram = true;
        
    } // Check if the window was closed
    while( !glfwWindowShouldClose( g_window ) && !exitProgram );
    
    // clean up:
    deleteCustomResources();
	cleanupExercise();
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    exit(0);
}
