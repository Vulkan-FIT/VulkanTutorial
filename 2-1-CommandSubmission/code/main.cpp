#include <algorithm>
#include <array>
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
						.apiVersion = vk::ApiVersion10,
					},
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = nullptr,
				.enabledExtensionCount = 0,
				.ppEnabledExtensionNames = nullptr,
			}
		);

		// get compatible devices
		vk::vector<vk::PhysicalDevice> deviceList = vk::enumeratePhysicalDevices();
		vector<tuple<vk::PhysicalDevice, uint32_t, vk::PhysicalDeviceProperties>> compatibleDevices;
		for(size_t i=0; i<deviceList.size(); i++) {

			// get queue family properties
			vk::PhysicalDevice pd = deviceList[i];
			vk::PhysicalDeviceProperties props = vk::getPhysicalDeviceProperties(pd);
			vk::vector<vk::QueueFamilyProperties> queueFamilyList = vk::getPhysicalDeviceQueueFamilyProperties(pd);
			for(uint32_t i=0, c=uint32_t(queueFamilyList.size()); i<c; i++) {

				// test for graphics operations support
				if(queueFamilyList[i].queueFlags & vk::QueueFlagBits::eCompute)
					compatibleDevices.emplace_back(pd, i, props);

			}

		}

		// print compatible devices
		cout << "Compatible devices:" << endl;
		for(auto& t : compatibleDevices)
			cout << "   " << get<2>(t).deviceName << " (compute queue: " << get<1>(t)
			     << ", type: " << to_cstr(get<2>(t).deviceType) << ")" << endl;

		// choose the best device
		auto bestDevice = compatibleDevices.begin();
		if(bestDevice == compatibleDevices.end())
			throw runtime_error("No compatible devices.");
		constexpr const array deviceTypeScore = {
			10, // vk::PhysicalDeviceType::eOther         - lowest score
			40, // vk::PhysicalDeviceType::eIntegratedGpu - high score
			50, // vk::PhysicalDeviceType::eDiscreteGpu   - highest score
			30, // vk::PhysicalDeviceType::eVirtualGpu    - normal score
			20, // vk::PhysicalDeviceType::eCpu           - low score
			10, // unknown vk::PhysicalDeviceType
		};
		int bestScore = deviceTypeScore[clamp(int(get<2>(*bestDevice).deviceType), 0, int(deviceTypeScore.size())-1)];
		for(auto it=compatibleDevices.begin()+1; it!=compatibleDevices.end(); it++) {
			int score = deviceTypeScore[clamp(int(get<2>(*it).deviceType), 0, int(deviceTypeScore.size())-1)];
			if(score > bestScore) {
				bestDevice = it;
				bestScore = score;
			}
		}
		cout << "Using device:\n"
		        "   " << get<2>(*bestDevice).deviceName << endl;
		uint32_t queueFamily = get<1>(*bestDevice);

		// create device
		vk::initDevice(
			get<0>(*bestDevice),  // physicalDevice
			vk::DeviceCreateInfo{  // pCreateInfo
				.flags = {},
				.queueCreateInfoCount = 1,  // at least one queue is mandatory
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
				uint64_t(3e9)  // timeout (3s)
			);
		if(r == vk::Result::eTimeout)
			throw std::runtime_error("GPU timeout. Task is probably hanging.");
		else
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

	return 0;
}
