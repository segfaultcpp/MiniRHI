#include "cnfg/os_generic.h"
#include <GLES3/gl3.h>
#include "glue.hpp"
#include "cnfg/CNFGAndroid.h"

#include <new>

#define CNFG3D
#define CNFG_IMPLEMENTATION
#include "cnfg/CNFG.h"

#include "Core/Core.hpp"


#include "MiniRHI/Buffer.hpp"
#include "MiniRHI/Format.hpp"
#include "MiniRHI/MiniRHI.hpp"

#include "MiniRHI/PipelineState.hpp"
#include "MiniRHI/RenderCommands.hpp"
#include "MiniRHI/Shader.hpp"
#include "MiniRHI/TypeInference.hpp"


void HandleKey( int keycode, int bDown )
{
	if( keycode == 4 ) { AndroidSendToBack( 1 ); } //Handle Physical Back Button.
}

void HandleDestroy()
{
	printf( "Destroying\n" );
	exit(10);
}

volatile int suspended;

void HandleSuspend()
{
	suspended = 1;
}

void HandleResume()
{
	suspended = 0;
}

void HandleButton( int x, int y, int button, int bDown )
{
}

void HandleMotion( int x, int y, int mask )
{
}

extern int android_width, android_height;

int main(int argc, char ** argv)
{
	CNFGBGColor = 0x000040ff;
	CNFGSetupFullscreen( "Test Bench", 0 );

	struct Vertex {
		std::array<f32, 2> position;
		std::array<f32, 3> color;
	};

	using Attrs = minirhi::MakeVertexAttributes<Vertex>;

	static constexpr FixedString kVS = R"str(
#version 300 es
layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

out vec3 vert_color;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vert_color = color;
})str";

	static constexpr FixedString kFS = R"str(
#version 300 es

in vec3 vert_color;
out  vec4 frag_color;

void main() {
    frag_color = vec4(vert_color, 1.0);
}
)str";

	auto pipeline = minirhi::generate_pipeline_from_shaders<kVS, kFS>(
			minirhi::PrimitiveTopologyType::eTriangle,
			minirhi::RasterizerStateDesc{}
	);

	static constexpr std::array vertices = {
			Vertex { {-1.f, -1.f}, {1.f, 0.f, 0.f} },
			Vertex { {0.f, -1.f}, {0.f, 1.f, 0.f} },
			Vertex { {-0.5f, 1.f}, {0.f, 0.f, 1.f} },

			Vertex { {0.f, 1.f}, {1.f, 1.f, 0.f} },
			Vertex { {1.f, 1.f}, {0.f, 1.f, 1.f} },
			Vertex { {0.5f, -1.f}, {1.f, 0.f, 1.f} },
	};

	static constexpr std::array indexed_vertices = {
			Vertex { {-0.5f, -0.5f}, {1.f, 0.f, 0.f} },
			Vertex { {0.5f, -0.5f}, {0.f, 1.f, 0.f} },
			Vertex { {-0.5f, 0.5f}, {0.f, 0.f, 1.f} },
			Vertex { {0.5f, 0.5f}, {1.f, 1.f, 0.f} },
	};

	static constexpr std::array indices = {
			u32(0), u32(1), u32(2), u32(2), u32(1), u32(3)
	};

	auto vb = minirhi::make_vertex_buffer_rc(std::span<const Vertex>(vertices.begin(), vertices.end()));
	auto indexed_vb = minirhi::make_vertex_buffer_rc(std::span<const Vertex>(indexed_vertices.begin(), indexed_vertices.end()));
	auto ib = minirhi::make_index_buffer_rc(std::span<const u32>(indices.begin(), indices.end()));

	minirhi::RenderCommands cmd;
	minirhi::Viewport vp{ static_cast<size_t>(android_width), static_cast<size_t>(android_height) };

	auto draw_params = minirhi::make_draw_params(vp, pipeline, vb);
	auto indexed_draw_params = minirhi::make_draw_params_indexed(vp, pipeline, indexed_vb, ib);

	while(1)
	{
		CNFGHandleInput();
		if (suspended)
		{
			usleep(50000);
			continue;
		}

		CNFGClearFrame();

		cmd.clear_color_buffer(1.0, 0.0, 0.0, 0.0);
		cmd.draw_indexed(indexed_draw_params, indices.size(), 0);

		CNFGFlushRender();
		CNFGSwapBuffers();

	}
}
