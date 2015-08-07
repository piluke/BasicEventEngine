/*
* Copyright (c) 2015 Luke Montalvo <lukemontalvo@gmail.com>
*
* This file is part of BEE.
* BEE is free software and comes with ABSOLUTELY NO WARANTY.
* See LICENSE for more details.
*/

#ifndef _BEE_UTIL_STRING_H
#define _BEE_UTIL_STRING_H 1

// String handling functions

#include <string>
#include <algorithm>
#include <regex>
#include <map>

std::string chr(int c) {
        char character = c;
        return std::to_string(character);
}

template <typename T>
int ord(T s) {
        std::string str = std::to_string(s);
        return str[0];
}

std::string string_lower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
}
std::string string_upper(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
}

std::string string_letters(std::string str){
        return std::regex_replace(str, std::regex("[^[:alnum:]]"), "");
}
std::string string_digits(std::string str){
        return std::regex_replace(str, std::regex("[^[:digit:]]"), "");
}
std::string string_lettersdigits(std::string str){
        return std::regex_replace(str, std::regex("[^[:alnum:][:digit:]]"), "");
}

std::map<int,std::string> handle_newlines(std::string input) {
	std::map<int,std::string> output;
	if (input.size() > 0) {
		std::istringstream input_stream (input);
		while (!input_stream.eof()) {
			std::string tmp;
			getline(input_stream, tmp);
			output.insert(std::make_pair(output.size(), tmp));
		}
	}
	return output;
}

bool clipboard_has_text() {
        return (SDL_HasClipboardText() == SDL_TRUE) ? true : false;
}
std::string clipboard_get_text() {
        char* cstr = SDL_GetClipboardText();
        std::string s (cstr);
        free(cstr);
        return s;
}
int clipboard_set_text(std::string str) {
        return SDL_SetClipboardText(str.c_str());
}

#endif // _BEE_UTIL_STRING_H