#pragma once

#include <string> // related type

#include "timer.h" // 'Milliseconds' type



// # Script #
// - ABSTRACT
// - Base class for all unique scripts
// - Script is an invisible physicless logic element, that is a part of the level
class Script {
public:
	virtual ~Script() = default;

	void update(Milliseconds elapsedTime);

	bool setOutput(const std::string emitOutput, int emitLifetime); // returns if set result is non-trivial

	virtual bool checkTrigger() const;
	virtual void trigger();

protected:
	std::string emit_output = ""; // emit that happens of trigger. If == "", no emit is outputed
	int emit_output_lifetime = 0; // emit is instant by default
};