#include "systems/timer.h"



// # Timer #
Timer::Timer() :
	timer_duration(-1), // not 0 to avoid nasty result during 'elapsed/duration' division for uninitialized timers
	time_elapsed(0),
	is_finished(true)
{
	TimerController::ACCESS->timers.insert(this);
}

Timer::~Timer() {
	TimerController::ACCESS->timers.erase(this);
}

void Timer::update(Milliseconds elapsedTime) {
	if (!this->is_finished) {
		if ((this->time_elapsed += elapsedTime) > this->timer_duration) {
			this->is_finished = true;
		}
	}
}

void Timer::start(Milliseconds duration) {
	this->timer_duration = duration;
	this->time_elapsed = 0.;
	this->is_finished = false;
}

void Timer::stop() {
	this->time_elapsed = this->timer_duration;
	this->is_finished = true;
}

bool Timer::finished() const {
	return this->is_finished;
}

Milliseconds Timer::elapsed() const {
	return this->time_elapsed;
}
Milliseconds Timer::duration() const {
	return this->timer_duration;
}
bool Timer::was_set() const {
	return this->timer_duration > 0;
}
double Timer::elapsedPercentage() const {
	return this->timer_duration ? this->time_elapsed / this->timer_duration : 1.;
}



// # TimerController #
const TimerController* TimerController::READ;
TimerController* TimerController::ACCESS;

TimerController::TimerController() {
	this->READ = this;
	this->ACCESS = this;
}

void TimerController::update(Milliseconds elapsedTime) {
	for (auto &timer : this->timers) {
		timer->update(elapsedTime);
	}
}