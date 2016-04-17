/* Based on ... */
/* fileman.c -- A tiny application which demonstrates how to use the
   GNU Readline library.  This application interactively allows users
   to manipulate files and their modes. */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "kerl.h"

#ifndef whitespace
#define whitespace(c) (((c) == ' ') || ((c) == '\t'))
#endif

/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
  char *name;			     /* User printable name of the function. */
  kerl_bindable func;  /* Function to call to do the job. */
  char *doc;			     /* Documentation for this function.  */
} COMMAND;

int command_count = 0;
int command_cap = 0;
COMMAND *commands = NULL;
int execute_line(char *line);
char *history_file = NULL;

/* Forward declarations. */
char *stripwhite ();
COMMAND *find_command ();
void initialize_readline ();
void kerl_add_history(const char *line);

void kerl_register(char *name, kerl_bindable func, char *doc)
{
  if (command_cap == 0) {
    command_cap = 8;
    commands = malloc(sizeof(COMMAND) * command_cap);
  } else if (command_cap == command_count) {
    command_cap *= 2;
    commands = realloc(commands, sizeof(COMMAND) * command_cap);
  }
  commands[command_count++] = (COMMAND) {name, func, doc};
}

/* When non-zero, this global means the user is done using this program. */
int done;

char *dupstr (char *s)
{
  char *r;

  r = malloc(strlen(s) + 1);
  strcpy(r, s);
  return r;
}

void kerl_run(const char *prompt)
{
  char *line, *s;

  initialize_readline ();	/* Bind our completer. */

  /* Loop reading and executing lines until the user quits. */
  for ( ; done == 0; ) {
    line = readline(prompt);

    if (!line) break;

    /* Remove leading and trailing whitespace from the line.
       Then, if there is anything left, add it to the history list
       and execute it. */
    s = stripwhite(line);

    if (*s) {
      kerl_add_history(s);
      execute_line(s);
    }

    free(line);
  }
}

void kerl_add_history(const char *s)
{
  add_history(s);
  if (history_file) {
    FILE *fp = fopen(history_file, "a");
    fprintf(fp, "%s\n", s);
    fclose(fp);
  }
}

void kerl_set_history_file(const char *path)
{
  if (history_file) free(history_file);
  history_file = strdup(path);
  char buf[1024];
  FILE *file = fopen(path, "r");
  if (file) {
    while (NULL != (fgets(buf, 1024, file))) {
      buf[strlen(buf)-1] = 0; // get rid of \n
      add_history(buf);
    }
  }
}

/* Execute a command line. */
int execute_line(char *line)
{
  register int i;
  COMMAND *command;
  char *word;

  /* Isolate the command word. */
  i = 0;
  while (line[i] && whitespace (line[i]))
    i++;
  word = line + i;

  while (line[i] && !whitespace (line[i]))
    i++;

  if (line[i]) line[i++] = '\0';

  command = find_command(word);

  if (!command) {
    fprintf (stderr, "%s: No such command.\n", word);
    return (-1);
  }

  /* Get argument to command, if any. */
  while (whitespace(line[i]))
    i++;

  word = line + i;

  /* Call the function. */
  return command->func(word);
}

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
COMMAND *find_command(char *name)
{
  register int i;

  for (i = 0; i < command_count; i++) {
    if (strcmp(name, commands[i].name) == 0) {
      return &commands[i];
    }
  }

  return (COMMAND *)NULL;
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char *stripwhite(char *string)
{
  register char *s, *t;

  for (s = string; whitespace(*s); s++);
  
  if (*s == 0)
    return s;

  t = s + strlen(s) - 1;
  while (t > s && whitespace(*t))
    t--;
  *++t = '\0';

  return s;
}

/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */

char *command_generator ();
char **kerl_completion ();

/* Tell the GNU Readline library how to complete.  We want to try to complete
   on command names if this is the first word in the line, or on filenames
   if not. */
void initialize_readline ()
{
  /* Allow conditional parsing of the ~/.inputrc file. */
  rl_readline_name = "kerl";

  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = kerl_completion;
}

/* Attempt to complete on the contents of TEXT.  START and END show the
   region of TEXT that contains the word to complete.  We can use the
   entire line in case we want to do some simple parsing.  Return the
   array of matches, or NULL if there aren't any. */
char **kerl_completion(char *text, int start, int end)
{
  char **matches;

  matches = (char **)NULL;

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  if (start == 0) {
    matches = rl_completion_matches(text, command_generator);
  }

  return matches;
}

/* Generator function for command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. */
char *command_generator (char *text, int state)
{
  static int list_index, len;
  char *name;

  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state) {
    list_index = -1;
    len = strlen(text);
  }

  /* Return the next name which partially matches from the command list. */
  while (++list_index < command_count) {
    name = commands[list_index].name;

    if (strncmp(name, text, len) == 0)
      return (dupstr(name));
  }

  /* If no names matched, then return NULL. */
  return (char *)NULL;
}
