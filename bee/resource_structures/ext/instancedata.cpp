/*
* Copyright (c) 2015 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#ifndef _BEE_INSTANCEDATA
#define _BEE_INSTANCEDATA 1

#include "instancedata.hpp"

BEE::InstanceData::InstanceData() {
	velocity.clear();
	for (int i=0; i<ALARM_COUNT; i++) {
		alarm_end[i] = 0xffffffff;
	}
}
BEE::InstanceData::InstanceData(BEE* new_game, int new_id, Object* new_object, int new_x, int new_y) {
	game = new_game;
	init(new_id, new_object, new_x, new_y);
}
int BEE::InstanceData::init(int new_id, Object* new_object, int new_x, int new_y) {
	id = new_id;
	object = new_object;
	subimage_time = SDL_GetTicks();
	depth = object->get_depth();

	x = (float)new_x;
	y = (float)new_y;
	xprevious = x;
	yprevious = y;
	xstart = x;
	ystart = y;
	velocity.clear();
	gravity = 0.0;

	for (int i=0; i<ALARM_COUNT; i++) {
		alarm_end[i] = 0xffffffff;
	}

	return 0;
}
int BEE::InstanceData::print() {
	std::cout <<
	"InstanceData { "
	"\n	id		" << id <<
	"\n	object		" << object <<
	"\n	subimage_time	" << subimage_time <<
	"\n	x, y		" << x << ", " << y <<
	//"\n	velocity	" << velocity <<
	"\n	gravity		" << gravity <<
	"\n}\n";

	return 0;
}

bool BEE::InstanceData::operator< (const InstanceData& other) {
	if (depth == other.depth) {
		return (id <= other.id);
	}
	return (depth > other.depth);
}

int BEE::InstanceData::remove() {
	object->destroy(this);
	game->get_current_room()->remove_instance(id);
	return 0;
}

int BEE::InstanceData::set_alarm(int alarm, Uint32 elapsed_ticks) {
	alarm_end[alarm] = elapsed_ticks + SDL_GetTicks();
	return 0;
}

int BEE::InstanceData::move(double new_magnitude, double new_direction) {
	if (new_direction < 0.0) {
		new_direction = 360.0 + new_direction;
	}
	velocity.push_back(std::make_pair(new_magnitude, fmod(new_direction, 360.0)));
	return 0;
}
int BEE::InstanceData::move_to(double new_magnitude, double other_x, double other_y) {
	if (distance(x, y, other_x, other_y) < new_magnitude) {
		return 1;
	}
	move(new_magnitude, direction_of(x, y, other_x, other_y));
	return 0;
}
int BEE::InstanceData::move_away(double new_magnitude, double other_x, double other_y) {
	move(new_magnitude, direction_of(x, y, other_x, other_y)+180.0);
	return 0;
}
int BEE::InstanceData::set_friction(double new_friction) {
	friction = new_friction;
	return 0;
}
int BEE::InstanceData::set_gravity(double new_gravity) {
	gravity = new_gravity;
	return 0;
}
int BEE::InstanceData::set_gravity_direction(double new_gravity_direction) {
	gravity_direction = new_gravity_direction;
	return 0;
}

std::pair<double,double> BEE::InstanceData::get_motion() {
	double xsum = x;
	double ysum = y;

	velocity.push_back(std::make_pair(gravity, gravity_direction));
	for (auto& v : velocity) {
		xsum += sin(degtorad(v.second))*v.first;
		ysum += -cos(degtorad(v.second))*v.first;
	}

	double d = direction_of(x, y, xsum, ysum);
	xsum += sin(degtorad(d))*friction;
	ysum += -cos(degtorad(d))*friction;

	return std::make_pair(xsum, ysum);
}
std::pair<double,double> BEE::InstanceData::get_position() {
	return std::make_pair(x, y);
}
double BEE::InstanceData::get_hspeed() {
	return get_motion().first - xprevious;
}
double BEE::InstanceData::get_vspeed() {
	return get_motion().second - yprevious;
}
double BEE::InstanceData::get_direction() {
	double xsum=0.0, ysum=0.0;
	std::tie (xsum, ysum) = get_motion();
	return direction_of(x, y, xsum, ysum);
}
double BEE::InstanceData::get_speed() {
	double xsum=0.0, ysum=0.0;
	std::tie (xsum, ysum) = get_motion();
	if ((x == xsum)&&(y == ysum)) {
		return distance(xprevious, yprevious, x, y);
	}
	return distance(x, y, xsum, ysum);
}
double BEE::InstanceData::get_friction() {
	return friction;
}
double BEE::InstanceData::get_gravity() {
	return gravity;
}
double BEE::InstanceData::get_gravity_direction() {
	return gravity_direction;
}

bool BEE::InstanceData::is_place_free(int new_x, int new_y) {
	bool is_collision = false;
	SDL_Rect a = {new_x, new_y, object->get_mask()->get_subimage_width(), object->get_mask()->get_height()};
	for (auto& i : game->get_current_room()->get_instances()) {
		SDL_Rect b = {(int)i.second->x, (int)i.second->y, i.second->object->get_mask()->get_subimage_width(), i.second->object->get_mask()->get_height()};
		if (check_collision(&a, &b)) {
			if (i.second->object->get_is_solid()) {
				is_collision = true;
				break;
			}
		}
	}
	return !is_collision;
}
bool BEE::InstanceData::is_place_empty(int new_x, int new_y) {
	bool is_collision = false;
	SDL_Rect a = {new_x, new_y, object->get_mask()->get_subimage_width(), object->get_mask()->get_height()};
	for (auto& i : game->get_current_room()->get_instances()) {
		SDL_Rect b = {(int)i.second->x, (int)i.second->y, i.second->object->get_mask()->get_subimage_width(), i.second->object->get_mask()->get_height()};
		if (check_collision(&a, &b)) {
			is_collision = true;
			break;
		}
	}
	return !is_collision;
}
bool BEE::InstanceData::is_place_meeting(int new_x, int new_y, Object* other) {
	bool is_collision = false;
	SDL_Rect a = {new_x, new_y, object->get_mask()->get_subimage_width(), object->get_mask()->get_height()};
	for (auto& i : other->get_instances()) {
		SDL_Rect b = {(int)i.second->x, (int)i.second->y, i.second->object->get_mask()->get_subimage_width(), i.second->object->get_mask()->get_height()};
		if (check_collision(&a, &b)) {
			is_collision = true;
			break;
		}
	}
	return is_collision;
}
bool BEE::InstanceData::is_place_meeting(int new_x, int new_y, int other_id) {
	bool is_collision = false;
	SDL_Rect a = {new_x, new_y, object->get_mask()->get_subimage_width(), object->get_mask()->get_height()};
	for (auto& i : game->get_object(other_id)->get_instances()) {
		SDL_Rect b = {(int)i.second->x, (int)i.second->y, i.second->object->get_mask()->get_subimage_width(), i.second->object->get_mask()->get_height()};
		if (check_collision(&a, &b)) {
			is_collision = true;
			break;
		}
	}
	return is_collision;
}
bool BEE::InstanceData::is_snapped(int hsnap, int vsnap) {
	if (((int)x % hsnap == 0)&&((int)y % vsnap == 0)) {
		return true;
	}
	return false;
}

int BEE::InstanceData::move_random(int hsnap, int vsnap) {
	x = random(game->get_current_room()->get_width());
	y = random(game->get_current_room()->get_height());
	move_snap(hsnap, vsnap);
	return 0;
}
int BEE::InstanceData::move_snap(int hsnap, int vsnap) {
	int dx = (int)x % hsnap;
	int dy = (int)y % vsnap;

	if (dx >= hsnap/2) {
		x += hsnap;
	}
	x -= dx;
	if (dy >= vsnap/2) {
		y += vsnap;
	}
	y -= dy;

	return 0;
}
int BEE::InstanceData::move_wrap(bool is_horizontal, bool is_vertical, int margin) {
	int w = game->get_current_room()->get_width();
	int h = game->get_current_room()->get_height();

	if (is_horizontal) {
		if (x < -margin){
			x = w+margin-x;
		} else if (x > w+margin) {
			x -= w+margin;
		}
	}
	if (is_vertical) {
		if (y < -margin) {
			y = h+margin-y;
		} else if (x > h+margin) {
			y -= h+margin;
		}
	}

	return 0;
}

double BEE::InstanceData::get_distance(int dx, int dy) {
	return distance(x, y, dx, dy);
}
double BEE::InstanceData::get_distance(InstanceData* other) {
	return distance(x, y, other->x, other->y);
}
double BEE::InstanceData::get_distance(Object* other) {
	double shortest_distance = 0.0, current_distance = 0.0;
	for (auto& i : game->get_current_room()->get_instances()) {
		if (i.second->object->get_id() == other->get_id()) {
			current_distance = distance(x, y, i.second->x, i.second->y);
			if (current_distance < shortest_distance) {
				shortest_distance = current_distance;
			}
		}
	}
	return shortest_distance;
}

int BEE::InstanceData::path_start(Path* new_path, double new_path_speed, int new_end_action, bool absolute) {
	path = new_path;
	path_speed = new_path_speed;
	path_end_action = new_end_action;
	path_current_node = 0;

	if (absolute) {
		path_xstart = std::get<0>(path->get_coordinate_list().front());
		path_xstart = std::get<1>(path->get_coordinate_list().front());
	} else {
		path_xstart = x;
		path_ystart = y;
	}

	return 0;
}
int BEE::InstanceData::path_end() {
	path = NULL;
	path_speed = 0.0;
	path_end_action = 0;
	path_xstart = 0;
	path_ystart = 0;
	path_current_node = 0;
	return 0;
}
int BEE::InstanceData::path_update_node() {
	if (has_path()) {
		if (path_speed >= 0) {
			if (path_current_node+1 < (int) path->get_coordinate_list().size()) {
				path_coord c = path->get_coordinate_list().at(path_current_node+1);
				if (distance(x, y, path_xstart+std::get<0>(c), path_ystart+std::get<1>(c)) < get_speed()) {
					path_current_node++;
				}
			}
		} else {
			path_coord c = path->get_coordinate_list().at(path_current_node);
			if (distance(x, y, path_xstart+std::get<0>(c), path_ystart+std::get<1>(c)) < get_speed()) {
				path_current_node--;
			}
		}

		return 0;
	}
	return 1;
}
int BEE::InstanceData::set_path_drawn(bool new_path_is_drawn) {
	path_is_drawn = new_path_is_drawn;
	return 0;
}
int BEE::InstanceData::handle_path_end() {
	if (has_path()) {
		switch (path_end_action) {
			case 0: { // Stop path
				path_end();
				break;
			}
			case 1: { // Continue from start
				path_current_node = 0;
				x = path_xstart;
				y = path_ystart;
				xprevious = x;
				yprevious = y;
				break;
			}
			case 2: { // Continue from current position
				path_current_node = 0;
				path_xstart = x;
				path_ystart = y;
				break;
			}
			case 3: { // Reverse direction
				path_speed *= -1;
				if (path_speed >= 0) {
					path_current_node = 0;
				} else {
					path_current_node = path->get_coordinate_list().size()-2;
				}
				break;
			}
		}
		return 0;
	}
	return 1;
}
bool BEE::InstanceData::has_path() {
	return (path != NULL) ? true : false;
}
bool BEE::InstanceData::get_path_drawn() {
	return path_is_drawn;
}
int BEE::InstanceData::get_path_speed() {
	return path_speed;
}
int BEE::InstanceData::get_path_node() {
	return path_current_node;
}
std::vector<path_coord> BEE::InstanceData::get_path_coords() {
	std::vector<path_coord> no_path;
	return (has_path()) ? path->get_coordinate_list() : no_path;
}

int BEE::InstanceData::draw(int w, int h, double angle, RGBA color) {
	return object->get_sprite()->draw(x, y, subimage_time, w, h, angle, color);
}
int BEE::InstanceData::draw(int w, int h, double angle, rgba_t color) {
	return draw(w, h, angle, game->get_enum_color(color));
}
int BEE::InstanceData::draw() {
	return object->get_sprite()->draw(x, y, subimage_time);
}
int BEE::InstanceData::draw(int w, int h) {
	return object->get_sprite()->draw(x, y, subimage_time, w, h);
}
int BEE::InstanceData::draw(double angle) {
	return object->get_sprite()->draw(x, y, subimage_time, angle);
}
int BEE::InstanceData::draw(RGBA color) {
	return object->get_sprite()->draw(x, y, subimage_time, color);
}
int BEE::InstanceData::draw(rgba_t color) {
	return draw(game->get_enum_color(color));
}

#endif // _BEE_INSTANCEDATA