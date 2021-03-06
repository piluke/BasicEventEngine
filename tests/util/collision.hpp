/*
* Copyright (c) 2015-17 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#ifndef TESTS_UTIL_COLLISION
#define TESTS_UTIL_COLLISION 1

#include "doctest.h" // Include the required unit testing library

#include "../../bee/util/collision.hpp"

TEST_SUITE_BEGIN("util");

TEST_CASE("collision/rect") {
	SDL_Rect a = {0, 0, 10, 10};
	SDL_Rect b = {5, 5, 10, 20};
	REQUIRE(check_collision(a, b) == true);
}
TEST_CASE("collision/circle") {
	REQUIRE(check_collision_circle(0, 0, 5, 0, 7, 3) == true);
}
TEST_CASE("collision/line") {
	Line l1 = {0, 0, 5, 10};
	Line l2 = {5, 5, 0, 10};
	REQUIRE(check_collision_line(l1, l2) == true);

	l1 = {0, 0, 10, 0};
	l2 = {5, 5, 5, -5};
	REQUIRE(check_collision_line(l1, l2) == true);
}
TEST_CASE("collision/bounce") {
	REQUIRE(angle_hbounce(60.0) == 120.0);
	REQUIRE(angle_hbounce(120.0) == 60.0);

	REQUIRE(angle_vbounce(60.0) == 300.0);
	REQUIRE(angle_vbounce(300.0) == 60.0);
}

TEST_SUITE_END();

#endif // TESTS_UTIL_COLLISION
