# Scheduling Visualizer
Lila Masand, CS377 Operating Systems, 5/17/23

## Overview

Hello! This repo holds my final project for CS377 Operating Systems at UMass. For my project, 
I chose to alter one of the projects that we had completed earlier in the course (Project 3)
where we had implemented different process scheduling algorithms. One of the proposed options
for a final project was to make a visual representation of memory storage, so I chose to apply
that concept to scheduling instead. I love making interactive programs/games, so I decided to 
make a program that would help CS students visualize how schedulers run jobs using the algorithms 
we learned about. 

My hope is that this project will help students learn about the algorithms and understand how
processes are chosen and run as they arrive. It helped me a lot to look at visual representations
of time slices when I was learning about scheduling, and even now I have a lot of fun making my
own workloads, experimenting with different edge cases and seeing how they run in my program.
Scheduling is a fundamental part of running an operating system - all OSes need to do it! - so
I hope that this visual demonstration is satisfying and helpful. There is LOTS of room for
polishing and improvement, and I may update the performance or UI in the future.

## Design

Our class used C++, and I chose to continue using this for my project to get more familiar with
it. Using SFML with OpenGL for the graphics library, I took the algorithms that I had already 
written and added functionality to draw the jobs and their time slices on a graph, as well as 
other metrics and instructions for the user. The program uses keyboard inputs only, and all 
necessary controls are displayed on each screen. The four scheduling algorithms are FIFO 
(First In, First Out), SJF (Shortest Job First), STCF (Shortest to Completion First), and 
RR (Round-Robin).

After users choose from one of the three pre-loaded workloads and pick a scheduling algorithm,
the program shows a graph with different-colored time slices that represent each process!
Metrics for the processes about the time of arrival, time of first run, duration, and completion
time will also be displayed, as well as the average turnaround time and response time for the
jobs. Additionally, the user can create their own custom workload to use by creating processes
and assigning them an arrival time and duration. This last part was one of the most important
aspects of the project overall, since it allows the user to actually engage with the concepts
in a deeper, more curious way.

## Libraries
As mentioned, this project uses the SFML with OpenGL graphics library. The include files are in the repo,
and the make file is configured to work with them, along with VSCode configuration files if that is your
chosen IDE. SFML is an extremely simple graphics library so I would only recommend it for small
projects in general (and for more graphic-based demonstrations more than a program for which you
need polished UI elements). That being said, it's very easy to use aside from a conflict I found with the
std namespace.

## Usage
Download the provided files and run `make` in your C++ environment, after which `./scheduling_app.exe`
will run the program. There are no arguments required. The workload text files can be altered however
you would like - the arrival time is first, and then duration of the process (0 10 = a process with an
arrival of 0 and a duration of 10).

## Documentation
Further documentation is present in the source files.

`void read_workloads`  

This method used to be `read_workload` and was altered to load all 3 workload text files at the
start of the program. It creates the workloads and then copies them into an array that holds
these workloads, and then leaves an empty slot for the user's custom workload.

`std::string* metricsToText`  

Here, I used the old `show_processes` and `show_metrics` methods as a base and wrote this new one to return two
strings showing the overall metrics of the algorithm run. It returns each processes' attribute
values at the end of the algorithm, as well as the stats for average turnaround and average
response time. These strings are later assigned to text objects and drawn in the game loop.

`void draw_schedule`  

This method holds the main game loop that runs the program. SFML graphics objects such as text,
shapes, and the window itself are declared before entering the loop, and then the game loop runs
as long as the window is open. There is a second loop that runs as long as there are events in the
event queue - SFML provides both mouse and key listeners - and any valid input is mapped to an
outcome such as picking a workload, picking an algorithm, submitting the input and running the
algorithm, switching to the "workload maker" screen, etc. The flow of the game loop is controlled
by boolean variables that denote which program screen is active and needs to be drawn.

To display the workload schedule, each algorithm calculates the size of the time slice and adds a
RectangleShape SFML object to a global array. On the screen that actually draws the schedule on a
graph, this array is accessed independently of which algorithm ran, and the time slices are drawn
in their assigned color, position, and size. A legend to tell the processes apart is also included
for each algorithm, especially since the colors assigned to the processes may change based on which
processes ended up running first.

The workload creation screen was the most complicated one to figure out. As the user types, their
numeric input for the arrival and duration of the new processes is read character by character and
stored in strings. It is also added to an SFML text object and displayed. The strings are later
converted to integers and stored in two arrays, `int duration[]` and `int arrival[]`,  once the user 
officially enters their input. For each array, arrival[i] and duration[i] correspond to the same 
process.  
  
In order for this creation screen to work properly during the "editing" state, I made another while
loop that polled for events and only exited if the user pressed Backspace to edit, or D to create
their workload. Otherwise, there would have been issues with certain variables going out of scope
when the overarching while loop was exited and re-entered. 

`pqueue_arrival custom_workload`  

This method uses the arrival and duration arrays to create the user's custom input in a similar way
to the `read_workloads` method. It reads the arrival and duration from each array at the same index
and creates the process, adding it to the pqueue. It copies this queue into the fourth spot in the
global workloads array, where there are already 3 pre-loaded workloads. If the user chooses to make
another custom workload, it will replace the first one.

`int fifo`, `int sctf`, `int sjf`, `int rr`  

As previously mentioned, each algorithm adds RectangleShapes to a global array as it runs. This means
that for Round-Robin and Shortest to Completion First, these time slices had to be calculated either
based on the timeslice size for RR (1 time unit) or based on how long a process ran before it was 
stopped and replaced in STCF. Both of these algorithms kept a clock and a variable that tracked the
last time a process finished running, which in most cases was also the time that the next process
started running. These variables were used to figure out the width of each RectangleShape.  
  
An 'id' variable was added to the Process struct in order to allow each algorithm to properly assign
the processes a specific color that applied to each of their timeslices. 

Here is my presentation video: https://www.youtube.com/watch?v=GioZw47CHiM
