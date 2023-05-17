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

//using namespace std;

//global or new array for each method with specific number of processes?
//sf::RectangleShape slices[100];
sf::RectangleShape slices[200];
int sliceSize = 5;
int sliceNum = 0;
sf::Color colors[] = {sf::Color::Red, sf::Color::Blue, sf::Color::Green, sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan};

std::string filenames[3] = {"workload_01.txt", "workload_02.txt", "workload_03.txt"};
//3 preloaded workloads, one custom slot
pqueue_arrival workloads[4];
pqueue_arrival dummy;

//altered to load all workload files into the workloads array
void read_workloads() {

  for(int i = 0; i < 3; i++){

    pqueue_arrival workload;

    std::fstream input(filenames[i], std::ios::in);

    std::string currLine;
    std::string arrival;
    std::string duration;

    int numProcess = 0;

    if (input.is_open()){

      //std::cout << "open";

      while (!input.eof()){

        arrival = "";
        duration = "";


        input >> arrival >> duration;

        //arrival.replace(arrival.find("\n"));
        //arrival.erase(remove_if(arrival.begin(), arrival.end(), [](char c) { return !isalpha(c); } ), arrival.end());
        //duration.erase(remove_if(duration.begin(), duration.end(), [](char c) { return !isalpha(c); } ), duration.end());
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
        //workload.push(pair<Process, vector<Process>>(Process, set1));
        

      }

    }

    workloads[i] = workload;
    input.close();

  }

}

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


