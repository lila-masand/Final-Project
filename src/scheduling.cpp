/* Lila Masand, CS377 Operating Systems Final Project, 5/17/23

Hello! This is my Scheduling Visualizer program, based on Project 3 from our class.
The full readme is included in the repository, but this program essentially
runs  scheduling algorithms on different workloads - 3 workloads are provided,
but the user can create their own as well. After it runs the algorithm, the result is
drawn on a graph that shows the time slices from each of the processes that ran.

This program requires no arguments (unlike the original) and the workload text files
can of course be easily altered to preload different workloads into the program.

SFML with OpenGL is used as the graphics library, and all dependencies and configuration files are included 
in the repository.


*/

#include <scheduling.h>
#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <string>
#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

//Max of 150 slices since our maximum duration is 150, and RR uses
//1-time unit slices
sf::RectangleShape slices[200];

//Algorithms fill in the array and set this variable when they run
int sliceNum = 0;

//Used to scale time slice duration to Rectangle width
int sliceSize = 5;

//Colors to use when assigning each proccess a color. Unfortunately, SFML does not have
//many built-in colors, so we can only have 6 unique processes per workload
sf::Color colors[] = {sf::Color::Red, sf::Color::Blue, sf::Color::Green, sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan, sf::Color(242, 170, 24, 1)};

std::string filenames[3] = {"workload_01.txt", "workload_02.txt", "workload_03.txt"};

//3 preloaded workloads, one custom slot
pqueue_arrival workloads[4];
pqueue_arrival dummy;

//Altered to load all workload files into the workloads array, rather than just the file
//received as input
void read_workloads() {

  for(int i = 0; i < 3; i++){

    pqueue_arrival workload;

    std::fstream input(filenames[i], std::ios::in);

    std::string currLine;
    std::string arrival;
    std::string duration;

    int numProcess = 0;

    if (input.is_open()){

      while (!input.eof()){

        arrival = "";
        duration = "";


        input >> arrival >> duration;

        if(arrival.compare("") != 0 && duration.compare("") != 0){

          Process newprocess;
          newprocess.arrival = stoi(arrival);
          newprocess.duration = stoi(duration);

          newprocess.completion = -1;
          newprocess.first_run = -1;
          newprocess.id = numProcess;

          workload.push(newprocess);
          numProcess++;
        }        
      }
    }

    workloads[i] = workload;
    input.close();
  }
}

//Create the custom workload using the arrival and duration arrays that we
//make in draw_schedule.
pqueue_arrival custom_workload(int arrival[], int duration[], int size){

  pqueue_arrival workload;

  for(int i = 0; i < size; i++){

    Process newprocess;
    newprocess.arrival = arrival[i];
    newprocess.duration = duration[i];

    newprocess.completion = -1;
    newprocess.first_run = -1;
    newprocess.id = i;

    workload.push(newprocess);
  }

  return workload;
}

//Neither this function nor the next is used, but they are potentially useful if the program
//structure were to be changed, so I kept them just in case
void show_workload(pqueue_arrival workload) {
  pqueue_arrival xs = workload;
  std::cout << "Workload:" << std::endl;
  while (!xs.empty()) {
    Process p = xs.top();
    std::cout << '\t' << p.arrival << ' ' << p.duration << std::endl;
    xs.pop();
  }
}

void show_processes(std::list<Process> processes) {
  std::list<Process> xs = processes;
  std::cout << "Processes:" << std::endl;
  while (!xs.empty()) {
    Process p = xs.front();
    std::cout << "\tarrival=" << p.arrival << ", duration=" << p.duration
         << ", first_run=" << p.first_run << ", completion=" << p.completion
         << std::endl;

    xs.pop_front();
  }
}

//Used show_metrics and show_processes as models to make a function that returns
//their output as strings that we can draw later
std::string* metricsToText(int alg, std::list<Process> processes){

  static std::string output[2];
  std::string metrics = "";
  std::string stats = "";

  float avg_t = avg_turnaround(processes);
  float avg_r = avg_response(processes);

  metrics += "Average Turnaround Time: " + std::to_string(avg_t) + "\nAverage Response Time: " + std::to_string(avg_r) + "\n";

  //fetching process stats
  while (!processes.empty()) {

    Process p = processes.front();
    stats += "Process " + std::to_string(p.id) + ": arrival = " + std::to_string(p.arrival) + ", duration = " + std::to_string(p.duration) + ", first run = " + std::to_string(p.first_run) + ", completion = " + std::to_string(p.completion) + "\n";
    processes.pop_front();
  }

  output[0] = metrics;
  output[1] = stats;
  return output;
}

