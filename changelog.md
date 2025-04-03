# 20.01.05 - 20.04.10 #
	- Implemented classes 'Vector2' and 'Vector2d' in 'geometry_utils.h' header
	- Implemented class 'Rectangle' in 'geomentry_utils.h' header
	- Implemented basic 'Graphics' class using SDL2 and SDL2_images
	- Implemented 'consolelog' namespace containing functions for convenient debug printing
	- Implemented basic 'Game' class
	- Implemented class 'Input' for convenient polling and storage of events
	- Implemented 'globalconsts' namespace that stores hardcoded values
	- Implemented class 'Entity', capable of displaying static sprites and playing animations
	- Implemented class 'PhysicalEntity', containing basic physics logic
	- Implemented class 'Player' derived from 'PhysicalEntity'
	- Implemented class 'Level'. Levels are loaded from .TMX files, tilesets are
	embedded inside the map file. Map size is fixed and is identical to screen size
	due to static camera (or rather, a lack of camera)
	- Implemented header 'level_objects.h', containing classes corresponding to various
	objects (aka level exits, for now)

# 20.06.17 - 20.07.04 #
	- Changed map format from .TMX to .JSON, 'tinyxml' was abandoned in favor of
	'nlohmann_json' for parsing new format
	- Redone and optimised level parsing, tilesets are now stored externally instead of being
	embedded in a map file. Tilisets are stored as .JSON, parsing happens inside the tileset class
	itself
	- Implemented 'Tileset' and 'Tile' classes, tile objects are now unique and independent objects,
	rather than being an id referring to a tileset object
	- Improved tile system: tiles now have an optional 'interaction' member, that handles unique
	tile interaction. All interactive tiles are unique classes derived from regular 'Tile',
	storage and logic is handled via the use of polymorphism
	- Implemented static (aka global) access to Graphics object through static pointer. Deduced further
	applications of this feature and implemented such access to a 'Game' object. Objects with such
	access are initialized before the creation of 'Game' object
	- Implemented 'TileStorage' class that stores loaded tilesets inside an object to prevent repetitive
	loading of the same tilesets while changing levels

# 20.07.05 #
	- Implemented movable camera
	- New methods were added to 'Rectangle' class

# 20.07.06 #
	- Changed camera system
	- Implemented camera zoom and tilt
	- Implemented temporary solution for camera trap

# 20.07.07 #
	- Minor changes to 'Vector2' and 'Vector2d' classes
	- Adjusted size of camera trap

# 20.07.08 #
	- Background rendering changed to be independent from level and player position (i.e. static image)
	- Implemented custom map size (previously, size 20x12 was hardcoded)
	- A number of helper classes changed to be nested in main classes
	- 'Tag' functions were implemented ('tag' is any string that contains [prefix] and {suffix})

# 20.07.09 #
	- Small optimizations in 'Level' class
	- Implemented saving (but not loading)

# 20.07.10 #
	- Implemented loading from savefile
	- Implemented creation of a new savefile
	- Implemented debug commands for console startup
	- Removed temporary 'Save' class, all saving is handled by 'Saver' object
	- Fixed bug that caused incorrect physical interactions in low FPS scenarios (i.e. saving/loading)
	- Fixed bug that made level exits possible to glitch-out

# 20.07.11 #
	- Implemented ability to work with multiple savefiles
	- Implemented automatic creation of a new savefiles
	- Convenience changes to debug commands
	- Implemented input detection on interactive tiles, save orb now activates through USE key
	- Started implementing GUI
	- Implemented multiline text display with proper word wrap
	- 'WindowSettings' class replaced by a more general and streamlined 'LaunchInfo' class
	- Fixed a number of incorrect behaviours related to blending of transparent textures, order of rendering,
	texture targeting and texture clearing

# 20.07.16 #
	- Implemented constructor for 'Entity' that uses std::initializer_list for more convenient creation
	- Small changes to 'PhysicalEntity' flags
	- Implemented logic that allows GUI to draw elements as objects on a map (rather than overlay)
	- Improved tile interaction logic - added two-step trigger check: successfull first check puts tile into an 'activated'
	state, successfull second check triggers tile interaction

# 20.07.17 #
	- Minor functionality added to 'Vector2' class
	- Improved GUI text logic
	- GUI now supports numbers and symbols {,.!?-:}
	- Fixed bug that prevented 'Gui::make_text()' from displaying last word of the text

# 20.07.18 #
	- Implemented centered display of single-line text through new method 'make_line_centered()'

# 20.07.20 #
	- Started working on more advanced Entity system: from now on entities will be created and
	handled through 3 headers: 'entity_base.h', 'entity_type.h', 'entity_unique.h' that will
	contain generic methods. Class 'Entity' is drasticaly refactored, it now contains following
	classes: 'Entity::Visuals', 'Entity::Physics' - objects of these classes contains various
	generic methods and variables related to entity logic, 'Entity' objects store pointers
	to these helper clases - given part of logic is not present, 'Entity' holds nullptr
	- Marked all non-usable default constructors as '= delete' - apparently that's a thing

# 20.07.21 #
	- Finished implementing 'entity_base.h'
	- Implemented FPS counter and related GUI methods

# 20.07.22 #
	- Added enitity enabling/disabling functionality to 'entity_base.h'
	- Added functionality that allows removing of entities from the game
	- Started working on items

# 20.07.26 #
	- Renamed a few headers
	- Refactored default constructors/destructors as '= default' for improved clarity
	- Implemented basic item logic
	- Added 2 new function to 'tags' namespace: 'containsPrefix()' and 'containsSuffix()'
	- Implemented 'Player::Controls' class, moved all controls here
	- Set up logic that allows creation and storage of unique entities in level object
	- Implemented parsing of items from level .json
	- Drew and implemented first unique item - Brass Relic
	- Improved drawing of level backround, tiles and entities
	- Fixed a bug that prevented entities from being deleted properly

