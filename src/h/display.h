/* Display prompt to the user */
void prompt();

/* Display the users' home directory */
void displayHome();

/* Display the current path */
void displayPath();

/* Display the users Current Working Directory */
void displayCWD();

/* Display the last MAX_HISTORY commands entered by the user */
void dispHistory(int hIndex, char **history);

/* Display all alias' entered by the user */
void dispAlias(char **alias, int aIndex);