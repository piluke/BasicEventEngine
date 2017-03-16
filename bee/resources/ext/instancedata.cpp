/*
* Copyright (c) 2015-17 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#ifndef _BEE_INSTANCEDATA
#define _BEE_INSTANCEDATA 1

#include "instancedata.hpp"

BEE::InstanceData::InstanceData() {
	for (size_t i=0; i<ALARM_COUNT; i++) {
		alarm_end[i] = 0xffffffff;
	}
}
BEE::InstanceData::InstanceData(BEE* new_game, int new_id, Object* new_object, double new_x, double new_y, double new_z) {
	game = new_game;
	init(new_id, new_object, new_x, new_y, new_z);
}
BEE::InstanceData::~InstanceData() {
	delete body;
}
int BEE::InstanceData::init(int new_id, Object* new_object, double new_x, double new_y, double new_z) {
	id = new_id;
	object = new_object;
	subimage_time = SDL_GetTicks();
	depth = object->get_depth();

	if (body == nullptr) {
		PhysicsWorld* w = game->get_current_room()->get_phys_world();

		double p[3] = {(double)get_width(), (double)get_height(), 1.0};
		if ((p[0] != 0.0)&&(p[1] != 0.0)) {
			body = new PhysicsBody(w, BEE_PHYS_SHAPE_BOX, 0.0, new_x, new_y, new_z, p);
		} else {
			body = new PhysicsBody(w, BEE_PHYS_SHAPE_NONE, 0.0, new_x, new_y, new_z, nullptr);
		}
	} else {
		btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(new_x, new_y, new_z));
		body->get_body()->getMotionState()->setWorldTransform(t);
	}
	xstart = new_x;
	ystart = new_y;
	zstart = new_z;
	xprevious = new_x;
	yprevious = new_y;
	zprevious = new_z;

	for (size_t i=0; i<ALARM_COUNT; i++) {
		alarm_end[i] = 0xffffffff;
	}

	return 0;
}
int BEE::InstanceData::print() {
	std::stringstream s;
	s <<
	"InstanceData { "
	"\n	id            " << id <<
	"\n	object        " << object <<
	"\n	subimage_time " << subimage_time <<
	"\n	x, y, z       " << get_x() << ", " << get_y() << ", " << get_z() <<
	"\n	depth         " << depth <<
	"\n}\n";
	game->messenger_send({"engine", "resource"}, BEE_MESSAGE_INFO, s.str());

	return 0;
}

bool BEE::InstanceData::operator< (const InstanceData& other) const {
	if (depth == other.depth) {
		return (id < other.id);
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

int BEE::InstanceData::set_object(BEE::Object* new_object) {
	object->remove_instance(id);
	object = new_object;
	object->add_instance(id, this);
	return 0;
}

btVector3 BEE::InstanceData::get_position() const {
	return body->get_position();
}
double BEE::InstanceData::get_x() const {
	return get_position().getX();
}
double BEE::InstanceData::get_y() const {
	return get_position().getY();
}
double BEE::InstanceData::get_z() const {
	return get_position().getZ();
}
double BEE::InstanceData::get_corner_x() const {
	return get_x() - get_width()/2.0;
}
double BEE::InstanceData::get_corner_y() const {
	return get_y() - get_height()/2.0;
}
double BEE::InstanceData::get_xstart() const {
	return xstart;
}
double BEE::InstanceData::get_ystart() const {
	return ystart;
}
BEE::PhysicsBody* BEE::InstanceData::get_physbody() const {
	return body;
}

int BEE::InstanceData::get_width() const {
	if (object->get_mask() == nullptr) {
		return 0;
	}
	return object->get_mask()->get_subimage_width();
}
int BEE::InstanceData::get_height() const {
	if (object->get_mask() == nullptr) {
		return 0;
	}
	return object->get_mask()->get_height();
}
SDL_Rect BEE::InstanceData::get_aabb() const {
	return {(int)get_x(), (int)get_y(), get_width(), get_height()};
}

int BEE::InstanceData::set_position(btVector3 p) {
	btTransform t;

	t.setIdentity();
	t.setOrigin(p);
	body->get_body()->getMotionState()->setWorldTransform(t);

	return 0;
}
int BEE::InstanceData::set_position(double new_x, double new_y, double new_z) {
	return set_position(btVector3(new_x, new_y, new_z));
}
int BEE::InstanceData::move(double new_magnitude, double new_direction) {
	if (new_magnitude < 0.0) {
		new_direction -= 180.0;
		new_magnitude = fabs(new_magnitude);
	}
	new_direction = absolute_angle(new_direction);
	body->get_body()->activate(true);
	body->get_body()->applyCentralImpulse(btVector3(new_magnitude*cos(new_direction), new_magnitude*-sin(new_direction), 0.0) / body->get_scale());
	return 0;
}
int BEE::InstanceData::move_to(double new_magnitude, double other_x, double other_y) {
	if (distance(get_x(), get_y(), other_x, other_y) < new_magnitude) {
		return 1;
	}
	move(new_magnitude, direction_of(get_x(), get_y(), other_x, other_y));
	return 0;
}
int BEE::InstanceData::move_away(double new_magnitude, double other_x, double other_y) {
	move(new_magnitude, direction_of(get_x(), get_y(), other_x, other_y)+180.0);
	return 0;
}
int BEE::InstanceData::set_friction(double new_friction) {
	body->get_body()->setFriction(new_friction);
	return 0;
}
int BEE::InstanceData::set_gravity(btVector3 new_gravity) {
	body->get_body()->setGravity(new_gravity);
	return 0;
}
int BEE::InstanceData::set_gravity(double new_gx, double new_gy, double new_gz) {
	return set_gravity(btVector3(new_gx, new_gy, new_gz));
}
int BEE::InstanceData::move_outside(btVector3 dir) {
	/*double dist = distance(l.x1, l.y1, l.x2, l.y2);
	double dir = direction_of(l.x1, l.y1, l.x2, l.y2);
	x = l.x1;
	y = l.y1;

	int max_attempts = 10;
	double delta = 1.0/((double)max_attempts);
	int attempts = 0;

	mask.x = x; mask.y = y;
	while ((check_collision_polygon(mask, m))&&(attempts++ < max_attempts)) {
		x += cos(degtorad(dir)) * delta*dist * game->get_delta();
		y += -sin(degtorad(dir)) * delta*dist * game->get_delta();
		mask.x = x;
		mask.y = y;
	}

	if (check_collision_polygon(mask, m)) {
		x -= cos(degtorad(dir)) * delta*dist * game->get_delta();
		y -= -sin(degtorad(dir)) * delta*dist * game->get_delta();
		mask.x = x;
		mask.y = y;
	}*/

	return 0;
}