# 20.07.28 #
	- Refactored 'content' folder, organized textures and put them in a dedicated folder
	- Implemented 'Item' base class
	- Implemented 'Stack' class
	- Implemented 'Inventory' class
	- Implemented logic that allows convenient creation of unique items
	- Implemented first unique item: 'brass_relic'
	- Implemented logic that allows items to be picked up and added to inventory
	inside 'Entity_Item' class

# 20.07.28 #
	- Refactored some code that dealt with creation of unique objects
	- Standardized creation of tiles, entities and items
	- Implemented 3 namespaces: 'tiles', 'entities', 'items' and
	3 functions 'make_tile()', 'make_entity()', 'make_item()' for
	convenient standardized creation of such objects
	- Improved convenience of adding new unique objects and optimised functions
	'make_tile()', 'make_entity()', 'make_item()' by accessing const unordered_map
	of function pointers insteam of if-else chain
	- Reformated 'manual.txt'
	- Updated old parts of 'manual.txt' and added all new functionality

# 20.08.01 #
	- Drew inventory texture placeholder
	- Implementing basic inventory GUI logic
	- Succesfully displayed picked-up items

# 20.08.02 #
	- Added visibility toggle to inventory GUI
	- Fixed visual bug related to displaying of multiple items inside inventory GUI
	- Prepared a number of inventory GUI concept arts
	- Introduced a number of new functions to draw tabs and their headers
	- Switched to a new, more informative items tab
	- Started implementing scripts system. Created header 'script_base.h' and implemented
	'Script' abstract base class

# 20.08.03 #
	- Changed layer naming standard to have a tag format '[<object base type>]{<object sub type>}'
	- Separated layer parsing into a few functions to reduce clutter, made small adjustments to parsing
	- Added some virtual destructors that were lacking
	- Implemented most of 'LevelChange' class

# 20.08.04 #
	- Got 'LevelChange' working (object was destroying 'level' upon trigger, destroying itself)
	- Small level parsing optimizations
	- Implemented 'LevelSwitch' class

# 20.08.05 #
	- Implemented timescale: game can be sped up or slowed with no apparent limitations
	(extreme cases can be wonky)
	- Changed inventory tab 'items' GUI to its final version
	- Added ability to parse emit outputs to scripts
	- Optimized 'EmitStorage' and fixed some incorrect behaviours
	- Implemented 'AND' script and its parsing
	- Changed object naming and parsing to make use of tags for convenient parsing of arrays and
	to allow comments
	- Implemented 'PlayerInArea' script and its parsing
	- Implemented 'OR' script and its parsing
	- Updated DEV method '_drawHitboxes()'
	- Implemented DEV method '_drawEmits()'

# 20.08.06 #
	- Fixed deleting of dead entities
	- Implemented 'XOR' script and its parsing
	- Implemented 'NAND' script and its parsing
	- Implemented 'NOR' script and its parsing
	- Implemented 'XNOR' script and its parsing
	- Fixed bug that made game crash upon instant emits
	- Moved GUI and camera into separate headers to reduce clutter
	- Implemented 'RGBColor' class
	- Started implementing a more powerfull text GUI system with 'Font' and 'Text' classes
	- Implemented 'Font' class

# 20.08.07 #
	- Implemented centered text display
	- Implemented 'Font' class
	- Implemented font 'BLOCKY'
	- Implemented 'Text' class. Texts are now  independent objects that do not expire unless erased.
	- Implemented font and text storage in 'Gui'
	- Implemented text color change
	- Implemented smooth text display (with fixed delays between each new symbol appearing)
	- Implemented emit queue to ensure make input lifetime independent from the moment when emit was added
	during a frame. Instant emits are now ensured to have excatly 1 full frame of lifetime.
	- Switched to C++17 standard

# 20.08.08 #
	- Implemented templated container type 'Collection<>'. 'Collection' is a polymorphic container
	with a sole ownership over its content. 'Collection' allows insertion of elements, which provides
	caller with a handle to inserted element that can be used to access or erase the element.
	'Collection' allows convenient iteration without exposing implementation through the use
	of custom iterators
	- Switched to the use of 'Collection<Object>' insted of 'std::vector<std::unique_ptr<Object>>'
	wherever necessary 
	- Added iterator 'erase' to 'Collection<>' to allow cleanup of 'dead' objects through iteration
	- Implemented new activation logic to 'Tile' base class, now tiles only call 'activate()' once
	upon activation and 'deactivate()' (once upon deactivation)
	
# 20.08.09 #
	- Changed 'draw_text()', 'draw_line()', 'draw_line_centered()' GUI methods to mmethods
	'make_text()', 'make_line()', 'make_line_centered()' that handle creation of 'Text' objects
	and insertion of these objects into collection
	- Changed 'Tile_SaveOrb', and 'FPSCounter' to work with new text system
	- Added some convenience functions to 'Font' and 'Text'
	- Fixed a bug that made centered text align incorrectly
	- Changed debugging function '_drawEmits()' bypass text system and draw emits directly through font
	- Added 'draw_line()' method to 'Font' to make creation of simple non-'Text object' overlays
	easier and generally flashed out text system
	- Adapted 'InventoryGUI' drawing to a new text system, fixed incorrect positions and added
	drawing of page number (pages themselves are not yet implemented)

# 20.08.10 #
	- Implemented screen fade effect into GUI
	- Implemented smooth screen fade effect into GUI