std::string* metricsToText(int alg, std::list<Process> processes){

  static std::string output[2];
  std::string metrics = "";
  std::string stats = "";

  //std::list<Process> processes = fifo(workload);

  //make into a function that returns the two strings you need
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

/* 
plan to draw each resulting schedule:
  - create an array and add a Rectangle for every time slice (every time a slice finishes)
  - duration of slice = rectangle width, start time = x coordinate, etc.
  - slices labelled by process (A, B, C, etc.)

stretch goal:
  - interactive window with fields to put in process duration and arrival times
  - buttons that allow the user to choose an algorithm and run the corresponding method
*/

//call in main, pass in workload and workload size
int draw_schedule(){

  workloads[3] = dummy;

  // Request a 24-bits depth buffer when creating the window
  sf::ContextSettings contextSettings;
  contextSettings.depthBits = 24;

   // Create the main window
  sf::RenderWindow window(sf::VideoMode(840, 480), "SFML window with OpenGL", sf::Style::Default, contextSettings);

  // Make it the active window for OpenGL calls
  window.setActive();

  // Clear the color and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //separate method for drawing axes and axis labels

  bool atHome = true;
  bool atSchedule = false;
  bool atCreate = false;
  bool arrEdit = false;
  bool durEdit = false;
  //0 = fifo, 1 = sjf, 2 = rr
  int algType = -1;
  int workloadSize = 0;
  int arrival[15];
  int duration[15];
  pqueue_arrival customWorkload;
  pqueue_arrival currWorkload = workloads[0];

  for(int i = 0; i < 15; i++){

    arrival[i] = -1;
    duration[i] = -1;
  }

  std::string newarrival = "";
  std::string newduration = "";
  //char* test = "";
  sf::Text process;
  sf::Text arr;
  sf::Text dur;

  sf::Font font;
  font.loadFromFile("res/arial.ttf");

  /* Before running any algorithms, draw a "home screen" first */
  sf::Text home;
  home.setFont(font);
  home.setCharacterSize(20);
  home.setFillColor(sf::Color::White);
  home.setString("");

  for(int i = 0; i < 4; i++){

    if(!workloads[i].empty())
      home.setString(home.getString() + "\nPress " + std::to_string(i + 1) + " to select Workload " + std::to_string(i + 1));
  
  }

  home.setString(home.getString() + "\n\nPress C to create a custom workload.\n\nSelect an algorithm to run on your workload input.\n\nType F for FIFO, S for SJF, and R for RR. Press 'Enter' to run the algorithm.");


  //change to "axisnums"
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

  sf::Text workchoice;
  workchoice.setFont(font);
  workchoice.setCharacterSize(14);
  workchoice.setFillColor(sf::Color::Yellow);
  workchoice.setPosition(sf::Vector2f(50.f, 300.f));
  workchoice.setString("");

  sf::Text algchoice;
  algchoice.setFont(font);
  algchoice.setCharacterSize(14);
  algchoice.setFillColor(sf::Color::Yellow);
  algchoice.setPosition(sf::Vector2f(50.f, 350.f));
  algchoice.setString("");


  sf::Vertex line[] =
  {
  sf::Vertex(sf::Vector2f(40, 100)),
  sf::Vertex(sf::Vector2f(40, 280)),
  sf::Vertex(sf::Vector2f(39, 280)),
  sf::Vertex(sf::Vector2f(800, 280))
  };


  // Finally, display the rendered frame on screen

  // Start the game loop
  // might not want to adjust for window resize
  while (window.isOpen())
  {

    //loop between drawing home screen and schedule based on events
      // Process events
      sf::Event event;
      while (window.pollEvent(event))
      {
          // Close window: exit
          if (event.type == sf::Event::Closed)
              window.close();

          // Escape key: exit
          if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))
              window.close();

          if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Num1) && atHome){
              currWorkload = workloads[0];
              workchoice.setString("Workload 1 has been chosen");
          }

          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Num2) && atHome){
              currWorkload = workloads[1];
              workchoice.setString("Workload 2 has been chosen");
          }

          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Num3) && atHome){
              currWorkload = workloads[2];
              workchoice.setString("Workload 3 has been chosen");
          }

          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Num4) && atHome && !workloads[3].empty()){
              currWorkload = workloads[3];
              workchoice.setString("Workload 4 has been chosen");
          }

          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::F) && atHome){
              algType = 0;
              algchoice.setString("FIFO chosen");
          }

          //else if?
          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::S) && atHome){
              algType = 1;
              algchoice.setString("SJF chosen");
          }

          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::R) && atHome){
              algType = 2;
              algchoice.setString("RR chosen");
          }

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
                  home.setString(home.getString() + "\nPress " + std::to_string(i + 1) + " to select Workload " + std::to_string(i + 1));
              
              }

              home.setString(home.getString() + "\n\nPress C to create a custom workload.\n\nSelect an algorithm to run on your workload input.\n\nType F for FIFO, S for SJF, and R for RR. Then, press 'Enter'.");
          }

          //change so that both algorithm AND "workload" is chosen before running algorithm
          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Enter) && algType > -1){
              atSchedule = true;
              atHome = false;
          }

          // else print "please choose an algorithm"

          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::C) && atHome){

            atCreate = true;
            atHome = false;
            text.setString("To add a process to your new workload, press Tab.");
            text.setPosition(sf::Vector2f(10.f, 10.f));
            text.setCharacterSize(16);
          }

          else if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Tab) && atCreate && !arrEdit && !durEdit){

            text.setString("Type an arrival time, then press enter. Next, type a duration, and press enter again.\n\nPress Tab again to add another process to the workload.\n\nOnce you're done, press D to create the workload.\n\nUse Backspace to clear your entries if you make a mistake, and also to go back to the home screen.");
            arrEdit = true;

            for(int i = 0; i < 15; i++){

              arrival[i] = -1;
              duration[i] = -1;
            }

            workloadSize = 0;

          }


          // Resize event: adjust the viewport
          if (event.type == sf::Event::Resized)
              glViewport(0, 0, event.size.width, event.size.height);
      }

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      //add option listing possible workloads, map inputs to pick them
      if(atHome){

        //draw all workload options from the workloads array

        home.setPosition(sf::Vector2f(25.f, 10.f));

        window.draw(home);
        window.draw(workchoice);
        window.draw(algchoice);
      }

      else if(atCreate){

        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(arrEdit){
          //text.setString("To add a process to your new workload, press Tab. Enter values for arrival and duration, and press enter to save each one.\nNote: total duration must be less than or equal to 150.");
          process.setFont(font);
          process.setCharacterSize(18);
          process.setFillColor(sf::Color::White);
          process.setPosition(sf::Vector2f(50.f, 175.f));
          //process.move(sf::Vector2f(0.f, workloadSize*20));
          process.setString("Process " + std::to_string(workloadSize) + " -  Arrival: ");

          newarrival = "";
          newduration = "";
          //arr.setFont(font);
          //arr.setCharacterSize(16);
          //arr.setPosition(sf::Vector2f(130.f, 100.f));
          //arr.move(sf::Vector2f(0.f, workloadSize*20));
          //arr.setString(newarrival);
          //dur.setFont(font);
          //dur.setCharacterSize(16);
         // dur.setPosition(sf::Vector2f(200.f, 100.f));
          //dur.move(sf::Vector2f(0.f, workloadSize*20));
          //dur.setString(newduration);
        }

        //text.setString("To add a process to your new workload, press Tab. Enter values for arrival and duration, and press enter to save each one.\nNote: total duration must be less than or equal to 150.");

        window.draw(text);
        //window.draw(process);
        //window.draw(arr);

        while((arrEdit || durEdit) && (arrival[workloadSize] < 0 || duration[workloadSize] < 0) && window.isOpen()){

          //window.draw(process);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

          if(window.pollEvent(event)){

            if (event.type == sf::Event::Closed)
              window.close();

            else if(event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::BackSpace)){
              arrEdit = false;
              durEdit = false;
            }

            else if(event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Tab)){

              //process.move(sf::Vector2f(0.f, workloadSize*20));
              //process.setString(process.getString() + "   Arrival: " + std::to_string(atoi(newarrival.c_str())) + " Duration: " + std::to_string(atoi(newduration.c_str())) + "\nProcess " + std::to_string(workloadSize));
              process.setString(process.getString() + "\nProcess " + std::to_string(workloadSize) + " -  Arrival: ");
              newarrival = "";
              //arr.setString(newarrival);
              //arr.setPosition(sf::Vector2f(130.f, 100.f + 18*workloadSize));
              newduration = "";
              //dur.setString(newduration);
              //dur.setPosition(sf::Vector2f(200.f, 100.f + 18*workloadSize));
            }

            //limit to numeric
            else if(event.type == sf::Event::TextEntered && event.text.unicode < 58 && event.text.unicode > 47){

              //edit process string instead
              if(arrEdit){
                newarrival += static_cast<char>(event.text.unicode);
                //arr.setString(newarrival);
                //process.setString("Process " + std::to_string(workloadSize) + "   Arrival: " + std::to_string(atoi(newarrival.c_str())));
                process.setString(process.getString() + static_cast<char>(event.text.unicode));

              }

              else if(durEdit){

                newduration += static_cast<char>(event.text.unicode);
                //dur.setString(newduration);
                process.setString(process.getString() + static_cast<char>(event.text.unicode));
              }

            }

            
            /*
            //backspacing
            else if(event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::BackSpace)){

              
              // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


              // sf::Text process;
              // process.setFont(font);
              // process.setCharacterSize(16);
              // process.setPosition(sf::Vector2f(50.f, 100.f));

              // //make one long string with \n? should just make function to make the string
              // for(int i = 0; i < workloadSize; i++){

              //   process.move(sf::Vector2f(0.f, i*20));
              //   process.setString("Process " + std::to_string(i) + "  " + std::to_string(arrival[i]) + "  " + std::to_string(duration[i]));
              //   window.draw(process);
              // }

              

              if(arrEdit && strlen(newarrival) > 0){
                newarrival[strlen(newarrival) - 1] = '\0';
                arr.setString(newarrival);
                window.draw(arr);
              }

              else if(durEdit && strlen(newduration) > 0){
                newduration[strlen(newduration) - 1] = '\0';
                dur.setString(newduration);
                window.draw(dur);
              }

            }

            */

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
                //dur.setString(newduration);
                durEdit = false;
                arrEdit = true;
                workloadSize++;
              }

            }

            else if(event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::D)){

              text.setString("New custom process added.\nPress Backspace to return to the home screen, where you can select the new process as Process 4.\nIf you choose to make another custom workload, it will replace the current one.");
              workloads[3] = custom_workload(arrival, duration, workloadSize);
              //workload = workloads[3];
              arrEdit = false;
              durEdit = false;
            }


          }

          window.draw(text);
          window.draw(process);
          //window.draw(arr);
          //window.draw(dur);
          window.display();

        }

        //window.display();

      }

      else if(atSchedule){

        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        //switch statement
        if(algType == 0){

          std::string* metricStrings = metricsToText(0, fifo(currWorkload));
  
          metrics.setString(metricStrings[0]);
          stats.setString(metricStrings[1]);

        }
        else if(algType == 1){
          std::string* metricStrings = metricsToText(0, sjf(currWorkload));
  
          metrics.setString(metricStrings[0]);
          stats.setString(metricStrings[1]);

        }
        else if(algType == 2){
          std::string* metricStrings = metricsToText(0, rr(currWorkload));
  
          metrics.setString(metricStrings[0]);
          stats.setString(metricStrings[1]);
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
          
          //draws slices added by the algorithm
          window.draw(slices[i]);
        }

        //don't run algorithms again - wait for input
        algType = -1;
      }

      window.display();

  }

  return 0;
}