//This function initializes graphics object and runs the main game loop for the program. 
int draw_schedule(){

  //Set "custom workload" spot to a dummy empty workload
  workloads[3] = dummy;

  //Request a 24-bits depth buffer when creating the window
  sf::ContextSettings contextSettings;
  contextSettings.depthBits = 24;

   //Create the main window
  sf::RenderWindow window(sf::VideoMode(840, 480), "Schedule Visualizer", sf::Style::Default, contextSettings);

  //Make it the active window for OpenGL calls
  window.setActive();

  //Clear the color and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //Booleans to direct program flow - which screen are we on, what are we doing
  bool atHome = true;
  bool atSchedule = false;
  bool atCreate = false;
  bool arrEdit = false;
  bool durEdit = false;

  //0 = FIFO, 1 = SJF, 2 = RR
  int algType = -1;

  //Size of the custom workload
  int workloadSize = 0;

  //To be used during custom workload creation
  int arrival[15];
  int duration[15];

  pqueue_arrival customWorkload;

  //At program start, the first workload is pre-selected
  pqueue_arrival currWorkload = workloads[0];

  //Initialize the arrays that will hold the arrival and duration values for the
  //processes in the custom workload
  for(int i = 0; i < 15; i++){

    arrival[i] = -1;
    duration[i] = -1;
  }

  //These two strings hold the user's numeric (string) input for the process they are
  //creating, which will be converted to ints and inserted into the arrays above
  std::string newarrival = "";
  std::string newduration = "";
  
  //Draws text on the workload creation screen
  sf::Text process;

  sf::Font font;
  font.loadFromFile("res/arial.ttf");

  //Text that displays on the home screen
  sf::Text home;
  home.setFont(font);
  home.setCharacterSize(20);
  home.setFillColor(sf::Color::White);
  home.setString("");

  //Add all pre-loaded workload options
  for(int i = 0; i < 4; i++){

    if(!workloads[i].empty())
      home.setString(home.getString() + "Press " + std::to_string(i + 1) + " to select Workload " + std::to_string(i + 1) + "\n");
  }

  home.setString(home.getString() + "\nPress C to create a custom workload.\n\nSelect an algorithm to run on your workload input.\n\nType F for FIFO, S for SJF, T for STCF, and R for RR. Press 'Enter' to run the algorithm.");

  //Multipurpose text object - used on schedule screen and workload
  //creation screen
  sf::Text text;
  text.setFont(font);
  text.setCharacterSize(12);
  text.setFillColor(sf::Color::White);

  //Average turnaround and response times printout
  sf::Text metrics;
  metrics.setFont(font);
  metrics.setCharacterSize(14);
  metrics.setFillColor(sf::Color::White);
  metrics.setPosition(sf::Vector2f(470.f, 335.f));

  //Process stats - what show_processes used to show
  sf::Text stats;
  stats.setFont(font);
  stats.setCharacterSize(13);
  stats.setFillColor(sf::Color::White);
  stats.setPosition(sf::Vector2f(40.f, 325.f));

  //Displays the user's workload choice on the home screen
  sf::Text workchoice;
  workchoice.setFont(font);
  workchoice.setCharacterSize(14);
  workchoice.setFillColor(sf::Color::Yellow);
  workchoice.setPosition(sf::Vector2f(50.f, 300.f));
  workchoice.setString("");

  //Displays the user's algorithm choice on the home screen
  sf::Text algchoice;
  algchoice.setFont(font);
  algchoice.setCharacterSize(14);
  algchoice.setFillColor(sf::Color::Yellow);
  algchoice.setPosition(sf::Vector2f(50.f, 350.f));
  algchoice.setString("");

  //creating the line objects for the graph axes
  sf::Vertex line[] =
  {
  sf::Vertex(sf::Vector2f(40, 100)),
  sf::Vertex(sf::Vector2f(40, 280)),
  sf::Vertex(sf::Vector2f(39, 280)),
  sf::Vertex(sf::Vector2f(800, 280))
  };

  //Start the game loop
  while (window.isOpen())
  {

    //loop between drawing home screen and schedule based on events
      //Process events
      sf::Event event;
      while (window.pollEvent(event))
      {
          //Close window: exit
          if (event.type == sf::Event::Closed)
              window.close();

          //Escape key: exit
          if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))
              window.close();

          //1: picks first workload
          if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Num1) && atHome){
              currWorkload = workloads[0];
              workchoice.setString("Workload 1 has been chosen");
          }

          //2: picks second workload
          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Num2) && atHome){
              currWorkload = workloads[1];
              workchoice.setString("Workload 2 has been chosen");
          }

          //3: picks third workload
          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Num3) && atHome){
              currWorkload = workloads[2];
              workchoice.setString("Workload 3 has been chosen");
          }

          //4: picks fourth workload (the custom workload)
          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Num4) && atHome && !workloads[3].empty()){
              currWorkload = workloads[3];
              workchoice.setString("Workload 4 has been chosen");
          }

          //F: picks FIFO
          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::F) && atHome){
              algType = 0;
              algchoice.setString("FIFO chosen");
          }

          //S: picks SJF
          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::S) && atHome){
              algType = 1;
              algchoice.setString("SJF chosen");
          }

          //R: picks Round Robin
          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::R) && atHome){
              algType = 2;
              algchoice.setString("RR chosen");
          }

          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::T) && atHome){

            algType = 3;
            algchoice.setString("STCF chosen");
          }

          //Use Backspace on any screen to return to home. Reset all necessary variables
          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::BackSpace) && !atHome){
              sliceNum = 0;
              atHome = true;
              atSchedule = false;
              atCreate = false;
              home.setString("");
              workchoice.setString("");
              algchoice.setString("");

              for(int i = 0; i < 4; i++){

                if(!workloads[i].empty())
                  home.setString(home.getString() + "Press " + std::to_string(i + 1) + " to select Workload " + std::to_string(i + 1) + "\n");

              }

              home.setString(home.getString() + "\nPress C to create a custom workload.\n\nSelect an algorithm to run on your workload input.\n\nType F for FIFO, S for SJF, T for STCF, and R for RR. Then, press 'Enter'.");
          }

          //Enter: Once algorithm and workload are chosen, press enter to run the algorithm
          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Enter) && algType > -1){
              atSchedule = true;
              atHome = false;
          }

          //C: Move to workload creation screen
          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::C) && atHome){

            atCreate = true;
            atHome = false;
            text.setString("To add a process to your new workload, press Tab.");
            text.setPosition(sf::Vector2f(10.f, 10.f));
            text.setCharacterSize(18);
          }

          //Tab: Once on the workload creation screen, press Tab to make processes
          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Tab) && atCreate && !arrEdit && !durEdit){

            text.setString("Type an arrival time, then press enter. Next, type a duration, and press enter again.\n\nPress Tab to add another process to the workload.\n\nOnce you're done, press D to create the workload.\n\nUse Backspace to clear your entries if you make a mistake, and also to go back to the home screen.");
            arrEdit = true;

            for(int i = 0; i < 15; i++){

              arrival[i] = -1;
              duration[i] = -1;
            }

            workloadSize = 0;
          }

          //Resize event: adjust the viewport (doesn't work that well with Round Robin graphs)
          if (event.type == sf::Event::Resized)
              glViewport(0, 0, event.size.width, event.size.height);
      }

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      //At home screen
      if(atHome){

        home.setPosition(sf::Vector2f(10.f, 10.f));
        window.draw(home);
        window.draw(workchoice);
        window.draw(algchoice);
      }

      //At workload creation screen
      else if(atCreate){
        
        //When we enter edit mode, reset text object/strings
        if(arrEdit){
          process.setFont(font);
          process.setCharacterSize(18);
          process.setFillColor(sf::Color::White);
          process.setPosition(sf::Vector2f(50.f, 200.f));
          process.setString("Process " + std::to_string(workloadSize) + " -  Arrival: ");

          newarrival = "";
          newduration = "";
        }

        window.draw(text);

        //During the workload creation, we have to enter another loop that independently polls for events
        while((arrEdit || durEdit) && (arrival[workloadSize] < 0 || duration[workloadSize] < 0) && window.isOpen()){

          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

          if(window.pollEvent(event)){

            if (event.type == sf::Event::Closed)
              window.close();

            else if(event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::BackSpace)){
              arrEdit = false;
              durEdit = false;
            }

            else if(event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Tab)){

              process.setString(process.getString() + "\nProcess " + std::to_string(workloadSize) + " -  Arrival: ");
              newarrival = "";
              newduration = "";
            }

            else if(event.type == sf::Event::TextEntered && event.text.unicode < 58 && event.text.unicode > 47){

              if(arrEdit){

                newarrival += static_cast<char>(event.text.unicode);
                process.setString(process.getString() + static_cast<char>(event.text.unicode));

              }

              else if(durEdit){

                newduration += static_cast<char>(event.text.unicode);
                process.setString(process.getString() + static_cast<char>(event.text.unicode));
              }

            }

            else if(event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Enter) && newarrival.compare("") != 0){

              if(arrEdit && atoi(newarrival.c_str()) >= 0 && atoi(newarrival.c_str()) < 150){
                arrival[workloadSize] = atoi(newarrival.c_str());
                arrEdit = false;
                durEdit = true;
                process.setString(process.getString() + "   DONE   Duration: ");
              }

              else if(durEdit && atoi(newduration.c_str()) > 0 && (atoi(newarrival.c_str()) + atoi(newduration.c_str())) <= 150){

                duration[workloadSize] = atoi(newduration.c_str());
                newduration += "  DONE";
                process.setString(process.getString() + "  DONE");
                durEdit = false;
                arrEdit = true;
                workloadSize++;
              }

            }

            else if(event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::D)){

              text.setString("New custom process added.\nPress Backspace to return to the home screen, where you can select the new process as Process 4.\nIf you choose to make another custom workload, it will replace the current one.");
              workloads[3] = custom_workload(arrival, duration, workloadSize);
              arrEdit = false;
              durEdit = false;
            }


          }

          window.draw(text);
          window.draw(process);
          window.display();

        }
      }

      //At graph screen
      else if(atSchedule){

        switch(algType){
          std::string* metricStrings;
          case 0:
            metricStrings = metricsToText(0, fifo(currWorkload));
    
            metrics.setString(metricStrings[0]);
            stats.setString(metricStrings[1]);
            break;

          case 1:
            metricStrings = metricsToText(0, sjf(currWorkload));
            metrics.setString(metricStrings[0]);
            stats.setString(metricStrings[1]);
            break;
          
          case 2:
            metricStrings = metricsToText(0, rr(currWorkload));
            metrics.setString(metricStrings[0]);
            stats.setString(metricStrings[1]);
            break;

          case 3:
            metricStrings = metricsToText(0, stcf(currWorkload));
            metrics.setString(metricStrings[0]);
            stats.setString(metricStrings[1]);
            break;
        }

        //setting and printing axis labels
        text.setCharacterSize(12);
        text.setPosition(sf::Vector2f(32.f, 283.f));
        text.setString("0");
        window.draw(text);

        for(int i = 1; i < 16; i++){

          text.setString(std::to_string(i*10));
          text.move(sf::Vector2f(50.f, 0.f));
          window.draw(text);
        }

        text.setCharacterSize(14);
        text.setString("Press 'Backspace' to return to the home screen.");
        text.setPosition(sf::Vector2f(20.f, 20.f));
        window.draw(text);

        window.draw(metrics);
        window.draw(stats);

        //draw axes
        window.draw(line, 4, sf::Lines);

        //initializing slice color legend - colored squares
        sf::RectangleShape legend(sf::Vector2f(10.f, 10.f));
        legend.setPosition(sf::Vector2f(47.f, 83.f));

        //legend labels
        sf::Text label;
        label.setFont(font);
        label.setCharacterSize(12);
        label.setPosition(sf::Vector2f(30.f, 80.f));

        //drawing legend
        for(int i = 0; i < currWorkload.size(); i++){

          label.setString("P" + std::to_string(i));
          label.move(sf::Vector2f(40.f, 0.f));
          window.draw(label);
          
          legend.setFillColor(colors[i]);
          legend.move(sf::Vector2f(40.f, 0.f));
          window.draw(legend);
        }

        for(int i = 0; i < sliceNum; i++){
          
          //draws slices added by the chosen algorithm
          window.draw(slices[i]);
        }

        //don't run algorithm again - just draw and wait for input
        algType = -1;
      }

      window.display();

  }

  return 0;
}


