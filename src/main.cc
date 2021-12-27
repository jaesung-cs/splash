#include <iostream>

#include <splash/application.h>

int main()
{
  try
  {
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
