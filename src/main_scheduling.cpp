#include <scheduling.h>
#include <iostream>
#include <string>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

//using namespace std;

int main(int argc, char* argv[]) {

  //load all workloads included in the project folder as .txt files
  read_workloads();

  //start game loop
  draw_schedule();

  return 0;
}
