#include <iostream>
#include <thread>
#include <tbb/tbb.h>

#include <splash/application.h>

int main()
{
  try
  {
    // TBB initialization for multiprocessing
    const auto nThreads = tbb::task_scheduler_init::default_num_threads();
    std::cout << nThreads << " threads" << std::endl;
    tbb::task_scheduler_init init(nThreads);

    splash::Application app;
    app.run();
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
