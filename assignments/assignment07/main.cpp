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
GenericCamera g_camera;

unsigned int windowWidth  = 512;
unsigned int windowHeight = 512;
float nearPlane =   0.1f;
float farPlane  = 100.0f;
GLFWwindow* g_window;

// assigment specific includes:
#include "Tools/PNGReader.hpp"

// assignment specific variables:
glm::mat4 g_ProjectionMatrix;
ShaderProgram* g_skyBoxShader; // don't edit this shader, it's just used for the skybox!
ShaderProgram* g_shader;
GLuint skyboxTextureArray;
GLuint skyboxTextureArrayDebug;

ArrayBuffer* g_abCube;
VertexArrayObject* g_vaoCube;
ArrayBuffer* g_abBunny;
ArrayBuffer* g_abEarth;
VertexArrayObject* g_vaoBunny;
VertexArrayObject* g_vaoEarth;


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
    glfwWindowHint( GLFW_RESIZABLE, true );
	
	// the framebuffer should support sRGB if possible - note: this does not activate the support itself!
	glfwWindowHint( GLFW_SRGB_CAPABLE, true );

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

bool prepareExercise() {
	
	// here the shader gets created:
    initCustomResources();
	
	// don't edit this shader:
    g_skyBoxShader = new ShaderProgram("skybox.vsh", "skybox.fsh");
    if (!g_skyBoxShader->link()) exit(0);

    // Set uniforms that don't change:
    g_skyBoxShader->use();
    g_skyBoxShader->setUniform( "uTexture", (int)0 );
    
	////////////////////////////////////////////////////////////////////////////
    // Define geometry:

    ABReader abreader;

    g_abBunny   = abreader.readABFile("bunny.ab");
    g_abEarth   = abreader.readABFile("sphere32.ab");
    g_abCube    = abreader.readABFile("cube.ab");

    // define VAOs:
    g_vaoBunny    = new VertexArrayObject();
    g_vaoBunny->attachAllMatchingAttributes(g_abBunny, g_shader);
    
    g_vaoEarth = new VertexArrayObject();
    g_vaoEarth->attachAllMatchingAttributes(g_abEarth, g_shader);
    
    g_vaoCube = new VertexArrayObject();
    g_vaoCube->attachAllMatchingAttributes(g_abCube, g_skyBoxShader);
	
    glEnable(GL_DEPTH_TEST);
    
    
    /// skybox arrays:
    PNGReader pngreader;
    TextureData* texture;
    
    glGenTextures(1, &skyboxTextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, skyboxTextureArray);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	texture = pngreader.readFile("negy.png");
	size_t layerSize = texture->getWidth() * texture->getHeight() * 4;
	unsigned char *buffer = new unsigned char[ layerSize*6 ]; // 6 RGBA textures
	memcpy( buffer+0*layerSize, texture->getData(), layerSize );
	delete texture;
	
	texture = pngreader.readFile("posx.png");
    memcpy( buffer+1*layerSize, texture->getData(), layerSize );
    delete texture;
    
    texture = pngreader.readFile("posy.png");
    memcpy( buffer+2*layerSize, texture->getData(), layerSize );
    delete texture;
    
    texture = pngreader.readFile("negx.png");
    memcpy( buffer+3*layerSize, texture->getData(), layerSize );
    delete texture;
    
    texture = pngreader.readFile("posz.png");
    memcpy( buffer+4*layerSize, texture->getData(), layerSize );
    delete texture;
    
    texture = pngreader.readFile("negz.png");
    memcpy( buffer+5*layerSize, texture->getData(), layerSize );
	
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0, GL_SRGB, texture->getWidth(), texture->getHeight(), 6, 
        0, texture->getFormat(), texture->getType(), buffer );
    delete texture;
    delete buffer;

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    
    // debug skybox:
    
    glGenTextures(1, &skyboxTextureArrayDebug);
    glBindTexture(GL_TEXTURE_2D_ARRAY, skyboxTextureArrayDebug);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	texture = pngreader.readFile("negyd.png");
	layerSize = texture->getWidth() * texture->getHeight() * 4;
	buffer = new unsigned char[ layerSize*6 ]; // 6 RGBA textures
	memcpy( buffer+0*layerSize, texture->getData(), layerSize );
	delete texture;
	
	texture = pngreader.readFile("posxd.png");
    memcpy( buffer+1*layerSize, texture->getData(), layerSize );
    delete texture;
    
    texture = pngreader.readFile("posyd.png");
    memcpy( buffer+2*layerSize, texture->getData(), layerSize );
    delete texture;
    
    texture = pngreader.readFile("negxd.png");
    memcpy( buffer+3*layerSize, texture->getData(), layerSize );
    delete texture;
    
    texture = pngreader.readFile("poszd.png");
    memcpy( buffer+4*layerSize, texture->getData(), layerSize );
    delete texture;
    
    texture = pngreader.readFile("negzd.png");
    memcpy( buffer+5*layerSize, texture->getData(), layerSize );
	
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0, GL_SRGB, texture->getWidth(), texture->getHeight(), 6, 
        0, texture->getFormat(), texture->getType(), buffer );
    delete texture;
    delete buffer;

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY); // will fail if not all 6 faces were given!
    
	const glm::vec3 vLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

	const float fSpecularityExponent = 800.0f;
	
    g_shader->use();
    g_shader->setUniform("uLightColor",             vLightColor);
    g_shader->setUniform("uSpecularityExponent",    fSpecularityExponent);
    
    return true;
}

