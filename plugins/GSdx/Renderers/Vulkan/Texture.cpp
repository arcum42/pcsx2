#include "stdafx.h"
#include "Texture.h"

namespace Vulkan
{
	bool Texture::Update(const GSVector4i& r, const void* data, int pitch, int layer)
	{
		return false;
	}

	bool Texture::Map(GSMap& m, const GSVector4i* r, int layer)
	{
		return false;
	}

	void Texture::Unmap()
	{

	}

	bool Texture::Save(const std::string& fn)
	{
		return false;
	}
}