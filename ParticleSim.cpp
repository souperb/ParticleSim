/*
    This programs simulates ordinary gravity and also a blackhole using either Euler or fourth order Runge-Kutta integration.
    
    The large and small timesteps for both simulation types are the same for Euler and RK4 to show the difference between them.

    Screenshots that contrast the same simulation type with different integration methods (i.e. small step blackhole Euler vs small step blackhole RK4) are taken with the mouse held in the exact same position (mouse does not show up in screenshots).

    The key presses to switch between modes are as follows (sower case means small timestep, upper means large):
    's' or 'S' - Freeze simulation (particles won't age or move)
    'g' or 'G' - Standard gravity with Euler
    'b' or 'B' - Blackhole with Euler
    'r' or 'R' - Standard gravity with RK4
    'k' or 'K' - Blackhole with RK4
    'o' or 'O' - Clear all the particles off the screen
    '+' or '-' - Make the particles larger or smaller

    Click and hold the mouse anywhere to generate particles. Moving the mouse while clicking will give the particles initial velocity
    in relation to the speed and direction of the mouse. The initial velocity is slightly randomized, based on the mouse's velocity.

    Author: Ethan Campbell
    ID: 190057
    Class: CMPT315
    Assignment: Project - Runge-Kutta Animation
    Date: December 7, 2021
*/

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include <windows.h>
#define M_PI 3.14159265358979323846
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

double MAXTTL = 15.0; // Maximum Time to Live

const int SCREENSIZE = 1000; // Size of the screen in pixels

char simulationType = 's'; // Tracks the type of simulation
bool mouseCliked = false; // Track if the mouse is being held down

// Mouse coordinates
double mouseX;
double mouseY;
double mouseVX;
double mouseVY;

double pointSize = 5.0; // Particle size

// Used to slightly randomizes the particles' starting velocity
double randVX;
double randVY;

const float GRAVITY = -9.8; // Constant for gravity
const float BLACKHOLE = 10; // Constant for the black hole (c_1)
float TIME_STEP = 0.005; // Used as dt for all necessary calculations
bool RK = false;


// This class contains the coordinates, velocity, acceleration, and TTL of 1 particle
class Particle {
    public:
        bool exists = false;
        
        // x, y, and z coordinates
        float x = 0.0;
        float y = 0.0;
        float z = 0.0;
      
        // x, y, and z velocity
        float vx = 0.0;
        float vy = 0.0;
        float vz = 0.0;

        float acceleration = 0.0; // Acceleration towards (0, 0, 0)
        double TTL = MAXTTL; // Time to live

        // Sets the existance, coordinates, and velocity of a particle
        void setVariables(bool newExists, float newX, float newY, float newZ, float newVX, float newVY) {
            exists = newExists;
            x = newX;
            y = newY;
            z = newZ;
            vx = newVX;
            vy = newVY;
            acceleration = 0.0;
        }

        // Reduce the particles time to live
        void reduceTTL() {
            TTL -= 0.1;
        }
};


// Track all particles
int currentParticle = 0;
const int PARTICLE_COUNT = 500;
Particle particleArr[PARTICLE_COUNT];


// Initializes the random number generator
void init() {
    unsigned int seednum = time(NULL);
    srand(seednum);
}


// Decreases the time to live of all existing particles
// When its TTL reaches zero it's existence is set to false and its TTL reset
void ageParticles() {
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        if (particleArr[i].exists) {
            particleArr[i].reduceTTL();
            if (particleArr[i].TTL <= 0) {
                particleArr[i].exists = false;
                particleArr[i].TTL = MAXTTL;
                particleArr[i].acceleration = 0.0;
            }
        }
     }
}


// Used when switching between simulation types to proportionally adjust the particles' TTL
void updateTTL(double oldTTL, double newTTL) {
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        if (particleArr[i].exists) {
            particleArr[i].TTL = particleArr[i].TTL * newTTL / oldTTL;
        }
    }

     MAXTTL = newTTL;
}


