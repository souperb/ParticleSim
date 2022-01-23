OpenGL is required to run this program.

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
