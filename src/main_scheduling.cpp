#include <scheduling.h>
#include <iostream>
#include <string>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

//using namespace std;

int main(int argc, char* argv[]) {

  //now, you only need to add the workload file
  if (argc != 2) {
    std::cout << "usage: [fifo|sjf|stcf|rr] workload_file" << std::endl;
    exit(1);
  }

  //std::string algorithm = argv[1];
  std::string workload_file = argv[1];

  pqueue_arrival workload = read_workload(workload_file);


  draw_schedule(workload);
  /* Call master game method here*/


  /*
  if (algorithm == "fifo") {
    show_metrics(fifo(workload));
    //draw_schedule(fifo(workload));
  } else if (algorithm == "sjf") {
    show_metrics(sjf(workload));
  } else if (algorithm == "stcf") {
    show_metrics(stcf(workload));
  } else if (algorithm == "rr") {
    show_metrics(rr(workload));
  } else {
    std::cout << "Error: Unknown algorithm: " << algorithm << std::endl;
    std::cout << "usage: [fifo|sjf|stcf|rr] workload_file" << std::endl;
    exit(1);
  }

  */


   return 0;
}
