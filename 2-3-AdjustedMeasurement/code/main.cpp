#include <algorithm>
#include <array>
#include <chrono>
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
						.pApplicationName = "2-3-AdjustedMeasurement",
						.applicationVersion = 0,
						.pEngineName = nullptr,
						.engineVersion = 0,
						.apiVersion = vk::ApiVersion12,
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
		vk::AccessFlags2 b = vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
		vk::UniqueCommandPool commandPool =
			vk::createCommandPoolUnique(
				vk::CommandPoolCreateInfo{
					.flags = vk::CommandPoolCreateFlagBits::eTransient |
					         vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
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

		// output header
		cout << "\n"
		        " Measurement        Number of         Computation     Performance\n"
		        "  time stamp     local workgroups        time" << endl;

		uint32_t groupCountX = 1;
		uint32_t groupCountY = 1;
		uint32_t groupCountZ = 1;
		double totalTime = 0.;
		do {

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
			vk::cmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);

			// end command buffer
			vk::endCommandBuffer(commandBuffer);


			// fence
			vk::UniqueFence renderingFinishedFence =
				vk::createFenceUnique(
					vk::FenceCreateInfo{
						.flags = {}
					}
				);

			// submit work
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
				renderingFinishedFence
			);

			// wait for the work
			vk::Result r =
				vk::waitForFence_noThrow(
					renderingFinishedFence,
					uint64_t(3e9)  // timeout (3s)
				);
			chrono::time_point t2 = chrono::high_resolution_clock::now();
			if(r == vk::Result::eTimeout) {
				cout << "GPU timeout. Task is probably hanging." << endl;
				// use exit() to terminate application
				// (reason: device is still busy and we cannot safely destroy it;
				// so, let's not destroy any Vulkan objects)
				exit(-1);
			} else
				vk::checkForSuccessValue(r, "vkWaitForFences");

			// print results
			double delta = chrono::duration<double>(t2 - t1).count();
			uint64_t numInstructions = uint64_t(20000) * 128 * groupCountX * groupCountY * groupCountZ;
			cout << fixed << setprecision(2)
			     << setw(9) << totalTime * 1000 << "ms       "
			     << setw(9) << groupCountX * groupCountY * groupCountZ << "        "
			     << setw(9) << delta * 1e3 << "ms   "
			     << setw(9) << double(numInstructions) / delta * 1e-12 << " TFLOPS" << endl;

			// update number of local workgroups
			// to reach computation time of about 20ms
			constexpr double targetDelta = 0.02;
			if(delta < targetDelta / 10.) {
				if(groupCountX <= 1000)
					groupCountX *= 10;
				else if(groupCountY <= 1000)
					groupCountY *= 10;
				else if(groupCountZ <= 1000)
					groupCountZ *= 10;
			}
			else {
				double ratio = targetDelta / delta;
				uint64_t newNumGroups = uint64_t(ratio * (uint64_t(groupCountX) * groupCountY * groupCountZ));
				if(newNumGroups > 10000 * 10000) {
					groupCountZ = 1 + ((newNumGroups - 1) / (10000 * 10000));
					uint64_t remainder = newNumGroups / groupCountZ;
					groupCountY = 1 + ((remainder - 1) / 10000);
					groupCountX = remainder / groupCountY;
				}
				else {
					if(newNumGroups == 0)
						newNumGroups = 1;
					groupCountZ = 1;
					groupCountY = 1 + ((newNumGroups - 1) / 10000);
					groupCountX = newNumGroups / groupCountY;
				}
			}

			totalTime += delta;

		} while(totalTime < 3.);

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
