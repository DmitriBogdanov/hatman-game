#pragma once

#include <unordered_set> // related type (logic gate emit inputs)

#include "script_base.h" // 'Script' base class
#include "geometry_utils.h" // geometry types



namespace scripts {



	// # LevelChange #
	// - Spawns player at another level upon entering hitbox
	class LevelChange : public Script {
	public:
		LevelChange(const dRect &hitbox, const std::string &goesToLevel, const Vector2 &goesToPos);

		bool checkTrigger() const override;
		void trigger() override;

	private:
		dRect hitbox;

		std::string goes_to_level;
		Vector2 goes_to_pos;
	};



	// # LevelSwitch #
	// - Spawns player at another level upon interaction inside hitbox
	class LevelSwitch : public Script {
	public:
		LevelSwitch(const dRect &hitbox, const std::string &goesToLevel, const Vector2 &goesToPos);

		bool checkTrigger() const override;
		void trigger() override;

	private:
		dRect hitbox;

		std::string goes_to_level;
		Vector2 goes_to_pos;
	};



	// # PlayerInArea #
	// - Constantly emits while player collides with hitbox
	class PlayerInArea : public Script {
	public:
		PlayerInArea(const dRect &hitbox);

		bool checkTrigger() const override;
		// trigger() does nothing, emit output is present by default

	private:
		dRect hitbox;
	};



	// # AND #
	class AND : public Script {
	public:
		AND(const std::unordered_set<std::string> &emitInputs);
		AND(std::unordered_set<std::string> &&emitInputs); // optimization

		bool checkTrigger() const override;
		// trigger() does nothing, emit output is present by default

	private:
		std::unordered_set<std::string> emit_inputs;
	};



	// # OR #
	class OR : public Script {
	public:
		OR(const std::unordered_set<std::string> &emitInputs);
		OR(std::unordered_set<std::string> &&emitInputs); // optimization

		bool checkTrigger() const override;
		// trigger() does nothing, emit output is present by default

	private:
		std::unordered_set<std::string> emit_inputs;
	};



	// # XOR #
	class XOR : public Script {
	public:
		XOR(const std::unordered_set<std::string> &emitInputs);
		XOR(std::unordered_set<std::string> &&emitInputs); // optimization

		bool checkTrigger() const override;
		// trigger() does nothing, emit output is present by default

	private:
		std::unordered_set<std::string> emit_inputs;
	};



	// # NAND #
	class NAND : public Script {
	public:
		NAND(const std::unordered_set<std::string> &emitInputs);
		NAND(std::unordered_set<std::string> &&emitInputs); // optimization

		bool checkTrigger() const override;
		// trigger() does nothing, emit output is present by default

	private:
		std::unordered_set<std::string> emit_inputs;
	};



	// # NOR #
	class NOR : public Script {
	public:
		NOR(const std::unordered_set<std::string> &emitInputs);
		NOR(std::unordered_set<std::string> &&emitInputs); // optimization

		bool checkTrigger() const override;
		// trigger() does nothing, emit output is present by default

	private:
		std::unordered_set<std::string> emit_inputs;
	};



	// # XNOR #
	class XNOR : public Script {
	public:
		XNOR(const std::unordered_set<std::string> &emitInputs);
		XNOR(std::unordered_set<std::string> &&emitInputs); // optimization

		bool checkTrigger() const override;
		// trigger() does nothing, emit output is present by default

	private:
		std::unordered_set<std::string> emit_inputs;
	};



	/// IMPLEMENT!!!
	// # Delay #
	class Delay : public Script {};
}