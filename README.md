# Unix-Shell
Implemented a Unix shell command. To compile run:

gcc -o shell shell.c

Run:
./shell

Only works in a Linux or Unix environment. "Mocks" the cwd (current working directory) 
by keeping a string of it that can be updated.

List of availiable commands:

#  cd [directory]

Short for change directory. Substitue your directory for [directory]
Examples:
  cd /home/user
  cd .. (This will go up one directory, notice the space between cd and ..)

# pwd
  Print the working directory

# history [-c]

  Prints out a list of recently typed commands. The -c parameter (no brackets) clears the history.

# quit

Quits the shell

# run program [parameters]

Run a program with given optional parameters.

# background program [parameters]

Similiar to run, but runs a program in the background and prints out the PID of that running program.

# murder PID
 Terminates a given PID
