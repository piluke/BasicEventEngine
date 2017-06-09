/*
* Copyright (c) 2015-17 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#ifndef BEE_PATH
#define BEE_PATH 1

#include <sstream>

#include "path.hpp" // Include the class resource header

#include "../debug.hpp"
#include "../engine.hpp"

#include "../util/string.hpp"
#include "../util/platform.hpp"

#include "../render/rgba.hpp"

namespace bee {
	/*
	* Path::Path() - Construct the path and set its engine pointer
	* ! This constructor should only be directly used for temporary paths, the other constructor should be used for all other cases
	*/
	Path::Path () :
		id(-1),
		name(),
		path(),
		coordinate_list(),
		is_curved(false),
		is_closed(true)
	{}
	/*
	* Path::Path() - Construct the path, reset all variables, add it to the path resource list, and set the new name and path
	* @new_name: the name of the path to use
	* @new_path: the path filename of the path's config file
	*/
	Path::Path (const std::string& new_name, const std::string& new_path) :
		Path() // Default initialize all variables
	{
		add_to_resources(); // Add the path to the appropriate resource list
		if (id < 0) { // If the path could not be added to the resource list, output a warning
			messenger_send({"engine", "resource"}, E_MESSAGE::WARNING, "Failed to add path resource: \"" + new_name + "\" from " + new_path);
			throw(-1); // Throw an exception
		}

		set_name(new_name); // Set the path name
		set_path(new_path); // Set the config file path
	}
	/*
	* Path::~Path() - Remove the path from the resource list
	*/
	Path::~Path() {
		resource_list->paths.remove_resource(id);
	}
	/*
	* Path::add_to_resources() - Add the path to the appropriate resource list
	*/
	int Path::add_to_resources() {
		if (id < 0) { // If the resource needs to be added to the resource list
			id = resource_list->paths.add_resource(this); // Add the resource and get the new id
		}

		return 0; // Return 0 on success
	}
	/*
	* Path::reset() - Reset all resource variables for reinitialization
	*/
	int Path::reset() {
		// Reset all properties
		name = "";
		path = "";
		coordinate_list.clear();
		is_curved = false;
		is_closed = true;

		return 0; // Return 0 on success
	}
	/*
	* Path::print() - Print all relevant information about the resource
	*/
	int Path::print() const {
		std::string coordinate_string = get_coordinate_string(); // Get the list of coordinates in string form

		std::stringstream s; // Declare the output stream
		s << // Append all info to the ouput
		"Path { "
		"\n	id              " << id <<
		"\n	name            " << name <<
		"\n	path            " << path <<
		"\n	coordinate_list\n" << debug_indent(coordinate_string, 2) <<
		"	is_curved	" << is_curved <<
		"\n	is_closed       " << is_closed <<
		"\n}\n";
		messenger_send({"engine", "resource"}, E_MESSAGE::INFO, s.str()); // Send the info to the messaging system for output

		return 0; // Return 0 on success
	}

	/*
	* Path::get_*() - Return the requested resource information
	*/
	int Path::get_id() const {
		return id;
	}
	std::string Path::get_name() const {
		return name;
	}
	std::string Path::get_path() const {
		return path;
	}
	std::vector<path_coord_t> Path::get_coordinate_list() const {
		return coordinate_list;
	}
	std::string Path::get_coordinate_string() const {
		if (coordinate_list.size() == 0) { // If there are no coordinates in the list, return a none-string
			return "none\n";
		}

		std::vector<std::vector<std::string>> table; // Declare a table to hold the coordinates
		table.push_back({"(x", "y", "z", "speed)"}); // Append the table header

		for (auto it = coordinate_list.begin(); it != coordinate_list.end(); ++it) { // Iterate over the coordinates and add each of them to the table
			table.push_back({bee_itos(std::get<0>(*it)), bee_itos(std::get<1>(*it)), bee_itos(std::get<2>(*it)), bee_itos(std::get<3>(*it))});
		}

		return string_tabulate(table); // Return the table as a properly spaced string
	}
	bool Path::get_is_curved() const {
		return is_curved;
	}
	bool Path::get_is_closed() const {
		return is_closed;
	}

	/*
	* Path::set_name() - Set the resource name
	* @new_name: the new name to use for the resource
	*/
	int Path::set_name(const std::string& new_name) {
		name = new_name; // Set the name
		return 0; // Return 0 on success
	}
	/*
	* Path::set_path() - Set the resource path
	* @new_path: the new file path to use for the resource
	*/
	int Path::set_path(const std::string& new_path) {
		path = "resources/paths/"+new_path; // Append the filename to the path directory
		return 0; // Return 0 on success
	}
	/*
	* Path::set_is_curved() - Set whether the path should curve
	* @new_is_curved: the new curvature to use
	*/
	int Path::set_is_curved(bool new_is_curved) {
		is_curved = new_is_curved; // Set the curvature
		return 0; // Return 0 on success
	}
	/*
	* Path::set_is_closed() - Set whether the path should loop after the end
	* @new_is_closed: the new closure to use
	*/
	int Path::set_is_closed(bool new_is_closed) {
		is_closed = new_is_closed; // Set the closure
		return 0; // Return 0 on success
	}
	/*
	* Path::add_coordinate() - Add a coordinate to the end of the coordinate list
	* @new_coordinate: the new coordinate to add
	*/
	int Path::add_coordinate(path_coord_t new_coordinate) {
		coordinate_list.push_back(new_coordinate); // Add the coordinate
		return 0; // Return 0 on success
	}
	/*
	* Path::add_coordinate() - Add the given coordinate and speed to the coordinate list
	* @x: the x-coordinate of the desired point
	* @y: the y-coordinate of the desired point
	* @z: the z-coordinate of the desired point
	* @speed: the speed at the desired point
	*/
	int Path::add_coordinate(double x, double y, double z, double speed) {
		return add_coordinate(path_coord_t(x, y, z, speed)); // Construct the path coordinate and return the attempt to add it
	}
	/*
	* Path::add_coordinate() - Add the given coordinate and speed to the coordinate list
	* ! When the function is called without a z-coordinate, simply call it with z=0.0
	* @x: the x-coordinate of the desired point
	* @y: the y-coordinate of the desired point
	* @speed: the speed at the desired point
	*/
	int Path::add_coordinate(double x, double y, double speed) {
		return add_coordinate(x, y, 0.0, speed);
	}
	/*
	* Path::remove_last_coordinate() - Remove the last coordinate from the list
	*/
	int Path::remove_last_coordinate() {
		if (coordinate_list.empty()) { // Do not attempt to remove the coordinate if the list is empty
			return 1; // Return 1 when the list is empty
		}

		coordinate_list.pop_back(); // Remove the element

		return 0; // Return 0 on success
	}
	/*
	* Path::remove_coordinate() - Remove the coordinate at the given index
	* @index: the index of the element to remove
	*/
	int Path::remove_coordinate(unsigned int index) {
		if (index >= coordinate_list.size()) { // Do not attempt to remove the coordinate if the index is not in the list
			return 1;
		}

		coordinate_list.erase(coordinate_list.begin()+index); // Erase the element at the given index

		return 0; // Return 0 on success
	}

	/*
	* Path::draw() - Draw the path for debugging purposes
	* @xstart: the x-coordinate to offset the path by
	* @ystart: the y-coordinate to offset the path by
	* @zstart: the z-coordinate to offset the path by
	*/
	int Path::draw(double xstart, double ystart, double zstart) {
		RGBA c_line = get_enum_color(E_RGB::AQUA); // Define the drawing color
		glm::vec3 vs (xstart, ystart, zstart); // Define the start point
		glm::vec3 v1, v2; // Declare two vectors to use for the start and end points of each line

		for (auto it = coordinate_list.begin(); it != --coordinate_list.end(); ) { // Iterate over the coordinates
			v1 = glm::vec3(std::get<0>(*it), std::get<1>(*it), std::get<2>(*it)); // Get the start point from the current coordinate
			++it; // Move to the next coordinate
			v2 = glm::vec3(std::get<0>(*it), std::get<1>(*it), std::get<2>(*it)); // Get the end point from the next coordinate

			draw_line(v1+vs, v2+vs, c_line); // Draw the line with the start point added
		}

		return 0; // Return 0 on success
	}
	/*
	* Path::draw() - Draw the path for debugging purposes
	* ! When the zstart is not provided, simply draw it with it set to 0.0
	* @xstart: the x-coordinate to offset the path by
	* @ystart: the y-coordinate to offset the path by
	*/
	int Path::draw(double xstart, double ystart) {
		return draw(xstart, ystart, 0.0); // Return the draw call with no zstart
	}
}

#endif // BEE_PATH