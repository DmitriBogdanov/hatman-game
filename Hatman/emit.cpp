#include "emit.h"



// # _emit_properties #
_emit_properties::_emit_properties(int lifetime) :
	emit_duration(lifetime),
	emit_time_elapsed(0)
{}



// # EmitStorage #
const EmitStorage* EmitStorage::READ;
EmitStorage* EmitStorage::ACCESS;

EmitStorage::EmitStorage() :
	changed_held(false)
{
	this->READ = this;
	this->ACCESS = this;
}

void EmitStorage::update(Milliseconds elapsedTime) {
	if (this->changed_held) {
		this->changed_held = false;
		this->changed_released = true;
	}
	else {
		this->changed_released = false;
	}

	for (auto iter = this->emits.begin(); iter != this->emits.end();) {
		bool must_erase = false;

		// if emit has a lifetime, handle it
		if (iter->second.emit_duration > 0) { 
			iter->second.emit_time_elapsed += elapsedTime;

			if (iter->second.emit_time_elapsed > iter->second.emit_duration) {
				must_erase = true;
			}
		}
		// if emit is instant, erase it
		else if (iter->second.emit_duration == 0.) { // instant emits have no lifetime
			must_erase = true;
		}
		// if emit is infinite, do nothing

		// Erase or advance iterator
		if (must_erase) {
			iter = this->emits.erase(iter);
			this->changed_held = true;
		}
		else {
			++iter;
		}
	}

	this->emits.merge(this->emit_queue); // push queue into storage (must happen at the end)
}

bool EmitStorage::changed() const {
	return this->changed_released;
}

void EmitStorage::emit_add(const std::string &emit, int lifetime) {
	this->emit_queue[emit] = _emit_properties(lifetime);

	this->changed_held = true;
}
bool EmitStorage::emit_present(const std::string &emit) const {
	return this->emits.count(emit);
}
void EmitStorage::emit_remove(const std::string &emit) {
	this->emits.erase(emit); // erasing non-existant key is completely legal, no check needed

	this->changed_held = true;
}

void EmitStorage::clear() {
	this->emits.clear();

	this->changed_held = true;
}