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
		     << "   Version: " << vk::apiVersionMajor(instanceVersion) << "."
		     << vk::apiVersionMinor(instanceVersion) << "." << vk::apiVersionPatch(instanceVersion) << endl;

		// instance extensions
		vk::Vector<vk::ExtensionProperties> instanceExtensionList = vk::enumerateInstanceExtensionProperties(nullptr);
		cout << "   Extensions:\n";
		for(size_t i=0; i<instanceExtensionList.size(); i++)
			cout << "      " << instanceExtensionList[i].extensionName << endl;

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
						.apiVersion = vk::ApiVersion12,
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

			// supported extensions
			vk::Vector<vk::ExtensionProperties> extensionList =
				vk::enumerateDeviceExtensionProperties(pd, nullptr);
			bool videoQueueSupported = vk::isExtensionSupported(extensionList, "VK_KHR_video_queue") &&
			                           instanceVersion >= vk::ApiVersion11;
			bool raytracingSupported = vk::isExtensionSupported(extensionList, "VK_KHR_acceleration_structure") &&
			                           vk::isExtensionSupported(extensionList, "VK_KHR_ray_tracing_pipeline") &&
			                           vk::isExtensionSupported(extensionList, "VK_KHR_ray_query") &&
			                           instanceVersion >= vk::ApiVersion11;

			// get device properties
			vk::PhysicalDeviceProperties2 properties2;
			vk::PhysicalDeviceProperties& properties = properties2.properties;
			properties = vk::getPhysicalDeviceProperties(pd);

			// extended device properties
			vk::PhysicalDeviceVulkan12Properties properties12{
				.pNext = nullptr,
			};
			vk::PhysicalDeviceVulkan11Properties properties11{  // requires ApiVersion12 (really Vulkan 1.2, not 1.1)
				.pNext = (properties.apiVersion>=vk::ApiVersion12) ? &properties12 : nullptr,
			};
			if(properties.apiVersion >= vk::ApiVersion12)
				properties2.pNext = &properties11;
			if(properties.apiVersion >= vk::ApiVersion11 && instanceVersion >= vk::ApiVersion11)
				vk::getPhysicalDeviceProperties2(pd, properties2);

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

			// VendorID and DeviceID
			cout << "      VendorID:        0x" << hex << properties.vendorID << endl;
			cout << "      DeviceID:        0x" << properties.deviceID << dec << endl;

			// device UUID
			cout << "      Device UUID:     ";
			if(properties.apiVersion >= vk::ApiVersion12 && instanceVersion >= vk::ApiVersion11) {
				auto printBytes =
					[](uint8_t* a, uint8_t count) {
						for(uint8_t* e=a+count; a<e; a++)
							cout << (*a >> 4) << (*a & 0x0f);
					};
				cout << hex;
				printBytes(&properties11.deviceUUID[0], 4);
				cout << '-';
				printBytes(&properties11.deviceUUID[4], 2);
				cout << '-';
				printBytes(&properties11.deviceUUID[6], 2);
				cout << '-';
				printBytes(&properties11.deviceUUID[8], 2);
				cout << '-';
				printBytes(&properties11.deviceUUID[10], 6);
				cout << dec << endl;
			} else
				cout << "< unknown >" << endl;

			// driver info
			if(properties.apiVersion >= vk::ApiVersion12 && instanceVersion >= vk::ApiVersion11) {
				cout << "      Driver name:     " << properties12.driverName << endl;
				cout << "      Driver info:     " << properties12.driverInfo << endl;
			}
			else {
				cout << "      Driver name:     < unknown >" << endl;
				cout << "      Driver info:     < unknown >" << endl;
			}

			// device limits
			cout << "      MaxTextureSize:  " << properties.limits.maxImageDimension2D << endl;

			// device features
			vk::PhysicalDeviceVulkan12Features features12{
				.pNext = nullptr,
			};
			vk::PhysicalDeviceFeatures2 features2{
				.pNext = (properties.apiVersion>=vk::ApiVersion12) ? &features12 : nullptr,
			};
			vk::PhysicalDeviceFeatures& features = features2.features;
			if(properties.apiVersion >= vk::ApiVersion11 && instanceVersion >= vk::ApiVersion11)
				vk::getPhysicalDeviceFeatures2(pd, features2);
			else
				features = vk::getPhysicalDeviceFeatures(pd);

			// geometry shader support
			cout << "      Geometry shader:     ";
			if(features.geometryShader)
				cout << "supported" << endl;
			else
				cout << "not supported" << endl;

			// support for doubles and halfs
			cout << "      Double precision:    ";
			if(features.shaderFloat64)
				cout << "supported" << endl;
			else
				cout << "not supported" << endl;
			cout << "      Half precision:      ";
			if(properties.apiVersion >= vk::ApiVersion12 && features12.shaderFloat16 && instanceVersion >= vk::ApiVersion11)
				cout << "supported" << endl;
			else
				cout << "not supported" << endl;

			// video queue support
			cout << "      Vulkan Video:        ";
			if(videoQueueSupported)
				cout << "supported" << endl;
			else
				cout << "not supported" << endl;

			// video queue support
			cout << "      Vulkan Ray Tracing:  ";
			if(raytracingSupported)
				cout << "supported" << endl;
			else
				cout << "not supported" << endl;

			// memory properties
			cout << "      Memory heaps:" << endl;
			vk::PhysicalDeviceMemoryProperties memoryProperties = vk::getPhysicalDeviceMemoryProperties(pd);
			for(uint32_t i=0, c=memoryProperties.memoryHeapCount; i<c; i++) {
				vk::MemoryHeap& h = memoryProperties.memoryHeaps[i];
				cout << "         " << i << ": " << h.size/1024/1024 << "MiB";
				if(h.flags & vk::MemoryHeapFlagBits::eDeviceLocal)  cout << "  (device local)";
				cout << endl;
			}

			// queue family properties
			cout << "      Queue families:" << endl;
			vk::Vector<vk::QueueFamilyProperties2> queueFamilyList;
			vk::Vector<vk::QueueFamilyVideoPropertiesKHR> queueVideoPropertiesList;
			if(properties.apiVersion >= vk::ApiVersion11 && instanceVersion >= vk::ApiVersion11)
				queueFamilyList = vk::getPhysicalDeviceQueueFamilyProperties2(
					pd, queueVideoPropertiesList, videoQueueSupported);
			else {
				vk::Vector<vk::QueueFamilyProperties> v = vk::getPhysicalDeviceQueueFamilyProperties(pd);
				queueFamilyList.resize(v.size());
				for(size_t i=0; i<v.size(); i++)
					queueFamilyList[i].queueFamilyProperties = v[i];
			}
			for(uint32_t i=0, c=uint32_t(queueFamilyList.size()); i<c; i++) {
				cout << "         " << i << ": ";
				vk::QueueFamilyProperties& queueFamilyProperties = queueFamilyList[i].queueFamilyProperties;
				if(queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
					cout << "g";
				if(queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute)
					cout << "c";
				if(queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer)
					cout << "t";
				if(queueFamilyProperties.queueFlags & vk::QueueFlagBits::eSparseBinding)
					cout << "s";
				if(queueFamilyProperties.queueFlags & (vk::QueueFlagBits::eVideoDecodeKHR | vk::QueueFlagBits::eVideoEncodeKHR))
					cout << "v";
				cout << "  (count: " << queueFamilyProperties.queueCount;
				if(videoQueueSupported) {
					if(queueVideoPropertiesList[i].videoCodecOperations & vk::VideoCodecOperationFlagBitsKHR::eDecodeH264)
						cout << ", decode H264";
					if(queueVideoPropertiesList[i].videoCodecOperations & vk::VideoCodecOperationFlagBitsKHR::eDecodeH265)
						cout << ", decode H265";
					if(queueVideoPropertiesList[i].videoCodecOperations & vk::VideoCodecOperationFlagBitsKHR::eDecodeAV1)
						cout << ", decode AV1";
				}
				cout << ")" << endl;
			}

			// color attachment R8G8B8A8Srgb format support
			vk::FormatProperties formatProperties = vk::getPhysicalDeviceFormatProperties(pd, vk::Format::eR8G8B8A8Srgb);
			cout << "      R8G8B8A8Srgb format support for color attachment:" << endl;
			cout << "         Images with linear tiling: " <<
				string(formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment ? "yes" : "no") << endl;
			cout << "         Images with optimal tiling: " <<
				string(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment ? "yes" : "no") << endl;
			cout << "         Buffers: " <<
				string(formatProperties.bufferFeatures & vk::FormatFeatureFlagBits::eColorAttachment ? "yes" : "no") << endl;

			// print extensions
			if(!extensionList.empty())
				cout << "      Extensions:  " << extensionList[0].extensionName;
			else
				cout << "      Extensions:  < none >";
			for(size_t i=1,c=extensionList.size(); i<c; i++)
				cout << ", " << extensionList[i].extensionName;
			cout << endl;
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
