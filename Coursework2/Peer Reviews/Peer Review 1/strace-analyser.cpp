/*
 * strace-analyser.cpp — Starter Code (Sequential Version)
 *
 * This program processes an strace log file, where each line
 * represents a system call and its return result, e.g.:
 *
 *     openat("/etc/ld.so.cache", O_RDONLY) = 3
 *
 * The sequential version reads the file line-by-line, extracts the system call
 * name and return value, and produces statistics on:
 *   - how many times each system call appears, and
 *   - how many of those calls failed (indicated by a negative return value).
 *
 * You will extend this program in later stages to add concurrency,
 * The WorkQueue class below is **intentionally**
 * incomplete and will be implemented when concurrency is added.
 *
 * Usage (sequential):
 *     ./strace-analyser <trace_file>
 *
 * Usage (concurrent, implemented later):
 *     ./strace-analyser <trace_file> [num_threads]
 */

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include <thread>
#include <mutex>
#include <condition_variable>

/*
 * WorkQueue
 *
 * A work queue for storing lines read from the input file, to be processed
 * later by worker threads.
 *
 * In this **sequential** version, this queue is not required and is left
 * unimplemented. You will complete this class for your solution.
 *
 * This WorkQueue must:
 *   - Store strings representing work items (lines from the file).
 *   - Allow one producer to push() lines into the queue.
 *   - Allow one or more consumers to pop() lines for processing.
 *   - Support a close() operation to indicate that no more items will be added.
 *   - Use appropriate synchronization primitives and avoid busy waiting.
 */
class WorkQueue {
public:
    /*
     * push adds a new work item (a line of text) to the queue.
     *
     * In the sequential version, this function is not implemented.
     * You will implement push() when adding concurrency.
     *
     * Parameters:
     *   line - a string representing a line from the trace file.
     */
    void push(std::string line) {

	    std::unique_lock<std::mutex> lock(mtx_);
	    q_.push(std::move(line));
	    cv_.notify_one();
    }

    /*
     * pop retrieves one line of work from the queue.
     *
     * Returns:
     *   true if a line was successfully retrieved and stored in 'out'.
     *   false if no more work is available (e.g., after close() is called).
     *
     * In the sequential version, this function is not implemented.
     * You will implement pop() when adding concurrency.
     *
     * Parameters:
     *   out - reference to a string where the retrieved line will be stored.
     */
    bool pop(std::string &out) {
	    
	    std::unique_lock<std::mutex> lock(mtx_);

	    while (q_.empty() && !closed_) {
		    cv_.wait(lock);
	    }

	    if (q_.empty() && closed_) {
		return false;
	}

	out = std::move(q_.front());
	q_.pop();
	return true;
    }

    /*
     * close marks the queue as finished, indicating that no additional items
     * will be pushed. Any threads waiting for work should be released.
     *
     * In the sequential version, this function is not implemented.
     * You will implement close() when adding concurrency.
     */
    void close() {
	    std::unique_lock<std::mutex> lock(mtx_);
	    closed_ = true;
	    cv_.notify_all();
    }

private:
    std::queue<std::string> q_;
    bool closed_ = false;
    std::mutex mtx_;
    std::condition_variable cv_;
};

/*
 * Stats represents statistics for a single system call.
 *
 * Fields:
 *   count - total number of times the system call appeared in the trace.
 *   fails - number of calls returning a negative value (indicating failure).
 */
struct Stats {
    std::uint64_t count = 0;
    std::uint64_t fails = 0;
};

/* Maps system call names to their corresponding statistics. */
using StatsMap = std::unordered_map<std::string, Stats>;

/*
 * parse_line extracts the system call name and result value from a trace line.
 *
 * Expected input format:
 *     syscallName(args...) = result
 *
 * Returns:
 *   1 if parsing succeeded and outputs are written to 'syscall' and 'result'.
 *   0 if parsing failed (e.g., malformed line).
 *
 * Parameters:
 *   line    - the input line to parse.
 *   syscall - output parameter: will contain the system call name (no args).
 *   result  - output parameter: will contain the result as an integer.
 */
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

/*
 * update_stats updates the statistics entry for the given system call.
 *
 * This function increments the total call count, and if the result is negative,
 * increments the failure count.
 *
 *
 * Parameters:
 *   stats   - reference to the map storing statistics per system call.
 *   syscall - the name of the system call.
 *   result  - the return value of the system call.
 */
void update_stats(StatsMap &stats, const std::string &syscall, int result) {
    Stats &s = stats[syscall];
    s.count++;
    if (result < 0) {
        s.fails++;
    }
}

/*
 * print_stats outputs the statistics for all system calls, sorted alphabetically
 * by system call name. Each line is printed in the form:
 *
 *   syscall: count=X, fails=Y
 *
 * Parameters:
 *   stats - a map from system call names to their statistics.
 */
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

/*
 * main is the entry point of the program.
 *
 * Usage:
 *   ./strace-analyser <trace_file> [num_threads]
 *
 * Parameters:
 *   argv[1] - the trace file to process (required).
 *   argv[2] - optional number of threads (ignored in sequential version).
 *
 * Returns:
 *   0 on success, non-zero on error (e.g., missing file).
 */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <trace_file> [num_threads]\n";
        return 1;
    }

    /* Parse optional num_threads (ignored for sequential version) */
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

    /* Open the trace file */
    std::ifstream infile(argv[1]);
    if (!infile) {
        std::cerr << "Error: cannot open input file: "
                  << argv[1] << "\n";
        return 1;
    }

    WorkQueue workQueue;
    StatsMap stats;
    std::mutex statsMutex;

    std::vector<std::thread> workers;
    workers.reserve(numThreads);

    for (int i = 0; i < numThreads; ++i) {
	    workers.emplace_back([&workQueue, &stats, &statsMutex]() {
		    std::string workItem;
		    while (workQueue.pop(workItem)) {
		    std::string syscall;
		    int result;
		    if (parse_line(workItem, syscall, result)) {
		    	std::lock_guard<std::mutex> lock(statsMutex);
		    	update_stats(stats, syscall, result);
		    }
		    }
		    });
    }

    std::string line;
    while (std::getline(infile, line)) {
	    workQueue.push(line);
    }

    workQueue.close();

    for (auto &t : workers) {
	    t.join();
    }

    print_stats(stats);
	    return 0;
}
