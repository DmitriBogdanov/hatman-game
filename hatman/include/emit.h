#pragma once

#include <string> // related type
#include <unordered_map> // related type

#include "timer.h" // 'Milliseconds' type



// # _emit_properties #
// - NOT INTENDED FOR EXTERNAL USE!
// - Stores properties of an emit, used to hanle it's lifetime in a storage
struct _emit_properties {
	_emit_properties(int lifetime = 0);

	Milliseconds emit_duration;
		// (emit_duration < 0) => emit never expires
		// (emit_duration == 0) => emit is instant
		// (emit_duration > 0) => emit has finite lifetime
	Milliseconds emit_time_elapsed;
};



// # EmitStorage #
// - Can be accessed wherever #include'ed through static 'READ' and 'ACCESS' fields
// - Emits with negative lifetime never expire
// - Emits with 0 lifetime live exactly 1 frame
// - Umits with positive lifetime expire after a given time in ms
class EmitStorage {
public:
	EmitStorage();

	static const EmitStorage* READ; // used for aka 'global' access
	static EmitStorage* ACCESS;

	void update(Milliseconds elapsedTime); // updates the storage, removes single-frame emits

	bool changed() const; // returns if emit storage content was changed during last update()

	void emit_add(const std::string &emit, int lifetime = 0); // adds an emit
	void emit_remove(const std::string &emit); // removes an emit
	bool emit_present(const std::string &emit) const; // checks if given emit is present

	void clear();

	std::unordered_map<std::string, _emit_properties> emits; // contains current emits and their lifetime (and if lifetime is even limited)
private:
	std::unordered_map<std::string, _emit_properties> emit_queue;

	 // upon any change next frame is marked as 'changed'
	bool changed_held; // holds 'changed' to be applied to the next frame
	bool changed_released; // 'changed' applied to current frame
};