double BEE::InstanceData::get_speed() const {
	btVector3 v = get_velocity();
	return distance(v.getX(), v.getY(), v.getZ());
}
btVector3 BEE::InstanceData::get_velocity() const {
	return body->get_body()->getLinearVelocity();
}
btVector3 BEE::InstanceData::get_velocity_ang() const {
	return body->get_body()->getAngularVelocity();
}
double BEE::InstanceData::get_friction() const {
	return body->get_body()->getFriction();
}
btVector3 BEE::InstanceData::get_gravity() const {
	return body->get_body()->getGravity();
}

bool BEE::InstanceData::is_place_free(int new_x, int new_y) const {
	SDL_Rect mask = get_aabb();
	mask.x = new_x;
	mask.y = new_y;

	for (auto& i : game->get_current_room()->get_instances()) {
		if (i.second == this) {
			continue;
		}

		SDL_Rect other = i.second->get_aabb();

		if (i.second->object->get_is_solid()) {
			if (check_collision(mask, other)) {
				if (object->check_collision_list(*this, *i.second)) {
					if (i.second->object->check_collision_list(*i.second, *this)) {
						return false;
					}
				}
			}
		}
	}

	return true;
}
bool BEE::InstanceData::is_place_empty(int new_x, int new_y) const {
	SDL_Rect mask = get_aabb();
	mask.x = new_x;
	mask.y = new_y;

	for (auto& i : game->get_current_room()->get_instances()) {
		if (i.second == this) {
			continue;
		}

		SDL_Rect other = i.second->get_aabb();

		if (check_collision(mask, other)) {
			return false;
		}
	}

	return true;
}
bool BEE::InstanceData::is_place_meeting(int new_x, int new_y, Object* other) const {
	SDL_Rect mask = get_aabb();
	mask.x = new_x;
	mask.y = new_y;

	for (auto& i : other->get_instances()) {
		if (i.second == this) {
			continue;
		}

		SDL_Rect other = get_aabb();

		if (check_collision(mask, other)) {
			return true;
		}
	}

	return false;
}
bool BEE::InstanceData::is_place_meeting(int new_x, int new_y, int other_id) const {
	return is_place_meeting(new_x, new_y, game->get_object(other_id));
}
bool BEE::InstanceData::is_move_free(double magnitude, double direction) {
	double dx = cos(degtorad(direction)) * magnitude;
	double dy = -sin(degtorad(direction)) * magnitude;
	return is_place_free(get_x()+dx, get_y()+dy);
}
bool BEE::InstanceData::is_snapped(int hsnap, int vsnap) const {
	if (((int)get_x() % hsnap == 0)&&((int)get_y() % vsnap == 0)) {
		return true;
	}
	return false;
}

