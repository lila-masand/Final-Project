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

pqueue_arrival read_workload(std::string filename) {
  pqueue_arrival workload;


  std::fstream input(filename, std::ios::in);


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

  input.close();

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
int draw_schedule(pqueue_arrival workload){

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
  //0 = fifo, 1 = sjf, 2 = rr
  int algType = -1;

  sf::Font font;
  font.loadFromFile("res/arial.ttf");

  /* Before running any algorithms, draw a "home screen" first */
  sf::Text home;
  home.setFont(font);
  home.setCharacterSize(20);
  home.setFillColor(sf::Color::White);
  home.setString("Select an algorithm to run on your workload input.\n\nType 0 for FIFO, 1 for SJF, and 2 for RR. Then, press enter.");


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

          if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Num0))
              algType = 0;

          //else if?
          if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Num1))
              algType = 1;

          if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Num2))
              algType = 2;

          if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::BackSpace)){
              sliceNum = 0;
              atHome = true;
              atSchedule = false;
          }

          if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Enter) && algType > -1){
              atSchedule = true;
              atHome = false;
          }

          // else print "please choose an algorithm"

          // Resize event: adjust the viewport
          if (event.type == sf::Event::Resized)
              glViewport(0, 0, event.size.width, event.size.height);
      }

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      if(atHome){

        home.setPosition(sf::Vector2f(150.f, 200.f));

        window.draw(home);

      }

      else if(atSchedule){

        //switch statement
        if(algType == 0){

          std::string* metricStrings = metricsToText(0, fifo(workload));
  
          metrics.setString(metricStrings[0]);
          stats.setString(metricStrings[1]);

        }
        else if(algType == 1){
          std::string* metricStrings = metricsToText(0, sjf(workload));
  
          metrics.setString(metricStrings[0]);
          stats.setString(metricStrings[1]);

        }
        else if(algType == 2){
          std::string* metricStrings = metricsToText(0, rr(workload));
  
          metrics.setString(metricStrings[0]);
          stats.setString(metricStrings[1]);
        }

        //setting and printing axis labels
        text.setPosition(sf::Vector2f(32.f, 283.f));
        text.setString("0");
        window.draw(text);

        for(int i = 1; i < 16; i++){

          text.setString(std::to_string(i*10));
          text.move(sf::Vector2f(50.f, 0.f));
          window.draw(text);
        }

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
        for(int i = 0; i < workload.size(); i++){

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
    while(!total.empty() && total.top().arrival <= lastfinish){

      arrived.push(total.top());
      total.pop();

      //std::cout << arrived.size() + "\n";
    }

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
        }

        else if(arrived.empty()){

          totalturn = false;
          current = total.top();
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