# 20.08.11 #
	- Implemented background parsing, backgrounds are now specified in map properties
	- Implemented more stable level-change system
	- Optimized fade logic

# 20.08.13 #
	- Set up filters, omitted 'level_objects.h' and 'consolelog.h'
	- Implemented 'Damage' class
	- Implemented 'Healthbar' module
	- Changed casts to C++ style, got rid of all the warnings
	- Separated 'Inventory' class into 'modules' category
	- Moved player from 'Game' object to 'Level' object to standardize logic
	- Refactored all headers - cleared up most outdated comments, wrote new comments,
	standardized style, changed some class/header names, moved some nested classes outside
	for better readability, removed some unnecessary includes and etc

# 20.08.14 #
	- Added 'Healthbar' module to the entity base class
	- Added healthbar to the player
	- Implemented 'Fraction' module
	- Implemented 'damageInArea()' method, that handles fraction-checking and damage application to entities
	- Implemented 'Destructible' entity type

# 20.08.15 #
	- Implemented '_init' methods to entity types to make creation of unique entities more simple and robust
	- Made 'Entity::Visuals' and 'Entity::Physics' more modular by making them attachable to a position
	instead of whole entity
	- Implemented 'make_init_list()' function for creation of 'std::initializer_list's with a deducable
	type (to allow usage of 'std::initizlizer_list' parameters in template functions like 'std::make_unique()')
	- Separated 'Entity::Visuals::Animation' into a separate module 'Animation' in 'sprite.h' header
	- Implemented 'Sprite' base class
	- Separated 'Entity::Visuals' into a separate module 'ControllableSprite'
	- Implemented 'AnimatedSprite' module
	- Implemented 'StaticSprite' module
	- Changed 'Entity' to work with new sprite system
	- Changed 'Tile' to work with new sprite system
	- Optimized algorithms for drawing and updating of sprites
	- Separated 'TileHitbox', 'TileAnimation', 'TileInteraction'
	- Separated 'Entity::Physics' into a separate module 'SolidRectangle'
	- Separated 'Entity::Physics::Flags' into 'SolidFlags'

# 20.08.16 #
	- Optimized entity drawing by allowing entities to use simplier types of sprites
	- Implemented entity type 'Destructible' that triggers some effect and plays an animation
	upon being killed
	- Implemented destructible entity 'TNT'
	- Simplified entity parsing - now new entity types don't require writing of new parsing methods

# 20.08.17 #
	- Implemented 'Skill' base class
	- Minor additions to geometry utils
	- Implemented 'Slam' ability and attached it to the player
	- Minor addition to 'ControllableSprite' to allow replaying of current animation
	- Changed 'Skill' class to be properly move-constructible, that allows 'Skill' objects
	to be allocated without the use of std::unique_ptr wrapper

# 20.08.18 #
	- Merged 'Fraction' into 'Healthbar' module
	- Added 'Vector2' and 'Vector2d' methods to calculate length both precisely and roughly (but faster)
	- Implemented mechanic that 'freezes' logic for objects too far from the player, improved performance
	in big maps
	