// Draws all of the existing particles with GL_POINTS
void drawParticles() {
    glBegin(GL_POINTS);

    // Draw the existing particles
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        if (particleArr[i].exists) {

            // The particles colour fades as its TTL decreases
            // Use different colours depending on if its Euler or RK4
            if (RK) {
                glColor3f(particleArr[i].TTL/MAXTTL, 0.0, particleArr[i].TTL/MAXTTL);
            } else {
                glColor3f(0.0, particleArr[i].TTL/MAXTTL, particleArr[i].TTL/MAXTTL);
            }

            glVertex3f(particleArr[i].x, particleArr[i].y, particleArr[i].z);
        }
    }
        
    glEnd();
}


void display(void) {
    glClearColor (0.0, 0.0, 0.0, 0.0f); // background colour
    glClear(GL_COLOR_BUFFER_BIT); // clear the background
    glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f); // define the size of the world
    glPointSize(pointSize);

    drawParticles(); // Draw the particles

    glFlush(); // make sure the polygon is pushed to the graphics card

    glutSwapBuffers(); // flip the buffers so the polygon I just drew is now on the front buffer
}


// Calculates the velocity and position of each particle as if gravity is acting on them
void gravitySim(void) {
    for (int i = 0; i < PARTICLE_COUNT; i++) {
            if (particleArr[i].exists) {

                // y velocity of the particle
                particleArr[i].vy += GRAVITY * TIME_STEP;

                // x and y positions of the particle
                particleArr[i].y += particleArr[i].vy * TIME_STEP;
                particleArr[i].x += particleArr[i].vx * TIME_STEP;
            }
            
    } 

    ageParticles();
}


// Does the gravity simulation with fourth order Runge-Kutta integration
void gravityRK(void) {
    for (int i = 0; i < PARTICLE_COUNT; i++) {
            if (particleArr[i].exists) {

                // Find the velocitites of the particle and the start and end of the timestep
                double k1 = particleArr[i].vy;
                double k4 = k1 + GRAVITY * TIME_STEP;

                // Find the velocity of the particle in the middle
                double k2 = k1 + (k4-k1) / 2;
                double k3 = k2;



                // x and y positions of the particle
                particleArr[i].vy = k4;
                particleArr[i].y += TIME_STEP * (k1 + 2*k2 + 2*k3 + k4)/6; // Use RK4
                particleArr[i].x += particleArr[i].vx * TIME_STEP;
            }
            
    }

    ageParticles();
}


// Calculates the acceleration, velocity, and position of each particle as if there is a black hole at (0, 0, 0)
void blackholeSim(void) {
    for (int i = 0; i < PARTICLE_COUNT; i++) {
            if (particleArr[i].exists) {
                
                // The distance of the particle from (0, 0, 0)
                float distance = particleArr[i].x*particleArr[i].x + particleArr[i].y*particleArr[i].y + particleArr[i].z*particleArr[i].z;

                // Acceleration towards (0, 0, 0)
                particleArr[i].acceleration = BLACKHOLE / distance;

                // x and y velocity of the particle
                particleArr[i].vx += -particleArr[i].x * particleArr[i].acceleration  * TIME_STEP;
                particleArr[i].vy += -particleArr[i].y * particleArr[i].acceleration * TIME_STEP;

                // x and y position of the particle
                particleArr[i].y += particleArr[i].vy * TIME_STEP;
                particleArr[i].x += particleArr[i].vx * TIME_STEP;
            }
        }

    ageParticles();
}


