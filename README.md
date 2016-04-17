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

### Persistent history across sessions

```C
kerl_set_history_file(".my-history-file");
// ...
kerl_run("$ ");
```

## Todo

Currently 3rd argument to kerl_register (the description) does nothing.
