/*******************************************************************************
 * Name        : bstree.c
 * Author      : Ryan Eshan
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System. 
 ******************************************************************************/
#include "bstree.h"
#include "utils.h"
#include <stdio.h>

void add_node(void* newdata, size_t sizeb, tree_t* tree, int (*cmpr)(void*,void*)){
    node_t* newnode = (node_t*) malloc (sizeof(node_t)); //Used the int* m = (int*) malloc (10 * sizeof(int)) example in class, this line creates a memory space for storing the data(more specifically a node in the tree)
    //Loop to copy byte one at a time 
    newnode->data = malloc(sizeb);
    for (char i = 0; i < sizeb; i++){
        *((char*)(newnode->data) + i)=*((char*)(newdata) + i);
    }
    newnode->left = NULL;
    newnode->right = NULL;

    // Check whether tree_t's root is NULL 
    if (tree->root == NULL){
        tree->root = newnode;
        return; 
    }
    // If the tree_t's root is not NULL, then insert the node 
    node_t* actual_root = tree->root;
    while (tree->root != NULL){
        if (cmpr(newnode->data, tree->root->data) < 0){
            if (tree->root->left == NULL){
                tree->root->left = newnode;
                break; 
            }
            tree->root = tree->root->left; 
            } else if (cmpr(newnode->data, tree->root->data) > 0) {
                if (tree->root->right == NULL){
                    tree->root->right = newnode; 
                    break;
                }
                tree->root = tree->root->right;
            }
    }
    tree->root = actual_root;
    return;
    
}


void print_tree (node_t* root, void(*printer)(void*)){
    if (root == NULL){
        return;
    }
    print_tree(root->left, printer);
    printer(root->data);
    print_tree(root->right, printer); 
}


void destroy_nodes (node_t* node){
    if (node == NULL){
        return;
    }
    destroy_nodes(node->left);
    destroy_nodes(node->right);
    free(node->data);
    free(node);
}

void destroy(tree_t* tree){
    if (tree == NULL){
        return;
    }
    destroy_nodes(tree->root);
    tree->root = NULL;
}


