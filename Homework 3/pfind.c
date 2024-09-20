/*******************************************************************************
 * Name        : pfind.c
 * Author      : Ryan Eshan
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System. 
 ******************************************************************************/
#include <stdio.h> //Most frequent used library used to print things to the console, recieve input from keyboard to console 
#include <string.h> // This library allows for strlen to be used 
#include <stdlib.h> // This library allows for the exit function and two macros EXIT_SUCESS and EXIT_FAILURE
#include <sys/stat.h> // This library allows for the stat function
#include <dirent.h> // This library allows for the opendir function 
#include <unistd.h> // This library is for POSIX as well as chdicr function 


#define PATH_MAX 4096 // The max path length in linux is 4096 bytes

// Task 1: Validating Input
void valid_permission (char* pstring){ // This function takes in a string pointer (as strings are just an array of char which points to the first element of the array address)
    if (strlen(pstring) != 9){ // If the length of the is not equal to 9 then produce an error, since valid permissions string should be 9 characters. 
        fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", pstring); // This is the fprintf error used to print a stderr when this if condition is true, this is done using the stdio.h library 
        exit(EXIT_FAILURE); // EXIT status of EXIT_Failure should be returned according to the document. 
    }

    char* valid_pstring_chars = "-rwx"; // These are the only valid chars in the valid permission string so this if condition checks if theses strings are present
    size_t length_comp = strcmp(pstring, valid_pstring_chars); // Compare the strings with the valid ones
    for (int i = 0; i < 9; i++){
        if (((i % 3 == 0 && pstring[i] != 'r') || (i % 3 == 1 && pstring[i] != 'w') || (i % 3 == 2 && pstring[i] != 'x')) && (pstring[i] != '-')) {
            fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", pstring);
            exit(EXIT_FAILURE);
        }
    }
} 

// Task 2: Finding Files with Specified Permission
// readdir() used to traverse the directory tree
// opendir() used to open a directory
// stat checking the file permission, also checks what type of file it is 

// open directory, 
// iterate throught directories contents, 
// get the the absolute path of the directories 
// get the file information 
// file permission
// check if its a directory
// rursively go to the directory

// Helper to get the file permission 
// to take the pstring and convert it into binary where 0 is - and rwx is 1
int pstring_bin (char* pstring){
    // pstring to binary representation 
    int binarycount = 0;
    for (int i = 0; i < 9; i++) {
        binarycount = (binarycount << 1) + (pstring[i] == 'r' || pstring[i] == 'w' || pstring[i] == 'x');
    }

    return binarycount;   
}

// calculate the sum of the bitwise, as well as the operations for each set of permission bits.
// compare the sum with the sum of the corresponding bits in the file permission.
int check_permission (int binarycount, int filepermission){
  return ( 
        ((S_IRUSR & binarycount) + (S_IWUSR & binarycount) + (S_IXUSR & binarycount) +
         (S_IRGRP & binarycount) + (S_IWGRP & binarycount) + (S_IXGRP & binarycount) +
         (S_IROTH & binarycount) + (S_IWOTH & binarycount) + (S_IXOTH & binarycount)) ==
        ((S_IRUSR & filepermission) + (S_IWUSR & filepermission) + (S_IXUSR & filepermission) +
         (S_IRGRP & filepermission) + (S_IWGRP & filepermission) + (S_IXGRP & filepermission) +
         (S_IROTH & filepermission) + (S_IWOTH & filepermission) + (S_IXOTH & filepermission))
    );
}

void find_file_perm (char* directory, char* pstring){
    struct dirent* dire;//holds information 
    struct stat fileinfo; // file informatiion 
    char pathsize[PATH_MAX]; // Used to store the absolute path during the traversal
    chdir(directory); // ensures that the correct current working directory is used for concatenation. So it changes the working directory to specified directory
    // open Directory
    DIR* direct = opendir(directory);
    // iterate through directories contents
    while ((dire = readdir(direct)) != NULL){ // From the reading on page 81 on pdf or page 69 of the textbook 
        chdir(directory); // restore the working directory from the iterations
        if (strcmp(dire->d_name, ".") == 0 || strcmp(dire->d_name, "..") == 0) { // d_name is from the struct of dirent, we checking if the directory we are on is not equal to the current/parent directory}
            continue;
        } 
        else{
        // get the the absolute path of the directories
        strcpy(pathsize, directory);
        if(directory[strlen(directory)-1] != '/'){
            strcat(pathsize,"/");
        }
        // get the file info
        if (stat(dire->d_name,&fileinfo) == -1){
            exit(EXIT_FAILURE);
        }
        // file permision 
        if(S_ISREG(fileinfo.st_mode)){
            if (check_permission(pstring_bin(pstring),fileinfo.st_mode)) {
                printf("%s%s\n",pathsize,dire->d_name);
            }
        }
        // check if its directory 
        if (S_ISDIR(fileinfo.st_mode)) {
            //printf("1\n");
            // Rursively go to the directory
            strcat(pathsize, dire->d_name);
            find_file_perm(pathsize, pstring); // Pass the concatenated path
        }
        }
    }
    closedir(direct);
}

int main(int argc, char* argv []){
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <permissions>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // validate the permissions string
    valid_permission(argv[2]);
    // start finding files with specified permissions
    find_file_perm(argv[1], argv[2]);

    return EXIT_SUCCESS;
}
