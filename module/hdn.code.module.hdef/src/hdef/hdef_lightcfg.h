#pragma once
#include "hdef.h"

namespace hdn
{
	class HLightConfig : public HDefinition
	{
	public:
		HLightConfig() = default;
		virtual void Deserialize(FBufferReader& archive, HObjectLoadFlags flags = HObjectLoadFlags::Default) override;
		virtual void Serialize(FBufferWriter& archive, HObjectSaveFlags flags = HObjectSaveFlags::Default) override;
		virtual hash64_t GetTypeHash() const override { return GenerateTypeHash<HLightConfig>(); }
		inline u32 GetMaxPrimaryLightCount() const { return m_MaxPrimaryLightCount; }
		inline void SetMaxPrimaryLightCount(u32 count) { m_MaxPrimaryLightCount = count; }
		inline u32 GetMaxSecondaryLightCount() const { return m_MaxSecondaryLightCount; }
		inline void SetMaxSecondaryLightCount(u32 count) { m_MaxSecondaryLightCount = count; }
		virtual ~HLightConfig() = default;
	private:
		u32 m_MaxPrimaryLightCount = 0;
		u32 m_MaxSecondaryLightCount = 0;
	};
}