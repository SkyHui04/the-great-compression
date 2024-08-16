
#ifndef GENERAL_HELPERS
#define GENERAL_HELPERS
#include <iostream>
#include <chrono>

// record code runtime
class Timer
{
public:
    Timer() {}
    void begin()
        { begin_time = std::chrono::system_clock::now(); }
    void end()
        { end_time = std::chrono::system_clock::now(); }
    double getDuration() const
        { return (double) (end_time - begin_time).count() / 1E6; }
    void report() const
        { std::cout << "\tTime elapsed: " << getDuration() << "s\n"; }
    
private:
    std::chrono::time_point<std::chrono::system_clock> begin_time, end_time;
};

#endif // GENERAL_HELPERS