std::pair<int,int> BEE::InstanceData::get_snapped(int hsnap, int vsnap) const {
	if (hsnap < 1) {
		hsnap = 1;
	}
	if (vsnap < 1) {
		vsnap = 1;
	}

	int xsnap = (int)get_x();
	int ysnap = (int)get_y();

	int dx = xsnap % hsnap;
	int dy = ysnap % vsnap;

	if (get_x() < 0) {
		xsnap -= hsnap;
	}
	if (get_y() < 0) {
		ysnap -= hsnap;
	}
	xsnap -= dx;
	ysnap -= dy;

	return std::make_pair(xsnap, ysnap);
}
std::pair<int,int> BEE::InstanceData::get_snapped() const {
	if (object->get_sprite() == nullptr) {
		return std::make_pair((int)get_x(), (int)get_y());
	}
	return get_snapped(object->get_sprite()->get_width(), object->get_sprite()->get_height());
}
int BEE::InstanceData::move_random(int hsnap, int vsnap) {
	double rx = random(game->get_current_room()->get_width());
	double ry = random(game->get_current_room()->get_height());

	set_position(rx, ry, get_z());

	move_snap(hsnap, vsnap);
	return 0;
}
int BEE::InstanceData::move_snap(int hsnap, int vsnap) {
	double sx, sy;
	std::tie(sx, sy) = get_snapped(hsnap, vsnap);

	set_position(sx, sy, get_z());

	return 0;
}
int BEE::InstanceData::move_snap() {
	if (object->get_sprite() == nullptr) {
		return 0;
	}
	return move_snap(object->get_sprite()->get_width(), object->get_sprite()->get_height());
}
int BEE::InstanceData::move_wrap(bool is_horizontal, bool is_vertical, int margin) {
	int w = game->get_current_room()->get_width();
	int h = game->get_current_room()->get_height();

	double nx = get_x();
	double ny = get_y();
	if (is_horizontal) {
		if (nx < -margin){
			nx = w+margin-nx;
		} else if (nx > w+margin) {
			nx -= w+margin;
		}
	}
	if (is_vertical) {
		if (ny < -margin) {
			ny = h+margin-ny;
		} else if (ny > h+margin) {
			ny -= h+margin;
		}
	}

	set_position(nx, ny, get_z());

	return 0;
}