std::list<Process> fifo(pqueue_arrival workload) {

  std::list<Process> complete;

  //ordered by arrival time
  pqueue_arrival processes = workload;
  int lastfinish = 0;

  while(!processes.empty()){

    Process curr = processes.top();

    if(curr.arrival > lastfinish)
      lastfinish = curr.arrival;

    curr.first_run = lastfinish;
    curr.completion = lastfinish + curr.duration;
    lastfinish = curr.completion;

    complete.insert(complete.end(), curr);

    //after process has run, make a time slice for it
    sf::RectangleShape newrec(sf::Vector2f((curr.duration)*sliceSize, 150.f));

    newrec.setFillColor(colors[curr.id]);


    newrec.setPosition(sf::Vector2f((float)(curr.first_run*sliceSize + 40), (float)130));

    slices[sliceNum] = newrec;

    processes.pop();
    sliceNum++;
  }

  return complete;
}

std::list<Process> sjf(pqueue_arrival workload) {
  std::list<Process> complete;

  //One queue ordered by arrival, the other ordered by duration
  pqueue_arrival total = workload;
  pqueue_duration arrived;

  //this contains processes that have arrived
  //and orders them by duration, as needed for sjf
  arrived.push(total.top());
  total.pop();

  int lastfinish = 0;
  Process curr = arrived.top();

  while(!arrived.empty() || !total.empty()){

    //if the next process arrives after the last process finished, jump ahead to it
    if(curr.arrival > lastfinish){
      lastfinish = arrived.top().arrival;
    }

    //push any processes that have arrived during the last job's run or at the same time
    //as our current process onto the arrived queue
    while(!total.empty() && total.top().arrival <= lastfinish){

      arrived.push(total.top());
      total.pop();
      curr = arrived.top();
    }

    curr.first_run = lastfinish;
    curr.completion = lastfinish + curr.duration;
    lastfinish = curr.completion;

    complete.insert(complete.end(), curr);

    //after the process has run, make a time slice for it
    sf::RectangleShape newrec(sf::Vector2f((curr.duration)*sliceSize, 150.f));

    newrec.setFillColor(colors[curr.id]);

    newrec.setPosition(sf::Vector2f((float)(curr.first_run*sliceSize + 40), (float)130));

    slices[sliceNum] = newrec;

    arrived.pop();

    if(arrived.empty() && !total.empty()){
      arrived.push(total.top());
      total.pop();
      curr = arrived.top();
    }      

    else if(!arrived.empty()){

      curr = arrived.top();
    }

    sliceNum++;
  }

  return complete;
}

