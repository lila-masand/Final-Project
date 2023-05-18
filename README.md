# Scheduling Visualizer
Lila Masand, CS377 Operating Systems, 5/17/23

## Overview

Hello! This repo holds my final project for CS377 Operating Systems at UMass. For my project, 
I chose to alter one of the projects that we had completed earlier in the course (Project 3)
where we had implemented different process scheduling algorithms. I love making interactive
programs/games, so I decided to make a program that would help CS students visualize how
schedulers run jobs using the algorithms we learned about. 

Our class used C++, and I chose to continue using this for my project to get more familiar with
it. Using SFML with OpenGL for the graphics library, I took the algorithms that I had already 
written and added functionality to draw the jobs and their time slices on a graph, as well as 
other metrics and instructions for the user. The program uses keyboard inputs only, and all 
necessary controls are displayed on each screen. The three scheduling algorithms are FIFO 
(First In, First Out), SJF (Shortest Job First), STCF (Shortest to Completion First), and 
RR (Round-Robin).

After users choose from one of the three pre-loaded workloads and pick a scheduling algorithm,
the program shows a graph with different-colored time slices that represent each process!
Metrics for the processes about the time of arrival, time of first run, duration, and completion
time will also be displayed, as well as the average turnaround time and response time for the
jobs. Additionally, the user can create their own custom workload to use by creating processes
and assigning them an arrival time and duration.

My hope is that this project will help students learn about the algorithms and understand how
processes are chosen and run as they arrive. It helped me a lot to look at visual representations
of time slices when I was learning about scheduling, and even now I have a lot of fun making my
own workloads, experimenting with different edge cases and seeing how they run in my program.
Scheduling is a fundamental part of running an operating system - all OSes need to do it! - so
I hope that this visual demonstration is satisfying and helpful. There is LOTS of room for
polishing and improvement, and I may update the performance or UI in the future.

Here is my presentation video: https://www.youtube.com/watch?v=GioZw47CHiM