double BEE::InstanceData::get_distance(int dx, int dy) const {
	return distance(get_x(), get_y(), dx, dy);
}
double BEE::InstanceData::get_distance(InstanceData* other) const {
	return distance(get_x(), get_y(), other->get_x(), other->get_y());
}
double BEE::InstanceData::get_distance(Object* other) const {
	double shortest_distance = 0.0, current_distance = 0.0;
	for (auto& i : game->get_current_room()->get_instances()) {
		if (i.second->object->get_id() == other->get_id()) {
			current_distance = distance(get_x(), get_y(), i.second->get_x(), i.second->get_y());
			if (current_distance < shortest_distance) {
				shortest_distance = current_distance;
			}
		}
	}
	return shortest_distance;
}
double BEE::InstanceData::get_direction_of(int dx, int dy) const {
	return direction_of(get_x(), get_y(), dx, dy);
}
double BEE::InstanceData::get_direction_of(InstanceData* other) const {
	return direction_of(get_x(), get_y(), other->get_x(), other->get_y());
}
double BEE::InstanceData::get_direction_of(Object* other) const {
	double shortest_distance = 0.0, current_distance = 0.0;
	InstanceData* closest_instance = nullptr;
	for (auto& i : game->get_current_room()->get_instances()) {
		if (i.second->object->get_id() == other->get_id()) {
			current_distance = distance(get_x(), get_y(), i.second->get_x(), i.second->get_y());
			if (current_distance < shortest_distance) {
				shortest_distance = current_distance;
				closest_instance = i.second;
			}
		}
	}

	if (closest_instance != nullptr) {
		return direction_of(get_x(), get_y(), closest_instance->get_x(), closest_instance->get_y());
	}
	return 0.0;
}
int BEE::InstanceData::get_relation(InstanceData* other) const {
	if ((other->get_y() < get_y())&&(abs(other->get_x() - get_x()) < other->get_width()/2 + get_width()/2)) { // Top block
		return 1;
	} else if ((other->get_x() > get_x())&&(abs(other->get_y() - get_y()) < other->get_height()/2 + get_height()/2)) { // Right block
		return 2;
	} else if ((other->get_y() > get_y())&&(abs(other->get_x() - get_x()) < other->get_width()/2 + get_width()/2)) { // Bottom block
		return 3;
	} else if ((other->get_x() < get_x())&&(abs(other->get_y() - get_y()) < other->get_height()/2 + get_height()/2)) { // Left block
		return 4;
	}
	return 0;
}

