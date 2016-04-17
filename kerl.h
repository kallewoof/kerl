#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

typedef int (*kerl_bindable) (const char *arg);

void kerl_register(char *name, kerl_bindable func, char *doc);
void kerl_run(const char *prompt);
void kerl_set_history_file(const char *path);
