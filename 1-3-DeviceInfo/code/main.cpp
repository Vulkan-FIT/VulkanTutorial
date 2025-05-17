#include <iostream>
#include "vkg.h"

using namespace std;


int main(int, char**)
{
	// catch exceptions
	// (vk functions throw if they fail)
	try {

		// load Vulkan library
		vk::loadLib();

		// instance version
		uint32_t instanceVersion = vk::enumerateInstanceVersion();
		cout << "Vulkan instance:\n"
		     << "   Version:  " << vk::apiVersionMajor(instanceVersion) << "."
		     << vk::apiVersionMinor(instanceVersion) << "." << vk::apiVersionPatch(instanceVersion) << endl;

		// Vulkan instance
		vk::initInstance(
			vk::InstanceCreateInfo{
				.flags = {},
				.pApplicationInfo =
					&(const vk::ApplicationInfo&)vk::ApplicationInfo{
						.pApplicationName = "1-3-DeviceInfo",
						.applicationVersion = 0,
						.pEngineName = nullptr,
						.engineVersion = 0,
						.apiVersion = vk::ApiVersion10,
					},
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = nullptr,
				.enabledExtensionCount = 0,
				.ppEnabledExtensionNames = nullptr,
			});

		// print device list
		cout << "Vulkan devices:\n";
		vk::vector<vk::PhysicalDevice> deviceList = vk::enumeratePhysicalDevices();
		for(size_t i=0; i<deviceList.size(); i++) {

			vk::PhysicalDevice pd = deviceList[i];

			// device properties
			vk::PhysicalDeviceProperties properties = vk::getPhysicalDeviceProperties(pd);
			cout << "   " << properties.deviceName << endl;

			// device Vulkan version
			cout << "      Vulkan version:  " << vk::apiVersionMajor(properties.apiVersion) << "."
			     << vk::apiVersionMinor(properties.apiVersion) << "." << vk::apiVersionPatch(properties.apiVersion) << endl;

			// device type
			const char* s;
			switch(properties.deviceType) {
			case vk::PhysicalDeviceType::eIntegratedGpu: s = "IntegratedGpu"; break;
			case vk::PhysicalDeviceType::eDiscreteGpu:   s = "DiscreteGpu"; break;
			case vk::PhysicalDeviceType::eVirtualGpu:    s = "VirtualGpu"; break;
			case vk::PhysicalDeviceType::eCpu:           s = "Cpu"; break;
			default: s = "Other";
			}
			cout << "      Device type:     " << s << endl;

			// VendorID and DeviceID
			cout << "      VendorID:        0x" << hex << properties.vendorID << endl;
			cout << "      DeviceID:        0x" << properties.deviceID << dec << endl;

			// device limits
			cout << "      MaxTextureSize:  " << properties.limits.maxImageDimension2D << endl;

			// device features
			vk::PhysicalDeviceFeatures features = vk::getPhysicalDeviceFeatures(pd);
			cout << "      Geometry shader:   ";
			if(features.geometryShader)
				cout << "supported" << endl;
			else
				cout << "not supported" << endl;
			cout << "      Double precision:  ";
			if(features.shaderFloat64)
				cout << "supported" << endl;
			else
				cout << "not supported" << endl;

			// memory properties
			cout << "      Memory heaps:" << endl;
			vk::PhysicalDeviceMemoryProperties memoryProperties = vk::getPhysicalDeviceMemoryProperties(pd);
			for(uint32_t i=0, c=memoryProperties.memoryHeapCount; i<c; i++) {
				vk::MemoryHeap& h = memoryProperties.memoryHeaps[i];
				cout << "         " << i << ": " << h.size/1024/1024 << "MiB";
				if(h.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
					cout << "  (device local)";
				cout << endl;
			}

			// queue family properties
			cout << "      Queue families:" << endl;
			vk::vector<vk::QueueFamilyProperties> queueFamilyList = vk::getPhysicalDeviceQueueFamilyProperties(pd);
			for(uint32_t i=0, c=uint32_t(queueFamilyList.size()); i<c; i++) {
				cout << "         " << i << ": ";
				vk::QueueFamilyProperties& queueFamilyProperties = queueFamilyList[i];
				if(queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
					cout << "g";
				if(queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute)
					cout << "c";
				if(queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer)
					cout << "t";
				cout << "  (count: " << queueFamilyProperties.queueCount << ")" << endl;
			}

			// format support for images with optimal tiling
			cout << "      Format support for rendering into images with optimal tiling:" << endl;
			vk::FormatProperties formatProperties = vk::getPhysicalDeviceFormatProperties(pd, vk::Format::eR8G8B8A8Srgb);
			if(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment)
				cout << "         R8G8B8A8Srgb:        yes" << endl;
			else
				cout << "         R8G8B8A8Srgb:        no" << endl;
			formatProperties = vk::getPhysicalDeviceFormatProperties(pd, vk::Format::eR8G8B8Srgb);
			if(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment)
				cout << "         R8G8B8Srgb:          yes" << endl;
			else
				cout << "         R8G8B8Srgb:          no" << endl;
			formatProperties = vk::getPhysicalDeviceFormatProperties(pd, vk::Format::eR16G16B16A16Sfloat);
			if(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment)
				cout << "         R16G16B16A16Sfloat:  yes" << endl;
			else
				cout << "         R16G16B16A16Sfloat:  no" << endl;
			formatProperties = vk::getPhysicalDeviceFormatProperties(pd, vk::Format::eR16G16B16Sfloat);
			if(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment)
				cout << "         R16G16B16Sfloat:     yes" << endl;
			else
				cout << "         R16G16B16Sfloat:     no" << endl;

		}

	// catch exceptions
	} catch(vk::Error& e) {
		cout << e.what() << endl;
	} catch(exception& e) {
		cout << "Failed because of exception: " << e.what() << endl;
	} catch(...) {
		cout << "Failed because of unspecified exception." << endl;
	}

	return 0;
}
