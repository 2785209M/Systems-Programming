#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>

class WorkQueue {
public:
/* ==================Start of My Code(1)=====================*/

    void push(std::string line) {
        std::unique_lock<std::mutex> lock(mutex_);

        q_.push(std::move(line)); // Add the input to the back of queue

        cv_.notify_one(); // Wake up one thread (consumer)
    }

    bool pop(std::string &out) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        cv_.wait(lock, [&]{
            return !q_.empty() || closed_;
        }); // Lock the mutex whilst checking the predicate. Unlock the mutex whilst sleeping.

        if(q_.empty() && closed_){ // If the queue is empty and closed
            return false; // ... stop.
        }

        out = std::move(q_.front()); // Change pointed value to the pop value
        q_.pop(); // Delete the front element of the queue
        return true; // Continue the process
    }

    void close() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        closed_ = true; // close the queue

        cv_.notify_all(); // Wake up all threads so they can exit once the queue is empty and closed
    }

private:
    std::queue<std::string> q_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool closed_ = false;

/*==================================End of my Code(1)==================================*/
};

struct Stats {
    std::uint64_t count = 0;
    std::uint64_t fails = 0;
};


using StatsMap = std::unordered_map<std::string, Stats>;


int parse_line(const std::string &line, std::string &syscall, int &result) {
    std::size_t posOpen = line.find('(');
    if (posOpen == std::string::npos) return 0;
    syscall = line.substr(0, posOpen);

    std::size_t posEq = line.rfind('=');
    if (posEq == std::string::npos) return 0;

    std::size_t resultStart = line.find_first_not_of(' ', posEq + 1);
    if (resultStart == std::string::npos) return 0;

    std::string resultStr = line.substr(resultStart);
    try {
        result = std::stoi(resultStr);
    } catch (...) {
        return 0;  /* failed to parse integer */
    }
    return 1;
}


void update_stats(StatsMap &stats, const std::string &syscall, int result) {
    Stats &s = stats[syscall];
    s.count++;
    if (result < 0) {
        s.fails++;
    }
}


void print_stats(const StatsMap &stats) {
    std::vector<std::string> names;
    names.reserve(stats.size());

    for (const auto &pair : stats) {
        names.push_back(pair.first);
    }
    std::sort(names.begin(), names.end());

    for (const auto &name : names) {
        const Stats &s = stats.at(name);
        std::cout << name << ": count=" << s.count
                  << ", fails=" << s.fails << "\n";
    }
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <trace_file> [num_threads]\n";
        return 1;
    }

    int numThreads = 1;
    if (argc >= 3) {
        try {
            numThreads = std::stoi(argv[2]);
            if (numThreads <= 0) {
                std::cerr << "Warning: num_threads must be > 0. Using 1.\n";
                numThreads = 1;
            }
        } catch (...) {
            std::cerr << "Warning: invalid num_threads. Using 1.\n";
        }
    }

    std::ifstream infile(argv[1]);
    if (!infile) {
        std::cerr << "Error: cannot open input file: "
                  << argv[1] << "\n";
        return 1;
    }

    /*======================Start of my code (2)=========================*/
    // Thread-wise analysis
    WorkQueue workQueue;

    std::vector<std::thread> threads; // Store an Array of active Threads
    std::vector<StatsMap> statsMapArray(numThreads); // Create a StatsMap for each thread

    threads.reserve(numThreads); // Reserve (numThreads) threads for use
    for(int i = 0; i < numThreads; i++){
        threads.emplace_back([&workQueue, &statsMapArray, i](){ // Add [numThreads] threads to the threads array with pointers to the workQueue and StatsMap, and an index
            std::string line;
            statsMapArray[i].reserve(1000); // Reserve memory for the StatsMaps
            while(workQueue.pop(line)){ // Loop whilst the workQueue is not (empty and closed). This is how the threads wait for work.
                std::string syscall;
                int result;
                if(parse_line(line, syscall, result)){ // Parse the line from pop ...
                    update_stats(statsMapArray[i], syscall, result); // ... and if it's successful update the StatsMap
                }
            }
        });
    }

    std::string line;
    while(std::getline(infile, line)){
        workQueue.push(line);  // Push lines from the input to the queue
    }

    workQueue.close(); // Close the workQueue. Wake up the threads so they can finish popping.

    for(auto &t : threads){
        t.join(); // Rejoin all of the threads
    }

    // Aggregate per-thread StatsMaps into a single StatsMap
    StatsMap finalStats;
    for(const auto &statsMap : statsMapArray){
        for(const auto &pair : statsMap){ 
            // Merge Stats from all the threads into a single StatsMap (finalStats):
            Stats &dst = finalStats[pair.first]; // If the syscall doesn't exist yet, create it
            dst.count += pair.second.count; // Add the count
            dst.fails += pair.second.fails; // Add the fails
        }
    }

    print_stats(finalStats); // Print result
    return 0;
    /*==================================End of my Code(2)================================*/
}