std::list<Process> fifo(pqueue_arrival workload) {


  std::list<Process> complete;
  //sf::RectangleShape slices[workload.size()];

  //ordered by arrival time
  pqueue_arrival processes = workload;
  int lastfinish = 0;

  //int numprocess = 0;
  //pqueue_arrival arrived;

  while(!processes.empty()){

    Process curr = processes.top();

    if(curr.arrival > lastfinish)
      lastfinish = curr.arrival;

    curr.first_run = lastfinish;
    curr.completion = lastfinish + curr.duration;
    lastfinish = curr.completion;

    complete.insert(complete.end(), curr);

    //for other methods, width might be lastfinish + completion?
    sf::RectangleShape newrec(sf::Vector2f((curr.duration)*sliceSize, 150.f));

    newrec.setFillColor(colors[curr.id]);


    newrec.setPosition(sf::Vector2f((float)(curr.first_run*sliceSize + 40), (float)130));

    slices[sliceNum] = newrec;

    //printf("\n Rectangle origin: %f\n", newrec.getPosition().x);

    processes.pop();
    sliceNum++;
  }

  //draw_schedule(slices, workload.size());

  return complete;
}

std::list<Process> sjf(pqueue_arrival workload) {
  std::list<Process> complete;

  pqueue_arrival total = workload;
  pqueue_duration arrived;

  //sf::RectangleShape slices[workload.size()];


  //this contains processes that have arrived
  //and orders them by duration, as needed for sjf
  arrived.push(total.top());
  total.pop();

  int lastfinish = 0;
  //int numprocess = 0;

  while(!arrived.empty() || !total.empty()){

    //std::cout << "running\n";

    //at the beginning of every loop, see whether any new processes
    //have arrived since the last process ran
    Process curr = arrived.top();

    curr.first_run = lastfinish;
    curr.completion = lastfinish + curr.duration;
    lastfinish = curr.completion;

    complete.insert(complete.end(), curr);

    //for other methods, width might be lastfinish + completion?
    sf::RectangleShape newrec(sf::Vector2f((curr.duration)*sliceSize, 150.f));

    newrec.setFillColor(colors[curr.id]);


    newrec.setPosition(sf::Vector2f((float)(curr.first_run*sliceSize + 40), (float)130));

    slices[sliceNum] = newrec;


    arrived.pop();
    sliceNum++;

    //if the next process arrives after the last process finished
    if(!total.empty() && total.top().arrival > lastfinish){
      arrived.push(total.top());
      total.pop();
      lastfinish = arrived.top().arrival;
    }

    else{
      //mark any processes that have arrived during the last job's run as "arrived"
      while(!total.empty() && total.top().arrival <= lastfinish){

        arrived.push(total.top());
        total.pop();

        //std::cout << arrived.size() + "\n";
      }

    }

  }


  //draw_schedule(slices, workload.size());
  return complete;
}

