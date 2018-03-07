# About
POSIX Shell by [Dimonchik0036](https://github.com/Dimonchik0036)

## Features
* Run commands in the background
* Job Control
* Pipelining
* Redirection of input / output

# Build
```
git clone https://github.com/Dimonchik0036/shell.git
cd shell
```

## For CMake
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## For Makefile
```
make
```

# Usage
`./myshell`

# Builtin commands
`fg [%job]`  
`bg [%job]`  
`jobs`  
`jkill [%job]`  
