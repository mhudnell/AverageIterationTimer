#include "AverageIterationTimer.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
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
    if (!current_timechunks_.empty() && !current_timechunks_.back().end) {  // indicates there is a timechunk we need to end
		EndTimepoint(t);
    }

	// start the next chunk
	current_timechunks_.emplace_back(timepoint_name, t);
}

void AverageIterationTimer::EndTimepoint(const std::chrono::time_point<std::chrono::steady_clock>& t) {
	if (current_timechunks_.empty()) {
		throw std::runtime_error("Calling EndTimepoint() in an invalid location. There is no timechunk to end.");
	}
	if (current_timechunks_.back().end) {
		throw std::runtime_error("Calling EndTimepoint() in an invalid location. The most recent timechunk ("+ current_timechunks_.back().name +") has already been ended.");
	}

	current_timechunks_.back().end = t;
}

void AverageIterationTimer::EndTimepoint() {
	auto t = get_time_point();
	EndTimepoint(t);
}

void AverageIterationTimer::IterationFinished() {
	assert(!current_timechunks_.empty());

	for (const TimeChunk& tc : current_timechunks_) {
        if (!tc.begin || !tc.end) {
			throw std::runtime_error("Timechunk (" + tc.name + ") was never properly ended! Use EndTimepoint() or ScrapTimepoint() / ScrapIteration() to describe your intent.");
        }

		if (duration_totals_.count(tc.name)) {  // already exists
			auto& tup = duration_totals_[tc.name];
            std::get<0>(tup) += *tc.end - *tc.begin;
			std::get<1>(tup) += 1;
		}
		else {
			insertion_order_.push_back(tc.name);
			duration_totals_[tc.name] = std::make_tuple(*tc.end - *tc.begin, 1);
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

void AverageIterationTimer::PrintAverageDurations() const
{
	std::cout << std::string(25, '=') << " AverageIterationTimer Results " << std::string(25, '=') << std::endl;
	std::cout << std::fixed << std::setprecision(3);  // print numbers to 3 decimal places

    // pre-determine spacing parameters
	int max_width = 5;
	int max_num_digits = 1;  // number of digits left of the decimal point
    for (const std::string& key : insertion_order_) {
		if (key.size() > max_width) { max_width = static_cast<int>(key.size()); }

		const auto& tup = duration_totals_.at(key);
		const auto& duration = std::get<0>(tup);
		const int count = std::get<1>(tup);

        double elapsed = static_cast<double>(duration.count()) / static_cast<double>(count) / 1e3;  // convert micro to milli

		int num_digits = 1;
        while (elapsed > 10) {
			elapsed /= 10;
			num_digits++;
        }
		max_num_digits = std::max(max_num_digits, num_digits);
    }
	max_width = std::min(max_width, 60);  // if a key name is longer than 60 chars, it will no longer be aligned, just let it run on

	for (const std::string& key : insertion_order_) {
		const auto& tup = duration_totals_.at(key);
		const auto& duration = std::get<0>(tup);
		const int count = std::get<1>(tup);

		const double elapsed = static_cast<double>(duration.count()) / static_cast<double>(count) / 1e3;  // convert micro to milli

        // print out key name, add padding for alignment
		std::cout << std::left << std::setw(max_width) << std::setfill('-') << key.c_str() << "--> ";

        // print out time in ms, align numbers at the decimal point
		std::cout << std::right << std::setw(max_num_digits + 4) << std::setfill(' ') << elapsed << " ms" << std::endl;
		// +4 for the decimal point, and 3 trailing digits
	}
}
