#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <vector>
#include "vkg.h"

using namespace std;


// constants
constexpr const char* appName = "2-3-PipelineCache";


// shader code as SPIR-V binary
static const uint32_t performanceSpirv[] = {
#include "performance.comp.spv"
};


int main(int argc, char* argv[])
{
	// catch exceptions
	// (vk functions throw if they fail)
	try {

		// parse command-line arguments
		bool printHelp = false;
		size_t selectedDeviceIndex = 0;
		char* deviceFilterString = nullptr;
		for(int i=1; i<argc; i++) {

			// parse options starting with '-'
			if(argv[i][0] == '-') {

				// print help
				if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
					printHelp = true;
					continue;
				}

				// parse device index
				if(argv[i][1] >= '0' && argv[i][1] <= '9') {
					char* endp = nullptr;
					selectedDeviceIndex = strtoull(&argv[i][1], &endp, 10);
					if(selectedDeviceIndex == 0 || endp == &argv[i][1] || (endp && *endp != 0))
						printHelp = true;
					continue;
				}

				printHelp = true;
				continue;
			}

			// parse device filter string
			deviceFilterString = argv[i];

		}

		// print help
		if(printHelp) {
			cout << appName << " prints the performance of a Vulkan device\n"
			        "\n"
			        "Usage: " << appName << " [-<deviceIndex>] [deviceNameFilter]\n"
			        "   -<deviceIndex> - for example -1 or -3, specifies the index\n"
			        "      of the device used for the performance test; it might be\n"
			        "      useful when more devices are present in the system;\n"
			        "      devices are numbered starting from one\n"
			        "   deviceNameFilter - only devices matching the given string\n"
			        "      will be considered; for example AMD, GeForce, Intel\n" << endl;
			return 99;
		}

		// load Vulkan library
		vk::loadLib();

		// Vulkan instance
		vk::initInstance(
			vk::InstanceCreateInfo{
				.flags = {},
				.pApplicationInfo =
					&(const vk::ApplicationInfo&)vk::ApplicationInfo{
						.pApplicationName = appName,
						.applicationVersion = 0,
						.pEngineName = nullptr,
						.engineVersion = 0,
						.apiVersion = vk::ApiVersion13,  // highest api version used by the application
					},
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = nullptr,
				.enabledExtensionCount = 0,
				.ppEnabledExtensionNames = nullptr,
			}
		);

		// get compatible and incompatible devices
		//
		// required functionality: Vulkan 1.2, shaderInt64, bufferDeviceAddress, compute queue
		// optional functionality: Vulkan 1.3, pipelineCreationCacheControl
		vk::vector<vk::PhysicalDevice> deviceList = vk::enumeratePhysicalDevices();
		vector<tuple<vk::PhysicalDevice, uint32_t, vk::PhysicalDeviceProperties>> compatibleDevices;
		vector<vk::PhysicalDeviceProperties> incompatibleDevices;
		for(vk::PhysicalDevice pd : deviceList) {

			// device version 1.2+
			// (we need it for bufferDeviceAddress)
			vk::PhysicalDeviceProperties props = vk::getPhysicalDeviceProperties(pd);
			if(props.apiVersion < vk::ApiVersion12) {
				incompatibleDevices.emplace_back(props);
				continue;
			}

			// shaderInt64 and bufferDeviceAddress are required
			vk::PhysicalDeviceVulkan12Features features12;
			vk::PhysicalDeviceFeatures2 features10 {
				.pNext = &features12
			};
			vk::getPhysicalDeviceFeatures2(pd, features10);
			if(features10.features.shaderInt64 == false || features12.bufferDeviceAddress == false) {
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

		// select the device
		decltype(compatibleDevices)::iterator selectedDevice;
		if(deviceFilterString)
		{
			// select the device by name and index:
			// (1) filter devices by name and
			// (2) on the resulting list, choose the device on the index selectedDeviceIndex-1
			size_t counter = 1;
			for(size_t i=0, c=compatibleDevices.size(); i<c; i++) {
				if(strstr(get<2>(compatibleDevices[i]).deviceName, deviceFilterString)) {
					if(counter == selectedDeviceIndex || selectedDeviceIndex == 0) {
						selectedDevice = compatibleDevices.begin() + i;
						goto deviceFound;
					}
					counter++;
				}
			}

			// if no device was selected, print error and exit
			if(counter == 1)
				cout << "No device selected. Invalid filter string: " << deviceFilterString << "." << endl;
			else
				cout << "Invalid device index.\n"
				     << "Index value: " << selectedDeviceIndex << ", filter string: "
				     << deviceFilterString << "." << endl;
			return 99;

		deviceFound:;
		}
		else if(selectedDeviceIndex > 0)
		{
			// select the device by index
			if(selectedDeviceIndex > compatibleDevices.size()) {
				cout << "Invalid device index." << endl;
				return 99;
			}
			selectedDevice = compatibleDevices.begin() + selectedDeviceIndex - 1;
		}
		else
		{
			// choose the device automatically
			// using score heuristic
			selectedDevice = compatibleDevices.begin();
			constexpr const array deviceTypeScore = {
				10,  // vk::PhysicalDeviceType::eOther         - lowest score
				40,  // vk::PhysicalDeviceType::eIntegratedGpu - high score
				50,  // vk::PhysicalDeviceType::eDiscreteGpu   - highest score
				30,  // vk::PhysicalDeviceType::eVirtualGpu    - normal score
				20,  // vk::PhysicalDeviceType::eCpu           - low score
				10,  // unknown vk::PhysicalDeviceType
			};
			int score = deviceTypeScore[clamp(int(get<2>(*selectedDevice).deviceType), 0, int(deviceTypeScore.size())-1)];
			for(auto it=compatibleDevices.begin()+1; it!=compatibleDevices.end(); it++) {
				int newScore = deviceTypeScore[clamp(int(get<2>(*it).deviceType), 0, int(deviceTypeScore.size())-1)];
				if(newScore > score) {
					selectedDevice = it;
					score = newScore;
				}
			}
		}

		// device to use
		cout << "Using device:\n"
		        "   " << get<2>(*selectedDevice).deviceName << endl;
		vk::PhysicalDevice pd = get<0>(*selectedDevice);
		uint32_t queueFamily = get<1>(*selectedDevice);

		// get pipeline creation cache control support
		bool vulkan13Support = get<2>(*selectedDevice).apiVersion >= vk::ApiVersion13;
		bool pipelineCacheControlSupport;
		if(vulkan13Support) {
			vk::PhysicalDeviceVulkan13Features features13;
			vk::PhysicalDeviceFeatures2 features10 = {
				.pNext = &features13,
			};
			vk::getPhysicalDeviceFeatures2(pd, features10);
			pipelineCacheControlSupport = features13.pipelineCreationCacheControl;
		} else
			pipelineCacheControlSupport = false;

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
			}
			.setPNext(
				&(const vk::PhysicalDeviceVulkan12Features&)vk::PhysicalDeviceVulkan12Features{
					.bufferDeviceAddress = true,
				}
				.setPNext(
					vulkan13Support
						? &(const vk::PhysicalDeviceVulkan13Features&)vk::PhysicalDeviceVulkan13Features{
							  .pipelineCreationCacheControl = pipelineCacheControlSupport,
						  }
						: nullptr
				)
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

		// load pipeline from a cache
		cout << "Creating pipeline..." << flush;
		chrono::time_point compileStart = chrono::high_resolution_clock::now();
		vk::UniquePipeline pipeline;
		if(pipelineCacheControlSupport) {
			vk::Result r =
				vk::createComputePipelineUnique_noThrow(
					nullptr,
					vk::ComputePipelineCreateInfo{
						.flags = vk::PipelineCreateFlagBits::eFailOnPipelineCompileRequired,
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
					},
					pipeline
				);
			chrono::time_point compileEnd = chrono::high_resolution_clock::now();
			double delta = chrono::duration<double>(compileEnd - compileStart).count();
			if(r == vk::Result::eSuccess)
				cout << " done.\n   The pipeline was retrieved from a cache in " << delta * 1e3 << "ms." << endl;
			else if(r == vk::Result::ePipelineCompileRequired)
				;  // compile the pipeline in the following code block
			else
				vk::throwResultException(r, "vkCreateComputePipelines");
		}

		// compile pipeline
		if(!pipeline) {
			pipeline =
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
			chrono::time_point compileEnd = chrono::high_resolution_clock::now();
			double delta = chrono::duration<double>(compileEnd - compileStart).count();
			if(pipelineCacheControlSupport)
				// pipeline was compiled - we know it from pipeline cache control
				cout << " done.\n   The pipeline was compiled in " << delta * 1e3 << "ms." << endl;
			else
				// pipeline was created from cache or by compilation - no pipeline cache control support to know more
				cout << " done.\n   The pipeline was created in " << delta * 1e3 << "ms." << endl;
		}

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
