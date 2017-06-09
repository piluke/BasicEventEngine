/*
* Copyright (c) 2015-17 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#ifndef BEE_TIMELINE_H
#define BEE_TIMELINE_H 1

#include <map>
#include <functional>

#include <SDL2/SDL.h> // Include the required SDL headers

#include "../resources.hpp"

namespace bee {
	typedef std::multimap<Uint32, std::pair<std::string,std::function<void()>>> timeline_list_t;

	class Timeline: public Resource {
			// Add new variables to the print() debugging method
			int id = -1;
			std::string name;
			std::string path;
			timeline_list_t action_list;
			timeline_list_t::iterator next_action;
			std::function<void()> end_action = nullptr;

			Uint32 start_frame = 0xffffffff;
			Uint32 position_frame = 0xffffffff;
			Uint32 start_offset = 0;
			Uint32 pause_offset = 0;
			bool is_looping = false;
			bool is_paused = false;
		public:
			Timeline();
			Timeline(const std::string&, const std::string&);
			~Timeline();
			int add_to_resources();
			int reset();
			int print() const;

			int get_id() const;
			std::string get_name() const;
			std::string get_path() const;
			timeline_list_t get_action_list() const;
			std::string get_action_string() const;
			bool get_is_running() const;
			bool get_is_looping() const;

			int set_name(const std::string&);
			int set_path(const std::string&);
			int add_action(Uint32, std::string, std::function<void()>);
			int add_action(Uint32, std::function<void()>);
			int remove_action(Uint32);
			int set_offset(Uint32);
			int clip_offset();
			int set_is_looping(bool);
			int add_ending(std::function<void()>);
			int set_pause(bool);

			int start();
			int step_to(Uint32);
			int end();
	};
}

#endif // BEE_TIMELINE_H