int BEE::InstanceData::path_start(Path* new_path, double new_path_speed, int new_end_action, bool absolute) {
	path = new_path;
	path_speed = new_path_speed;
	path_end_action = new_end_action;
	path_current_node = 0;

	if (absolute) {
		path_xstart = std::get<0>(path->get_coordinate_list().front());
		path_ystart = std::get<1>(path->get_coordinate_list().front());
		//path_ystart = std::get<2>(path->get_coordinate_list().front());
	} else {
		path_xstart = get_x();
		path_ystart = get_y();
		//path_zstart = get_z();
	}

	return 0;
}
int BEE::InstanceData::path_end() {
	path = nullptr;
	path_speed = 0.0;
	path_end_action = 0;
	path_xstart = 0;
	path_ystart = 0;
	path_current_node = 0;
	return 0;
}
int BEE::InstanceData::path_reset() {
	bool a = false;
	if (path_xstart == std::get<0>(path->get_coordinate_list().front())) {
		if (path_ystart == std::get<1>(path->get_coordinate_list().front())) {
			a = true;
		}
	}

	return path_start(path, path_speed, path_end_action, a);
}
int BEE::InstanceData::path_update_node() {
	if (has_path()) {
		if (path_speed >= 0) {
			if (path_current_node+1 < (int) path->get_coordinate_list().size()) {
				bee_path_coord c = path->get_coordinate_list().at(path_current_node+1);
				if (distance(get_x(), get_y(), path_xstart+std::get<0>(c), path_ystart+std::get<1>(c)) < get_speed()) {
					path_current_node++;
				}
			}
		} else {
			bee_path_coord c = path->get_coordinate_list().at(path_current_node);
			if (distance(get_x(), get_y(), path_xstart+std::get<0>(c), path_ystart+std::get<1>(c)) < get_speed()) {
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
int BEE::InstanceData::set_path_pausable(bool new_path_is_pausable) {
	path_is_pausable = new_path_is_pausable;
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
				set_position(path_xstart, path_ystart, 0.0);
				xprevious = path_xstart;
				yprevious = path_ystart;
				break;
			}
			case 2: { // Continue from current position
				path_current_node = 0;
				path_xstart = get_x();
				path_ystart = get_y();
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
	return (path != nullptr) ? true : false;
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
std::vector<bee_path_coord> BEE::InstanceData::get_path_coords() {
	std::vector<bee_path_coord> no_path;
	return (has_path()) ? path->get_coordinate_list() : no_path;
}
bool BEE::InstanceData::get_path_pausable() {
	return path_is_pausable;
}

int BEE::InstanceData::draw(int w, int h, double angle, RGBA color, SDL_RendererFlip flip) {
	if (object->get_sprite() == nullptr) {
		return 1;
	}
	int xo=0, yo=0;
	if (object->get_sprite() != nullptr) {
		std::tie(xo, yo) = object->get_mask_offset();
	}
	return object->get_sprite()->draw(get_corner_x()-xo, get_corner_y()-yo, subimage_time, w, h, angle, color, flip);
}
int BEE::InstanceData::draw(int w, int h, double angle, bee_rgba_t color, SDL_RendererFlip flip) {
	if (object->get_sprite() == nullptr) {
		return 1;
	}
	return draw(w, h, angle, game->get_enum_color(color), flip);
}
int BEE::InstanceData::draw() {
	if (object->get_sprite() == nullptr) {
		return 1;
	}
	int xo=0, yo=0;
	if (object->get_sprite() != nullptr) {
		std::tie(xo, yo) = object->get_mask_offset();
	}
	return object->get_sprite()->draw(get_corner_x()-xo, get_corner_y()-yo, subimage_time);
}
int BEE::InstanceData::draw(int w, int h) {
	if (object->get_sprite() == nullptr) {
		return 1;
	}
	int xo=0, yo=0;
	if (object->get_sprite() != nullptr) {
		std::tie(xo, yo) = object->get_mask_offset();
	}
	return object->get_sprite()->draw(get_corner_x()-xo, get_corner_y()-yo, subimage_time, w, h);
}
int BEE::InstanceData::draw(double angle) {
	if (object->get_sprite() == nullptr) {
		return 1;
	}
	int xo=0, yo=0;
	if (object->get_sprite() != nullptr) {
		std::tie(xo, yo) = object->get_mask_offset();
	}
	return object->get_sprite()->draw(get_corner_x()-xo, get_corner_y()-yo, subimage_time, angle);
}
int BEE::InstanceData::draw(RGBA color) {
	if (object->get_sprite() == nullptr) {
		return 1;
	}
	int xo=0, yo=0;
	if (object->get_sprite() != nullptr) {
		std::tie(xo, yo) = object->get_mask_offset();
	}
	return object->get_sprite()->draw(get_corner_x()-xo, get_corner_y()-yo, subimage_time, color);
}
int BEE::InstanceData::draw(bee_rgba_t color) {
	if (object->get_sprite() == nullptr) {
		return 1;
	}
	return draw(game->get_enum_color(color));
}
int BEE::InstanceData::draw(SDL_RendererFlip flip) {
	if (object->get_sprite() == nullptr) {
		return 1;
	}
	int xo=0, yo=0;
	if (object->get_sprite() != nullptr) {
		std::tie(xo, yo) = object->get_mask_offset();
	}
	return object->get_sprite()->draw(get_corner_x()-xo, get_corner_y()-yo, subimage_time, flip);
}

int BEE::InstanceData::draw_path() {
	if (path != nullptr) {
		return path->draw(path_xstart, path_ystart);
	}
	return 0;
}

#endif // _BEE_INSTANCEDATA
