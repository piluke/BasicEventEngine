/*
* Copyright (c) 2015-18 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARRANTY.
* See LICENSE for more details.
*/

#ifndef BEE_OBJ_UI_GAUGE
#define BEE_OBJ_UI_GAUGE 1

#include "../../../util.hpp"
#include "../../../all.hpp"

#include "obj_gauge.hpp"

ObjUIGauge::ObjUIGauge() : ObjUIElement("__obj_ui_gauge", "$/ui/obj_gauge.cpp") {
	implemented_events.insert({
		bee::E_EVENT::CREATE,
		bee::E_EVENT::DRAW
	});
}
void ObjUIGauge::create(bee::Instance* self) {
	ObjUIElement::create(self);

	_i("range") = 100;
	_i("value") = 0;
	_i("pulse_state") = 0;

	_c("color_back_r") = 0;
	_c("color_back_g") = 0;
	_c("color_back_b") = 0;
	_c("color_back_a") = 255;
}
void ObjUIGauge::draw(bee::Instance* self) {
	if (!_i("is_visible")) {
		return;
	}

	bee::render::set_is_lightable(false);

	int w = _i("w");
	int h = _i("h");

	bee::RGBA c_border = {0, 0, 0, 255};
	bee::RGBA c_back = {_c("color_back_r"), _c("color_back_g"), _c("color_back_b"), _c("color_back_a")};
	bee::RGBA c_front = {_c("color_r"), _c("color_g"), _c("color_b"), _c("color_a")};

	int ox = 0, oy = 0;
	const std::pair<const std::string,bee::ViewPort>* vp = bee::get_current_room()->get_current_viewport();
	if (vp != nullptr) {
		ox = vp->second.view.x;
		oy = vp->second.view.y;
	}

	int cx, cy;
	std::tie(cx, cy) = self->get_corner();

	bee::render::draw_rectangle(cx-ox, cy-oy, w, h, -1, c_back);

	if (_i("pulse_state") != 0) {
		int pulse_width = 20;
		bee::render::draw_rectangle(cx + w * _i("value") / _i("range") - ox, cy - oy, w * pulse_width / _i("range"), h, -1, c_front);

		int inc = _i("pulse_state") * 100.0 * bee::get_delta();
		if (
			(_i("pulse_state") > 0)
			&&(_i("value") + pulse_width + inc >= _i("range"))
		) { // If the pulse is increasing and has reached the maximum value, reverse the pulse
			if (_i("value") + pulse_width == _i("range")) {
				_i("pulse_state") = -1;
			} else {
				_i("value") = _i("range") - pulse_width;
			}
		} else if (
			(_i("pulse_state") < 0)
			&&(_i("value") + inc <= 0)
		) { // If the pulse is decreasing and has reached the minimum value, reverse the pulse
			if (_i("value") == 0) {
				_i("pulse_state") = 1;
			} else {
				_i("value") = 0;
			}
		} else { // Increment the pulse in its current direction
			_i("value") += inc;
		}
	} else {
		bee::render::draw_rectangle(cx-ox, cy-oy, w * _i("value") / _i("range"), h, -1, c_front);
	}

	bee::render::draw_rectangle(cx-ox, cy-oy, w, h, 2, c_border);

	bee::render::set_is_lightable(true);
}

int ObjUIGauge::get_range(bee::Instance* self) {
	return _i("range");
}
int ObjUIGauge::get_value(bee::Instance* self) {
	return _i("value");
}
void ObjUIGauge::set_range(bee::Instance* self, int new_range) {
	_i("range") = new_range;
}
void ObjUIGauge::set_value(bee::Instance* self, int new_value) {
	_i("value") = new_value;
}
void ObjUIGauge::start_pulse(bee::Instance* self) {
	_i("range") = 100;
	_i("value") = 0;
	_i("pulse_state") = 1;
}
void ObjUIGauge::stop_pulse(bee::Instance* self) {
	_i("value") = 0;
	_i("pulse_state") = 0;
}

void ObjUIGauge::set_color_back(bee::Instance* self, bee::RGBA new_color) {
	_c("color_back_r") = new_color.r;
	_c("color_back_g") = new_color.g;
	_c("color_back_b") = new_color.b;
	_c("color_back_a") = new_color.a;
}

#endif // BEE_OBJ_UI_GAUGE
