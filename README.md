# fat-fs
FAT File System Implementation for PUCRS Operating Systems Course

Compile

````sh
gcc app.c -o app
```

Execution
```sh
./app.c
```
Operations
```sh
sh$ help
 init - initialize file system(overwrite existing one)
 load - loads file system from disk
 ls [/path/directory] - list directory
 mkdir [/path/directory] - create directory
 create [/path/file] - create file
 unlink [/path/file] - delete file or directory (directory must be empty)
 write "string" [/path/file] - write data to a file(overwrite existing data)
 append "string" [/path/file] - append data to a file
 read [/path/file] - read file contents
 help - print help information
 exit - exit from shell
```