# 20.08.19 #
	- Improved 'freeze' mechanic to freeze tiles and entities at different distances to prevent
	enitities clipping through the terrain
	- Implemented 'Enemy' entity type
	- Implemented first enemy with proper ai - ghost (doesn't use any abilities yet)

# 20.08.21 #
	- Minor improvements to 'RGBColor'
	- Fixed a bug that made fade alpha calculate incorrectly in certain cases
	- Implemented 'Timer' class
	- Implemented 'TimerController' class, that updates timers globaly, allowing
	for a simple and convenient use of timers
	- Implemented level change delay and smooth fade effect to make transitions cool and dandy
	- Revisited includes throughout the whole project, added comments and removed redundant
	includes
	- Added 'Ex' methods to 'Graphics', 'Gui' and 'Camera' classes to allow drawing of rotated and flipped
	textures
	- Added skill 'Slash' and made an existing enemy (Ghost) use it
	- Implemented enum 'Orientation' representing LEFT/RIGHT orientations for enemies/NPCs/skills/etc
	- Optimized skill drawing, removed some old clutches
	- Added method to query animation duration from 'ControllableSprite'
	- Improved 'Enemy' class to automatically flip sprite to the direction entity is facing,
	it allows more compact spritesheets and convenient animation setups since there is no need
	to create 2 mirrored versions for every animation

# 20.08.24 #
	- Brainstormed a core game concept (aka world, story, progression and etc), created a rough scheme
	- Introduced 'entity_primitive_types' namespace, as just only regular types were just not cutting it
	- Implemented 'Creature' primitive entity type
	- Bounded 'Skill' with 'Creature' instead of 'Entity' to give skills more control over parent object
	- Changed 'Enemy' and 'Player' to inherit from 'Creature' instead of 'Entity'
	- Adapted 'Player' to work as a 'Creature', as a side-effect player in now killable

# 20.08.26 #
	- Added an option for read-only access to all monostates
	- Implemented 'Controls' class for proper storage and possible modification of control keys
	- Reworked player class by making it an entity type, with derived unique classes that represent various
	forms
	- Introduced typedef 'Milliseconds = double' to represent time, changed time recording from 'int' ti 'Millisecons'
	which allowed better precision of calculations and a simple, robust timescale mechanic

# 20.08.27 #
	- Fixed a bug that broke FPS counter when timescale was set to 0
	- Implemented form change GUI and animations
	- Changed 'Health' class to keep hp the same percentage while changing max hp
	- Completely reworked player form system, as previous system had some unobvious but major
	downsides that rendered it unusable
	- Replaced all headerguards with '#pragma once' for faster compilation

# 20.08.28 #
	- Implemented player healthbar
	- Implemented form-change cooldown
	- Implemented form-change-cooldown GUI

# 20.09.02 #
	- Updated manual, include category about scripts, introduced stricter format
	- Implemented portrait GUI
	- Fixed player hitbox to fit into 1-tile gaps

# 21.01.04 #
	- Added 'inventory' to controls, replaced temporary code for showing inventory GUI with a proper one

# 21.01.30 #
	- Worked on assets

# 21.01.31 #
	- Implemented new enemy - 'Sludge'
	- Minor additions to time system. Changed all speeds and accelerations to the units/sec format
	- Implemented friction
	- Changed speed, acceleration handling to allow more complicated physics
	- Changed 'isGrounded' flag checking to a more robust logic. Fixed some related bugs
	- Reworked all physics to follow Newtonian laws, implemented mass, forces, impulses all solids
	now use forces and impulses in place of direct speed manipulation
	- Implemented 'KnockbackAOE' skill

# 21.02.01 #
	- Implemented more complex and robust enemy behaviour, separated aggro/deaggro conditions
	and chase/attack conditions
	- Minor additions to 'geometry_utils.h'
	- Implemented wandering mechanic for sludge
	- Fixed a bug when objects with high friction would slightly move due to speed not converging at 0

# 21.02.02 #
	- Started implementing particle system

# 21.02.03 #
	- Fixed a bug where disabled entities would still execute unnecessary logic
	- Implemented 'Particle' entity type
	- Implemented algorithm for rounding values up to 32 (tile size)
	- Implemented hashing function for 'Vector2'
	- Revamped tile storage to use unordered_map (tile positions as keys) instead of collection,
	such design culls unnecessary calculations as it allows objects to iterate over only the adjastent
	tiles without checking distance to all tiles on the map. Drastic perfomance improvement for big
	levels and situations with a lot of entities in the same spot. Further optimizations down the line.
	- Minor fixes

# 21.02.04 #
	- Optimized tile updating and drawing to use new map system
	- Optimized range check for entity freezing mechanic

# 21.02.05 #
	- Added new tileset 'alcwilliam_dungeon'
	- Fixed a bug that prevented some tiles from loading due to incorrect order of tilesets in the container
	- Minor optimizations of tile parsing
	- Separated tile layers into 2 categories: layers and backlayers, backlayer tiles are rendered without
	executing any of their logic behind regular tiles, which allows combination of regular tiles and
	'background' tiles for better visuals

# 21.07.05 #
	- Made sensible namespaces for player form consts, moved mass and friction const to form consts
	- Removed elapsed time from runLeft(), runRight() parameters as with new force system it became redundant
	- Implemented crude from change
	- Fixed a bug where enemies would not chase player if he's standing to the left of them

# 21.07.06 #
	- Fixed 'magic numbers' in camera trap handling
	- Implemented sensible form-change system with sensible input handling

# 21.07.07 #
	- Changed rendering system from rendering in a native 640x360 resolution to rendering in full
	window/fullscreen resolution, properly scaling all textures from native in textureToCamera()
	and textureToGui() (aka made game render 'fake pixel art')

# 21.07.08 #
	- Fixed a bug that made sprite position stick the '640x360 grid' producing vibrating effect during movement
	- Dealt with a longstanding issue of integer 'Rectangle' that was used as a cookie-cutter representation for
	all rectangles, despite the fact that when dealing with physics and texture-scaling they should be float,
	and when dealing with textures regular SDL_Rect should be used to avoid unnecessary conversions
	- Introduced 'srcRect' and 'dstRect' akin to SDL_Rect to separate rendering logic from physical logic,
	that also lead to a small optimization
	- Changed 'Rectangle' to use double precision instead of integer, went through a ton of code fixing precision
	to the correct one, eliminated a lot of useless rounding
	- Minor optimizations of 'Rectangle' methods
	- Deprecated 'Vector2.length2_rough()' since it's ineffective
	- Added 'movespeed' functionality to solids, this allows more sensible movement for creatures since they
	don't have to force themselves to move against the friction. Also fixed an issue where creatures would
	accelerate faster mid-air

# 21.07.09 #
	- Improved event polling system allowing it to poll multiple inputs during a single frame
	- Added handling of mouse button events
	- Added handling of mouse position

# 21.07.10 #
	- Worked on assets
	- Organized google drive
	- Uploaded project to github
	- Set up object types in 'Tiled'

# 21.07.11 #
	- After a long and painfull process of redoing collision system over and over again implemented robust
	system that resolved a plethora of issues: getting stuck on random edges between tiles, vibrating
	slightly when running into the wall, small teleportation when hitting a corner of a tile midair
	- A lot of new methods in 'dRect' to optimize some things and generally make the class simpler
	to use, a little polish of 'geometry_utils' in general
	- Figured out how to make fullscreen work in 'Release' mode
	- Added 'SolidRectangle' convenient methods for creature movement

# 21.07.12 #
	- Implemented toggling hitboxes and other debug info on F3
	- Added various display color for tile hitboxes, tile actionboxes, entity hitboxes
	- Improved friction mechanic to instantly stop body unaffected by external forces when its
	horizontal speed changes direction due to friction deceleration, this mechanic removes a short
	period when horizontal speed oscillates around 0 before full stop

# 21.07.13 #
	- Worked on assets
	- Implemented barebones player-hand-following-cursor

# 21.07.14 #
	- Fixed 'Camera' class to return unscaled FOV correctly
	- Implemented convenient conversions between level- and screen- positions
	- Finished up player-hand-following-cursor, players hand now follows cursor and determines player
	orientation, moving left-right no longer changes orientation

# 21.07.15 #
	- Added 'LMB', 'RMB', and 'Shift' to 'Controls', these inputs are now tracked by the player form
	- Implemented 'Projectile' entity type
	- Drew and implemented 'ArcaneProjectileBlue' used as a main attack by the player
	- Added ability to set sprite rotation in both radians and degrees, internal measurement is still degrees
	due to SDL limitations
	- Cleaned up 'PlayerForm_Human' and optimized hand rotation
	- Deprecated 'up'/'down' controls
	- Improved entity erasion system to allow erasing after a delay, which simplifies implementing upon-destruction effects
	- Fixed visual bug where projectiles would appear flipped vertically when fired to the left
	- Simplified 'Faction' mechanic, removed unnecessary faction 'NONE'
	- Separated 'makable' enities that can be parsed from map files and 'spawnable'  entities that can be only
	spawned during the game into different file, refactored entity-related namespaces to be less cumbersome
	- Since now there's a bazillion entity-related files, separated them all into a new filter

# 21.07.16 #
	- Fixed a bug where projectile would deal damage multiple time
	- Improved attack condition for sludge, fixed a bug where enemy was unable to attack if player hitbox is too big
	- Standardized names for namespaces with consts
	- Made most of 'Vector2', 'Vector2d' and related functions constexpr-friendly

# 21.07.17 #
	- Implemented explosion animation for 'ArcaneProjectileBlue'
	- Fixed projectiles to autodetect explosion animation duration and dissapear right as it ends
	- Implemented enemy healthbars

# 21.07.19 #
	- Fixed a bug where holding a key would cause repeated 'KEYDOWN' events after a while
	- Started working on a massive overhaul of entity parsing/updating/drawing

# 21.07.20 #
	- Overhauled entity storage to give generic interface for iterating over entities, optimized iteration
	by pre-sorting all entities by their types which removes unnecessary checks for entities that can't be
	interacted with in that specific case
	- Overhauled namespaces so entity classes are stored in a more logical and centralized fashion
	- Optimized spawning and erasing of entities

# 21.07.21 #
	- Added knockback to projectiles
	- Made a 'house' tileset and a bunch of decorations for it

# 21.07.22 #
	- Fixed 'Sludge' to apply damage/knockback directly instead of using skills
	- Fixed particles to accept tifetime parameter correctly
	- 'Sludge' uses 'OnDeathParticles' for their intended purpose
	- Fixed hand prite being flipped vertically when looking left
	- Implemented methods for random ints/double

# 21.07.23 #
	- Implemented .color_mod for sprites
	- Implemented selectable colors for 'OnDeathParticle'
	- Implemented methods for random choises and random linear combinations
	- Implemented more decorative tile layers with following rendering priority:
	[backlayer]->[layer]->[midlayer]->(entitites)->[frontlayer], logic and physics are only handled for [layer]
	- Went from storing tiles in a map to storing them as 1D vector with 2D indexation for better performance,
	gain of ~50-100% FPS depending on screen resolution
	- Finetuned performance-related constants

# 21.07.24 #
	- Deprecated 'Skill' mechanic as it proved to be excessive and overengineered
	- Implemented player respawning, player resurrects on the last save
	- Deprecated level version mechanic as it was an unnecessary complication with no benefits
	- Implemented player respawn mechanic
	- Streamlined level change and loading from the savefile
	- Optimized some really old code in scripts

# 21.07.25 #
	- Added some decorations and banners

# 21.07.26 #
	- Implemented audio system that allows playing sounds and looping music

# 21.07.30 #
	- Implemented barebones of 'SkeletonHalberd' enemy
	- United 'Animation' and 'TileAnimation' under a single class
	- Replace ambiguous std::pair with 'AnimationFrame' struct
	- Optimized the way animations are passed to sprites
	- Implemented interface that allows sprites to use multiple textures, that allows storing animations
	in different files instead of having to always place them on a single spritesheet
	- Implemented consts for filepaths to step away from using hard-to-track literals
	- Implemented methods for parsing animation data from .json files, no more manually typing out every single
	animation in-code

# 21.07.31 #
	- Fixed new bugs, updated all entities to use new sprite features
	- Improved sprite parsing to use "default" filenames if no details are specified
	- Made 'SolidFlags' into bit-flags for better convenience and performance

# 21.08.01 #
	- Compiled project with clang, found and fixed a number of warnings. Also, clang doesn't seem to provide any
	performance boost
	- Finetuned gravity
	- Reimplemeted all creatures as finite state machines, got skeleton halberd enemy working, weird sprite
	'flicker' remains to fix
	- Improved 'ControllableSprite', implemented methods that allow smooth animation transitions
	- Implemented proper aggro logic for enemies, now they will target any creatures of different faction

# 21.08.02 #
	- Traced the cause of sprite 'flicker', fixed the issue
	- Changed containers with observer pointers to entities to unordered_set to allow quick validation
	of entity existance (mostly needed to implement target deselection for creatures)
	- Implemented deaggro if target no longer exists for entities

# 21.08.04 #
	- Drew a dwarf NPC

# 21.08.05 #
	- Made entities update() return bool which tells wheter furter levels of derivation should execute
	update() logic
	- Made it so enemy hp bar disappears at the same time as enemy 'dies' instead of staying up untill
	death animation is finished

# 21.08.06 #
	- Drew mage NPC

# 21.08.07 #
	- Implemented platforms that can be jumped through and jumped down via <S>+<Spade>
	- Fixed a bug where mouse position would not account for camera zoom

# 21.08.08 #
	- Fixed that .animation_finished() returned true on the last frame of the animation even
	if that frame is still going

# 21.08.09 #
	- Redone 'Sludge' and 'SkeletonHalberd' sprites for new monochromatic palette
	- Drew player animation for walking, attacking and standing idle
	- Drew basic tileset for monochronatic palette

# 21.08.10 #
	- Due to changes in style and core idea of the game abolished form change mechanic
	- Implemented player as a state machine similar to other creatures
	- Made F3 menu more readable and customizable, it also displays player state now
	- Replaced player sprite with a new one, that has actual animation for idle/movement and melee attacks
	- Fixed a bug where 1 frame long animations would not trigger 'animation_finish()'

# 21.08.11 #
	- Increased precision of time measurements by using SDL performance counter
	- Drew and implemented fire attunement ult
	- Allowed player to shorten aftercast of attack by moving for more fluid gameplay
	- Fixed a bug that broke jumping down mechanic after all recent changes

# 21.08.12 #
	- Added colors from new sprite pallete to the system
	- Updated sludge on-death particle color
	- Implemented uber-algorithm that for horizontal teleportation that respects terrain
	and doesn't allow phasing through tiles

# 21.08.13 # 
	- Made camera center at player immideately upon level creation, before first update(), nothing changed
	visually but internally it's a more robust logic
	- Implemented smooth camera that moves faster the furter away player is, mostly noticeble during teleportation

# 21.08.14 # 
	- Refactored some old GUI code
	- Implemented buttons that can display text, track being hovered over and pressed and change colors accordingly
	- Implemented logic for generalized 'requests' to game that are handles at a specific time before update(),
	currently such requests are used to change level, open and close menus
	- Implemented Esc menu with working 'Resume' and 'Exit to desktop' buttons
	- Implemented sounds for hovering over buttons and pressing them

# 21.08.15 #
	- Lowered number of sounds played simultaneously, GUI sounds less ridiculous
	- Made 'Damage' constexpr-friendly
	- Cleared relic initializer_lists that were still passed by reference
	- Finetuned performance-related consts
	- Fixed a bug where character would sometimes fall through platforms during alt+tab
	- Made the entirety of 'dRect' constexpr-friendly
	- Made the rest of geometry_utils.h constexpr-frindly for the company
	- Moved 'rand_...' methods out of the namespace to be less lenghty and more like original rand()

# 21.08.17 #
	- Implemented main menu
	- Created artwork for main menu
	- Separated GUI controls into GUI update function
	- Refactored main loop in a way the separates level and object updating and allows using menus without
	having a loaded level and camera
	- Deprecated 'damageInArea()'
	- Updated 'Saver' class to be in a the same style as newer code
	- Implemented conditional dislaying of 'Continue' button in main menu
	- Got 'New game' and 'Exit' buttons in main menu working
	- Made transition from menu into the game smooth

# 21.08.20 #
	- Implemented new enemy 'Devourer' that ambushes player when he looks away
	- Fixed enemies not deaggroing properly
	- Found solution for enemies getting stuck in the last frame of animation when their state changes
	at a certain precise time
	- Added new header with methods for faster testing/debugging

# 21.08.24 #
	- Refactored player healthbar and changed it to fit new style
	- Refactored player portrait and changed it to fit new style
	- Refactored player inventory and changed it to fit new style
	- Added 4 new items to the game: 'Spider Signet', 'Power Shard', 'Eldritch Battery', 'Watching Eye'

# 21.08.26 #
	- Redrew save orb and changed its GUI to fit new style

# 21.08.27 #
	- Made save orb display a message when game is saved

# 21.08.28 #
	- Added new enemy 'SpritBomber' that attacks player with explosive magic from range
	- Added 'SpiritBomb' projectile

# 21.08.29 #
	- Changed parsing logic for tiles and tilesets
	- Allowed placing entities through 'entity tilesets'
	- Creaned up deprectated files, structured names for remaining files
	- Started building first actually playable level for the end product

# 21.08.31 #
	- Chanded color of inventory icons and text to be distinct from the environment

# 22.07.21 #
	- Fixed a bunch of places using 'ACCESS' ptr insead of 'READ'
	- Added inventory saving/loading to 'Saver'
	- Fixed player loading twice during loading form save

# 22.07.22 #
	- Added 'Flags' system to control game state logic and allow conditional loading
	- Implemented conditional loading for entities
	- Implemented flag emits on entity death
	- Added flag support to 'Saver', which allows the game to save its state and make non-respawnable objects
	- Fixed objects entities spawning 1 tile lower than they should due to Tiled usign bottom-left corner
	coords for tile objects instead of standard upper-left corner
	- Fixed 'SaveOrb' not erasing 'Progess saved.' pop-up when player leaves level while staying near the orb

# 22.07.23 - 22.07.24 #
	- Contemplated lore, final goal and player progression. Formulated a general game progression.
	Added comments about purposes of different sections of the map
	- Began working on making scripts more simple and robust

# 22.07.25 #
	- Improved script system
	- Implenmented new script - 'Hint'
	- Set up hints to introduce basic mechanics to the player
	- Finished 1st level - 'Desolation'

# 22.07.26 #
	- Implemented jump boost from 'Spider Signet' and hp regen boost form 'Eldritch Battery'
	- Built 2nd level - 'The Great Bridge'
	- Built 3rd level - 'Descent'
	- Implemented tracking of being grounded at the left/right side in 'Solid' module
	- Improved mob AI to be aware of ledges and not fall off when wandering/chasing the player
	for following creatures: 1) SkeletonHalberd 2) Sludge
	- Implemented new enemy - 'Worm'
	- Stat adjustments across the board
	- Fixed an issue with friction compensation that caused mobs to slide of the cliffs
	from time to time

