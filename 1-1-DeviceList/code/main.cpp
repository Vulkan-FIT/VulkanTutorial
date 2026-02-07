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
				.flags = {},
				.pApplicationInfo =
					&(const vk::ApplicationInfo&)vk::ApplicationInfo{
						.pApplicationName = "1-1-DeviceList",
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
		vk::vector<vk::PhysicalDevice> deviceList = vk::enumeratePhysicalDevices();
		cout << "Physical devices:" << endl;
		for(vk::PhysicalDevice pd : deviceList) {
			vk::PhysicalDeviceProperties p = vk::getPhysicalDeviceProperties(pd);
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

	vk::cleanUp();
	return 0;
}