void cleanupExercise() {
	delete g_skyBoxShader;
	glDeleteTextures(1, &skyboxTextureArrayDebug);
 	glDeleteTextures(1, &skyboxTextureArray);
 	
    delete g_vaoCube;
    delete g_abCube;
}


glm::mat4 buildFrustum(float phiInDegree, float aspectRatio, float near, float far) {

    float phiHalfInRadians = 0.5f * phiInDegree * (M_PI / 180.0f);
    float t = near * tan(phiHalfInRadians);
    float b = -t;
    float left = b * aspectRatio;
    float right = t * aspectRatio;

    return glm::frustum(left, right, b, t, near, far);
}


void resizeCallback( GLFWwindow* p, int newWidth, int newHeight )
{
    windowWidth  = newWidth;
    windowHeight = newHeight;
    
    glViewport(0, 0, newWidth, newHeight);
    g_ProjectionMatrix = buildFrustum(50.0f, (newWidth / (float) newHeight), 0.5f, 1000.0f);
}

void renderSkybox( bool debugTexture, glm::mat4 viewMatrix )
{
	// skybox rendering:
	g_skyBoxShader->use();
	g_skyBoxShader->setUniform( "uMVP", g_ProjectionMatrix*viewMatrix*glm::scale( glm::vec3(500.0f) ) );
	glActiveTexture( GL_TEXTURE0 );
	if (debugTexture) {
    	glBindTexture(GL_TEXTURE_2D_ARRAY, skyboxTextureArrayDebug);
	} else {
		glBindTexture(GL_TEXTURE_2D_ARRAY, skyboxTextureArray);
	}
	g_vaoCube->bind();
	for (int i = 0; i < 6; i++) {
		g_skyBoxShader->setUniform("layer", i);
		glDrawArrays( GL_TRIANGLES, i*2*3, 2*3 );
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void handleInput( double time )
{
    static double timeOfLastFrame = 0.0;
    if (timeOfLastFrame == 0.0) {
        timeOfLastFrame = time;
        // ignore the first frame, as the user has not done useful inputs till now anyway
        return;
    }

    // make camera movements based on the elapsed time and not based on frames rendered!
    double timeElapsed = time - timeOfLastFrame;

    double speed = 10.0; // magic value to scale the camera speed

    // as long as the keys are hold down, these are triggered each frame:
    if (glfwGetKey( g_window, 'W')) { // upper case!
        g_camera.moveForward( timeElapsed*speed );
    }
    if (glfwGetKey( g_window, 'A')) { // upper case!
        g_camera.moveLeft( timeElapsed*speed );
    }
    if (glfwGetKey( g_window, 'S')) { // upper case!
        g_camera.moveBack( timeElapsed*speed );
    }
    if (glfwGetKey( g_window, 'D')) { // upper case!
        g_camera.moveRight( timeElapsed*speed );
    }


    timeOfLastFrame = time;
}

void mouseMoveCallback( GLFWwindow *, double x, double y )
{
    static glm::vec2 initialPosition;
    static bool leftMouseButtonDown = false;
    
    if (!leftMouseButtonDown && glfwGetMouseButton( g_window, GLFW_MOUSE_BUTTON_1 )) {
        leftMouseButtonDown = true;
        initialPosition = glm::vec2( x,y );
        glfwSetInputMode( g_window, GLFW_CURSOR_HIDDEN, true ); // hide the cursor
        return;
    } else if (leftMouseButtonDown && !glfwGetMouseButton( g_window, GLFW_MOUSE_BUTTON_1 )) {
        leftMouseButtonDown = false;
        glfwSetInputMode( g_window, GLFW_CURSOR_HIDDEN, false ); // unhide the cursor
        glfwSetCursorPos( g_window, initialPosition.x, initialPosition.y );
        return;
    }

    glm::vec2 movement = glm::vec2( x,y ) - initialPosition;

    if (leftMouseButtonDown) {
        glm::vec2 realtiveMovement = glm::vec2(movement) / glm::vec2(windowWidth, windowHeight);
        g_camera.FPSstyleLookAround( realtiveMovement.x, realtiveMovement.y );
        //glfwSetCursorPos( g_window, initialPosition.x, initialPosition.y );
        initialPosition = glm::vec2( x,y );
    }
}


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
	bool holdDownEKey = false;
    //int sceneToDraw = 1;
    glGetError(); // clear errors
	
	
	initCustomResources();
	if(!prepareExercise())
        exit(-1);
	
	resizeCallback( NULL, windowWidth, windowHeight );
	glfwSetCursorPosCallback( g_window, mouseMoveCallback );
		
	bool objectRotates = false;
	int  meshNumber    = 0;
	bool cubeMapping   = true;
	bool debugTexture  = false;
	bool environmentOnly = true;
	
	g_camera.setPosition( glm::vec3(0.0, 2.5, 10.0) );
	
	glEnable( GL_FRAMEBUFFER_SRGB );
	
    double startTimeInSeconds = glfwGetTime();
    do {
	    glfwPollEvents();
		
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        double runTime = glfwGetTime() - startTimeInSeconds;
		handleInput( runTime );
		
        
		if((glfwGetKey(g_window, 'R') == GLFW_PRESS) && (!holdDownAKey)) {
            holdDownAKey = true;
            objectRotates = !objectRotates;
        }
        if((glfwGetKey(g_window, 'R') == GLFW_RELEASE) && (holdDownAKey))
            holdDownAKey = false;

        if((glfwGetKey(g_window, 'M') == GLFW_PRESS) && (!holdDownBKey)) {
            holdDownBKey = true;
            meshNumber    = (meshNumber+1)%2;
        }
        if((glfwGetKey(g_window, 'M') == GLFW_RELEASE) && (holdDownBKey))
            holdDownBKey = false;

        if((glfwGetKey(g_window, 'C') == GLFW_PRESS) && (!holdDownCKey)) {
            holdDownCKey = true;
            cubeMapping   = !cubeMapping;
        }
        if((glfwGetKey(g_window, 'C') == GLFW_RELEASE) && (holdDownCKey))
            holdDownCKey = false;

        if((glfwGetKey(g_window, 'X') == GLFW_PRESS) && (!holdDownDKey)) {
            holdDownDKey = true;
            debugTexture  = !debugTexture;
        }
        if((glfwGetKey(g_window, 'X') == GLFW_RELEASE) && (holdDownDKey))
            holdDownDKey = false;
        
        if((glfwGetKey(g_window, 'E') == GLFW_PRESS) && (!holdDownEKey)) {
            holdDownEKey = true;
            environmentOnly = !environmentOnly;
        }
        if((glfwGetKey(g_window, 'E') == GLFW_RELEASE) && (holdDownEKey))
            holdDownEKey = false;
		
        
        if (glfwGetKey(g_window, GLFW_KEY_ESCAPE ) == GLFW_PRESS) exitProgram = true;
		
		
		// matrices:
        float angle = runTime * 360.0f / 10.0f;
		static glm::vec3 eyePos = glm::vec3(0.0f, 3.0f, 10.0f);
		static float objectRotation = 0.0f;
		
		const glm::vec3 vLightPosition = glm::vec3(-10.0f, 10.0f, 4.0f);
		
		if (objectRotates) {
			objectRotation = angle; // if the camera is fix, the object should rotate
		}
		
		//glm::mat4 viewMatrix = glm::lookAt(eyePos, glm::vec3(0, 3.0, 0), glm::vec3(0, 1, 0));
		glm::mat4 viewMatrix   = g_camera.getViewMatrix();
		eyePos = g_camera.getPosition();
		glm::mat4 modelMatrixEarth = glm::translate(0.0f,3.0f,0.0f) * glm::rotate(objectRotation, glm::vec3(0.0f, -1.0f, 0.0f)) * glm::rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0f)) *glm::scale(3.0f,3.0f,3.0f);
		glm::mat4 modelMatrixBunny = glm::rotate(objectRotation, glm::vec3(0.0f, -1.0f, 0.0f));
		
		
		
		g_shader->use();
		if (meshNumber == 0) {
			glm::mat4 invTranspModelView = glm::inverse(glm::transpose(viewMatrix*modelMatrixEarth));
			g_shader->setUniform("uModelMatrix",                modelMatrixEarth);
			g_shader->setUniform("uInvTranspModelViewMatrix",   invTranspModelView);
			
			glm::mat3 uInvTranspModelMatrix = glm::inverse(glm::transpose(glm::mat3(modelMatrixEarth)));
			g_shader->setUniform("uInvTranspModelMatrix", uInvTranspModelMatrix);
		} else {
			glm::mat4 invTranspModelView = glm::inverse(glm::transpose(viewMatrix*modelMatrixBunny));
			g_shader->setUniform("uModelMatrix",                modelMatrixBunny);
			g_shader->setUniform("uInvTranspModelViewMatrix",   invTranspModelView);
			
			glm::mat3 uInvTranspModelMatrix = glm::inverse(glm::transpose(glm::mat3(modelMatrixBunny)));
			g_shader->setUniform("uInvTranspModelMatrix", uInvTranspModelMatrix);
		}
		
		// per frame changing uniforms:
		g_shader->setUniform("uProjectionMatrix", g_ProjectionMatrix );
		g_shader->setUniform("uCameraPosition",   eyePos );
		g_shader->setUniform("uViewMatrix",       viewMatrix);
		glm::vec3 worldLightPos = glm::vec3( viewMatrix * glm::vec4(vLightPosition,1.0) );
		g_shader->setUniform("uLightPosition",    worldLightPos);

		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderSkybox( debugTexture, viewMatrix );
        
        g_shader->use();
        drawScene(environmentOnly, meshNumber, cubeMapping, debugTexture);
		
		// Swap buffers
        glfwSwapBuffers( g_window );
        
    } // Check if the window was closed
    while( !glfwWindowShouldClose( g_window ) && !exitProgram );
    
    // clean up:
    deleteCustomResources();
	cleanupExercise();
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    exit(0);
}
