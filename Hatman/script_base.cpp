#include "script_base.h"

#include "emit.h" /// REWORK



// # Script #
void Script::update(Milliseconds elapsedTime) {
	if (this->checkTrigger()) {
		if (this->emit_output != "") { // output emit is present => emit it
			EmitStorage::ACCESS->emit_add(this->emit_output, this->emit_output_lifetime);
		}

		this->trigger();	
	}
}

bool Script::setOutput(const std::string emitOutput, int emitLifetime) {
	if (emitOutput != "") {
		this->emit_output = emitOutput;
		this->emit_output_lifetime = emitLifetime;
		return true;
	}
	else {
		return false;
	}
	
}

bool Script::checkTrigger() const { return false; }
void Script::trigger() {}