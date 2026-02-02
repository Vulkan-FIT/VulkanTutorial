#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <vector>
#include "vkg.h"

using namespace std;

// shader code as SPIR-V binary
static const uint32_t performanceSpirv[] = {
#include "performance.comp.spv"
};


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
						.pApplicationName = "2-2-ComputeShader",
						.applicationVersion = 0,
						.pEngineName = nullptr,
						.engineVersion = 0,
						.apiVersion = vk::ApiVersion12,  // highest api version used by the application
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
		vector<vk::PhysicalDeviceProperties> incompatibleDevices;
		for(size_t i=0,c=deviceList.size(); i<c; i++) {

			// device version 1.2+
			// (we need it for bufferDeviceAddress)
			vk::PhysicalDevice pd = deviceList[i];
			vk::PhysicalDeviceProperties props = vk::getPhysicalDeviceProperties(pd);
			if(props.apiVersion < vk::ApiVersion12) {
				incompatibleDevices.emplace_back(props);
				continue;
			}

			// append compatible queue families
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
				.pEnabledFeatures =
					&(const vk::PhysicalDeviceFeatures&)vk::PhysicalDeviceFeatures{
						.shaderInt64 = true,
					},
			}.setPNext(
				&(const vk::PhysicalDeviceVulkan12Features&)vk::PhysicalDeviceVulkan12Features{
					.bufferDeviceAddress = true,
				}
			)
		);

		// get queue
		vk::Queue queue = vk::getDeviceQueue(queueFamily, 0);

		// shader module
		vk::UniqueShaderModule shaderModule =
			vk::createShaderModuleUnique(
				vk::ShaderModuleCreateInfo{
					.flags = {},
					.codeSize = sizeof(performanceSpirv),
					.pCode = performanceSpirv,
				}
			);

		// pipeline layout
		vk::UniquePipelineLayout pipelineLayout =
			vk::createPipelineLayoutUnique(
				vk::PipelineLayoutCreateInfo{
					.flags = {},
					.setLayoutCount = 0,
					.pSetLayouts = nullptr,
					.pushConstantRangeCount = 0,
					.pPushConstantRanges = nullptr,
				}
			);

		// pipeline
		vk::UniquePipeline pipeline =
			vk::createComputePipelineUnique(
				nullptr,
				vk::ComputePipelineCreateInfo{
					.flags = {},
					.stage =
						vk::PipelineShaderStageCreateInfo{
							.flags = {},
							.stage = vk::ShaderStageFlagBits::eCompute,
							.module = shaderModule,
							.pName = "main",
							.pSpecializationInfo = nullptr,
						},
					.layout = pipelineLayout,
					.basePipelineHandle = nullptr,
					.basePipelineIndex = -1,
				}
			);

		// command pool
		vk::UniqueCommandPool commandPool =
			vk::createCommandPoolUnique(
				vk::CommandPoolCreateInfo{
					.flags = vk::CommandPoolCreateFlagBits::eTransient,
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

		// bind pipeline
		vk::cmdBindPipeline(commandBuffer, vk::PipelineBindPoint::eCompute, pipeline);

		// dispatch computation
		constexpr const uint32_t workgroupCountX = 1000;
		constexpr const uint32_t workgroupCountY = 100;
		constexpr const uint32_t workgroupCountZ = 1;
		vk::cmdDispatch(commandBuffer, workgroupCountX, workgroupCountY, workgroupCountZ);

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
		cout << "Submiting work and waiting for it..." << endl;
		chrono::time_point t1 = chrono::high_resolution_clock::now();
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
		vk::Result r =
			vk::waitForFence_noThrow(
				computingFinishedFence,
				uint64_t(1.5e9)  // timeout (1.5 seconds)
			);
		chrono::time_point t2 = chrono::high_resolution_clock::now();
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

		// print results
		double delta = chrono::duration<double>(t2 - t1).count();
		cout << "Computation time: " << delta * 1e3 << "ms." << endl;
		constexpr uint64_t numInstructions = uint64_t(20000) * 128 * workgroupCountX * workgroupCountY * workgroupCountZ;
		cout << "Computing performance: " << double(numInstructions) / delta * 1e-12 << " TFLOPS." << endl;

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