// Blackhole simulation with fourth order Runge-Kutta integration
void blackholeRK(void) {
    for (int i = 0; i < PARTICLE_COUNT; i++) {
            if (particleArr[i].exists) {
                
                // The distance of the particle from (0, 0, 0)
                float distance = particleArr[i].x*particleArr[i].x + particleArr[i].y*particleArr[i].y + particleArr[i].z*particleArr[i].z;


                // Find the velocitites of the particle and the start and end of the timestep
                double k1 = particleArr[i].acceleration;
                double k4 = BLACKHOLE / distance;
                // Find the velocity of the particle in the middle
                double k2 = (k4+k1)/2;
                double k3 = k2;

                // Save the acceleration towards (0, 0, 0) at the end of the time step
                particleArr[i].acceleration = k4;

                // Find the velocities at the start, end, and middle for y
                double ky1 = particleArr[i].vy;
                double ky4 = particleArr[i].vy - particleArr[i].y * ((k1 + 2*k2 + 2*k3 + k4)/6) * TIME_STEP;
                double ky2 = (ky4+ky1) / 2;
                double ky3 = ky2;

                particleArr[i].vy = ky4; // Save the y velocity at the end of the time step
                particleArr[i].y += TIME_STEP * (ky1 + 2*ky2 + 2*ky3 + ky4)/6; // Find the new y position

                // Find the velocities at the start, end, and middle for y
                double kx1 = particleArr[i].vx;
                double kx4 = particleArr[i].vx - particleArr[i].x * ((k1 + 2*k2 + 2*k3 + k4)/6) * TIME_STEP;
                double kx2 = (kx4+kx1) / 2;
                double kx3 = kx2;

                particleArr[i].vx = kx4; // Save the x velocity at the end of the time step
                particleArr[i].x += TIME_STEP * (kx1 + 2*kx2 + 2*kx3 + kx4)/6; // Find the new x position
            }
        }

    ageParticles();
}


// Determines if the left mouse button is being held down
void mouseF(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        mouseCliked = true;
        mouseX = x;
        mouseY = y;
    } else {
        mouseCliked = false;
    }
}


// Tracks the coordinates of the mouse when it is being clicked
// Calculates the x and y velocity of the mouse movement
void motionF(int x, int y) {
    if (x >= 0 && x <= SCREENSIZE && y >= 0 && y <= SCREENSIZE) {
        mouseVX = (x - mouseX) / 0.005;
        mouseVY = (y - mouseY) / 0.005;
        
        mouseX = x;
        mouseY = y;
    }

}

// Sets the simulation type, max time to live, time step, and point size depending on the user input
void keyboardF(unsigned char key, int x, int y) {
    if (key == 's' || key == 'S') {
        simulationType = 's';
        cout << "Simulation Stopped" << endl;
    } else if (key == 'g') {
        simulationType = 'g';
        TIME_STEP = 0.005;
        updateTTL(MAXTTL, 15.0);
        RK = false;
        cout << "Gravity - Small Time Step - Euler Integration" << endl;
    } else if (key == 'G') {
        simulationType = 'G';
        TIME_STEP = 0.02;
        updateTTL(MAXTTL, 5.0);
        RK = false;
        cout << "Gravity - Big Time Step - Euler Integration" << endl;
    } else if (key == 'b') {
        simulationType = 'b';
        TIME_STEP = 0.0018;
        updateTTL(MAXTTL, 70.0);
        RK = false;
        cout << "Black Hole - Small Time Step - Euler Integration" << endl;
    } else if (key == 'B') {
        simulationType = 'B';
        TIME_STEP = 0.01;
        updateTTL(MAXTTL, 50.0);
        RK = false;
        cout << "Black Hole - Big Time Step - Euler Integration" << endl;
    } else if (key == 'r') {
        simulationType = 'r';
        TIME_STEP = 0.005;
        updateTTL(MAXTTL, 15.0);
        RK = true;
        cout << "Gravity - Small Time Step - Runge-Kutta Integration" << endl;
    } else if (key == 'R') {
        simulationType = 'R';
        TIME_STEP = 0.02;
        updateTTL(MAXTTL, 5.0);
        RK = true;
        cout << "Gravity - Big Time Step - Runge-Kutta Integration" << endl;
    } else if (key == 'k') {
        simulationType = 'k';
        TIME_STEP = 0.0018;
        updateTTL(MAXTTL, 70.0);
        RK = true;
        cout << "Black Hole - Small Time Step - Runge-Kutta Integration" << endl;
    } else if (key == 'K') {
        simulationType = 'K';
        TIME_STEP = 0.01;
        updateTTL(MAXTTL, 50.0);
        RK = true;
        cout << "Black Hole - Big Time Step - Runge-Kutta Integration" << endl;
    } else if (key == 'o' || key == 'O') {
        // Erase all of the particles
        for (int i = 0; i < PARTICLE_COUNT; i++) {
            particleArr[i].exists = false;
            particleArr[i].TTL = MAXTTL;
        }
        cout << "Particles Cleared" << endl;
    } else if (key == '+') {
        pointSize += 1.0;
        cout << "Point size increased to " << pointSize << endl;
    } else if (key == '-' && pointSize > 1.0) {
        pointSize -= 1.0;
        cout << "Point size decreased to " << pointSize << endl;
    }
}


