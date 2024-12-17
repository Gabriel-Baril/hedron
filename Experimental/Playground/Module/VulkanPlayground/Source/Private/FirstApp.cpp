#include "VulkanPlayground/FirstApp.h"

#include "Core/Core.h"

#include <stdexcept>
#include <array>

namespace hdn
{
	struct SimplePushConstantData
	{
		vec2f32 offset;
		alignas(16) vec3f32 color;
	};

	FirstApp::FirstApp()
	{
		LoadModels();
		CreatePipelineLayout();
		RecreateSwapchain();
		CreateCommandBuffers();
	}

	FirstApp::~FirstApp()
	{
		vkDestroyPipelineLayout(m_Device.device(), m_PipelineLayout, nullptr);
	}

	void FirstApp::Run()
	{
		while (!m_Window.ShouldClose())
		{
			glfwPollEvents();
			DrawFrame();
		}

		vkDeviceWaitIdle(m_Device.device());
	}

	void FirstApp::LoadModels()
	{
		std::vector<HDNModel::Vertex> vertices{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		m_Model = std::make_unique<HDNModel>(&m_Device, vertices);
	}

	void FirstApp::CreatePipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr; // Used to pass data other than our vertex data, to our vertex and fragment shader. For example, texture and uniform buffer.
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // A way to push a very small amount of data to our shader
		if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}
	}

	void FirstApp::CreatePipeline()
	{
		assert(m_Swapchain);
		assert(m_PipelineLayout);

		PipelineConfigInfo pipelineConfig{};
		HDNPipeline::DefaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = m_Swapchain->getRenderPass(); // A render pass describe the structure and format of our framebuffer objects and their attachments
		pipelineConfig.pipelineLayout = m_PipelineLayout;
		m_Pipeline = std::make_unique<HDNPipeline>(&m_Device, "Shaders/simple_shader.vert.spv", "Shaders/simple_shader.frag.spv", pipelineConfig);
	}

	void FirstApp::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(m_Swapchain->imageCount()); // Likely 2 (double buffering) or 3 (triple buffering)
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Primary = Can be submitted to a queue for execution, but cannot be called by other command buffer
		allocInfo.commandPool = m_Device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32>(m_CommandBuffers.size());
		if (vkAllocateCommandBuffers(m_Device.device(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffer");
		}

	}

	void FirstApp::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(m_Device.device(), m_Device.getCommandPool(), static_cast<uint32>(m_CommandBuffers.size()), m_CommandBuffers.data());
		m_CommandBuffers.clear();
	}

	void FirstApp::DrawFrame()
	{
		uint32 imageIndex;
		auto result = m_Swapchain->acquireNextImage(&imageIndex); // Fetch the index of the next image to be used for rendering

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapchain();
			return;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire the next swapchain image");
		}

		RecordCommandBuffer(imageIndex);
		result = m_Swapchain->submitCommandBuffers(&m_CommandBuffers.at(imageIndex), &imageIndex); // It will submit command buffer to the device graphics queue while handling cpu and gpu synchronization

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.WasWindowResized())
		{
			m_Window.ResetWindowResizedFlag();
			RecreateSwapchain();
			return;
		}

		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swapchain image");
		}
	}

	void FirstApp::RecreateSwapchain()
	{
		auto extent = m_Window.GetExtent(); // Get the current window size
		while (extent.width == 0 || extent.height == 0)
		{
			extent = m_Window.GetExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_Device.device()); // This is an easy way that we can wait until the current swapchain is no longer being used

		if (m_Swapchain == nullptr)
		{
			m_Swapchain = std::make_unique<HDNSwapChain>(m_Device, extent);
		}
		else
		{
			m_Swapchain = std::make_unique<HDNSwapChain>(m_Device, extent, std::move(m_Swapchain));
			if (m_Swapchain->imageCount() != m_CommandBuffers.size())
			{
				FreeCommandBuffers();
				CreateCommandBuffers();
			}
		}

		// If render pass compatible do nothing else
		CreatePipeline();
	}

	void FirstApp::RecordCommandBuffer(int imageIndex)
	{
		static int frame = 0;

		VkCommandBuffer& currentCommandBuffer = m_CommandBuffers.at(imageIndex);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(currentCommandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_Swapchain->getRenderPass();
		renderPassInfo.framebuffer = m_Swapchain->getFrameBuffer(imageIndex);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_Swapchain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{}; // Control what should be the initial value of our attachment
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 0.1f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(currentCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // VK_SUBPASS_CONTENTS_INLINE = Signals that the subsequent render pass command will be directly embedded in the primary command buffer itself
		
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_Swapchain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(m_Swapchain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor({0, 0}, m_Swapchain->getSwapChainExtent());
		vkCmdSetViewport(currentCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(currentCommandBuffer, 0, 1, &scissor);

		m_Pipeline->Bind(currentCommandBuffer);
		m_Model->Bind(currentCommandBuffer);
		for (int j = 0; j < 4; j++)
		{
			SimplePushConstantData push{};
			push.offset = { 0.0f + glm::clamp<float>(sinf(frame * 0.001f + j * 0.2f) / glm::pi<float>(), -1, 1), -0.4f + j * 0.25f};
			push.color = { 0.0f, glm::clamp<float>(sinf(frame * 0.001f + j * 0.2f) / glm::pi<float>(), -1, 1), 0.2f + j * 0.2f };
			vkCmdPushConstants(currentCommandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
			m_Model->Draw(currentCommandBuffer);
		}

		vkCmdEndRenderPass(currentCommandBuffer);

		if (vkEndCommandBuffer(currentCommandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to end command buffer");
		}
		frame++;
	}
}
