/*
* Copyright (c) 2015-16 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#ifndef _BEE_GAME_H
#define _BEE_GAME_H 1

#include <iostream> // Include the required library headers
#include <time.h>
#include <functional>
#include <list>
#include <getopt.h>

#include <SDL2/SDL.h> // Include the required SDL headers
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_net.h>

#define GLM_FORCE_RADIANS // Force all GLM functions to use radians instead of degrees

#define GLEW_STATIC // Statically link GLEW

#include <GL/glew.h> // Include the required OpenGL headers
#include <SDL2/SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define DEFAULT_WINDOW_WIDTH 1280 // Define the default window dimensions
#define DEFAULT_WINDOW_HEIGHT 720

#define DEFAULT_GAME_FPS 60 // Define the default game fps goal

#define BEE_MAX_LIGHTS 8 // Define the maximum amount of processed lights

#ifndef BEE_GAME_ID // BEE_GAME_ID should always be defined but just in case
#define BEE_GAME_ID 4294967295 // pow(2,32), the maximum value
#endif // BEE_GAME_ID

#include "debug.hpp" // Include the required debug and utility functions
#include "util.hpp"

#ifndef _BEE_RESOURCE_STRUCTURES_H
#include "resource_structures.hpp" // Include the resource structure classes
#endif

#include "enum.hpp" // Include the required engine enumerations

class BEE { // The master engine class which effectively acts as a namespace
	public:
		class Sprite; class Sound; class Background; class Font; class Path; class Timeline; class Object; class Room; // The main resource types
		class Particle; class ParticleData; class ParticleEmitter; class ParticleAttractor; class ParticleDestroyer; class ParticleDeflector; class ParticleChanger; class ParticleSystem; // The particle system components
		class Light; // The OpenGL-only resources (poor SDL implementations may exist)
		class ProgramFlags; class GameOptions; class CollisionTree; class CollisionPolygon; class RGBA; // The engine related data
		class SpriteDrawData; class SoundGroup; class InstanceData; class LightData; // The additional resource data types
		class ViewData; class BackgroundData; class NetworkData; // The configurational structs
	private:
		// These contain data about the engine initialization
		int argc;
		char** argv; // The provided commandline flags
		std::list<ProgramFlags*> flags; // The available commandline flags
		GameOptions* options = nullptr; // The engine options

		// These contain data about the event loop
		bool quit, is_ready, is_paused;
		Room *first_room = nullptr, *current_room = nullptr;

		// This defines the platform where 0=Linux, 1=Windows, 2=OSX, 3=other
		#ifdef __linux__
			const int platform = 0;
		#elif _WIN32
			const int platform = 1;
		#elif __APPLE__
			const int platform = 2;
		#else
			const int platform = 3;
		#endif

		int width, height;
		SDL_Cursor* cursor = nullptr;

		// These are used for the SDL renderer
		SDL_Window* window = nullptr;
		SDL_Renderer* renderer = nullptr;

		// This is the OpenGL renderer context
		SDL_GLContext context = nullptr;

		// The following GLint's are all uniforms in the OpenGL shaders
		GLuint program = 0; // This is the location of the OpenGL program (where the shaders are compiled)
		GLint vertex_location = -1;
		GLint fragment_location = -1;
		GLuint target = 0;

		GLint projection_location = -1;
		GLint view_location = -1;
		GLint model_location = -1;
		GLint port_location = -1;

		GLint rotation_location = -1;

		GLint texture_location = -1;
		GLint colorize_location = -1;
		GLint primitive_location = -1;
		GLint flip_location = -1;

		GLint lightable_location = -1;
		GLint light_amount_location = -1;
		struct {
			GLint type;
			GLint position;
			GLint direction;
			GLint attenuation;
			GLint color;
		} lighting_location[BEE_MAX_LIGHTS];

		// This is the current drawing color
		RGBA* color = nullptr;

		// These contain data about the current window state
		bool has_mouse, has_focus;

		SDL_Event event;
		Uint32 tickstamp, new_tickstamp, fps_ticks, tick_delta;

		NetworkData* net = nullptr;

		double volume = 1.0;

		unsigned int fps_max, fps_goal, fps_unfocused;
		unsigned int fps_count;
		Uint32 frame_number = 0;

		Sprite* texture_before = nullptr;
		Sprite* texture_after = nullptr;
		int transition_type = 0;
		unsigned int transition_speed = 80;
		const int transition_max = 21;

		const Uint8* keystate;

		std::vector<std::string> console_input;
		int console_line = 0;
	public:
		unsigned int fps_stable;

		static MetaResourceList* resource_list;
		static bool is_initialized;

		BEE(int, char**, const std::list<ProgramFlags*>&, Room**, GameOptions*);
		~BEE();
		int handle_flags(const std::list<ProgramFlags*>&, bool);
		int loop();
		int close();

		// User defined
		static int init_resources();
		static int close_resources();

		// bee/game/resources.cpp
		int load_media() const;
		int free_media() const;

		static void sound_finished(int);

		double get_volume() const;
		int set_volume(double);
		int sound_stop_all() const;

		Sprite* add_sprite(const std::string&, const std::string&);
		Sound* add_sound(const std::string&, const std::string&, bool);
		Background* add_background(const std::string&, const std::string&);
		Font* add_font(const std::string&, const std::string&, int, bool);
		Path* add_path(const std::string&, const std::string&);
		Timeline* add_timeline(const std::string&, const std::string&);
		Object* add_object(const std::string&, const std::string&);
		Room* add_room(const std::string&, const std::string&);

		static Sprite* get_sprite(int);
		static Sound* get_sound(int);
		static Background* get_background(int);
		static Font* get_font(int);
		static Path* get_path(int);
		static Timeline* get_timeline(int);
		static Object* get_object(int);
		static Room* get_room(int);

		Object* get_object_by_name(const std::string&) const;

		// bee/game.cpp
		Uint32 get_ticks() const;
		Uint32 get_seconds() const;
		Uint32 get_frame() const;
		double get_delta() const;
		unsigned int get_fps_goal() const;

		GameOptions get_options() const;
		int set_options(const GameOptions&);

		int opengl_init();
		int opengl_close();
		std::string opengl_prepend_version(const std::string&);
		int sdl_renderer_init();
		int sdl_renderer_close();

		int render_clear();
		int render() const;
		int render_reset();

		int restart_game() const;
		int end_game() const;

		// bee/game/info.cpp
		static std::list<BEE::ProgramFlags*> get_standard_flags();
		static std::string get_usage_text();

		// bee/game/room.cpp
		void restart_room() const;
		int change_room(Room*, bool);
		int change_room(Room*);
		int room_goto(int);
		int room_goto_previous();
		int room_goto_next();

		Room* get_current_room() const;
		bool get_is_ready() const;
		int get_room_width() const;
		int get_room_height() const;
		bool is_on_screen(const SDL_Rect&) const;

		int set_viewport(ViewData*) const;

		bool set_is_paused(bool);
		bool get_is_paused() const;

		// bee/game/transition.cpp
		int reset_render_target();
		int set_render_target(Sprite*, int, int);
		int set_render_target(Sprite*);
		int set_render_target(Background*, int, int);
		int set_render_target(Background*);

		int set_transition_type(int);
		int draw_transition();

		// bee/game/display.cpp
		SDL_DisplayMode get_display() const;
		Uint32 get_display_format() const;
		int get_display_width() const;
		int get_display_height() const;
		int get_display_refresh_rate() const;
		int set_display(int, int, int) const;
		int set_display_size(int, int) const;
		int set_display_refresh_rate(int) const;

		// bee/game/window.cpp
		std::string get_window_title() const;
		SDL_Cursor* get_cursor() const;
		int get_window_x() const;
		int get_window_y() const;
		int get_width() const;
		int get_height() const;

		int set_window_title(const std::string&) const;
		int set_cursor(SDL_SystemCursor);
		int set_show_cursor(bool) const;
		int set_window_position(int, int) const;
		int set_window_x(int) const;
		int set_window_y(int) const;
		int set_window_center() const;
		int set_window_size(int, int);
		int set_width(int);
		int set_height(int);

		// bee/game/input.cpp
		std::pair<int,int> get_mouse_global_position() const;
		int get_mouse_global_x() const;
		int get_mouse_global_y() const;
		std::pair<int,int> get_mouse_position() const;
		int get_mouse_x() const;
		int get_mouse_y() const;
		int set_mouse_global_position(int, int) const;
		int set_mouse_global_x(int) const;
		int set_mouse_global_y(int) const;
		bool is_mouse_inside(const InstanceData&) const;
		bool get_key_state(SDL_Scancode) const;
		bool get_key_state(SDL_Keycode) const;
		bool get_mod_state(Uint8) const;
		bool get_mouse_state(Uint8) const;
		char append_input(std::string*, SDL_KeyboardEvent*);

		// bee/game/draw.cpp
		RGBA get_enum_color(bee_rgba_t, Uint8) const;
		RGBA get_enum_color(bee_rgba_t) const;

		int convert_view_coords(int&, int&) const;
		int convert_window_coords(int&, int&) const;

		int draw_line(int, int, int, int, const RGBA&, bool);
		int draw_line(int, int, int, int, bool);
		int draw_line(int, int, int, int, bee_rgba_t, bool);
		int draw_line(const Line&, const RGBA&, bool);
		int draw_rectangle(int, int, int, int, bool, const RGBA&, bool);
		int draw_rectangle(int, int, int, int, bool, bool);
		int draw_rectangle(int, int, int, int, bool, bee_rgba_t, bool);
		int draw_rectangle(const SDL_Rect&, bool, const RGBA&, bool);

		int draw_set_color(const RGBA&);
		int draw_set_color(bee_rgba_t);
		RGBA draw_get_color() const;
		int draw_set_blend(SDL_BlendMode);
		SDL_BlendMode draw_get_blend();

		RGBA get_pixel_color(int, int) const;
		int save_screenshot(const std::string&) const;

		int set_is_lightable(bool);

		// bee/game/network.cpp
		int net_init();
		bool net_get_is_initialized() const;
		int net_close();
		int net_handle_events();

		int net_session_start(const std::string&, int, const std::string&);
		std::map<std::string,std::string> net_session_find();
		int net_session_join(const std::string&, const std::string&);
		bool net_get_is_connected();
		int net_session_end();
};

typedef std::tuple<int, int, double> bee_path_coord;
typedef std::multimap<Uint32, std::pair<std::string,std::function<void()>>> bee_timeline_list;

class BEE::ProgramFlags {
	public:
		ProgramFlags(std::string l, char s, bool p, int a, std::function<void (BEE*, char*)> f) {
			longopt = l; shortopt = s; pre_init = p; has_arg = a; func = f;
		}
		std::string longopt = "";
		char shortopt = '\0';
		bool pre_init = true;
		int has_arg = no_argument;
		std::function<void (BEE*, char*)> func = nullptr;
};
class BEE::GameOptions {
	public:
		GameOptions(bool f, bool b, bool r, bool m, bool h, bool v, bee_renderer_t rend, bool vsync, bool n, bool d) {
			is_fullscreen = f; is_borderless = b; is_resizable = r; is_maximized = m; is_highdpi = h; is_visible = v;
			renderer_type = rend; is_vsync_enabled = vsync; is_network_enabled = n; is_debug_enabled = d;
		}

		// Window options
		bool is_fullscreen, is_borderless;
		bool is_resizable, is_maximized;
		bool is_highdpi, is_visible;
		bool is_minimized = false;

		// Renderer options
		bee_renderer_t renderer_type;
		bool is_vsync_enabled;

		// Miscellaneous options
		bool is_network_enabled;
		bool is_debug_enabled;

		// Commandline flags
		bool should_assert = true;
		bool single_run = false;
};

class BEE::CollisionPolygon {
	public:
		unsigned int line_amount = 0;
		double x=0.0, y=0.0, w=0.0, h=0.0;
		std::vector<Line> lines;

		int add_vertex(double, double);
		int finalize();
};

class BEE::RGBA {
	public:
		Uint8 r, g, b, a;
};

class BEE::ViewData {
	public:
		bool is_visible;
		int view_x, view_y, view_width, view_height;
		int port_x, port_y, port_width, port_height;
		BEE::InstanceData* following;
		int horizontal_border, vertical_border;
		int horizontal_speed, vertical_speed;
};

class BEE::BackgroundData { // Used to pass data to the Room class in bee/resource_structures/room.hpp
	public:
		BEE::Background* background = nullptr;
		bool is_visible = false;
		bool is_foreground = false;
		int x=0, y=0;
		bool is_horizontal_tile=false, is_vertical_tile=false;
		int horizontal_speed=0, vertical_speed=0;
		bool is_stretched=false;

		BackgroundData();
		BackgroundData(BEE::Background*, bool, bool, int, int, bool, bool, int, int, bool);
		int init(BEE::Background*, bool, bool, int, int, bool, bool, int, int, bool);
};

class BEE::NetworkData {
	public:
		bool is_initialized = false;
		UDPsocket udp_send = nullptr;
		UDPsocket udp_recv = nullptr;
		UDPpacket* udp_data = nullptr;

		int id = BEE_GAME_ID >> 4;
		bool is_connected = false;
		bool is_host = false;
		int channel = -1;
		Uint32 timeout = 1000;
		std::map<std::string,std::string> servers;

		std::string name = "";
		unsigned int max_players = 0;
		int self_id = -1;
		std::map<int,UDPsocket> players;
		std::map<std::string,std::string> data;
};

#include "resource_structures/sprite.hpp"
#include "resource_structures/sound.hpp"
#include "resource_structures/background.hpp"
#include "resource_structures/font.hpp"
#include "resource_structures/path.hpp"
#include "resource_structures/timeline.hpp"
#include "resource_structures/object.hpp"
#include "resource_structures/light.hpp"
#include "resource_structures/ext/instancedata.hpp"
#include "resource_structures/ext/collisiontree.hpp"
#include "resource_structures/ext/particle.hpp"
#include "resource_structures/ext/soundgroup.hpp"
#include "resource_structures/room.hpp"

#endif // _BEE_GAME_H