std::list<Process> stcf(pqueue_arrival workload) {
  std::list<Process> complete;

  pqueue_arrival total = workload;
  pqueue_duration arrived;
  int clock = 0;
  int lastfinish = 0;

  //this contains processes that have arrived
  //and orders them by duration

  Process firstprocess;
  firstprocess = total.top();

  firstprocess.first_run = firstprocess.arrival;

  firstprocess.completion = firstprocess.duration;

  clock = firstprocess.arrival;

  arrived.push(firstprocess);

  total.pop();

  //Process* curr = &firstprocess;


  while(!arrived.empty() || !total.empty()){

    //at the beginning of every loop, see whether any new processes
    //are arriving during or after the last (current) job
    while(!total.empty() && (total.top()).arrival <= (lastfinish + arrived.top().duration)){

      Process newprocess = total.top();

      Process currprocess = arrived.top();

      if(newprocess.arrival > clock){
        arrived.pop();

        clock = newprocess.arrival;

        if(currprocess.duration - (clock - lastfinish) <= 0){

          currprocess.duration = 0;
        }

        else{

          currprocess.duration = currprocess.duration - (clock - lastfinish);
        }

        arrived.push(currprocess);
      }


      if((newprocess.duration) < (currprocess.duration)){

        //set that job's stop point as the most recent stop point
        lastfinish = clock;

        if(newprocess.first_run == -1){

          //std::cout << "here";
          newprocess.first_run = lastfinish;
        }

      }

/*
      else if(newprocess.duration == currprocess.duration && newprocess.arrival == currprocess.arrival){

        arrived.pop();

        newprocess.completion = newprocess.duration;

        //add job to arrived queue
        currprocess.duration = 0;
        int temp = currprocess.duration;
        currprocess.duration = currprocess.completion;
        currprocess.completion = lastfinish + temp;

        lastfinish = currprocess.completion;

        if(currprocess.completion > clock){

          clock = currprocess.completion;
        }

        complete.insert(complete.end(), currprocess);
        
        newprocess.first_run = clock;
        arrived.push(newprocess);

      }

      else{
        
        newprocess.completion = newprocess.duration;
        //add job to arrived queue
        arrived.push(newprocess);
      }
      */

      //going to use this to hold the duration so that we can retrieve the value 
      //after the job is done
      newprocess.completion = newprocess.duration;
      arrived.push(newprocess);


      //set clock to this arrival time
      total.pop();
    }


    //job is done - either no other jobs finish sooner, or the newest arrivals will come after it finishes
    if(total.empty() || total.top().arrival > (lastfinish + arrived.top().duration) || arrived.top().duration == 0){

      Process currprocess = arrived.top();
      arrived.pop();

      //properly set duration and completed times
      int temp = currprocess.duration;
      currprocess.duration = currprocess.completion;
      currprocess.completion = lastfinish + temp;

      lastfinish = currprocess.completion;

      if(currprocess.completion > clock){
        clock = currprocess.completion;
      }

      complete.insert(complete.end(), currprocess);
      //set job with next shortest duration as the new job

      if(!arrived.empty() && arrived.top().first_run == -1){

        Process newprocess;
        newprocess = arrived.top();
        arrived.pop();
        newprocess.first_run = clock;
        arrived.push(newprocess);
      }
    }

    //if a job is interrupted, maybe make a copy with adjusted duration
    //and then add it back to the "arrived" queue?

    //std::cout << arrived.size();
  }

  return complete;
}

std::list<Process> rr(pqueue_arrival workload) {
  std::list<Process> complete;

  pqueue_arrival total = workload;
  pqueue_arrival arrived;
  //sf::RectangleShape slices[200];


  int clock = 0;
  int lastfinish = 0;
  //int numprocess = 0;

  Process current = total.top();

  bool totalturn = false;


  //run through all jobs based on arrival time, keep track of arrivals
  //and manage processes that arrive at the same time
  //pass processes back and forth between queues?

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

    //if the current process hasn't arrived yet, skip it for now
    //@me please fix this awful logic jfc hahahaha
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
  show_metrics(complete);

  //draw_schedule(slices, numprocess);
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
