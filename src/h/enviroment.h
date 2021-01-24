/* Set a new PATH variable */
void setPath(char *newPath);

/* Append new directory to the environmental variable PATH*/
void addPath(char *newPath);

/* Return the current PATH */
char *getPath();

/* Set the users home directory and navigate there */
void setHome(char *dir);

/* Return the users home directory */
char *getHome();

/* Returns the current working directory */
char *printCwd();