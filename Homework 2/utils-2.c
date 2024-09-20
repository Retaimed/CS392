/*******************************************************************************
 * Name        : utils.c
 * Author      : Ryan Eshan 
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System. 
 ******************************************************************************/
#include "utils.h"
#include <stdio.h>

int cmpr_int(void* x, void* y){
    int x1 = *((int*)x);
    int y1 = *((int*)y);
    if (x1 > y1){
        return 1; 
    } else if (x1 < y1){
        return -1;
    } else 
        return 0;
}

int cmpr_float(void* x, void* y){
    float x2 = *((float*)x); 
    float y2 = *((float*)y);
    if (x2 > y2){
        return 1;
    } else if (x2 < y2){
        return -1;
    } else 
        return 0;
}

void print_int(void* x){
    printf("%d ", *((int*)x));
}

void print_float(void* x){
    printf("%f ", *((float*)x)); 
}