# 22.07.27 #
	- Implemented new enemy - 'PygmyWarrior'
	- Improved AI for 'SkeletonHalberd' and 'PygmyWarrior' to not play running animation in
	place while player is unreachable due to being on another vertical level

# 22.07.28 #
	- Fixed an issue where game would crash when player pressed 'Esc' during level transition or
	death animation
	- Renamed 'DoT res' back to 'Chaos res'
	- Added new item - 'Twin Souls' that increases chaos res
	- Implemented dmg increase from Power Shards
	- Fixed 'Damage' type not having const qualifiers on operators
	- Added sound que when activationg Save Orb
	- Added new item - 'Magic Negator' that increases magic res
	- Added a new secret to 'The Great Bridge' level
	- Built 4th level - 'Devouring Dephs'
	- Worked on assets

# 22.07.29 #
	- Implemented new enemy - 'Golem'
	- Reduced aggro range across the board
	- Implemented 'Portal' 2x2 tile
	- Implemented 'Ruined Portal' 2x2 tile
	- Added new item - 'Bone Mask' that increases phys res
	- Expanded 'shadow_basic' tileset
	- Added some decor to previous levels
	- Fixed a bug that caused crashes near right border of the map due to vector going out of range
	- Built 5th level - 'Plains'

# 22.07.30 #
	- Worked on assets for future enemies
	- Impelemented proper 'Portal' script as using 'LevelSwitch' insted would cause enemies to respawn
	- Expanded levels:
	1) 'Desolation'
	2) 'The Great Bridge'
	3) 'Descent'
	4) 'Devouring Dephs'
	5) 'Plains'
	with new sections and secrets. Increased number of artifacts scattered around the maps.

