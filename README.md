Critters must catch food (represented by green circles) and avoid predators
(represented by red squares). The brain of each critter is a small feedforward
neural network.

Brains are trained in the background using a genetic algorithm. The graphical
user interface (GUI) is updated every 20 seconds with the five best brains of
the current generation (which makes them absolutely useless for the first 20
seconds).

![Screenshot](https://raw.githubusercontent.com/phaubertin/critters/master/doc/screenshot.png)

Build Requirements
------------------

This software is intended to be built on a Unix-like operating system with GCC.
It is known to build and run on Linux and MacOS X.

This software has a dependency on the SDL library for the graphical user 
interface (GUI), so please ensure the SDL library is installed before building. 
Some Linux distributions split run-time and developement files in separate 
packages. If it is the case for your distribution, you will need both. On 
CentOS, this translates to the following yum install command:
```
yum install SDL SDL-devel
```

An X86 processor is required because some parts of the software use compiler
intrinsics for SSE2 instructions.

Build Instructions
------------------

If starting from a clean clone of the Git repository, run the following:
```
./autogen.sh
```

Then, configure the sources:
```
./configure
```

To compile (or re-build after having made some changes to source files):
```
make
```

To install locally and then run:
```
make install
critters
```

Alternatively, you can run from the compiled source directory without installing:
```
src/critters
```

Experiment
----------

You can change the number of neurons in the hidden layer and their activation
function (sigmoid-like, gaussian-like or ReLU) by changing the constants at the
top of src/brain.h (and then re-building). You can also modify the parameters of
the genetic algorithm at the top of src/breeder.h.

Design Overview
---------------

Critters have two senses: vision and smell (**src/scene.c**). Vision is limited 
to a +/-63 degrees angle. Critters know the distance and angle of the nearest 
food item, predator and outer wall within that angle and subject to a distance 
limit. The closest item of each type within the visual field might not be the 
closest overall. Critters' sense of smell allows them to know the distance of 
the closest food item and predator omnidirectionally up to a distance limit 
(they cannot smell walls). That makes eight inputs (**src/stimuli.h**):

* (Vision) Distance and angle of nearest food item within the visual field.
* (Vision) Distance and angle of nearest predator within the visual field.
* (Vision) Distance and angle of nearest outer wall within the visual field.
* (Smell) Distance of nearest food item.
* (Smell) Distance of nearest predator.

Each critter's brain is a small feedforward artificial neural network with a 
single hidden layer (**src/brain.c**). The two outputs of the neural network 
are the speed of the left side and the right side of the critter. If these 
speeds differ, the critter turns. The weights of the neural network are part of 
a critter's genome (**src/genome.h**). Also part of the genome is the colour 
of the critter's head. This provides a visual indication of what happens to a 
characteristic that is not selected for.

Training is performed with a genetic algorithm. At each generation, each 
critter is simulated for a short time and a fitness function is computed that 
depends on the number of times the critter catches food or is caught by a 
predator. For simplicity of implementation, critters are not eliminated 
outright when they get caught by a predator, but the associated fitness cost 
penalty coupled with the selection procedure are such that selection becomes
very unlikely when that happens.

The selection procedure works as follow (**src/breeder.c**): First, the genomes 
with the lowest fitness score are discarded. Then, a pool of genomes is created 
by picking the genomes with top fitness score, then picking a few other genomes 
randomly, and finally adding a few randomly-generated novel genomes. Genomes 
with top fitness score are added multiple times to increase the probability 
they get chosen. Once the pool has been created, pairs of genomes are selected 
randomly with uniform distribution within that pool.

New genomes are created from each selected pair by recombination with mutation 
(**src/genome.c**).
