#pragma once

#include "timer.h" // 'Milliseconds' type
#include "geometry_utils.h" // geometry types
#include "gui.h" // 'Text' creation
#include "collection.hpp" // 'Collection<Text>::handle' type



// # Script #
// - ABSTRACT
// - Base class for all unique scripts
// - Script is an invisible physics-less logic element, that is a part of the level
class Script {
public:
	virtual ~Script() = default;

	virtual void update(Milliseconds elapsedTime) = 0;
};



namespace scripts {



	// # LevelChange #
	// - Spawns player at another level upon entering hitbox
	class LevelChange : public Script {
	public:
		LevelChange(const dRect &hitbox, const std::string &goesToLevel, const Vector2 &goesToPos);

		void update(Milliseconds elapsedTime) override;

	private:
		dRect hitbox;

		std::string goes_to_level;
		Vector2 goes_to_pos;
	};



	// # LevelSwitch #
	// - Spawns player at another level upon <<interaction>> inside hitbox
	class LevelSwitch : public Script {
	public:
		LevelSwitch(const dRect &hitbox, const std::string &goesToLevel, const Vector2 &goesToPos);

		void update(Milliseconds elapsedTime) override;

	private:
		dRect hitbox;

		std::string goes_to_level;
		Vector2 goes_to_pos;
	};



	// # Portal #
	// - Teleports player to given coords upon interaction inside hitbox
	class Portal : public Script {
	public:
		Portal(const dRect &hitbox, const Vector2 &goesToPos);

		void update(Milliseconds elapsedTime) override;

	private:
		dRect hitbox;

		Vector2 goes_to_pos;

		bool activated;
		Timer fade_timer;
	};



	// # Hint #
	// - Displays a hint when player is inside the hitbox
	class Hint : public Script {
	public:
		Hint(const dRect &hitbox, const dRect &textField, const std::string &text);

		~Hint(); // don't forget to erase text pop-up if level unloads

		void update(Milliseconds elapsedTime) override;

	private:
		dRect hitbox;

		bool is_active;

		dRect text_field;
		std::string text;
		
		Collection<Text>::handle popup_handle; // handle to popup created upon tile activation
	};

	// # Checkpoint #
	// - Saves when player is inside the hitbox
	// - Only triggers once
	class Checkpoint : public Script {
	public:
		Checkpoint(const dRect &hitbox, const std::string &emits_flag);

		void update(Milliseconds elapsedTime) override;

	private:
		dRect hitbox;

		bool was_triggered;

		std::string emits_flag;
	};
}