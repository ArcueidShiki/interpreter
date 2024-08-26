# interpreter

[CITS2002 Project1 Programming Language Interpreter](https://teaching.csse.uwa.edu.au/units/CITS2002/projects/project1.php)

## Environment

Linux / Unix C, gcc

## Build

Makefile or CMakeList

## Test

see projects exmaples.

## Workflow

```bash
# download code repository
1. git clone https://github.com/ArcueidShiki/SmartEntrySystem.git

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
