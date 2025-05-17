#include <array>
#include <iostream>
#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#else
# include <dlfcn.h>
#endif
#include "vkg.h"

using namespace std;


// return name of library that contains the specified address (address of function, symbol, etc.)
string getLibraryOfAddr(void* addr)
{
	if(addr == nullptr)
		throw std::runtime_error("Function getLibraryOfAddr() cannot resolve nullptr.");

#ifdef _WIN32

	// get module handle of belonging to address
	HMODULE handle;
	BOOL b = GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
	                            reinterpret_cast<LPCSTR>(addr), &handle);
	if(b==0 || handle==nullptr)
		throw std::runtime_error("Function GetModuleHandleExA() failed.");

	// get module path
	WCHAR wpath[MAX_PATH];
	DWORD r = GetModuleFileNameW(handle, wpath, MAX_PATH);
	if(r==0 || r==MAX_PATH)
		throw std::runtime_error("Function GetModuleFileNameW() failed.");

	// convert to utf-8
	char path[MAX_PATH];
	int l = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, path, MAX_PATH, NULL, NULL);
	if(l==0)
		throw std::runtime_error("Function WideCharToMultiByte() failed.");

	// return string
	return string(path, l);

#else
	Dl_info dlInfo;
	dladdr(addr, &dlInfo);
	return string(dlInfo.dli_fname);
#endif
}


int main(int argc, char* argv[])
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
						.pApplicationName = "1-5-LogicalDevice",
						.applicationVersion = 0,
						.pEngineName = nullptr,
						.engineVersion = 0,
						.apiVersion = vk::ApiVersion10,
					},
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = nullptr,
				.enabledExtensionCount = 0,
				.ppEnabledExtensionNames = nullptr,
			}
		);

		// instance function pointers
		cout << "Instance function pointers:" << endl;
		cout << "   vkCreateInstance()     points to: " << getLibraryOfAddr(vk::getGlobalProcAddr<void*>("vkCreateInstance")) << endl;
		cout << "   vkCreateShaderModule() points to: " << getLibraryOfAddr(vk::getInstanceProcAddr<void*>("vkCreateShaderModule")) << endl;
		cout << "   vkQueueSubmit()        points to: " << getLibraryOfAddr(vk::getInstanceProcAddr<void*>("vkQueueSubmit")) << endl;

		// print device list
		vk::vector<vk::PhysicalDevice> deviceList = vk::enumeratePhysicalDevices();
		for(size_t i=0; i<deviceList.size(); i++) {

			// get device properties
			vk::PhysicalDevice pd = deviceList[i];
			vk::PhysicalDeviceProperties properties = vk::getPhysicalDeviceProperties(pd);

			// create device
			vk::initDevice(
				pd,  // physicalDevice
				vk::DeviceCreateInfo{  // pCreateInfo
					.flags = {},
					.queueCreateInfoCount = 1,  // at least one queue is mandatory
					.pQueueCreateInfos = array<vk::DeviceQueueCreateInfo,1>{
						vk::DeviceQueueCreateInfo{
							.flags = {},
							.queueFamilyIndex = 0,
							.queueCount = 1,
							.pQueuePriorities = &(const float&)1.f,
						}
					}.data(),
					.enabledLayerCount = 0,  // no enabled layers
					.ppEnabledLayerNames = nullptr,
					.enabledExtensionCount = 0,  // no enabled extensions
					.ppEnabledExtensionNames = nullptr,
					.pEnabledFeatures = nullptr,  // no enabled features
				}
			);

			// device function pointers
			cout << "Device function pointers for " << properties.deviceName << ":" << endl;
			cout << "   vkCreateShaderModule() points to: " << getLibraryOfAddr(vk::getDeviceProcAddr<void*>("vkCreateShaderModule")) << endl;
			cout << "   vkQueueSubmit()        points to: " << getLibraryOfAddr(vk::getDeviceProcAddr<void*>("vkQueueSubmit")) << endl;

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