# 22.07.31 #
	- Built 6th level - 'Floating Isles'

# 22.08.01 #
	- Implemeted charges that are used to limit skill usage and powerfull jumps.
	Each skill use OR powerfull jump consumes 1 charge. Player has 3 charges by default,
	charges regenerate with time
	- Implemented player GUI for charges
	- Added effect to 'watching eye' item: increse number of charges by 1 up to 5
	- Cleared up deprecated code from 'Gui.h' and 'Gui.cpp'

# 22.08.03 #
	- Added new boss - 'Hellhound'
	- Built optional level - 'Treasury'
	- Allowed player to change orientation during attack animation
	- Removed deprecated code that dealt with attunements
	- Made current charges update dynamically with max charges, player now loads with full charges
	- Added functionality for playing effect animations independently from main sprite in 'Player'
	- Added charged jump animation
	- Fixed crashes caused by using F3 and/or standing on the very last column of tiles in the level

# 23.01.31 #
	- Placed required dll's into the project instead of the system itself
	- Figured out how to create distributable for the game
	- Added config file 'CONFIG.json' that holds launch params between sessions and allows easier testing
	- Added conversion to string for 'Vector2' and 'Vector2d' (explicit)
	- Slightly improved performance by falling back to 'RendererCopy()' instead of
	'RendererCopyEx()' when sprite has no flip or rotation (~750 FPS -> ~800 FPS)
	- Switched to a newer version of SDL2 through it didn't fix the issue with blurry scaling
	- Switched renderer driver to OpenGL, improved performance (~800 FPS -> ~950 FPS)
	- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Due to texture interpolation issue began transitioning the entire project from SDL2 to SFML
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	- Mostly reworked graphics system, managed to compile and display main menu
	- Reworked input polling and 'Input' class to SFML due to proper polling requiring existance
	of a window which SDL2 no longer has. Controls remain to be adapted

