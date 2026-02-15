#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <vector>
#include "vkg.h"

using namespace std;


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
						.pApplicationName = "2-1-CommandSubmission",
						.applicationVersion = 0,
						.pEngineName = nullptr,
						.engineVersion = 0,
						.apiVersion = vk::ApiVersion10,  // highest api version used by the application
					},
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = nullptr,
				.enabledExtensionCount = 0,
				.ppEnabledExtensionNames = nullptr,
			}
		);

		// get compatible and incompatible devices
		//
		// required functionality: compute queue
		// optional functionality: none
		vk::vector<vk::PhysicalDevice> deviceList = vk::enumeratePhysicalDevices();
		vector<tuple<vk::PhysicalDevice, uint32_t, vk::PhysicalDeviceProperties>> compatibleDevices;
		vector<vk::PhysicalDeviceProperties> incompatibleDevices;
		for(vk::PhysicalDevice pd : deviceList) {

			// append compatible queue families
			vk::PhysicalDeviceProperties props = vk::getPhysicalDeviceProperties(pd);
			vk::vector<vk::QueueFamilyProperties> queueFamilyPropList = vk::getPhysicalDeviceQueueFamilyProperties(pd);
			bool found = false;
			for(uint32_t i=0, c=uint32_t(queueFamilyPropList.size()); i<c; i++) {

				// test for compute operations support
				vk::QueueFamilyProperties& qfp = queueFamilyPropList[i];
				if(qfp.queueFlags & vk::QueueFlagBits::eCompute) {
					found = true;
					compatibleDevices.emplace_back(pd, i, props);
				}

			}

			// append incompatible devices
			if(!found)
				incompatibleDevices.emplace_back(props);

		}

		// print device list
		cout << "List of devices:" << endl;
		for(size_t i=0, c=compatibleDevices.size(); i<c; i++) {
			auto& t = compatibleDevices[i];
			cout << "   " << i+1 << ": " << get<2>(t).deviceName << " (compute queue: "
			     << get<1>(t) << ", type: " << to_cstr(get<2>(t).deviceType) << ")" << endl;
		}
		for(size_t i=0, c=incompatibleDevices.size(); i<c; i++) {
			auto& props = incompatibleDevices[i];
			cout << "   incompatible: " << props.deviceName
			     << " (type: " << to_cstr(props.deviceType) << ")" << endl;
		}

		// handle empty compatibleDevices list
		if(compatibleDevices.empty()) {
			cout << "No compatible devices." << endl;
			return 0;
		}

		// choose the best device
		auto bestDevice = compatibleDevices.begin();
		constexpr const array deviceTypeScore = {
			10,  // vk::PhysicalDeviceType::eOther         - lowest score
			40,  // vk::PhysicalDeviceType::eIntegratedGpu - high score
			50,  // vk::PhysicalDeviceType::eDiscreteGpu   - highest score
			30,  // vk::PhysicalDeviceType::eVirtualGpu    - normal score
			20,  // vk::PhysicalDeviceType::eCpu           - low score
			10,  // unknown vk::PhysicalDeviceType
		};
		int bestScore = deviceTypeScore[clamp(int(get<2>(*bestDevice).deviceType), 0, int(deviceTypeScore.size())-1)];
		for(auto it=compatibleDevices.begin()+1; it!=compatibleDevices.end(); it++) {
			int score = deviceTypeScore[clamp(int(get<2>(*it).deviceType), 0, int(deviceTypeScore.size())-1)];
			if(score > bestScore) {
				bestDevice = it;
				bestScore = score;
			}
		}

		// device to use
		cout << "Using device:\n"
		        "   " << get<2>(*bestDevice).deviceName << endl;
		vk::PhysicalDevice pd = get<0>(*bestDevice);
		uint32_t queueFamily = get<1>(*bestDevice);

		// release resources
		compatibleDevices.clear();
		incompatibleDevices.clear();

		// create device
		vk::initDevice(
			pd,  // physicalDevice
			vk::DeviceCreateInfo{  // pCreateInfo
				.flags = {},
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos =
					array{
						vk::DeviceQueueCreateInfo{
							.flags = {},
							.queueFamilyIndex = queueFamily,
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

		// get queue
		vk::Queue queue = vk::getDeviceQueue(queueFamily, 0);

		// command pool
		vk::UniqueCommandPool commandPool =
			vk::createCommandPoolUnique(
				vk::CommandPoolCreateInfo{
					.flags = {},
					.queueFamilyIndex = queueFamily,
				}
			);

		// allocate command buffer
		vk::CommandBuffer commandBuffer =
			vk::allocateCommandBuffer(
				vk::CommandBufferAllocateInfo{
					.commandPool = commandPool,
					.level = vk::CommandBufferLevel::ePrimary,
					.commandBufferCount = 1,
				}
			);

		// begin command buffer
		vk::beginCommandBuffer(
			commandBuffer,
			vk::CommandBufferBeginInfo{
				.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
				.pInheritanceInfo = nullptr,
			}
		);

		// end command buffer
		vk::endCommandBuffer(commandBuffer);


		// fence
		vk::UniqueFence computingFinishedFence =
			vk::createFenceUnique(
				vk::FenceCreateInfo{
					.flags = {}
				}
			);

		// submit work
		cout << "Submiting work..." << endl;
		vk::queueSubmit(
			queue,
			vk::SubmitInfo{
				.waitSemaphoreCount = 0,
				.pWaitSemaphores = nullptr,
				.pWaitDstStageMask = nullptr,
				.commandBufferCount = 1,
				.pCommandBuffers = &commandBuffer,
				.signalSemaphoreCount = 0,
				.pSignalSemaphores = nullptr,
			},
			computingFinishedFence
		);

		// wait for the work
		cout << "Waiting for the work..." << endl;
		vk::Result r =
			vk::waitForFence_noThrow(
				computingFinishedFence,
				uint64_t(1.5e9)  // timeout (1.5 seconds)
			);
		if(r == vk::Result::eTimeout) {
			cout << "Vulkan device timeout. Task is probably hanging." << endl;
			// use std::quick_exit() to terminate the application
			// (Do not throw, do not return, do not call std::exit().
			// The device is still busy and it uses number of handles such as
			// computingFinishedFence or device handle itself.
			// Destruction of the handles in use or the unallowed access to them
			// is forbidden by Vulkan specification.
			quick_exit(-1);
		} else
			vk::checkForSuccessValue(r, "vkWaitForFences");

		cout << "Done." << endl;

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
