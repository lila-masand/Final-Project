#include <list>
#include <queue>
#include <string>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

//using namespace std;

typedef struct Process Process;
struct Process {
  int arrival;
  int first_run;
  int duration;
  int completion;
  int id;
};

class ArrivalComparator {
 public:
  bool operator()(const Process lhs, const Process rhs) const {
    if (lhs.arrival != rhs.arrival)
      return lhs.arrival > rhs.arrival;
    else
      return lhs.duration > rhs.duration;
  }
};

class DurationComparator {
 public:
  bool operator()(const Process lhs, const Process rhs) const {
    if (lhs.duration != rhs.duration)
      return lhs.duration > rhs.duration;
    else
      return lhs.arrival > rhs.arrival;
  }
};

typedef std::priority_queue<Process, std::vector<Process>, ArrivalComparator>
    pqueue_arrival;
typedef std::priority_queue<Process, std::vector<Process>, DurationComparator>
    pqueue_duration;

pqueue_arrival read_workload(std::string filename);
void show_workload(pqueue_arrival workload);
void show_processes(std::list<Process> processes);
int draw_schedule(pqueue_arrival workload);
std::string* metricsToText(int alg, std::list<Process> processes);

std::list<Process> fifo(pqueue_arrival workload);
std::list<Process> sjf(pqueue_arrival workload);
std::list<Process> stcf(pqueue_arrival workload);
std::list<Process> rr(pqueue_arrival workload);

float avg_turnaround(std::list<Process> processes);
float avg_response(std::list<Process> processes);
void show_metrics(std::list<Process> processes);
