#pragma once

#include "stdafx.h"
#include "Renderers/Common/GSDevice.h"

namespace Vulkan
{
	class Device : public GSDevice
	{
	public:
		Device();
		bool Create(const std::shared_ptr<GSWnd> &wnd);
	protected:
		void EnableDebug();
		void CreateInstance();
		void CreateLogicalDevice();
		std::vector<const char*> GetValidLayers(std::vector<const char*> layers);
		void DoMerge(GSTexture* sTex[3], GSVector4* sRect, GSTexture* dTex, GSVector4* dRect, const GSRegPMODE& PMODE, const GSRegEXTBUF& EXTBUF, const GSVector4& c);
		GSTexture* CreateSurface(int type, int w, int h, bool msaa, int format);
		void DoInterlace(GSTexture* sTex, GSTexture* dTex, int shader, bool linear, float yoffset);
	private:
		vk::Instance m_instance;
		vk::Device m_dev;

		std::vector<const char *> m_layers;
	};
}
