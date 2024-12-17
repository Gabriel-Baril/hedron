#include "VulkanPlayground/HDNGameObject.h"

namespace hdn
{
	mat4f32 TransformComponent::mat4()
	{
		const float32 c3 = glm::cos(rotation.z);
		const float32 s3 = glm::sin(rotation.z);
		const float32 c2 = glm::cos(rotation.x);
		const float32 s2 = glm::sin(rotation.x);
		const float32 c1 = glm::cos(rotation.y);
		const float32 s1 = glm::sin(rotation.y);
		return mat4f32{
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{translation.x, translation.y, translation.z, 1.0f}
		};
	}

	mat3f32 hdn::TransformComponent::NormalMatrix()
	{
		const float32 c3 = glm::cos(rotation.z);
		const float32 s3 = glm::sin(rotation.z);
		const float32 c2 = glm::cos(rotation.x);
		const float32 s2 = glm::sin(rotation.x);
		const float32 c1 = glm::cos(rotation.y);
		const float32 s1 = glm::sin(rotation.y);
		const vec3f32 invScale = 1.0f / scale;

		return mat3f32{
			{
				invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1)
			},
			{
				invScale.y * (c3 * s1 * s2 - c1 * s3),
				invScale.y * (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3),
			},
			{
				invScale.z * (c2 * s1),
				invScale.z * (-s2),
				invScale.z * (c1 * c2),
			}
		};
	}
}