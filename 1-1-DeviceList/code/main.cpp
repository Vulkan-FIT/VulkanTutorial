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
						.pApplicationName = "1-1-DeviceList",
						.applicationVersion = 0,
						.pEngineName = nullptr,
						.engineVersion = 0,
						.apiVersion = vk::ApiVersion1_0,
					},
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = nullptr,
				.enabledExtensionCount = 0,
				.ppEnabledExtensionNames = nullptr,
			});

		// print device list
		vk::Vector<vk::PhysicalDevice> deviceList = vk::enumeratePhysicalDevices();
		cout << "Physical devices:" << endl;
		for(size_t i=0; i<deviceList.size(); i++) {
			vk::PhysicalDeviceProperties p = vk::getPhysicalDeviceProperties(deviceList[i]);
			cout << "   " << p.deviceName << endl;
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
