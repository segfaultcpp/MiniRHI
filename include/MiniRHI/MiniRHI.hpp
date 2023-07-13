#pragma once

#ifndef ANDROID
#include <glew/glew.h>
#else
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>
#endif

#include "RenderCommands.hpp"
#include "Format.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "PipelineState.hpp"
#include "Shader.hpp"

namespace minirhi {
	void init();
}