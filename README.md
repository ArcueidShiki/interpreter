# interpreter

[CITS2002 Project1 Programming Language Interpreter](https://teaching.csse.uwa.edu.au/units/CITS2002/projects/project1.php)

## Environment

Linux / Unix

## Build

```bash
# under root directory of this project run:
make all # (or) make debug (or) make release (make test)

# It will build executable under /out/ directory
make clean # remove all the outpur under /out/
```

## Test

```bash
# under root directory of this project run:
make all
cd out
./test
./runml_debug ../test/filename
./runml_release ../test/filename
```

See test cases on project page.
Using automatic testing, assert.

## Submission

only a single `runml.c` files.
Other files will not be submitted.

## Requirements and Solutions

| Requirements                                                                                                                                                                                                                                                                                                                             | Solutions See Functions:                                             |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------- |
| 1. ~~Statements are written one per line without a terminating semicolon~~                                                                                                                                                                                                                                                               | endwith_semicolon                                                    |
| 2. ~~All syntax errors detected in invalid ml programs must be reported via stderr on a line commencing with the '!' character. Your runml program must be able to detect all invalid ml programs - EXCEPT that your program will not be tested with any invalid expressions, so you do not need to validate the syntax of expressions~~ | LOGE                                                                 |
| 3. ~~The only 'true' output produced by your translated and compiled program (when running) is the result of executing ml's print statement. Any 'debug' printing should appear on a line commencing with the '@' character~~                                                                                                            | LOGD                                                                 |
| 4. ~~When printed, numbers that are exact integers must be printed without any decimal places; other numbers must be printed with exactly 6 decimal places~~                                                                                                                                                                             | is_intf, is_intd, ISINT, FMT, PRINT                                  |
| 5. ~~Only one datatype is supported: real numbers (e.g., 2.71828)~~.                                                                                                                                                                                                                                                                     | PRINT, only handle numbers                                           |
| 6. ~~Identifiers (variable and function names) consist of 1 to 12 lowercase alphabetic characters~~                                                                                                                                                                                                                                      | is_valid_indentifier                                                 |
| ~~7. There will be at most 50 unique identifiers appearing in any program~~                                                                                                                                                                                                                                                                  | This is a precondition, they will ensure it. |
| 8. ~~The variables arg0, arg1, and so on, provide access to the program's command-line arguments which provide real-valued numbers~~                                                                                                                                                                                                     | is_cmdline_arg, translate_cmdline_arg                                |
| 9. ~~Statements within a function must be indented with a tab~~                                                                                                                                                                                                                                                                          | startwith_tab                                                        |
| 10. ~~Functions must have been defined before it is called in an expression~~                                                                                                                                                                                                                                                                | struct Function                                                      |
| 11. ~~Programs are written in text files whose names end in .ml~~                                                                                                                                                                                                                                                                        | check_file                                                           |
| 12. ~~'#' appearing anywhere on a line introudces a comment which extends until the end of that line~~                                                                                                                                                                                                                                   | rm_comment                                                           |
| 13. ~~Variables do not need to be defined before being used in an expression, and are automatically initialised to the (real) value 0.0~~                                                                                                                                                                                                    |                                                                      |
| 14. ~~A function's parameters and any other identifiers used in a function body are local to that function, and become unavailable when the function's execution completes~~                                                                                                                                                                 |                                                                      |

## Tasks Breakdown

Program runml should do the following:

1. Read each line of the file
2. Extract tokens from each line:
   - keywords[function, return, arg0, arg1, '\t'],
   - operands[x, y],
   - operators[<-, +, *, (, ), ',', #, whitespace],
   - variables[value, one],
   - functions[print, increment],
   - parameters[local para]
   - types[real]
3. Check new line syntax based on previous line tokens
4. Write translated tokens to c code and format.
5. Validates and Report any errors commencing with the ! character. (Must detect all invalid ml programs)
6. Generate C11 code in a file name, for exmaple ml-$pid.c
7. Compile ml-$pid.c
8. Execute ./ml-$pid, passing any optional command-line arguments(real numbers)
9. Removes any files that it created (cleanup).

## Workflow

See [CONTRIBUTING.md](CONTRIBUTING.md)
