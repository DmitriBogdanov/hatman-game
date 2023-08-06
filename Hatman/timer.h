#pragma once

#include <unordered_set> // related type ('TimerController' storage)
#include <limits> // infinity



using Milliseconds = double;

constexpr auto MAX_POSSIBLE_TIME = std::numeric_limits<double>::max();

constexpr inline double sec_to_ms(double seconds) { return seconds * 1000.; }

constexpr inline double ms_to_sec(double value) { return value * 0.001; } // returns (value / <Milliseconds in a seccond>)



// # Timer #
// - Can be set to a given duration
class Timer {
public:
	Timer();

	~Timer();

	void update(Milliseconds elapsedTime);

	void start(Milliseconds duration);
	void stop(); // finished the timer instantly
	
	// Getters
	bool finished() const;
	Milliseconds elapsed() const;
	Milliseconds duration() const;
	bool was_set() const; // true if timer was never started, equivalent to checking for negative duration

	double elapsedPercentage() const; // == .elapsed() / .duration()

private:
	Milliseconds timer_duration;
	Milliseconds time_elapsed;
	bool is_finished;
};



// # TimerController #
// - Can be accessed wherever #include'ed through static 'READ' and 'ACCESS' fields
// - Only one instance at a time should exits (creation of new instances however is not controlled in any way)
// - Handles updating of all existing timers
class TimerController {
public:
	TimerController();

	static const TimerController* READ;
	static TimerController* ACCESS;

	void update(Milliseconds elapsedTime);

private:
	friend class ::Timer;

	std::unordered_set<Timer*> timers;
};