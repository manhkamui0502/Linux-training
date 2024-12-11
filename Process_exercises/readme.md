## Usage
```
./open_file [file_name]
```

## Check the returned value:
```
echo $?
```
* If operation succeeded, return `0`
* If operation failed, return `-1`, which show `255` after using `echo $?`
