#pragma once
#include "stdafx.h"
#include "Renderers/Common/GSTexture.h"

namespace Vulkan
{
	class Texture final : public GSTexture
	{
	public:
		bool Update(const GSVector4i& r, const void* data, int pitch, int layer = 0) final;
		bool Map(GSMap& m, const GSVector4i* r = NULL, int layer = 0) final;
		void Unmap() final;
		bool Save(const std::string& fn) final;
	};
}