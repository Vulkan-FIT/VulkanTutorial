#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <tuple>
#include <vector>
#include "vkg.h"

using namespace std;


// constants
constexpr const char* appName = "2-4-TimestampQueries";


// shader code as SPIR-V binary
static const uint32_t performanceSpirv[] = {
#include "performance.comp.spv"
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
		vector<tuple<vk::PhysicalDevice, uint32_t, vk::PhysicalDeviceProperties,
			vk::QueueFamilyProperties>> compatibleDevices;
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

				// test for compute operations support and for timestamp support
				vk::QueueFamilyProperties& qfp = queueFamilyPropList[i];
				if(qfp.queueFlags & vk::QueueFlagBits::eCompute) {
					if(qfp.timestampValidBits != 0) {
						found = true;
						compatibleDevices.emplace_back(pd, i, props, qfp);
					}
				}

			}

			// append incompatible devices
			// (reason: lack of compute queue or lack of timestamp support)
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
		uint32_t timestampValidBits = get<3>(*selectedDevice).timestampValidBits;
		uint64_t timestampValidBitMask = (timestampValidBits >= 64) ? ~uint64_t(0) : (uint64_t(1) << timestampValidBits) - 1;
		float timestampPeriod = get<2>(*selectedDevice).limits.timestampPeriod;

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

		// output header
		cout << "\n"
		        " Measurement        Number of         Computation     Performance\n"
		        "  time stamp     local workgroups         time" << endl;

		uint32_t groupCountX = 1;
		uint32_t groupCountY = 1;
		uint32_t groupCountZ = 1;
		chrono::time_point startTime = chrono::high_resolution_clock::now();
		do {

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

			// print results
			double time = float((timestamps[1] - timestamps[0]) & timestampValidBitMask) * timestampPeriod / 1e9;
			double totalTime = chrono::duration<double>(chrono::high_resolution_clock::now() - startTime).count();
			uint64_t numInstructions = uint64_t(20000) * 128 * groupCountX * groupCountY * groupCountZ;
			cout << fixed << setprecision(2)
			     << setw(9) << totalTime * 1000 << "ms       "
			     << setw(9) << groupCountX * groupCountY * groupCountZ << "        "
			     << "     " << formatFloatSI(time) << "s   "
			     << "    " << formatFloatSI(double(numInstructions) / time) << "FLOPS" << endl;

			// stop measurements after three seconds
			if(totalTime >= 3.)
				break;

			// update number of local workgroups
			// to reach computation time of about 20ms
			constexpr double targetTime = 0.02;
			if(time < targetTime / 10.) {
				if(groupCountX <= 1000)
					groupCountX *= 10;
				else if(groupCountY <= 1000)
					groupCountY *= 10;
				else if(groupCountZ <= 1000)
					groupCountZ *= 10;
			}
			else {
				double ratio = targetTime / time;
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

		} while(true);

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
