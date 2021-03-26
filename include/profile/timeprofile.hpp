#pragma once
#include <chrono>
#include <iomanip>
#include <iostream>

class TimeProfile
{
public:
    TimeProfile(bool show = false) {
        start = std::chrono::high_resolution_clock::now();
        showTimeDestr = show;
    }
    ~TimeProfile() {
        end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        if (showTimeDestr)
            std::cout << duration.count() << " ms\n"; //µs
    }
    int64_t getTime() noexcept
    {
        end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    bool showTimeDestr = false;
};
