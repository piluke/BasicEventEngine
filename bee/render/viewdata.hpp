/*
* Copyright (c) 2015-17 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#ifndef BEE_RENDER_VIEWDATA_H
#define BEE_RENDER_VIEWDATA_H 1

namespace bee {
	class Instance;

	struct ViewData {
		bool is_visible;

		int view_x, view_y, view_width, view_height;
		int port_x, port_y, port_width, port_height;

		Instance* following;

		int horizontal_border, vertical_border;
		int horizontal_speed, vertical_speed;

		//ViewData();
	};
}

#endif // BEE_RENDER_VIEWDATA_H