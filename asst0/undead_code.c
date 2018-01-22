// Author: John-Austen Francisco
// Date: 11 January 2018
//
// Preconditions: Appropriate C libraries, iLab machines
// Postconditions: Generates Segmentation Fault for
//                               signal handler self-hack

// Student name: Paul Vorobyev (pv149)
// Ilab machine used: cpp.cs.rutgers.edu

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void segment_fault_handler(int signum)
{
    printf("I am slain!\n");

    //Use the signnum to construct a pointer to flag on stored stack
    void * ptr  = &signum; //int ptr will have increments by 4, so use void ptr instead

    //Increment pointer down to the stored PC
    ptr += 0x3c; // distance to eip in prev frame according to gdb

    //Increment value at pointer by length of bad instruction
    *(int *)ptr += 0x6; // JUMPing in gdb to the line after SIGSEGV gives a distance of 6 bytes
    
    
}


int main()
{
    int r2 = 0;

    signal(SIGSEGV, segment_fault_handler);

    r2 = *( (int *) 0 );
    
    printf("I live again!\n");

    return 0;
}
