# kerl
Readline extension for command line tools in C, with command completion, persistent history support, and so on.

## Example

### `ls`

```C
int com_ls();

int main() {
  kerl_register("ls", com_ls, "List directory contents.");
  kerl_run("$ "); // l<tab> should give "ls " and argument tabulation should give files/folders
}

int com_ls(char *a rg) {
  char buf[1024]; 
  if (!arg) arg = "";

  sprintf(buf, "ls -FClg %s", arg);
  return system(buf);
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
