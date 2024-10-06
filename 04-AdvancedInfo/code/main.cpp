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
		uint32_t version = vk::enumerateInstanceVersion();
		cout << "Vulkan instance:\n"
		     << "   Version: " << vk::apiVersionMajor(version) << "."
		     << vk::apiVersionMinor(version) << "." << vk::apiVersionPatch(version) << endl;

		// instance extensions
		vk::Vector<vk::ExtensionProperties> instanceExtensionList = vk::enumerateInstanceExtensionProperties(nullptr);
		cout << "   Extensions:\n";
		for(size_t i=0; i<instanceExtensionList.size(); i++)
			cout << "      " << instanceExtensionList[i].extensionName << endl;

		// warning on low instance version
		if(version < vk::ApiVersion1_1)
			cout << "Vulkan instance version 1.1 or higher is required.\n"
			        "Some functionality will not be available." << endl;

		// Vulkan instance
		vk::initInstance(
			vk::InstanceCreateInfo{
				.sType = vk::StructureType::eInstanceCreateInfo,
				.pNext = nullptr,
				.flags = 0,
				.pApplicationInfo =
					&(const vk::ApplicationInfo&)vk::ApplicationInfo{
						.sType = vk::StructureType::eApplicationInfo,
						.pNext = nullptr,
						.pApplicationName = "04-AdvancedInfo",
						.applicationVersion = 0,
						.pEngineName = nullptr,
						.engineVersion = 0,
						.apiVersion = vk::ApiVersion1_1,
					},
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = nullptr,
				.enabledExtensionCount = 0,
				.ppEnabledExtensionNames = nullptr,
			});

		// print device list
		cout << "Vulkan devices:\n";
		vk::Vector<vk::PhysicalDevice> deviceList = vk::enumeratePhysicalDevices();
		for(size_t i=0; i<deviceList.size(); i++) {

			vk::PhysicalDevice pd = deviceList[i];

			// device properties
			vk::StructureChain<vk::PhysicalDeviceProperties2> propertiesStructureChain =
				vk::getPhysicalDeviceProperties2<vk::PhysicalDeviceProperties2>(pd);
			vk::PhysicalDeviceProperties& properties = propertiesStructureChain.get<vk::PhysicalDeviceProperties2>().properties;

			// device name
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

			// device limits
			cout << "      MaxTextureSize:  " << properties.limits.maxImageDimension2D << endl;

			// device features
			vk::PhysicalDeviceFeatures f = vk::getPhysicalDeviceFeatures(pd);
			cout << "      Geometry shader:  ";
			if(f.geometryShader)
				cout << "supported" << endl;
			else
				cout << "not supported" << endl;
			cout << "      Double precision:  ";
			if(f.shaderFloat64)
				cout << "supported" << endl;
			else
				cout << "not supported" << endl;

			// memory properties
			vk::PhysicalDeviceMemoryProperties m = vk::getPhysicalDeviceMemoryProperties(pd);
			cout << "      Memory heaps:" << endl;
			for(uint32_t i=0, c=m.memoryHeapCount; i<c; i++)
				cout << "         " << i << ": " << m.memoryHeaps[i].size/1024/1024 << "MiB" << endl;

			// queue family properties
			vk::Vector<vk::QueueFamilyProperties> queueFamilyList = vk::getPhysicalDeviceQueueFamilyProperties(pd);
			cout << "      Queue families:" << endl;
			for(uint32_t i=0, c=uint32_t(queueFamilyList.size()); i<c; i++) {
				cout << "         " << i << ": ";
				if(queueFamilyList[i].queueFlags & vk::QueueFlagBits::eGraphics)
					cout << "g";
				if(queueFamilyList[i].queueFlags & vk::QueueFlagBits::eCompute)
					cout << "c";
				if(queueFamilyList[i].queueFlags & vk::QueueFlagBits::eTransfer)
					cout << "t";
				cout << "  (count: " << queueFamilyList[i].queueCount << ")" << endl;
			}

			// color attachment R8G8B8A8Unorm format support
			vk::FormatProperties fp = vk::getPhysicalDeviceFormatProperties(pd, vk::Format::eR8G8B8A8Unorm);
			cout << "      R8G8B8A8Unorm format support for color attachment:" << endl;
			cout << "         Images with linear tiling: " <<
				string(fp.linearTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment ? "yes" : "no") << endl;
			cout << "         Images with optimal tiling: " <<
				string(fp.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment ? "yes" : "no") << endl;
			cout << "         Buffers: " <<
				string(fp.bufferFeatures & vk::FormatFeatureFlagBits::eColorAttachment ? "yes" : "no") << endl;

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
