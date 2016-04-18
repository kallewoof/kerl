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

### Argument handling

```C
int com_argcv(const char *arg) {
  size_t argc;
  char **argv;
  if (kerl_make_argcv(arg, &argc, &argv)) {
    // user aborted
    printf("user abort\n");
    return -1;
  }
  printf("argc = %lu:", argc);
  for (size_t i = 0; i < argc; i++) printf(" [%lu] %s", i, argv[i]);
  if (argc > 0) printf("\n");
  kerl_free_argcv(argc, argv);
  return 0;
}
```

The above will convert the arg string into a proper argc/argv size/char array pair. It will handle quotes and escaping, and even prompt for more input upon imbalanced quotes (if the user hits `^D` here, -1 is returned, hence the user abort check).

```Bash
foo$ argcv foo
argc = 1: [0] foo
foo$ argcv foo bar
argc = 2: [0] foo [1] bar
foo$ argcv foo\ bar
argc = 1: [0] foo bar
foo$ argcv foo" "bar
argc = 1: [0] foo bar
foo$ argcv 'foo" "bar'
argc = 1: [0] foo" "bar
foo$ argcv foo" bar
dquote> zed"
argc = 1: [0] foo bar
zed
foo$ argcv foo bar \
> xyz
argc = 3: [0] foo [1] bar [2] xyz
foo$ argcv foo bar\
> zed
argc = 2: [0] foo [1] barzed
foo$ argcv foo bar "quoted ' and \" work too"
argc = 3: [0] foo [1] bar [2] quoted ' and " work too
foo$ argcv "as does \\ i.e. escaped escape"
argc = 1: [0] as does \ i.e. escaped escape
foo$ argcv "starting a quoted thing
dquote> but deciding to abort by htting Ctrl-D
dquote> ^D
user abort
```

It is also possible to escape e.g. apostrophe or quotation mark using `kerl_make_argcv_escape`. 

   * `argcv this\'\"it` -> `this'"it`
   * `argcv' this\'\"it` -> `this\'"it`
   * `argcv" this\'\"it` -> `this'\"it`

which makes it convenient when passing the results to some system function, e.g. 
```C
  size_t argc;
  char **argv;
  if (kerl_make_argcv_escape(arg, &argc, &argv, "'")) return;
  if (argc != 2) {
    fprintf(stderr, "2 args required!\n");
  } else {
    char buf[1024];
    sprintf(buf, "myprog '%s' '%s'", argv[0], argv[1]);
    system(buf);
  }
  kerl_free_argcv(argc, argv);
```

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
	kerl_register("greet", com_greet, "Greet Henry, Henrietta, Hemingway, or whoever.");
	kerl_set_completor("greet", compl_greet);
	kerl_run("GreetShell> ");
}

int com_greet(const char *arg) {
	printf("\"Greetings, %s!\" you exclaim.\n", arg);
	return 0;
}

static const char *names[] = {"Henry", "Henrietta", "Hemingway"};

char *compl_greet(const char *text, int continued)
{
	static int list_index, len;
	const char *name;

	/* If this is a new word to complete, initialize now.  This includes
	 saving the length of TEXT for efficiency, and initializing the index
	 variable to 0. */
	if (!continued) {
		list_index = -1;
		len = strlen(text);
	}

	/* Return the next name which partially matches from the names list. */
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

The completion handler has as its sole task to return the next possible value based on the given prefix for the given string. It is called repeatedly until it runs out of options. (Internally, this is done directly by the readline library.)

Completion handlers only deal with the first argument to the given command. Subsequent commands are passed to the default, i.e. the file picker.
