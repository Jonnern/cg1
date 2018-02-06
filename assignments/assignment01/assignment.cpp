/*
 * Basics of Computer Graphics Exercise
 */
 
#include "assignment.h"
#include <iostream>

using namespace std;

void drawScene(int scene, float runTime) {
    cout << "Scene no: " << scene << "; Run Time is: " << runTime << "\n";
}

void initCustomResources() {
	//
    // Print some general information about the supported OpenGL, the GPU vendor and sometimes the driver version:
    //
    cout << "OpenGL context version: " << glGetString(GL_VERSION)  << endl;
    cout << "OpenGL vendor:          " << glGetString(GL_VENDOR)   << endl;
    cout << "OpenGL renderer:        " << glGetString(GL_RENDERER) << endl;
    cout << "GLSL version:           " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void deleteCustomResources() {
}
