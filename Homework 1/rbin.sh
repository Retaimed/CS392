#!/bin/bash

# *******************************************************************************
#  Author  : Ryan Eshan 
#  Date    : February 6th, 2024 
#  Description: CS392 - Homework 1
#  Pledge  : I pledge my honor that I have abided by the Stevens Honor System
# ******************************************************************************

# TODO: Fill the header above, and then complete rbin.sh below'

# Task 1 

# This function displays the usage message and uses a heredoc 
function message_usage { cat << EOF
Usage: rbin.sh [-hlp] [list of files]
   -h: Display this help;
   -l: List files in the recycle bin;
   -p: Empty all files in the recycle bin;
   [list of files] with no other flags,
        these files will be moved to the
        recycle bin.
EOF
}


# case for when no argument is given 
if [[ $# -eq 0 ]]; then
    message_usage
    exit 1 
fi 

# This is where the getop
flag_H=0 
flag_L=0 
flag_P=0


while getopts ":hlp" options; do 
    case "${options}" in 
        h) 
            flag_H=1 
            ;;
        l) 
            flag_L=1
            ;;
        p)
            flag_P=1
            ;;
        *)  
            echo "Error: Unknown option '-${OPTARG}'." >&2 
            message_usage
            exit 1 
esac
done 


if [[ $# -gt 1 && $(( $flag_H + $flag_L + $flag_P )) -ge 1 ]]; then ## Checks if the number of command line arguments is greater than 1 and Sum of flags greater than or equal 1 (more than 1 flag)
    echo "Error: Too many options enabled." >&2
    message_usage
    exit 1
fi


# Task 2 

# The .recycle is declared as a read-only variable 
readonly RECYCLE=~/.recycle

# If there is no .recycle in your home directory, this script creates a new one 
if [ ! -d $RECYCLE ]; then 
    mkdir $RECYCLE
fi

# A case for when argument -h,
if [[ $flag_H -eq 1 && $(( $flag_L + $flag_P )) -eq 0 ]]; then ## Checks if -h is printed on the command line and if sum of -l and -p equal to 0 
    message_usage
    exit 0
fi 

# This is when -p is invoked, the script should remove all files and directories in .recycle
if [[ $flag_P -eq 1 ]]; then 
    rm -r $RECYCLE
    exit 0 
fi 

# This is when -l is invoked, the script should list the files in the recycle bin 
if [[ $flag_L -eq 1 ]]; then 
    ls -lAF $RECYCLE
    exit 0 
fi 


# Task 3 

# Move files and directories, item is
for filedirect in "$@"; do
    #If the file/directory exists 
    if [ -e $filedirect ]; then
        if [ -f $filedirect ]; then
            #File as indicated by -f
            mv $filedirect $RECYCLE 
        elif [ -d $filedirect ]; then
            #Directory as indicated by -d 
            mv $filedirect $RECYCLE
        else
            echo "Warning: '$filedirect' not found." >&2
        fi
    else
        echo "Warning: '$filedirect' not found." >&2
    fi
done
