# kerl
Kerl lets you set up a command line tool in C in a few lines of code, where adding commands takes one line.

It is a readline extension, with inherent command completion, persistent history support, and so on.

## Example

### `ls`

```C
int com_ls();

int main() {
  kerl_register("ls", com_ls, "List directory contents.");
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
