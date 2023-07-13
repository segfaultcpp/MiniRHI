#include "MiniRHI/MiniRHI.hpp"

void minirhi::init() {
#ifndef ANDROID
	glewExperimental = true;
	glewInit();
#endif
}
