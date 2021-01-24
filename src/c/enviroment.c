// Here we include methods relating to setting and getting parameters relating to the environment
// This includes details such as PATH and HOME

/**
* Sets the environmental variable PATH to newPath
* @param newPath: String for the new directory to be added to path
*/
void setPath(char *newPath) {
    // Path must be specified
    if (newPath == NULL) {
        red("[Error] ");
        printf("You must specify a directory!\n");
        return;
    }

    // Check if path exists on system, can we read from it?
    if(access(newPath, R_OK) != 0) {
        red("[Error] ");
        printf("Directory \"%s\" doesn't exist, or you do not have read access.\n", newPath);
        return;
    }

    // Set new path
    setenv("PATH", newPath, 1);
    blue("[Info] ");
    printf("PATH has been updated to: %s\n", getPath());
}


/**
  * Appends a directory to the system PATH
  * Checks that the directory does not already exist in path, if it does then the directory is not added
  * @param *newPath: String for the new directory to be added to the path
  */
void addPath(char *newPath){
    // Path must be specified
    if (newPath == NULL) {
        red("[Error] ");
        printf("You must specify a path\n");
        return;
    }

    printf("Adding: %s to path\n", newPath);

    // Check if path exists on system, can we read from it?
    if(access(newPath, R_OK) != 0) {
        yellow("[Warning] ");
        printf("Path \"%s\" doesn't exist, or you do not have read access - Adding anyway.\n", newPath);
    }

    // Remove leading / from end of path if it exists
    if (newPath[strlen(newPath)-1] == '/' && strlen(newPath) > 1) {
        newPath[strlen(newPath)-1] = 0;
    }

    /* Check if the new path already exists in path.
     * The idea here is to add a colon to either end of the PATH variable, i.e. :PATH:, and newPath, i.e. :newPath:
     * and then check if :newPath: exists in :PATH:
     * If it does exist, then tell user this is the case, otherwise add the new path
     */

    // Allocate memory for the current PATH plus space for two characters ":"
    char pathCompare[strlen(getPath()) + 2];
    strcpy(pathCompare, ":");
    strcpy(pathCompare, getPath());
    strcat(pathCompare, ":");

    // Allocate memory for the new PATH variable plus space for two characters ":"
    char newPathCompare[strlen(newPath) + 2];
    strcpy(newPathCompare, ":");
    strcpy(newPathCompare, newPath);
    strcat(newPathCompare, ":");

    // Perform comparison, returns pointer if newPath found in PATH
    if(strstr(pathCompare, newPathCompare) != NULL) {
        yellow("[Warning] ");
        printf("%s already exists in PATH\n", newPath);
        return;
    }

    /* Append newPath to the current PATH, the result should be "PATH:newPath"
     * We need a buffer with enough space to store this, copy & concatenate the PATH, :, and newPath together
     * then set the new path using setenv.
     */
    char *buffer = malloc(strlen(getPath()) + strlen(newPath) + 1);

    // Concatenate the new path onto the PATH variable
    strcpy(buffer, getPath());
    strcat(buffer, ":");
    strcat(buffer, newPath);

    // Set the new PATH Variable
    setenv("PATH", buffer, 1);

    // Display the new PATH
    blue("[Info] ");
    printf("Path has been updated to: %s\n",  getPath());
}


/* Return the current environmental path */
char *getPath() {
    return getenv("PATH");
}


/* Set home directory */
void setHome(char *dir) {
    // Path must be specified
    if (dir == NULL) {
        red("[Error] ");
        printf("You must specify a directory!\n");
        return;
    }

    // Check if path exists on system, can we read from it?
    if(access(dir, R_OK) != 0) {
        red("[Error] ");
        printf("Directory \"%s\" doesn't exist, or you do not have read access.\n", dir);
        return;
    }

    // Set path
    setenv("HOME", dir, 1);
    blue("[Info] ");
    printf("Home directory has been updated to: %s\n", getHome());
}


/* Return the current environmental home directory */
char *getHome() {
    return getenv("HOME");
}


/* Return the current working directory */
char *printCwd() {
    char cwd[MAX_PATH];
    return getcwd(cwd, MAX_PATH);
}