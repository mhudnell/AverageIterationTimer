#pragma once
#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <tuple>

struct TimeChunk {
	TimeChunk(std::string name, const std::chrono::time_point<std::chrono::steady_clock>& begin) : name(std::move(name)), begin(begin) {}

	std::string name;
	std::optional<std::chrono::time_point<std::chrono::steady_clock>> begin;  // use optional to initialize to null, so we can know if these have ever been set
	std::optional<std::chrono::time_point<std::chrono::steady_clock>> end;
};

class SingleUseTimer
{
public:
    /**
     * \param name Name of the timer.
     * \param auto_start Automatically starts the timer when object is created. If false, you must call Start() manually.
     */
	SingleUseTimer(std::string name, bool auto_start=true);

	void Start();

	void End();

	void PrintDuration() const;

private:
	std::optional<TimeChunk> timechunk_;  // use optional so we can know if the timechunk has been properly initialized
	const std::string name_;
};

class AverageIterationTimer {
public:

    // Starts a new timechunk with name `timepoint_name`. Ends the previous timechunk if it exists.
	void SetTimepoint(const std::string& timepoint_name);

	// Terminates the previous timechunk. Inteded to be used only once, for the last timechunk;
	// normally you should just call SetTimepoint, which will end the previous timepoint if it exists.
	void EndTimepoint();

	// Adds timechunk to the duration totals, and clears current_timechunks_
	void IterationFinished();

    // Throws away the current iteration timechunks, they will not be included in `duration_totals_`
	void ScrapIteration();

    // Scraps the most recent timepoint with was started with SetTimepoint()
	void ScrapTimepoint();

	void PrintAverageDurations() const;

private:

	void EndTimepoint(const std::chrono::time_point<std::chrono::steady_clock>& t);

	int iter_count_ = 0;
	std::vector<TimeChunk> current_timechunks_;
	std::vector<std::string> insertion_order_;  // the same string which is the key in `duration_totals_`, just keeps track of the order they are inserted so we can print them out in order
	std::unordered_map<std::string, std::tuple<std::chrono::duration<float, std::micro>, int>> duration_totals_;  // tuple: (duration_total, total number of durations)
};
