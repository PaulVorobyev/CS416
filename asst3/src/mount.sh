#!/bin/bash

# Create testfsfile
if [ ! -d "/.freespace/$USER" ]; then
    # Control will enter here if $DIRECTORY doesn't exist.
    mkdir /.freespace/$USER
    touch /.freespace/$USER/testfsfile
fi

# Create mountdir
if [ ! -d "/tmp/$USER" ]; then
    # Control will enter here if $DIRECTORY doesn't exist.
    mkdir /tmp/$USER
    mkdir /tmp/$USER/mountdir
fi

~/CS416/asst3/src/sfs /.freespace/$USER/testfsfile /tmp/$USER/mountdir