std::list<Process> stcf(pqueue_arrival workload) {

  pqueue_arrival total = workload;
  int durations[workload.size()];
  int size = workload.size();

  for(int i = 0; i < size; i++){

    durations[workload.top().id] = workload.top().duration;
    workload.pop();
  }

  std::list<Process> complete;

  pqueue_duration arrived;
  int clock = 0;
  int lastfinish = 0;

  Process currprocess;
  arrived.push(total.top());
  total.pop();
  currprocess = arrived.top();

  while(!arrived.empty() || !total.empty()){

    if(currprocess.arrival > clock){

      clock = currprocess.arrival;
      lastfinish = clock;
    }

    //at the beginning of every loop, see whether any new processes
    //are arriving at the moment
    while(!total.empty() && (total.top()).arrival == clock){

      if(total.top().duration < currprocess.duration){

        Process clone;
        clone.id = currprocess.id;
        clone.first_run = currprocess.first_run;
        clone.arrival = currprocess.arrival;
        clone.duration = currprocess.duration;

        arrived.pop();
        arrived.push(clone);
        currprocess = clone;
      }
      arrived.push(total.top());
      total.pop();
    }

    if(arrived.top().id != currprocess.id && (arrived.top().duration) < (currprocess.duration)){

      sf::RectangleShape newrec(sf::Vector2f(((clock - lastfinish)*sliceSize), 150.f));

      newrec.setFillColor(colors[currprocess.id]);

      newrec.setPosition(sf::Vector2f((float)(lastfinish*sliceSize + 40), (float)130));

      slices[sliceNum] = newrec;

      sliceNum++;

      Process clone;
      clone.id = currprocess.id;
      clone.first_run = currprocess.first_run;
      clone.arrival = currprocess.arrival;
      clone.duration = currprocess.duration;

      lastfinish = clock;

      currprocess = arrived.top();
    }

    if(currprocess.first_run == -1){

      currprocess.first_run = lastfinish;
    }

    currprocess.duration = currprocess.duration - 1;
    clock++;

    if(currprocess.duration <= 0){

      currprocess.completion = clock;

      sf::RectangleShape newrec(sf::Vector2f(((clock - lastfinish)*sliceSize), 150.f));

      newrec.setFillColor(colors[currprocess.id]);

      newrec.setPosition(sf::Vector2f((float)(lastfinish*sliceSize + 40), (float)130));

      slices[sliceNum] = newrec;

      sliceNum++;

      lastfinish = clock;

      currprocess.duration = durations[currprocess.id];
      complete.insert(complete.end(), currprocess);
      arrived.pop();

      if(!arrived.empty()){
        currprocess = arrived.top();
      }

      else if(!total.empty()){

        arrived.push(total.top());
        total.pop();
        currprocess = arrived.top();
      }

    }

    if(total.empty() && arrived.size() == 1){

      if(currprocess.first_run == -1){

        currprocess.first_run = clock;
      }

      clock = clock + currprocess.duration;

      sf::RectangleShape newrec(sf::Vector2f(((clock - lastfinish)*sliceSize), 150.f));

      newrec.setFillColor(colors[currprocess.id]);

      newrec.setPosition(sf::Vector2f((float)(lastfinish*sliceSize + 40), (float)130));

      slices[sliceNum] = newrec;

      sliceNum++;
      currprocess.completion = clock;
      currprocess.duration = durations[currprocess.id];

      complete.insert(complete.end(), currprocess);
      arrived.pop();
    }

  }

  return complete;
}

