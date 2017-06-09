/*
* Copyright (c) 2015-17 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#ifndef BEE_TIMELINE
#define BEE_TIMELINE 1

#include <sstream>

#include "timeline.hpp"

#include "../debug.hpp"
#include "../engine.hpp"

#include "../util/string.hpp"
#include "../util/platform.hpp"

namespace bee {
	Timeline::Timeline () {
		reset();
	}
	Timeline::Timeline (const std::string& new_name, const std::string& new_path) {
		reset();

		add_to_resources();
		if (id < 0) {
			messenger_send({"engine", "resource"}, E_MESSAGE::WARNING, "Failed to add timeline resource: \" + new_name + \" from " + new_path);
			throw(-1);
		}

		set_name(new_name);
		set_path(new_path);
	}
	Timeline::~Timeline() {
		resource_list->timelines.remove_resource(id);
	}
	int Timeline::add_to_resources() {
		if (id < 0) { // If the resource needs to be added to the resource list
			id = resource_list->timelines.add_resource(this); // Add the resource and get the new id
		}

		return 0;
	}
	int Timeline::reset() {
		name = "";
		path = "";
		action_list.clear();
		next_action = action_list.end();

		start_frame = 0xffffffff;
		position_frame = 0xffffffff;
		start_offset = 0;
		is_looping = false;

		return 0;
	}
	int Timeline::print() const {
		std::string action_string = get_action_string();

		std::stringstream s;
		s <<
		"Timeline { "
		"\n	id             " << id <<
		"\n	name           " << name <<
		"\n	path           " << path <<
		"\n	start_frame    " << start_frame <<
		"\n	position_frame " << position_frame <<
		"\n	is_looping     " << is_looping <<
		"\n	action_list\n" << debug_indent(action_string, 2) <<
		"\n}\n";
		messenger_send({"engine", "resource"}, E_MESSAGE::INFO, s.str());

		return 0;
	}

	int Timeline::get_id() const {
		return id;
	}
	std::string Timeline::get_name() const {
		return name;
	}
	std::string Timeline::get_path() const {
		return path;
	}
	timeline_list_t Timeline::get_action_list() const {
		return action_list;
	}
	std::string Timeline::get_action_string() const {
		if (action_list.size() > 0) {
			std::vector<std::vector<std::string>> table;
			table.push_back({"(frame", "func_name)"});

			for (auto& a : action_list) {
				table.push_back({bee_itos(a.first), a.second.first});
			}

			return string_tabulate(table);
		}
		return "none\n";
	}
	bool Timeline::get_is_running() const {
		if (is_paused) {
			return false;
		}

		if (start_frame == 0xffffffff) {
			return false;
		} else {
			return true;
		}
	}
	bool Timeline::get_is_looping() const {
		return is_looping;
	}

	int Timeline::set_name(const std::string& new_name) {
		name = new_name;
		return 0;
	}
	int Timeline::set_path(const std::string& new_path) {
		path = new_path;
		return 0;
	}
	int Timeline::add_action(Uint32 frame_number, std::string func_name, std::function<void()> callback) {
		if (get_is_running()) {
			return 1;
		}

		action_list.emplace(frame_number, std::make_pair(func_name, callback));

		return 0;
	}
	int Timeline::add_action(Uint32 frame_number, std::function<void()> callback) {
		return add_action(frame_number, "anonymous", callback);
	}
	int Timeline::remove_action(Uint32 frame_number) {
		if (action_list.find(frame_number) != action_list.end()) {
			action_list.erase(action_list.find(frame_number));
			return 0;
		}
		return 1;
	}
	int Timeline::set_offset(Uint32 new_offset) {
		start_offset = new_offset;
		return 0;
	}
	int Timeline::clip_offset() {
		if (!action_list.empty()) {
			start_offset = action_list.begin()->first;
		}
		return 0;
	}
	int Timeline::set_is_looping(bool new_is_looping) {
		is_looping = new_is_looping;
		return 0;
	}
	int Timeline::add_ending(std::function<void()> callback) {
		end_action = callback;
		return 0;
	}
	int Timeline::set_pause(bool new_is_paused) {
		if (is_paused == new_is_paused) {
			return 1;
		}

		pause_offset = get_frame() - pause_offset;

		is_paused = new_is_paused;

		return 0;
	}

	int Timeline::start() {
		start_frame = get_frame() - start_offset;
		next_action = action_list.begin();
		return 0;
	}
	int Timeline::step_to(Uint32 new_frame) {
		if (!get_is_running()) {
			return 1;
		}

		position_frame = new_frame - start_frame - pause_offset;
		if (next_action == action_list.end()) {
			return 1;
		}

		while (next_action->first < position_frame) {
			if (step_to(next_action->first + start_frame)) {
				break;
			}
		}

		if (action_list.find(position_frame) != action_list.end()) {
			std::pair <timeline_list_t::iterator,timeline_list_t::iterator> range;
			range = action_list.equal_range(position_frame);
			for (timeline_list_t::iterator it=range.first; it!=range.second; ++it) {
				it->second.second();
				++next_action;
			}

			if ((next_action == action_list.end())||(position_frame >= (*action_list.rbegin()).first)) {
				start_frame = 0xffffffff;
				return 2;
			}
		}

		return 0;
	}
	int Timeline::end() {
		pause_offset = 0;
		is_paused = false;

		if (end_action == nullptr) {
			return 1;
		}

		end_action();

		return 0;
	}
}

#endif // BEE_TIMELINE