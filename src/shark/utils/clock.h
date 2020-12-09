#ifndef __CLOCK_H__
#define __CLOCK_H__

class Clock {
public:
    Clock() : last_tsc_(rdtsc()), last_time_(now_us() / usecs_per_msec) { }

    Clock(const Clock &) = delete;
    const Clock& operator= (const Clock& ) = delete;

    // cpu's timestamp counter. Return 0 if it's not available.
    uint64_t rdtsc() const;

    // high precision timestamp.   
    uint64_t get_us() const;
    
    // low precision timestamp. 
    // In tight loops, 10X faster than high precision timestamp.
    uint64_t get_ms();

private:
    const uint64_t usecs_per_msec = 1000;
    const uint64_t usecs_pre_sec  = 1000*1000;
    const uint64_t nsecs_pre_usec = 1000; // ns

private:
    uint64_t last_tsc_  = 0; // us
    uint64_t last_time_ = 0; // ms
};

#endif // __CLOCK_H__
