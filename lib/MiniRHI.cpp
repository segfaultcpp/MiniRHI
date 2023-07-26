#include "MiniRHI/MiniRHI.hpp"
#include <Core/Core.hpp>
#include <limits>

namespace minirhi {
	u32 gDefaultVAO = std::numeric_limits<u32>::max();

	void init() {
	#ifndef ANDROID
		glewExperimental = true;
		glewInit();
		glGenVertexArrays(1, &gDefaultVAO);
	#endif
	}
}