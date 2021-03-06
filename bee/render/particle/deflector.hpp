/*
* Copyright (c) 2015-17 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#ifndef BEE_RENDER_PARTICLE_DEFLECTOR_H
#define BEE_RENDER_PARTICLE_DEFLECTOR_H 1

#include "../../enum.hpp"

namespace bee {
	// Forward declaration
	class Instance;
	class ParticleData;

	class ParticleDeflector {
			Instance* following;

			double x, y;
			unsigned int w, h;

			double friction;

			double get_following_x(double);
			double get_following_y(double);
		public:
			ParticleDeflector();
			ParticleDeflector(double, double, unsigned int, unsigned int);

			int set_following(Instance*);
			int set_friction(double);

			int handle(ParticleData*, double, double, double, double);

			int draw_debug(double, double, E_RGB);
	};
}

#endif // BEE_RENDER_PARTICLE_DEFLECTOR_H