# 23.02.01 #
	- Adapted controls, got new input system working in order
	- GUI is now fully functional
	- Sprites and animations are now fully functional
	- Got the camera working by using separate view for camera nad GUI
	- Switching from SDL2 to SFML improved performance from ~750-950 FPS to ~1000-1200 FPS
	- Removed rotation from sprites (was only used for SpritBomber projectiles)
	- Fixed all implicit conversion warnings and got rid of redundant 'sf::Vector2f()' constructions
	- Fixed an issue that caused text to have visual tears at certain resolusion. Cause of the big not
	identified but putting 1 pixel wide transparent border around symbol textures fixed the issue

# 23.02.03 #
	- Implemented 'Controls' tab for Esc menu
	- Added functions to convert 'sf::Keyboard::Key' and 'sf::Mouse::Button' to 'std::string'

# 23.02.04 #
	- Added item lore and effect descriptions
	- Added parentheses to font
	- Implemented inventory GUI with full descriptions that functions similar to esc menu:
	game is now paused while inventory is open, background is darkened and player GUI disabled
	- Changed item color to full black when displayed in GUI so it doesn't merge with tile textures
	when displayed in front of them
	- Increased corpse lifetime for 'SkeletonHalberd'
	- Fixed (seemingly) the rounding issue that caused black stripes to appear on certain camera coords

# 23.02.05 #
	- Slightly increased player jump height (34->36)
	- Sped up attack animation by ~30%
	- Tested and adjusted terrain in some levels due to being inreachable or frustrating
	- In-level text is now hidden when EscMenu or Inventory GUI is up to prevent the situation
	when background text blends with GUI text of the same color
	- Simplified FPSCounter so it draws text directly instead of constantly creating and destroying
	text objects through GUI methods
	- Increased 'PygmyWarrior' DMG (150 phys -> 250 phys + 50 pure)
	- Increased 'SkeletonHalberd' DMG (350 phys -> 200 phys + 150 pure)
	- Increased 'Golem' DMG (150 phys -> 350 phys)
	- Reduced 'Hellhound' hitbox size (horizontal 46 -> 32) and health (5100 -> 4900)

