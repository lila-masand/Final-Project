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
  /*
  if (argc != 2) {
    std::cout << "usage: [fifo|sjf|stcf|rr] workload_file" << std::endl;
    exit(1);
  }
  */

  //std::string algorithm = argv[1];
  //std::string workload_file = argv[1];

  read_workloads();


  draw_schedule();

   return 0;
}