// Set the coordinates of the current particle to the coordinates of the mouse
// Calls the correct simulation based on the users input
void timer(int unused) {
    if (mouseCliked) {
       
        // Set a particle to existing and reinitialize its TTL
        particleArr[currentParticle].exists = true;
        particleArr[currentParticle].TTL = MAXTTL;
       
        // Randomize the x and y velocity based on the mouse's velocity. y velocity is only positive
        randVX = (mouseVX/SCREENSIZE*2) + (float) (rand() * 1.5 / RAND_MAX - 0.75);
        randVY = (-mouseVY/SCREENSIZE*2) + (float) (rand() * 0.75 / RAND_MAX);
        
        // Set the particles position and velocity based on the mouse
        particleArr[currentParticle].setVariables(true, mouseX/SCREENSIZE*2 - 1, -mouseY/SCREENSIZE*2 + 1, 0.0, randVX, randVY);
        
        currentParticle++; // Move to the next particle in the array
        if (currentParticle == PARTICLE_COUNT) currentParticle = 0;

    }

    // Call either the gravity or black hole simulation
    if (simulationType == 'g' || simulationType == 'G') {
        gravitySim();
    } else if (simulationType == 'b' || simulationType == 'B') {
        blackholeSim();
    } else if (simulationType == 'r' || simulationType == 'R') {
        gravityRK();
    } else if (simulationType == 'k' || simulationType == 'K') {
        blackholeRK();
    }

    glutTimerFunc(15, timer, 42);  // Recall the timer
    glutPostRedisplay(); // Redraw the screen
}


int main(int argc, char** argv) {
    // First set up GLUT
    glutInit( &argc, argv );
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);

    // Make the window
    int windowHandle = glutCreateWindow("Particle Simulator");
    glutSetWindow(windowHandle);

    // Place and size the window
    glutPositionWindow ( 100, 100 );
    glutReshapeWindow(SCREENSIZE, SCREENSIZE);

    cout << "Keypreses to change simulation types:\n"
         << "'s' or 'S' - Freeze simulation (particles won't age or move)\n"
         << "'g' or 'G' - Standard gravity with Euler\n"
         << "'b' or 'B' - Blackhole with Euler\n"
         << "'r' or 'R' - Standard gravity with RK4\n"
         << "'k' or 'K' - Blackhole with RK4\n"
         << "'o' or 'O' - Clear all the particles off the screen\n"
         << "'+' or '-' - Make the particles larger or smaller" << endl;

    // set the drawing and mouse callback functions
    // notice the names are the functions defined above
    init();
    glutDisplayFunc(display);
    glutMouseFunc(mouseF);
    glutMotionFunc(motionF);
    glutKeyboardFunc(keyboardF);
    glutTimerFunc(10, timer, 42);
    // Go into an infinite loop waiting for messages
    glutMainLoop();
    return 0;
}