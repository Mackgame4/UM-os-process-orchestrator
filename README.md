# Trivial-Orchestrator OS

## About the Project

This project was developed for the subject Operating systems from University of Minho - Software Engineering degree.

#### Grade ⭐️ 17/20

### Demo 📽️

https://github.com/Darguima/Trivial-Orchestrator-OS/assets/49988070/e2734955-29ba-4648-bad3-668370feef39


### The goal ⛳️

In some words, the goal of this project was to create two C programs. The first is a server that orchestrate tasks that were requested from the second one - the client.

The server have the task of asynchronously execute all the tasks, being always available to receive new tasks, and managing all them in the best way.

If you want, you can read the [project statement](./report/enunciado.pdf) (in Portuguese).

### About the Code 🧑‍💻

This project uses pipes and FIFOs to communicate between the client and the server.

On the server side, we implemented a pipeline feature using pipes and the `dup2` function to redirect the STDOUT of one command to the STDIN of another. Additionally, we developed a [Scheduler](./server/src/scheduler/README.md) to determine the next task to execute. The scheduler employs the `FCFS` algorithm (with a circular queue) and the `SJF` algorithm, which utilizes a min-heap.

## Getting Started 🚀

#### Cloning the repository

```bash
$ git clone https://github.com/Trivial-Orchestrator-OS
$ git clone git@github.com:Darguima/Trivial-Orchestrator-OS.git
```

#### Running the project 🏃‍♂️

##### Compiling the code 🛠️

```bash
# To work on client
$ make client

$ ./client/client execute < estimated time > -u < command >
$ ./client/client execute < estimated time > -p < pipeline command >
$ ./client/client execute 5 -u "sleep 5" # Example

$ ./client/client status

# To work on server
$ make orchestrator

$ ./server/server < output directory > < max tasks > < scheduling policy >
$ ./server/server datapipe/ 3 FCFS  # Example
```

##### Testing 🧪

```bash
# Compile test bins
$ make compile_commands

# Start the server
$ ./server/server datapipe/ 3 FCFS  # Example

# Running test script
$ ./bin/runner.sh < number of tasks to run > < order of the tasks - asc, desc, random > < minimum time > < maximum time >
$ ./bin/runner.sh 4 random 1 3 # Example
```

##### Cleaning object files, executable and logs 🧹
```bash
$ make clean
```

## Developed by 🧑‍💻:

- [Afonso Pedreira](https://github.com/afooonso)
- [Dário Guimarães](https://github.com/darguima)
- [Fábio Magalhães](https://github.com/Mackgame4)
