#include "VulkanPlayground/HDNModel.h"

#include "VulkanPlayground/HDNUtils.h"

#include <tinyobjloader/tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <unordered_map>

namespace std
{
	template<>
	struct hash<hdn::HDNModel::Vertex>
	{
		size_t operator()(const hdn::HDNModel::Vertex& vertex) const
		{
			size_t seed = 0;
			hdn::HashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace hdn
{
	HDNModel::HDNModel(HDNDevice* device, const HDNModel::Builder& builder)
		: m_Device{device}
	{
		CreateVertexBuffers(builder.vertices);
		CreateIndexBuffers(builder.indices);
	}


	HDNModel::~HDNModel()
	{
		vkDestroyBuffer(m_Device->device(), m_VertexBuffer, nullptr);
		vkFreeMemory(m_Device->device(), m_VertexBufferMemory, nullptr);

		if (m_HasIndexBuffer)
		{
			vkDestroyBuffer(m_Device->device(), m_IndexBuffer, nullptr);
			vkFreeMemory(m_Device->device(), m_IndexBufferMemory, nullptr);
		}
	}

	Scope<HDNModel> HDNModel::CreateModelFromFile(HDNDevice* device, const std::string& filepath)
	{
		Builder builder{};
		builder.LoadModel(filepath);
		// HDN_CORE_INFO("Vertex Count: {0}", builder.vertices.size());
		return CreateScope<HDNModel>(device, builder);
	}

	void HDNModel::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { m_VertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		if (m_HasIndexBuffer)
		{
			vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32); // The index type could be smaller based on the model
		}
	}

	void HDNModel::Draw(VkCommandBuffer commandBuffer)
	{
		if (m_HasIndexBuffer)
		{
			vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
		}
	}

	void HDNModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_VertexCount = static_cast<uint32>(vertices.size());
		assert(m_VertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_Device->createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // The buffer is going to transfer memory
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // Host = CPU, Device = GPU
			stagingBuffer,
			stagingBufferMemory);

		// Most host memory to device memory
		void* data;
		vkMapMemory(m_Device->device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(m_Device->device(), stagingBufferMemory);

		m_Device->createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, // Tell that we want to create a vertex buffer
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // Host = CPU, Device = GPU
			m_VertexBuffer,
			m_VertexBufferMemory);
		
		m_Device->copyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

		vkDestroyBuffer(m_Device->device(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device->device(), stagingBufferMemory, nullptr);
	}

	void HDNModel::CreateIndexBuffers(const std::vector<uint32>& indices)
	{
		m_IndexCount = static_cast<uint32>(indices.size());
		m_HasIndexBuffer = m_IndexCount > 0;
		if (!m_HasIndexBuffer)
		{
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_Device->createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // The buffer is going to transfer memory
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // Host = CPU, Device = GPU
			stagingBuffer,
			stagingBufferMemory);

		// Most host memory to device memory
		void* data;
		vkMapMemory(m_Device->device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(m_Device->device(), stagingBufferMemory);

		m_Device->createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, // Tell that we want to create a vertex buffer
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // Host = CPU, Device = GPU
			m_IndexBuffer,
			m_IndexBufferMemory);

		m_Device->copyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

		vkDestroyBuffer(m_Device->device(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device->device(), stagingBufferMemory, nullptr);
	}

	std::vector<VkVertexInputBindingDescription> HDNModel::Vertex::GetBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> HDNModel::Vertex::GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}

	void HDNModel::Builder::LoadModel(const std::string& filepath)
	{
		tinyobj::attrib_t attrib; // Store positions, colors, uvs
		std::vector<tinyobj::shape_t> shapes; // Index values for each elements
		std::vector<tinyobj::material_t> materials;
		std::string warns, errors;
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warns, &errors, filepath.c_str()))
		{
			throw std::runtime_error(warns + errors);
		}

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32> uniqueVertices;

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices) // Loop in each face element in the model
			{
				Vertex vertex{};
				
				if (index.vertex_index >= 0)
				{
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					auto colorIndex = 3 * index.vertex_index + 2;
					if (colorIndex < attrib.colors.size())
					{
						vertex.color = {
							attrib.colors[colorIndex - 2],
							attrib.colors[colorIndex - 1],
							attrib.colors[colorIndex - 0]
						};
					}
					else
					{
						vertex.color = { 1.0f, 1.0f, 1.0f };
					}
				}

				if (index.normal_index >= 0)
				{
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (index.texcoord_index >= 0)
				{
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				if (!uniqueVertices.contains(vertex))
				{
					uniqueVertices[vertex] = static_cast<uint32>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}
}
