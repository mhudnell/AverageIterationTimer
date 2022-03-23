#include "AverageIterationTimer.h"

#include <cassert>
#include <iostream>

static std::chrono::time_point<std::chrono::steady_clock> get_time_point() {
	return std::chrono::high_resolution_clock::now();
}

// calculates t2 - t1
static long long get_elapsed_ms(const std::chrono::time_point<std::chrono::steady_clock>& t1, const std::chrono::time_point<std::chrono::steady_clock>& t2) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
}

void AverageIterationTimer::SetTimepoint(const std::string& timepoint_name) {
	auto t = get_time_point();

	// end previous chunk if it exists
	EndTimepoint(t);

	// start the next chunk
	current_timechunks_.emplace_back(timepoint_name, t);
}

// TODO: check to see if the timepoint has already been 'ended', throw error if it has
void AverageIterationTimer::EndTimepoint(const std::chrono::time_point<std::chrono::steady_clock>& t) {
	if (!current_timechunks_.empty()) {
		current_timechunks_.back().end = t;
	}
}

void AverageIterationTimer::EndTimepoint() {
	auto t = get_time_point();
	EndTimepoint(t);
}

void AverageIterationTimer::IterationFinished() {
	assert(!current_timechunks_.empty());

	for (const TimeChunk& tc : current_timechunks_) {
		if (duration_totals_.count(tc.name)) {  // already exists
			auto& tup = duration_totals_[tc.name];
            std::get<0>(tup) += tc.end - tc.begin;
			std::get<1>(tup) += 1;
		}
		else {
			insertion_order_.push_back(tc.name);
			duration_totals_[tc.name] = std::make_tuple(tc.end - tc.begin, 1);
		}
	}

	current_timechunks_.clear();
	iter_count_++;
}

void AverageIterationTimer::ScrapIteration() {
	current_timechunks_.clear();
}

void AverageIterationTimer::ScrapTimepoint()
{
	if (current_timechunks_.empty()) {
		throw std::runtime_error("There is no recent timepoint to throw away.");
	}

	current_timechunks_.pop_back();
}

// TODO: print out in table format, names in left column, times in right column, should be easier to read
void AverageIterationTimer::PrintAverageDurations() const
{
	std::cout << "======== AverageIterationTimer Results ========" << std::endl;

    for (const std::string& key : insertion_order_) {
		auto& tup = duration_totals_[key];
		auto& duration = std::get<0>(tup);
		const int count = std::get<1>(tup);

		const double elapsed = static_cast<double>(duration.count()) / static_cast<double>(count) / 1e3;  // convert micro to milli
		std::cout << key.c_str() << ": " <<
			elapsed << " ms avg" << std::endl;
    }
}
