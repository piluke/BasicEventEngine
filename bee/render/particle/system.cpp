/*
* Copyright (c) 2015-17 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#ifndef BEE_RENDER_PARTICLE_SYSTEM
#define BEE_RENDER_PARTICLE_SYSTEM 1

#include <algorithm>

#include "system.hpp"

#include "../../engine.hpp"

#include "../../util/real.hpp"
#include "../../util/collision.hpp"

#include "../../core/instance.hpp"

#include "../drawing.hpp"
#include "../render.hpp"
#include "particle.hpp"
#include "particledata.hpp"
#include "emitter.hpp"
#include "attractor.hpp"
#include "destroyer.hpp"
#include "deflector.hpp"
#include "changer.hpp"

#include "../../resources/sprite.hpp"

namespace bee {
	ParticleSystem::ParticleSystem() :
		following(nullptr),

		particle_types(),
		particles(),
		emitters(),
		attractors(),
		destroyers(),
		deflectors(),
		changers(),

		draw_data(),

		is_oldfirst(true),
		depth(0),

		xoffset(0.0),
		yoffset(0.0),

		time_offset(0)
	{}
	ParticleSystem::~ParticleSystem() {
		clear();

		for (auto& e : emitters) {
			delete e;
		}
		for (auto& a : attractors) {
			delete a;
		}
		for (auto& d : destroyers) {
			delete d;
		}
		for (auto& d : deflectors) {
			delete d;
		}
		for (auto& c : changers) {
			delete c;
		}
		for (auto& pt : particle_types) {
			delete pt;
		}

		emitters.clear();
		attractors.clear();
		destroyers.clear();
		deflectors.clear();
		changers.clear();
		particle_types.clear();
	}
	int ParticleSystem::load() {
		for (auto& p : particle_types) {
			if (p->get_sprite() != nullptr) {
				if (!p->get_sprite()->get_is_loaded()) {
					p->get_sprite()->load();
				}
			}
		}
		return 0;
	}

	int ParticleSystem::add_particle_type(Particle* new_type){
		particle_types.push_back(new_type);
		return 0;
	}
	int ParticleSystem::add_emitter(ParticleEmitter* new_emitter) {
		emitters.push_back(new_emitter);
		return 0;
	}
	int ParticleSystem::add_attractor(ParticleAttractor* new_attractor) {
		attractors.push_back(new_attractor);
		return 0;
	}
	int ParticleSystem::add_destroyer(ParticleDestroyer* new_destroyer) {
		destroyers.push_back(new_destroyer);
		return 0;
	}
	int ParticleSystem::add_deflector(ParticleDeflector* new_deflector) {
		deflectors.push_back(new_deflector);
		return 0;
	}
	int ParticleSystem::add_changer(ParticleChanger* new_changer) {
		changers.push_back(new_changer);
		return 0;
	}

	int ParticleSystem::set_following(Instance* inst) {
		following = inst;
		return 0;
	}

	int ParticleSystem::fast_forward(int frames) { // Fast-forwading more than 500 milliseconds is not recommended due to how long it takes to calculate
		int t = get_ticks();
		int step = 1000.0/get_fps_goal();
		for (int i=0; i<frames; i++) {
			time_offset += step;
			draw(t+time_offset, 1.0/step, false);
		}

		particles.erase(
			std::remove_if(
				particles.begin(),
				particles.end(),
				[this, &t] (ParticleData* p) -> bool {
					if (p->get_is_old()){
						return true;
					}

					Uint32 ticks = t+time_offset - p->get_creation();
					if (t+time_offset < p->get_creation()) {
						ticks = 0;
					}

					if (p->is_dead(ticks)) {
						p->get_type()->on_death(this, p);
						return true;
					}
					return false;
				}
			),
			particles.end()
		);

		return 0;
	}
	int ParticleSystem::draw(Uint32 now, double delta, bool should_draw) {
		int system_x = xoffset, system_y = yoffset;
		if (following != nullptr) {
			system_x += following->get_x();
			system_y += following->get_y();
		}

		for (auto& pd : particles) {
			if (pd->get_is_old()){
				continue;
			}

			for (auto& d : destroyers) {
				if (d->handle(pd, system_x, system_y)) {
					pd->get_type()->on_death(this, pd);
					break;
				}
			}
			if (pd->get_is_old()){
				continue;
			}

			for (auto& c : changers) {
				c->handle(pd, system_x, system_y);
			}

			double old_px = pd->x, old_py = pd->y;
			pd->move(delta);

			for (auto& a : attractors) {
				a->handle(pd, system_x, system_y, delta);
			}

			for (auto& d : deflectors) {
				d->handle(pd, old_px, old_py, system_x, system_y);
			}
		}

		for (auto& e : emitters) {
			e->handle(this, system_x, system_y);
		}

		if (should_draw) {
			clear_draw_data();
			particles.remove_if(
				[this, &now] (ParticleData* p) -> bool {
					if (p->get_is_old()) {
						return true;
					}

					Uint32 ticks = now - p->get_creation();
					if (now < p->get_creation()) {
						ticks = 0;
					}

					int x = xoffset + p->x - p->get_w()/2;
					int y = yoffset + p->y - p->get_h()/2;
					if (following != nullptr) {
						x += following->get_x();
						y += following->get_y();
					}

					SpriteDrawData* sdd = new SpriteDrawData(x, y, p->get_creation(), p->get_w(), p->get_h(), absolute_angle(p->get_angle(ticks)));
					draw_data[p->get_type()].push_back(sdd);
					//p->draw(system_x, system_y, ticks);

					if (p->is_dead(ticks)) {
						p->get_type()->on_death(this, p);
						return true;
					}
					return false;
				}
			);
			for (auto& s : draw_data) {
				if (!s.first->is_lightable) {
					set_is_lightable(false);
				}

				bool is_sprite_lightable = s.first->get_sprite()->get_is_lightable();
				if (!s.first->is_sprite_lightable) {
					s.first->get_sprite()->set_is_lightable(false);
				}

				s.first->get_sprite()->draw_array(s.second, s.first->rotation_cache, s.first->color, SDL_FLIP_NONE);

				set_is_lightable(true);
				s.first->get_sprite()->set_is_lightable(is_sprite_lightable);
			}
		}

		return 0;
	}
	int ParticleSystem::draw() {
		return draw(get_ticks()+time_offset, get_delta(), true);
	}
	int ParticleSystem::draw_debug() {
		int system_x = xoffset, system_y = yoffset;
		if (following != nullptr) {
			system_x += following->get_x();
			system_y += following->get_y();
		}

		for (auto& p : particles) {
			p->draw_debug(system_x, system_y, E_RGB::AQUA);
		}
		for (auto& e : emitters) {
			e->draw_debug(system_x, system_y, E_RGB::GREEN);
		}
		for (auto& a : attractors) {
			a->draw_debug(system_x, system_y, E_RGB::MAGENTA);
		}
		for (auto& d : destroyers) {
			d->draw_debug(system_x, system_y, E_RGB::RED);
		}
		for (auto& d : deflectors) {
			d->draw_debug(system_x, system_y, E_RGB::NAVY);
		}
		for (auto& c : changers) {
			c->draw_debug(system_x, system_y, E_RGB::YELLOW);
		}

		return 0;
	}
	int ParticleSystem::clear_draw_data() {
		for (auto& p : draw_data) {
			for (auto& sdd : p.second) {
				delete sdd;
			}
		}
		draw_data.clear();
		return 0;
	}
	int ParticleSystem::clear() {
		for (auto& pt : particle_types) {
			pt->remove_old_particles();
		}

		for (auto& p : particles) {
			delete p;
		}

		particles.clear();
		clear_draw_data();

		return 0;
	}

	int ParticleSystem::add_particle(Particle* p, int x, int y) {
		ParticleData* pd = p->reuse_particle(x, y, get_ticks()+time_offset);
		particles.push_back(pd);
		return 0;
	}
}

#endif // BEE_RENDER_PARTICLE_SYSTEM
