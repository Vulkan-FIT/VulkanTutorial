#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <tuple>
#include <vector>
#include "vkg.h"

using namespace std;


// constants
constexpr const char* appName = "2-5-FloatPrecision";


// shader code as SPIR-V binary
static const uint32_t performanceHalfSpirv[] = {
#include "performance-half.comp.spv"
};
static const uint32_t performanceFloatSpirv[] = {
#include "performance-float.comp.spv"
};
static const uint32_t performanceDoubleSpirv[] = {
#include "performance-double.comp.spv"
};


// Convert float value to c-string.
//
// It prints float followed by SI suffix, such as K, M, G, m, u, n, etc.
// for kilo, mega, giga, milli, micro, nano,
// It uses precision of three digits, taking form of one of three variants:
// 1.23, 12.3, or 123, followed by space and SI suffix.
// To make it always the same length, third variant appends a space before the number:
// "1.23 K", "12.3 M", or " 123 G"
// Supported range is from "100 a" to "999 E". Bigger values are converted to "+inf   ".
// Lower values including negative numbers are converted to "   0  ".
static auto formatFloatSI(float v)
{
	// return type with implicit conversion to const char*
	struct SmallString {
		array<char,7> buffer;
		operator const char*() const { return buffer.data(); }
	};
	SmallString s;

	// compute significand and exponent
	int exponent = floorf(log10f(v));
	float divisor = expf(float(exponent - 2) * logf(10));  // this computes exp10f(exponent - 2)
	int significand = int(v / divisor + 0.5f);  // value is >=100 and <1000, actually it might be
		// a little out this range because of small floating computation imprecisions; +0.5 makes proper
		// rounding and avoids underflow to 99, but might cause overflow to 1000 (or even 1001?)

	// convert significand to numbers
	char n[4];
	n[3] = significand % 10;
	significand /= 10;
	n[2] = significand % 10;
	significand /= 10;
	n[1] = significand % 10;
	int thousandNumber = significand / 10;  // thousandNumber is 0 or 1; value 1 is present in some extreme cases
	n[0] = thousandNumber;
	exponent += thousandNumber;  // increment exponent if n contains >=1000

	// make exponent ready to index into SI prefix table
	constexpr const array<char,13> siPrefix = {
		'a', 'f', 'p', 'n', 'u', 'm', ' ', 'K', 'M', 'G', 'T', 'P', 'E'
	};
	exponent += 18;  // make zero exponent point on the ' ' in siPrefixes
	if(exponent < 0) {
		s.buffer = { ' ', ' ', ' ', '0', ' ', ' ', 0 };
		return s;
	}
	if(exponent >= 39) {
		s.buffer = { '+', 'i', 'n', 'f', ' ', ' ', 0 };
		return s;
	}

	// create final string
	s.buffer[6] = 0;
	s.buffer[5] = siPrefix[exponent / 3];
	s.buffer[4] = ' ';
	int dotPos = (exponent % 3) + 1;
	if(dotPos == 3)
		s.buffer[0] = ' ';
	else
		s.buffer[dotPos] = '.';
	s.buffer[3] = '0' + n[3 - thousandNumber];
	s.buffer[dotPos==2 ? 1 : 2] = '0' + n[2 - thousandNumber];
	s.buffer[dotPos==3 ? 1 : 0] = '0' + n[1 - thousandNumber];
	return s;
}


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
			exit(99);
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
		vector<tuple<vk::PhysicalDevice, uint32_t, vk::PhysicalDeviceProperties,
			vk::QueueFamilyProperties>> compatibleDevices;
		for(size_t i=0; i<deviceList.size(); i++) {

			// get queue family properties
			vk::PhysicalDevice pd = deviceList[i];
			vk::PhysicalDeviceProperties props = vk::getPhysicalDeviceProperties(pd);
			vk::vector<vk::QueueFamilyProperties> queueFamilyList = vk::getPhysicalDeviceQueueFamilyProperties(pd);
			for(uint32_t i=0, c=uint32_t(queueFamilyList.size()); i<c; i++) {

				// test for graphics operations support
				if(queueFamilyList[i].queueFlags & vk::QueueFlagBits::eCompute)
					compatibleDevices.emplace_back(pd, i, props, queueFamilyList[i]);

			}

		}

		// print compatible devices
		cout << "Compatible devices:" << endl;
		for(size_t i=0, c=compatibleDevices.size(); i<c; i++) {
			auto& t = compatibleDevices[i];
			cout << "   " << i+1 << ": " << get<2>(t).deviceName << " (compute queue: "
			     << get<1>(t) << ", type: " << to_cstr(get<2>(t).deviceType) << ")" << endl;
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
				if(strstr(get<2>(compatibleDevices[i]).deviceName, deviceFilterString))
					if(counter == selectedDeviceIndex || selectedDeviceIndex == 0) {
						selectedDevice = compatibleDevices.begin() + i;
						goto deviceFound;
					}
					else
						counter++;
			}
			if(counter == 0)
				throw runtime_error("No device selected.");
			else
				throw runtime_error("Invalid device index.");
		deviceFound:;
		}
		else if(selectedDeviceIndex > 0)
		{
			// select the device by index
			if(selectedDeviceIndex > compatibleDevices.size())
				throw runtime_error("Invalid device index.");
			selectedDevice = compatibleDevices.begin() + selectedDeviceIndex - 1;
		}
		else
		{
			// choose the device automatically
			// using score heuristic
			selectedDevice = compatibleDevices.begin();
			if(selectedDevice == compatibleDevices.end())
				throw runtime_error("No compatible devices.");
			constexpr const array deviceTypeScore = {
				10, // vk::PhysicalDeviceType::eOther         - lowest score
				40, // vk::PhysicalDeviceType::eIntegratedGpu - high score
				50, // vk::PhysicalDeviceType::eDiscreteGpu   - highest score
				30, // vk::PhysicalDeviceType::eVirtualGpu    - normal score
				20, // vk::PhysicalDeviceType::eCpu           - low score
				10, // unknown vk::PhysicalDeviceType
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
		cout << "Using device:\n"
		        "   " << get<2>(*selectedDevice).deviceName << endl;

		// get device and queue info
		uint32_t queueFamily = get<1>(*selectedDevice);
		uint32_t timestampValidBits = get<3>(*selectedDevice).timestampValidBits;
		uint64_t timestampValidBitMask = (timestampValidBits >= 64) ? ~uint64_t(0) : (uint64_t(1) << timestampValidBits) - 1;
		float timestampPeriod = get<2>(*selectedDevice).limits.timestampPeriod;
		if(timestampValidBitMask == 0)
			throw runtime_error("Vulkan timestamps are not supported on the queue.");

		// get supported features
		vk::PhysicalDeviceVulkan12Features features12;
		vk::PhysicalDeviceFeatures2 features10 = { .pNext = &features12 };
		vk::getPhysicalDeviceFeatures2(get<0>(*selectedDevice), features10);
		bool float64Supported = features10.features.shaderFloat64;
		bool float16Supported = features12.shaderFloat16;

		// create device
		vk::initDevice(
			get<0>(*selectedDevice),  // physicalDevice
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
						.shaderFloat64 = float64Supported,
						.shaderInt64 = true,
					},
			}.setPNext(
				&(const vk::PhysicalDeviceVulkan12Features&)vk::PhysicalDeviceVulkan12Features{
					.shaderFloat16 = float16Supported,
					.bufferDeviceAddress = true,
				}
			)
		);

		// get queue
		vk::Queue queue = vk::getDeviceQueue(queueFamily, 0);

		// shader module
		array<vk::UniqueShaderModule, 3> shaderModuleList;
		if(float16Supported)
			shaderModuleList[0] =
				vk::createShaderModuleUnique(
					vk::ShaderModuleCreateInfo{
						.flags = {},
						.codeSize = sizeof(performanceHalfSpirv),
						.pCode = performanceHalfSpirv,
					}
				);
		shaderModuleList[1] =
			vk::createShaderModuleUnique(
				vk::ShaderModuleCreateInfo{
					.flags = {},
					.codeSize = sizeof(performanceFloatSpirv),
					.pCode = performanceFloatSpirv,
				}
			);
		if(float64Supported)
			shaderModuleList[2] =
				vk::createShaderModuleUnique(
					vk::ShaderModuleCreateInfo{
						.flags = {},
						.codeSize = sizeof(performanceDoubleSpirv),
						.pCode = performanceDoubleSpirv,
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
		array<vk::UniquePipeline, 3> pipelineList;
		for(size_t i=0; i<shaderModuleList.size(); i++)
			if(shaderModuleList[i])
				pipelineList[i] =
					vk::createComputePipelineUnique(
						nullptr,
						vk::ComputePipelineCreateInfo{
							.flags = {},
							.stage =
								vk::PipelineShaderStageCreateInfo{
									.flags = {},
									.stage = vk::ShaderStageFlagBits::eCompute,
									.module = shaderModuleList[i],
									.pName = "main",
									.pSpecializationInfo = nullptr,
								},
							.layout = pipelineLayout,
							.basePipelineHandle = nullptr,
							.basePipelineIndex = -1,
						}
					);

		// timestamp pool
		vk::UniqueQueryPool timestampPool =
			vk::createQueryPoolUnique(
				vk::QueryPoolCreateInfo{
					.flags = {},
					.queryType = vk::QueryType::eTimestamp,
					.queryCount = 2,
					.pipelineStatistics = {},
				}
			);

		// command pool
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

		// fence
		vk::UniqueFence computingFinishedFence =
			vk::createFenceUnique(
				vk::FenceCreateInfo{
					.flags = {}
				}
			);

		auto performTest =
			[&](vk::Pipeline pipeline, size_t numWorkgroups) -> float {

				// dispatch dimensions
				uint32_t groupCountX;
				uint32_t groupCountY;
				uint32_t groupCountZ;
				if(numWorkgroups > 10000 * 10000) {
					groupCountZ = 1 + ((numWorkgroups - 1) / (10000 * 10000));
					uint64_t remainder = numWorkgroups / groupCountZ;
					groupCountY = 1 + ((remainder - 1) / 10000);
					groupCountX = remainder / groupCountY;
				}
				else {
					if(numWorkgroups == 0)
						numWorkgroups = 1;
					groupCountZ = 1;
					groupCountY = 1 + ((numWorkgroups - 1) / 10000);
					groupCountX = numWorkgroups / groupCountY;
				}


				// begin command buffer
				vk::beginCommandBuffer(
					commandBuffer,
					vk::CommandBufferBeginInfo{
						.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
						.pInheritanceInfo = nullptr,
					}
				);

				// reset timestamp pool
				vk::cmdResetQueryPool(
					commandBuffer,
					timestampPool,
					0,  // firstQuery
					2);  // queryCount

				// bind pipeline
				vk::cmdBindPipeline(commandBuffer, vk::PipelineBindPoint::eCompute, pipeline);

				// write timestamp 0
				vk::cmdWriteTimestamp(
					commandBuffer,
					vk::PipelineStageFlagBits::eTopOfPipe,
					timestampPool,
					0);  // query

				// dispatch computation
				vk::cmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);

				// write timestamp 1
				vk::cmdWriteTimestamp(
					commandBuffer,
					vk::PipelineStageFlagBits::eBottomOfPipe,
					timestampPool,
					1);  // query

				// end command buffer
				vk::endCommandBuffer(commandBuffer);


				// submit work
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
						uint64_t(3e9)  // timeout (3s)
					);
				if(r == vk::Result::eTimeout) {
					cout << "GPU timeout. Task is probably hanging." << endl;
					// use exit() to terminate application
					// (reason: device is still busy and we cannot safely destroy it;
					// so, let's not destroy any Vulkan objects)
					exit(-1);
				} else
					vk::checkForSuccessValue(r, "vkWaitForFences");

				// reset fence
				vk::resetFence(computingFinishedFence);

				// read timestamps
				array<uint64_t, 2> timestamps;
				vk::getQueryPoolResults(
					timestampPool,  // queryPool
					0,  // firstQuery
					2,  // queryCount
					2 * sizeof(uint64_t),  // dataSize
					timestamps.data(),  // pData
					sizeof(uint64_t),  // stride
					vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait  // flags
				);

				// return time as float in seconds
				return float((timestamps[1] - timestamps[0]) & timestampValidBitMask) * timestampPeriod / 1e9;

			};

		auto processResult =
			[](float time, size_t numWorkgroups, vector<float>& performanceList) {
				if(time >= 0.01f) {
					uint64_t numInstructions = uint64_t(20000) * 128 * numWorkgroups;
					float performance = float(numInstructions) / time;
					performanceList.push_back(performance);
				}
			};

		// compute number of workgroups
		// to reach computation time of about 20ms
		auto computeNumWorkgroups =
			[](size_t lastNumWorkgroups, float lastTime) -> size_t
			{
				constexpr float targetTime = 0.02;
				if(lastTime < (targetTime / 10.f)) {
					// multiply numWorkgroups by 10
					return lastNumWorkgroups * 10;
				}
				else {
					// multiply numWorkgroups by ratio
					float ratio = targetTime / lastTime;
					size_t newNumWorkgroups = size_t(lastNumWorkgroups * ratio);
					return (newNumWorkgroups >= 1) ? newNumWorkgroups : 1;
				}
			};

		size_t halfNumWorkgroups = 1;
		size_t floatNumWorkgroups = 1;
		size_t doubleNumWorkgroups = 1;
		float halfTime;
		float floatTime;
		float doubleTime;
		vector<float> halfPerformanceList;
		vector<float> floatPerformanceList;
		vector<float> doublePerformanceList;
		chrono::time_point startTime = chrono::high_resolution_clock::now();
		do {

			// perform tests
			if(float16Supported) {
				halfTime = performTest(pipelineList[0], halfNumWorkgroups);
				processResult(halfTime, halfNumWorkgroups, halfPerformanceList);
			}
			floatTime = performTest(pipelineList[1], floatNumWorkgroups);
			processResult(floatTime, floatNumWorkgroups, floatPerformanceList);
			if(float64Supported) {
				doubleTime = performTest(pipelineList[2], doubleNumWorkgroups);
				processResult(doubleTime, doubleNumWorkgroups, doublePerformanceList);
			}

			// stop measurements after three seconds
			double totalTime = chrono::duration<double>(chrono::high_resolution_clock::now() - startTime).count();
			if(totalTime >= 3.)
				break;

			// compute new numWorkgroups
			if(float16Supported)
				halfNumWorkgroups = computeNumWorkgroups(halfNumWorkgroups, halfTime);
			floatNumWorkgroups = computeNumWorkgroups(floatNumWorkgroups, floatTime);
			if(float64Supported)
				doubleNumWorkgroups = computeNumWorkgroups(doubleNumWorkgroups, doubleTime);
			cout << "f16 " << halfNumWorkgroups << ", t: " << halfTime << endl;
			cout << "f32 " << floatNumWorkgroups << ", t: " << floatTime << endl;
			cout << "f64 " << doubleNumWorkgroups << ", t: " << doubleTime << endl;

		} while(true);

		// sort the results
		sort(halfPerformanceList.begin(), halfPerformanceList.end());
		sort(floatPerformanceList.begin(), floatPerformanceList.end());
		sort(doublePerformanceList.begin(), doublePerformanceList.end());

		// print results
		auto printResult =
			[](const string_view text, bool supported, const vector<float>& performanceList) {
				cout << text;
				if(supported) {
					if(performanceList.empty())
						cout << "measurement error" << endl;
					else
						cout << formatFloatSI(performanceList[performanceList.size()/2]) << "FLOPS" << endl;
				}
				else
					cout << "not supported" << endl;
			};
		printResult("Half (float16) performance:    ", float16Supported, halfPerformanceList);
		printResult("Float (float32) performance:   ", true, floatPerformanceList);
		printResult("Double (float64) performance:  ", float64Supported, doublePerformanceList);

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
