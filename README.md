# kerl
Kerl lets you set up a command line tool in C in a few lines of code, where adding commands takes one line.

It is a readline extension, with inherent command completion, persistent history support, and so on.

## Example

### `ls`

```C
int com_ls();

int main() {
  kerl_register("ls", com_ls, "List files in current folder.");
  kerl_run("$ "); // l<tab> should give "ls " and argument tabulation should give files/folders
}

static char syscom[1024]; 

int com_ls(const char *arg) {
  if (!arg) arg = "";

  sprintf(syscom, "ls -FClg %s", arg);
  return system(syscom);
}
```

Typing `l` and hitting the tab key should complete `ls ` and argument completion finds files, just like a normal shell. Multiple commands with a shared prefix will be completed as they are in bash et al.

### Persistent history across sessions

```C
kerl_set_history_file(".my-history-file");
// ...
kerl_run("$ ");
```

The `kerl_set_history_file` command will load and save user input to the given file, so that a user will be able to use the up-arrow / ^R features to find input from previous instances.

### Help command

```C
kerl_register_help("help");
// $ help
// ls   List files in current folder.
// echo Echo the given argument string.
// help Show help information.
// $ 
```

### Custom per-command completion handler

```C
int com_greet();
char *compl_greet();

int main() {
	kerl_register("greet", com_greet, "Greet Henry, Fred, George, or whoever.");
	kerl_set_completor("greet", compl_greet);
	kerl_run("GreetShell> ");
}

int com_greet(const char *arg) {
	printf("\"Greetings, %s!\" you exclaim.\n", arg);
	return 0;
}

char *compl_greet(const char *text, int state)
{
	static int list_index, len;
	const char *name;

	/* If this is a new word to complete, initialize now.  This includes
	 saving the length of TEXT for efficiency, and initializing the index
	 variable to 0. */
	if (!state) {
		list_index = -1;
		len = strlen(text);
	}

	/* Return the next name which partially matches from the command list. */
	while (++list_index < 3) {
		name = names[list_index];

		if (strncasecmp(name, text, len) == 0)
			return strdup(name);
	}

	/* If no names matched, then return NULL. */
	return (char *)NULL;
}

// GreetShell> gr<tab>
// GreetShell> greet <tab>
// Hemingway Henrietta Henry
// GreetShell> greet He<tab>
// Hemingway Henrietta Henry
// GreetShell> greet Hem<tab>
// GreetShell> greet Hemingway 
// "Greetings, Hemingway!" you exclaim.
```
