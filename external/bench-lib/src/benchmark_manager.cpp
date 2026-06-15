#include "bench-lib/benchmark_manager.hpp"

namespace BenchLib {
  void BenchmarkManager::add_results(std::vector<BenchmarkCase> const &_executed_tasks) {
    executed_tasks.push_back(_executed_tasks);
  }

  void BenchmarkManager::average_by_task() {
    size_t n_executed_tasks = executed_tasks.size();
    if(n_executed_tasks <= 0) {
      std::cerr << "Error. Se necesita al menos una ejecución.";
      return;
    }
    size_t n_tasks = executed_tasks[0].size();
    tasks.resize(n_tasks);
    for(size_t i = 0; i < n_tasks; ++i) {
      double avg_avg_t = 0, avg_min_t = 0, avg_max_t = 0, avg_std_t = 0;
      for(size_t j = 0; j < n_executed_tasks; ++j) {
        avg_avg_t += executed_tasks[j][i].avg_time;
        avg_min_t += executed_tasks[j][i].min_time;
        avg_max_t += executed_tasks[j][i].max_time;
        avg_std_t += executed_tasks[j][i].std_time;
      }
      avg_avg_t /= n_executed_tasks;
      avg_min_t /= n_executed_tasks;
      avg_max_t /= n_executed_tasks;
      avg_std_t /= n_executed_tasks;
      tasks[i].explanation = executed_tasks[0][i].explanation;
      tasks[i].label = executed_tasks[0][i].label;
      tasks[i].input_size = executed_tasks[0][i].input_size;
      tasks[i].avg_time = avg_avg_t;
      tasks[i].min_time = avg_min_t;
      tasks[i].max_time = avg_max_t;
      tasks[i].std_time = avg_std_t;
      tasks[i].size_in_megabytes = executed_tasks[0][i].size_in_megabytes;
    }
  }
}