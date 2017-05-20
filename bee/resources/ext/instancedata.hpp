/*
* Copyright (c) 2015-17 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#ifndef _BEE_INSTANCEDATA_H
#define _BEE_INSTANCEDATA_H 1

#define ALARM_COUNT 8

#ifndef _BEE_ROOM_H
#define _BEE_ROOM_H
#define _BEE_ROOM_OPERATOR 1
#endif // _BEE_ROOM

#include <btBulletDynamicsCommon.h>

#include "../../game.hpp"

class BEE::InstanceData {
		btVector3 pos_start;

		Object* object = nullptr;
		Sprite* sprite = nullptr;

		PhysicsBody* body = nullptr;
		bool is_solid = false;

		Path* path = nullptr;
		double path_speed = 0.0;
		bee_path_end_t path_end_action = BEE_PATH_END_STOP;
		int path_current_node = 0;
		bool path_is_drawn = false;
		bool path_is_pausable = false;
		double path_previous_mass = 0.0;
	public:
		BEE* game = nullptr;

		int id = -1;
		Uint32 subimage_time = 0;
		Uint32 alarm_end[ALARM_COUNT];
		int depth = 0;

		btVector3 pos_previous;
		btVector3 path_pos_start;

		InstanceData();
		InstanceData(BEE*, int, Object*, double, double, double);
		~InstanceData();
		int init(int, Object*, double, double, double);
		int print();

		std::string serialize(bool) const;
		std::string serialize() const;
		int deserialize(const std::string&);

		bool operator< (const InstanceData& other) const;

		int remove();

		int set_alarm(int, Uint32);

		int set_object(Object*);
		int set_sprite(Sprite*);
		int add_physbody();

		SIDP get_data(const std::string&) const;
		int set_data(const std::string&, SIDP);

		btVector3 get_position() const;
		double get_x() const;
		double get_y() const;
		double get_z() const;
		double get_corner_x() const;
		double get_corner_y() const;
		btVector3 get_start() const;
		double get_xstart() const;
		double get_ystart() const;
		double get_zstart() const;

		Object* get_object() const;
		Sprite* get_sprite() const;
		PhysicsBody* get_physbody() const;
		bool get_is_solid() const;
		double get_mass() const;

		int get_width() const;
		int get_height() const;
		SDL_Rect get_aabb() const;

		int set_position(btVector3);
		int set_position(double, double, double);
		int set_to_start();
		int set_mass(double);
		int move(btVector3);
		int move(double, btVector3);
		int move(double, double);
		int move_to(double, double, double, double);
		int move_to(double, double, double);
		int move_away(double, double, double, double);
		int move_away(double, double, double);
		int set_friction(double);
		int set_gravity(btVector3);
		int set_gravity(double, double, double);
		int move_outside(btVector3);
		int set_is_solid(bool);
		int set_velocity(btVector3);
		int set_velocity(double, double);
		int limit_velocity(double);

		double get_speed() const;
		btVector3 get_velocity() const;
		btVector3 get_velocity_ang() const;
		double get_friction() const;
		btVector3 get_gravity() const;

		bool is_place_free(int, int) const;
		bool is_place_empty(int, int) const;
		bool is_place_meeting(int, int, Object*) const;
		bool is_place_meeting(int, int, int) const;
		bool is_place_meeting(int, int, Object*, std::function<void(InstanceData*,InstanceData*)>);
		bool is_move_free(double, double);
		bool is_snapped(int, int) const;

		std::pair<int,int> get_snapped(int, int) const;
		std::pair<int,int> get_snapped() const;
		int move_random(int, int);
		int move_snap(int, int);
		int move_snap();
		int move_wrap(bool, bool, int);

		double get_distance(int, int, int) const;
		double get_distance(InstanceData*) const;
		double get_distance(Object*) const;
		double get_direction_of(int, int) const;
		double get_direction_of(InstanceData*) const;
		double get_direction_of(Object*) const;
		int get_relation(InstanceData*) const;

		int path_start(Path*, double, bee_path_end_t, bool);
		int path_end();
		int path_reset();
		int path_update_node();
		int set_path_drawn(bool);
		int set_path_pausable(bool);
		int handle_path_end();
		bool has_path();
		bool get_path_drawn();
		int get_path_speed();
		int get_path_node();
		std::vector<bee_path_coord> get_path_coords();
		bool get_path_pausable();

		int draw(int, int, double, RGBA, SDL_RendererFlip);
		int draw(int, int, double, bee_rgba_t, SDL_RendererFlip);
		int draw();
		int draw(int, int);
		int draw(double);
		int draw(RGBA);
		int draw(bee_rgba_t);
		int draw(SDL_RendererFlip);

		int draw_path();
};

#ifdef _BEE_ROOM_OPERATOR
#undef _BEE_ROOM_H
#undef _BEE_ROOM_OPERATOR
#include "../room.hpp"
#endif // _BEE_ROOM_OPERATOR

#endif // _BEE_INSTANCEDATA_H
