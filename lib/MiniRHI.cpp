#include "MiniRHI/MiniRHI.hpp"

void minirhi::init() {
	glewExperimental = true;
	glewInit();
}