# 23.02.06 #
	- Added added text scaling functionality to 'Font' and 'Text', 'make_line_...()' methods
	still produce unscaled text for simplicity
	- Added 'GUI_LevelName' class which displayes fading level name upon entering a new level
	- Changed the hint before the treausery from 'Great foe lies ahead' to 'A great foe protects
	ample rewards' to better reflect optional nature of the encounter
	- Began working on a new level 'Mountainside fortress'

# 23.02.08 #
	- Slightly (50 ms -> 45 ms on some frames) sped up player attack animation
	- Increased skill cost from 1 charge to 2 charge making it drastically less OP
	- Finished level 'Mountainside fortress'
	- Added 'Necromancer' enemy that spawns 'Worm' and 'Sludge' enemies
	- Increased enemy damage:
	Golem (350 phys -> 300 phys + 50 pure)
	Worm (30 phys + 30 chaos -> (30 phys + 30 chaos + 20 pure)
	Sludge (150 phys + 50 chaos -> 100 phys + 100 chaos + 30 pure)
	SpritBomber (200 phys + 100 pure -> 200 magic + 100 pure)
	- Added 'CultistMage' enemy that spawns slowly moving fireballs under the player
	(projectile ignores terrain)

# 23.02.16 #
	- Finished up sprites for the 3 boss phases and their mechanics
	- Planned out boss mechanics and technical implementations of those mechanics

# 23.07.18 #
	- Added 'applyForceTillMaxSpeed_Up()' and 'applyForceTillMaxSpeed_Down()' to 'SolidRectangle'
	- Added 'Tentacle' enemy that will be used during bossfight
	- Fixed 'Necromancer' spawning creatures below the ground instead of above
	- Began working on final boss, added first phase. Boss follows player on a with vertical level, while
	keeping some distance horizontaly. Spawns Skeleton Halberds and summons tentacles around the player.

# 23.07.20 #
	- Added restart functionality through propagating exit codes to an external loop

# 23.07.20 #
	- Moved config-related functions to 'saver.h'
	- Added 'config_create()', 'config_create_default()' and 'config_parse()
	- Added function for restarting the game
	- Added parsing of a current config to settings, the menu sets its options according
	to current configuration, if nonstandard resolution was selected in 'CONFIG.json' it's
	added as another option when choosing resolutions
	- Implemented settings menu with following options: 1) Resolution (custom supported);
	2) Screen mode (windowed, borderless, fulscreen); 3) Music (0-10); 4) Sound (0-10);
	5) FPS counter (On/Off)
	- Added ability to adjust volume according to settings
	- Added ability to turn FPS counter on and off

# 23.07.24 #
	- Mostly implemented 2nd boss phase

# 23.07.25 #
	- Deprecated 'TNT' destructible that was used for testing
	- Added 'OrbOfBetrayal' destructible (used for 2nd boss phase)
	- Fixed 'make_line_centered()' not accounting for text scale
	- Abstracted healhbar displays behind a pure virtual class
	- Added 'BossHealthbarDisplay' that displays large healthbar with a boss title
	at the bottom of the screen. Added corresponding init-methods

# 23.07.31 #
	- Cut off the underground part of 'Tentacle' texture so it doesn't protrude through thin platforms
	- Implemented 'Library' level (last level before final boss)
	- Implemented location for the 1st boss phase ('Lair of Shadows')
	- Implemented location for the 2nd boss phase ('...')

# 23.08.03 #
	- Limited projectile lifetime to prevent them from accumulation off-screen due to update culling
	- Adjusted collision system to account for hitbox size and check more tiles for big solids
	(thus handling solids over 1 tile in size correctly)
	- Fixed Tentacle/Fireball spawn tries breaking the loop upon faliure instead of continuing it
	- Implemented spawn checks for tentacle summoning that require it to be grounded at both sides
	- Finished 2nd boss phase
	- Blacked out resize button in windowed screen mode
	- Implemented location for the 3rd boss phase ('...')
	- Implemented 3rd boss phase
	- Made player explode into particles upon death rather that persist
	- Added 'checkpoint' script, used to save game between boss phases
	
# 23.08.06 #
	- Increased player attack speed by reducing windup and recovery frames
	- Added ending screen with 2 options 'continue playthrough' and 'finish playthrough'
	- Added checkpoint reset to 'continue playthrough' to allow fighting the boss multiple times
	- Added game reset (with creation of a backup) to 'finish playthrough'
	- Renamed boss arenas to be more authentic
	- Added death delay to phases 1 and 2 so the boss doesn't appear to vanish
	- Added death particles and a longer fade after phase 3
	- Extended 'Flags' and 'Saver' classes to suit aforementioned features
	- Increased default sound levels
	- Added protection agains turning on 'Esc' during game ending screen transition
	- Changed default value for setting 'Show FPS' to 'Off'
	- Fixed animated sprite not zero-initializing time_elapsed
	- Added tracking of time since last damage received to 'Health'
	- Added increased out-of-combat regen for player (10 sec timer)

# 23.08.13 #
	- Fixed an issue that caused 'sticky' movement when switching
	from pressing 'A' to pressing 'D' or otherwise during a single frame
	- Added delay on resetting 'is_dropping_down' when player releases 'S'
	which prevents player from being rubberbanded back when releasing 'S' too quickly
	- Added computation of total bonuses to inventory, removed 'additive' and 'multiplicative'
	comments since they are no longer necessary
	- Reduced hellhound hurtbox to make it realistic to beat without trading hits
    
# 25.04.02 #
    - Ran everything through cppcheck and pedantic compiler flags
    - Fixed 2 memory safety issues that weren't apparent on Windowns
    - Made a Linux build
    - Smooth music transitions (finally)