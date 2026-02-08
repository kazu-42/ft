# get_next_line - Subject

## Summary

get_next_line is a function that reads a line from a file descriptor.
This project teaches static variables in C, file I/O, and buffer management.

---

## Objectives

- Learn about static variables
- Learn about file descriptors and buffered reading
- Add `get_next_line()` to your personal library for future projects

---

## Common Instructions

- Project must be written in C
- Must follow the 42 Norm
- Functions should not quit unexpectedly (segfault, bus error, double free, etc.)
- All heap-allocated memory must be properly freed (no memory leaks)
- A Makefile is NOT required for this project
- Must compile with: `cc -Wall -Wextra -Werror`

---

## Mandatory Part

| Item | Description |
|------|-------------|
| **Function name** | `get_next_line` |
| **Prototype** | `char *get_next_line(int fd);` |
| **Turn-in files** | `get_next_line.c`, `get_next_line_utils.c`, `get_next_line.h` |
| **Parameters** | `fd` - The file descriptor to read from |
| **Return value** | The line read (including `\n` if present), or `NULL` if nothing left to read or error |
| **External functions** | `read`, `malloc`, `free` |

### Description

Write a function that returns a line read from a file descriptor.

### Rules

- Repeated calls to `get_next_line()` should read the text file pointed to by `fd`, one line at a time
- The function should return the line that was read. If there is nothing else to read or if an error occurred, it should return `NULL`
- The returned line should include the terminating `\n` character, except if the end of file was reached and does not end with a `\n` character
- The header file `get_next_line.h` must contain the prototype of the function
- Helper functions should be added in `get_next_line_utils.c`
- The `BUFFER_SIZE` value will be specified at compilation using `-D BUFFER_SIZE=n`
  - Example: `cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 *.c`
- The program must compile with and without a default `BUFFER_SIZE`
- `get_next_line()` must work correctly with any `BUFFER_SIZE` value (1, 9999, 10000000, etc.)
- The function must read as little as possible each time it is called; do not read the entire file and then process it

### Forbidden

- Using `libft` in this project
- Using `lseek()`
- Using global variables

---

## Bonus Part

Bonus will only be evaluated if the mandatory part is **perfect**.

| Item | Description |
|------|-------------|
| **Turn-in files** | `get_next_line_bonus.c`, `get_next_line_bonus.h`, `get_next_line_utils_bonus.c` |

### Bonus Requirements

1. Develop `get_next_line()` using only **one static variable**
2. Your `get_next_line()` must be able to manage **multiple file descriptors** at the same time
   - For example: if you can read from fd 3, fd 4, and fd 5, you should be able to read from a different fd per call without losing the reading thread of each fd or returning a line from another fd
   - You should be able to call `get_next_line()` to read from fd 3, then fd 4, then fd 5, then fd 3 again, then once more from fd 4, etc.

---

## Compilation

```bash
# Mandatory
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c main.c

# Bonus
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line_bonus.c get_next_line_utils_bonus.c main.c

# Various buffer sizes to test
cc -Wall -Wextra -Werror -D BUFFER_SIZE=1 ...
cc -Wall -Wextra -Werror -D BUFFER_SIZE=9999 ...
cc -Wall -Wextra -Werror -D BUFFER_SIZE=10000000 ...
```
