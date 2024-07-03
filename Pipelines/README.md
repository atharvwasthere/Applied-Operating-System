# Pipeline Example in C++

This project demonstrates how to set up a pipeline between two processes in a Unix-like environment using `pipe` and `fork` in C++. The example provided executes `cat` and `grep` commands to filter the contents of a file.

## Prerequisites

- A Unix-like operating system (Linux, macOS, etc.)
- C++ compiler (e.g., `g++`)
- `cat` and `grep` utilities installed on your system

## Description

The `pipeline` function sets up a pipeline between two processes:
- The first process executes `cat` to read the contents of `main.cpp`.
- The second process executes `grep` to filter lines containing the word "hello".

## Usage

### Compile the Code

```bash
g++ -o pipeline pipeline.cpp
```
## Checking and Modifying Paths

### Checking Paths

To verify the paths for `cat` and `grep`, use the following commands:

```bash
which cat
which grep
```

## Modifying Paths

If the paths for cat and grep are different on your system, update the execlp calls in the pipeline function accordingly in pipeline.cpp:
```cpp
execlp("/usr/bin/cat", "cat", "main.cpp", nullptr);
execlp("/usr/bin/grep", "grep", "hello", nullptr);
```
Replace ```bash/usr/bin/cat``` and ```bash/usr/bin/grep``` with the correct paths from your system.
