#define PROMPT "$ " /* Prompt shown to the user */
#define MAX_COMMAND_LENGTH 514 /* Taken from the specification. Maximum length (in terms of characters) of any command. + 2 characters to account for \n and EOF when using fgets [514] */
#define T_MAX 50 /* Maximum number of tokens that can be stored per command, from the spec [50 when including 0] */
#define DELIMITERS " \t|><&;\n" /* Tokens as taken from the spec, addition of \n as well */
#define MAX_PATH 4096 /* Max size of the CWD */
#define MAX_HISTORY 21 /* Max number of commands to store in the history */
#define MAX_ALIAS 20 /* Number of alias' to store (Store 10) */

char *TOP_BOX =
        "+====================================================================================================+\n";

char *WELCOME =
"+   _____  _____ ___   __  ___               _____ _                 _         _____ _          _ _  +"
"\n+  / ____|/ ____| __ \\/_ |/ _ \\             / ____(_)               | |       / ____| |        | | | +"
"\n+  | |    | (___    ) || | | | |  ______   | (___  _ _ __ ___  _ __ | | ___  | (___ | |__   ___| | | +"
"\n+  | |     \\___ \\  / / | | | | | |______|   \\___ \\| | '_ ` _ \\| '_ \\| |/ _ \\  \\___ \\| '_ \\ / _ \\ | | +"
"\n+  | |____ ____) |/ /_ | | |_| |            ____) | | | | | | | |_) | |  __/  ____) | | | |  __/ | | +"
"\n+  \\_____| _____/|____||_|\\___/            |_____/|_|_| |_| |_| .__/|_|\\___| |_____/|_| |_|\\___|_|_| +"
"\n+                                                                                                    +";

char *CREDITS =
"\n+            By Shaun Greer, Callum Inglis, Mhari McGill, Niall Mcguire, Douglas Wheeler             +"
"\n+                                                                                                    +"
"\n";

//char *WELCOME =
//"=======================\n"
//"Welcome to Simple Shell\n"
//"=======================\n";

