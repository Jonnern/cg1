/*
 * Basics of Computer Graphics Exercise
 */
 
#include "assignment.h"
#define PI 3.14159265
using namespace std;

// Colors
const glm::vec3 blue(0.0, 0.0, 1.0);
const glm::vec3 white(1.0, 1.0, 1.0);
const glm::vec3 black(0.0, 0.0, 0.0);
const glm::vec3 gray(0.2, 0.2, 0.2);
const glm::vec3 red(1.0, 0.0, 0.0);
const glm::vec3 green(0.0, 1.0, 0.0);

// Constants
const double trackRadius        = 0.9;
const double trackWidth         = 0.2;
const double trackEdgeWidth     = 0.01;
const double specStandHeight    = 0.7;
const double specStandWidth     = 0.03;
const double dashWidth          = 0.01;
const double dashHeight         = 0.05;
const int    numDashes          = 20;
const int    numFinLines        = 9;
const double finishLineGap      = 0.015;
const double finishLineWidth    = (trackWidth-4*trackEdgeWidth-(numFinLines-1)*finishLineGap)/numFinLines;
const double finishLineHeight   = dashHeight;
const double carHeight          = 0.06;
const double carWidth           = 0.02;

glm::mat4 scale(double x, double y)
{
    return glm::transpose(glm::mat4(x, 0, 0, 0,
                                    0, y, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1));
}

glm::mat4 scale(double x)
{
    return scale(x, x);
}


glm::mat4 rotate(double angle)
{
    return glm::transpose(glm::mat4(cos(angle), -sin(angle), 0, 0,
                                    sin(angle), cos(angle),  0, 0,
                                    0,          0,           1, 0,
                                    0,          0,           0, 1));
}

glm::mat4 translate(double x, double y)
{
    return glm::transpose(glm::mat4(1, 0, 0, x,
                                    0, 1, 0, y,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1));
}

void drawScene(int scene, float runTime) {

    // Draw track
    drawCircle(blue, scale(trackRadius));
    drawCircle(gray, scale(trackRadius - trackEdgeWidth));
    drawCircle(blue, scale(trackRadius - trackWidth + trackEdgeWidth));
    drawCircle(black,scale(trackRadius - trackWidth));

    // Draw spectator stand
    drawCircle(gray, translate(-trackRadius - specStandWidth, 0)
                     *scale(specStandWidth, specStandHeight));

    // Draw dashes
    for(int i = 1; i < numDashes; i++)
    {
        drawCircle(white, rotate((double)2*PI/numDashes*i + PI)
                          *translate(trackRadius-trackWidth/2, 0)
                          *scale(dashWidth, dashHeight));
    }

    // Draw finish line
    for(int i = 0; i < numFinLines; i++)
    {
        drawCircle(white, translate(-trackRadius + 2*trackEdgeWidth + i*(finishLineWidth+finishLineGap), 0)
                          *scale(finishLineWidth, finishLineHeight));
    }

    // Draw racing cars
    drawCircle(red, rotate(runTime)
                    *translate(trackRadius-trackWidth/4, 0)
                    *scale(carWidth, carHeight));

    drawCircle(green, rotate(2*runTime)
                      *translate(trackRadius-3*trackWidth/4, 0)
                      *scale(carWidth, carHeight));
}

void initCustomResources()
{
}

void deleteCustomResources()
{
}

