This project implements Shear Sort algorithm (Parallel sort), using Cartesian Topology for communication between processors.

Input: Dat file with list of boxes organized as following - Box id, length, width, height.
In the project I inserted 3 dat files in order to  display 3 examples - cuboids.dat, boxes.dat, boxes2.dat.

Output: Dat file (result.dat) contains ids of bodies according to sorted criteria.

The number of lines (bodies) is equal to the number of processes launched.
The root process (I called it Master) displays the result of the sort and writes it to the file result.dat.
It uses Scatter command to send data to processes, and Gather command to collect result of sorting.

The bodies are sorted according to their volumes. 
In case of equal volumes, the bodies are compared according to their heights.

The project includs: 

Boxes_Sort.c - source file to compile and execute.

Boxes_sort.h - Header file with definitions of the functions Boxes_Sort.c use and the Box structs. 

cuboids.dat, boxes.dat, boxes2.dat - Dat files with list of boxes.

Running instructions: 

By using Ubuntu terminal :
1. mpicc Boxes_Sort.c -lm -o <binary executable name>.
2. mpiexec -np <number of processes = number of listed boxes (In my example 16)> <binary executable name>.


By using Eclipse:
Make Sure the library m is included in the Project libraries.
Click on "Build" --> Rhight click on project -->  Run as --> Run configuration --> Parallel Application --> <Project name> --> Go to  Resources tab:
1. On Target System Configuration choose : "Generic MPICH2 Interactive".
2. On Connection type Coose "Local".
2. Choose Number of processes (In my example 16).
3. Click Apply.
--> Go to Application tab:
1. Click "Browse" next to Application Program.
2. In Debug folder choose <Project name>.
3. Click Apply.
4. Click Run.

In order to print sorted result to the console the and compare it with the list before, define DEBUG_MOD to be 1.


(Number of processes have to be n*n type).
