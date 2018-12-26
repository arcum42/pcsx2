#include "stdafx.h"
#include "Device.h"
#include "Texture.h"

namespace Vulkan
{
	Device::Device()
	{

	}

	bool Device::Create(const std::shared_ptr<GSWnd> &wnd)
	{
		if (!GSDevice::Create(wnd))
			return false;

		// hold my beer
		EnableDebug();
		CreateInstance();
		CreateLogicalDevice();

		return true;
	}

	void Device::EnableDebug()
	{
		const std::vector<const char *> layers = {
			"VK_LAYER_LUNARG_standard_validation"
		};

		m_layers = GetValidLayers(layers);
	}

	void Device::CreateInstance()
	{
		vk::ApplicationInfo app(
			"PCSX2",
			VK_MAKE_VERSION(1, 5, 0),
			"GSdx",
			VK_MAKE_VERSION(1, 5, 0),
			VK_API_VERSION_1_0
		);

		vk::InstanceCreateInfo create_info(
			{}, &app, m_layers.size(), m_layers.data()
		);

		m_instance = vk::createInstance(create_info);
	}

	void Device::CreateLogicalDevice()
	{
		std::vector<vk::PhysicalDevice> devices;
		devices = m_instance.enumeratePhysicalDevices();

		vk::PhysicalDevice physical_device;
		for (auto &dev : devices)
		{
			vk::PhysicalDeviceProperties props;
			props = dev.getProperties();

			// just select the first discrete gpu we find for now
			if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			{
				physical_device = dev;

				fprintf(stderr, "Selected Device: %s\n", props.deviceName);

				break;
			}
		}

		std::vector<vk::ExtensionProperties> ext_props;
		ext_props = physical_device.enumerateDeviceExtensionProperties();

		fprintf(stderr, "Extensions:\n");

		for (auto &prop : ext_props)
		{
			fprintf(stderr, "\t%s\n", prop.extensionName);
		}

		auto queue_family_props = physical_device.getQueueFamilyProperties();

		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;

		// Just select the first graphics queue for now
		int i = 0;
		for (auto &prop : queue_family_props)
		{
			if (prop.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				float prio = 1.0f;
				vk::DeviceQueueCreateInfo create_info({}, i, 1, &prio);

				queue_create_infos.push_back(create_info);

				fprintf(stderr, "Enabling queue: %d\n", i);

				break;
			}

			++i;
		}

		vk::DeviceCreateInfo create_info(
			{}, queue_create_infos.size(), queue_create_infos.data(),
			m_layers.size(), m_layers.data()
		);

		m_dev = physical_device.createDevice(create_info);
	}

	std::vector<const char*> Device::GetValidLayers(std::vector<const char*> layers)
	{
		std::set<std::string> available_layers;
		auto available_layer_props = vk::enumerateInstanceLayerProperties();
		
		for (auto &prop : available_layer_props)
		{
			available_layers.insert(prop.layerName);
		}

		std::vector<const char *> valid_layers;

		for (auto &layer : layers)
		{
			if (available_layers.count(layer) != 0)
			{
				valid_layers.push_back(layer);
				fprintf(stderr, "Enabling layer: %s\n", layer);
			}
		}

		return valid_layers;
	}

	void Device::DoMerge(GSTexture* sTex[3], GSVector4* sRect, GSTexture* dTex, GSVector4* dRect, const GSRegPMODE& PMODE, const GSRegEXTBUF& EXTBUF, const GSVector4& c)
	{

	}

	GSTexture* Device::CreateSurface(int type, int w, int h, bool msaa, int format)
	{
		return new Texture();
	}

	void Device::DoInterlace(GSTexture* sTex, GSTexture* dTex, int shader, bool linear, float yoffset)
	{

	}
}