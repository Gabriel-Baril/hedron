
#pragma once

#include "hdn_device.h"
#include "hdn_window.h"
#include "hdn_game_object.h"
#include "hdn_renderer.h"
#include "hdn_descriptors.h"

#include "physics/physics_world.h"
#include "flecs/flecs.h"

namespace hdn
{

	class FirstApp
	{
	public:
		static constexpr u32 WIDTH = 800;
		static constexpr u32 HEIGHT = 600;
	public:
		FirstApp();
		virtual ~FirstApp();
		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void Run();
	private:
		void LoadGameObjects();
	private:

		HDNWindow m_Window{ WIDTH, HEIGHT, "First App"};
		HDNDevice m_Device{ m_Window };
		HDNRenderer m_Renderer{ &m_Window, &m_Device };

		// Order of declarations matters
		Scope<HDNDescriptorPool> globalPool{}; // System that require descriptors only relevant to their work should create their own HDNDescriptorPool

		PhysicsWorld m_PhysicsWorld;

		flecs::world m_EcsWorld;
	};
}