//This algorithm runs by passing processes back and forth between
//two queues.
std::list<Process> rr(pqueue_arrival workload) {
  std::list<Process> complete;

  pqueue_arrival total = workload;
  pqueue_arrival arrived;

  int clock = 0;
  int lastfinish = 0;

  Process current = total.top();

  bool totalturn = false;

  //finish current job, push onto arrived
  while(!total.empty() || !arrived.empty()){

    if(total.empty()){

      totalturn = true;
      current = arrived.top();
    }

    else if(arrived.empty()){

      totalturn = false;
      current = total.top();
    }

    clock++;

    //if the current process hasn't arrived yet, skip it for now. do this until you
    //reach the processes that have already arrived and/or started running
    while(current.arrival > (clock - 1)){

        if(totalturn == false){
          arrived.push(current);
          total.pop();
          current = total.top();
        }

        else{
          total.push(current);
          arrived.pop();
          current = arrived.top();
        }

         if(total.empty()){

          totalturn = true;
          current = arrived.top();

          if(current.arrival > (clock))
            clock = current.arrival;
        }

        else if(arrived.empty()){

          totalturn = false;
          current = total.top();

          if(current.arrival > (clock - 1)){
            clock = current.arrival + 1;
            lastfinish = clock - 1;
          }
        }


    }


    //has definitely arrived, is done with its timeslice
    if(current.arrival <= (clock - 1)){

      if(current.first_run == -1){

        current.completion = current.duration;
        current.first_run = clock - 1;
      }

      current.duration = current.duration - 1;

      //time slice is always 1 - 1*3 - 1 = 2
      sf::RectangleShape newrec(sf::Vector2f((1*sliceSize - 1), 150.f));

      newrec.setFillColor(colors[current.id]);

      newrec.setPosition(sf::Vector2f((float)(lastfinish*sliceSize + 40), (float)130));

      slices[sliceNum] = newrec;

      if(current.duration <= 0){

        int temp = current.completion;
        current.completion = clock;
        current.duration = temp;
        complete.insert(complete.end(), current);


        //take out of round robin permanently
        //if we're currently "reloading" total, current
        //will be at the top of arrived, and vice versa
        if(totalturn == true){
          arrived.pop();
          current = arrived.top();
        }

        else{

          total.pop();
          current = total.top();
        }

      }

      else{

        //finish, move to next job
        if(totalturn == false){
          arrived.push(current);
          total.pop();
          current = total.top();
        }

        else{
          total.push(current);
          arrived.pop();
          current = arrived.top();
        }

      }

       lastfinish = clock;
       sliceNum++;
    }

  }

  return complete;
}

float avg_turnaround(std::list<Process> processes) {

  std::list<Process> plist = processes;
  float avg_t = 0;

  for(auto it = plist.begin(); it != plist.end(); it++){

    avg_t += (it->completion - it->arrival);
  }

  return avg_t/plist.size();
}

float avg_response(std::list<Process> processes) {

  std::list<Process> plist = processes;
  float avg_r = 0;

  for(auto it = plist.begin(); it != plist.end(); it++){

    avg_r += (it->first_run - it->arrival);
  }

  return avg_r/plist.size();
}

void show_metrics(std::list<Process> processes) {
  float avg_t = avg_turnaround(processes);
  float avg_r = avg_response(processes);
  show_processes(processes);
  std::cout << '\n';
  std::cout << "Average Turnaround Time: " << avg_t << std::endl;
  std::cout << "Average Response Time:   " << avg_r << std::endl;

}
