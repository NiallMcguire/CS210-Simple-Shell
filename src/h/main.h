char* originalPATH; // PATH variable before the Simple Shell starts up
char* originalHOME; // Home directory before the Simple Shell starts up
int numCommands = 0; // Number of commands executed since Simple Shell start up

/* Clear the input buffer */
void clearBuffer(char command[]);

/* Parse user input, returning number of tokens generated */
int parseInput(char command[], int tIndex, char *tokens[], char *override, int aIndex, char **history, int hIndex);

/* Handle each of the tokens (Commands) entered by the user */
void processCommand(int n, char *tokens[], char **history, int *hIndex, char **alias, int *aIndex, int tIndex);

/* Handles startup processes for the shell */
void startShell(char** history, int* hIndex, char** alias, int* aIndex);

/* Change the working directory */
void cd(char *filepath);

/* Close the Simple Shell, restore path and save command history */
void closeShell(char **history, int hIndex, char **alias, int aIndex);

/* Increment index of next command in history */
int incrementHIndex(int hIndex);

/* Initialise the history array */
int initialiseHistory(char **history);

/* Clear the history array */
int clearHistory(char **history);

/* Return if the command was a history call */
int isHistory(char *command, int hIndex);

/* adds the users input alias into the alias array*/
int addAlias(char **alias, int aIndex, char *name, char *command, char *tokens[], int tIndex, char **history, int *hIndex);

/* Initialise the alias array */
int initialiseAlias(char** alias);

/* Remove an alias */
int unAlias(char **alias, int aIndex, char *command);

/* Check if a command is an alias */
int isAlias(char *command, char **alias, int aIndex);