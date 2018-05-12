Critters must catch food (represented by green circles) and avoid predators
(represented by red squares). The brain of each critter is a small feedforward
neural network (14 neurons).

Brains are trained in the background using a genetic algorithm. The graphical
user interface (GUI) is updated every 20 seconds with the five best brains of
the current generation (which makes them absolutely useless for the first 20
seconds).

![Screenshot](https://raw.githubusercontent.com/phaubertin/critters/master/doc/screenshot.png)

Build Requirements
------------------

This software has a dependency on the SDL library for the graphical user
interface (GUI), so please ensure the SDL library is installed.

This software is intended to be built on a Unix-like operating system with GCC.
It is known to build and run on Linux and MacOS X.

An X86 processor is required because some parts of the software use compiler
intrinsics for SSE instructions.

Build Instructions
------------------

If starting from a clean clone of the Git repository, run the following:
./autogen.sh

Then, to compile and run:
./configure
make
cd src
./critters
