#include "utility/launch_info.h"

#include "utility/globalconsts.hpp" // natural consts



// # LaunchInfo #
void LaunchInfo::setInfo_Window(int width, int height, Uint32 flag) {
	this->window_width = width;
	this->window_height = height;
	this->window_flag = flag;
}