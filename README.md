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

## Workflow

```bash
# download code repository
1. git clone https://github.com/ArcueidShiki/interpreter.git

# select a branch for development
2. git checkout dev
   or # solving current issue branch
   git checkout -b [issue_branch_name] [issue_branch_name]

3. Coding / solve issue / bugfix / writing test

# publish your changes
4. git add .
   or
   git add [path-to-your-files]

5. git commit -m "commit message"

# keep updated with remote main branch to avoid conflict
6. git pull origin main
   # if conflict with some files after running this command do following:
   1. git status # to see which files are conflict
   2. click [resolve] a blue button in vscode editor, compare and merges.
   3. git status # check all the conflicts are solved, if not, back to step2 until all the conflicts are solved.

7. git push origin [remote_branch_name]

8. New a pull request on GitHub, compare [main] to [branch_name]
```

## Requirements

```c
/**
 * Ref: https://www.toptal.com/scala/writing-an-interpreter
 * TODO: Program runml should do the following:
 * 1. Read each line of the file
 * 2. Extract tokens from each line:
 *      keywords[function, return, arg0, arg1, '\t'],
 *      operands[x, y],
 *      operators[<-, +, *, (, ), ',', #, whitespace],
 *      variables[value, one],
 *      functions[print, increment],
 *      parameters[local para]
 *      types[real]
 * 3. Check new line syntax based on previous line tokens
 * 4. Write translated tokens to c code and format.
 * 5. Validates and Report any errors commencing with the ! character. (Must detect all invalid ml programs)
 * 6. Generate C11 code in a file name, for exmaple ml-$pid.c
 * 7. Compile ml-$pid.c
 * 8. Execute ./ml-$pid, passing any optional command-line arguments(real numbers)
 * 9. Removes any files that it created (cleanup).
 *
 * Requirements:
 *
 * 0. Statements are written one per line without a terminating semicolon.
 * 1. All syntax errors detected in invalid ml programs must be reported via stderr on a line commencing with the '!' character. Your runml program must be able to detect all invalid ml programs - EXCEPT that your program will not be tested with any invalid expressions, so you do not need to validate the syntax of expressions.
 * 2. The only 'true' output produced by your translated and compiled program (when running) is the result of executing ml's print statement. Any 'debug' printing should appear on a line commencing with the '@' character.
 * 3. When printed, numbers that are exact integers must be printed without any decimal places; other numbers must be printed with exactly 6 decimal places.
 * 4. Only one datatype is supported: real numbers (e.g., 2.71828).
 * 5. Identifiers (variable and function names) consist of 1 to 12 lowercase alphabetic characters. (Check Condition)
 * 6. A maximum of 50 unique identifiers can appear in any program.
 * 7. Variables arg0, arg1,... argN etc., provide access to command-line arguments.
 * 8. Functions must be defined before they are called.
 * 9. Statements within a function must be indented with a tab.
 * 10. Functions have local scope for their parameters and identifiers.
 * 11. Programs execute statements sequentially from top to bottom, with function calls being the only form of control flow.
```
