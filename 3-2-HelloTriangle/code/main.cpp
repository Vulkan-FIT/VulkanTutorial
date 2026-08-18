// SPDX-FileCopyrightText: 2022-2026 PCJohn (Jan Pečiva, peciva@fit.vut.cz)
//
// SPDX-License-Identifier: MIT-0

#include "VulkanWindow.h"
#include <algorithm>
#include <array>
#include <iostream>
#include "vkg.hpp"

using namespace std;


// constants
constexpr const char* appName = "HelloTriangle";


// shader code in SPIR-V binary
static const uint32_t vsSpirv[] = {
#include "shader.vert.spv"
};
static const uint32_t fsSpirv[] = {
#include "shader.frag.spv"
};


// global application data
class App {
public:

	App(int argc, char* argv[]);
	~App();

	void init();
	void resize(VulkanWindow& window, uint32_t& widthToBeSet, uint32_t& heightToBeSet);
	void frame(VulkanWindow& window);

	// Vulkan device, instance and library release object
	// (they need to be released as the last one)
	vk::Context vulkanContext;

	// window needs to be destroyed after the swapchain
	// This is required especially by Wayland.
	VulkanWindow window;

	// Vulkan variables, handles and objects
	// (unique objects are not in arbitrary order;
	// some unique objects need to be destructed before others in the destructor)
	uint32_t graphicsQueueFamily;
	uint32_t presentationQueueFamily;
	vk::Queue graphicsQueue;
	vk::Queue presentationQueue;
	vk::SurfaceFormatKHR surfaceFormat;
	vk::UniqueRenderPass renderPass;
	vk::UniqueSwapchainKHR swapchain;
	vector<vk::UniqueImageView> swapchainImageViews;
	vector<vk::UniqueFramebuffer> framebuffers;
	vector<vk::UniqueSemaphore> renderingFinishedSemaphores;
	uint32_t acquiredImageIndex;
	vk::UniqueFence imageAvailableFence;
	vk::UniqueFence renderFinishedFence;
	vk::UniqueCommandPool commandPool;
	vk::CommandBuffer commandBuffer;
	vk::UniqueShaderModule vsModule;
	vk::UniqueShaderModule fsModule;
	vk::UniquePipelineLayout pipelineLayout;
	vk::UniquePipeline pipeline;

};


/// Construct application object
App::App(int argc, char** argv)
{
}


App::~App()
{
	// wait for device idle state
	// (do not throw here because the device might be in the lost state already, etc.)
	if(vk::device())
		vk::deviceWaitIdle_noThrow();
}


void App::init()
{
	// init VulkanWindow
	VulkanWindow::init();

	// On Xlib, VulkanWindow::finalize() needs to be called before instance destroy to avoid crash.
	// It is workaround for the known bug in libXext: https://gitlab.freedesktop.org/xorg/lib/libxext/-/issues/3,
	// that crashes the application inside XCloseDisplay(). The problem seems to be present
	// especially on Nvidia drivers (reproduced on versions 470.129.06 and 515.65.01, for example).
	vulkanContext.atDestroy([](void*){ VulkanWindow::finalize(); }, nullptr);

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
					.apiVersion = vk::ApiVersion10,  // highest api version used by the application
				},
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = nullptr,
			.enabledExtensionCount = VulkanWindow::requiredExtensionCount(),
			.ppEnabledExtensionNames = VulkanWindow::requiredExtensionNames(),
		}
	);

	// create surface
	vk::SurfaceKHR surface =
		window.create(vk::instance().handle(), 1024, 768, appName, vk::funcs.vkGetInstanceProcAddr);

	// get compatible and incompatible devices
	//
	// required functionality: VK_KHR_swapchain, queue presentation support, graphics queue
	// optional functionality: < none >
	vk::vector<vk::PhysicalDevice> deviceList = vk::enumeratePhysicalDevices();
	vector<tuple<vk::PhysicalDevice, uint32_t, uint32_t, vk::PhysicalDeviceProperties>> compatibleDevices;
	vector<tuple<string,string>> incompatibleDevices;
	for(vk::PhysicalDevice pd : deviceList) {

		// skip devices without VK_KHR_swapchain
		auto extensionList = vk::enumerateDeviceExtensionProperties(pd, nullptr);
		for(vk::ExtensionProperties& e : extensionList)
			if(strcmp(e.extensionName, "VK_KHR_swapchain") == 0)
				goto swapchainSupported;
		incompatibleDevices.emplace_back(
			vk::getPhysicalDeviceProperties(pd).deviceName,
			"VK_KHR_swapchain extension not supported");
		continue;
		swapchainSupported:

		// select queues for graphics rendering and for presentation
		uint32_t graphicsQueueFamily = UINT32_MAX;
		uint32_t presentationQueueFamily = UINT32_MAX;
		vk::vector<vk::QueueFamilyProperties> queueFamilyList = vk::getPhysicalDeviceQueueFamilyProperties(pd);
		for(uint32_t i=0, c=uint32_t(queueFamilyList.size()); i<c; i++) {

			// test for presentation support
			if(vk::getPhysicalDeviceSurfaceSupportKHR(pd, i, surface)) {

				// test for graphics operations support
				if(queueFamilyList[i].queueFlags & vk::QueueFlagBits::eGraphics) {
					// if presentation and graphics operations are supported on the same queue,
					// we will use single queue
					compatibleDevices.emplace_back(pd, i, i, vk::getPhysicalDeviceProperties(pd));
					goto nextDevice;
				}
				else
					// if only presentation is supported, we store the first such queue
					if(presentationQueueFamily == UINT32_MAX)
						presentationQueueFamily = i;
			}
			else {
				if(queueFamilyList[i].queueFlags & vk::QueueFlagBits::eGraphics)
					// if only graphics operations are supported, we store the first such queue
					if(graphicsQueueFamily == UINT32_MAX)
						graphicsQueueFamily = i;
			}
		}

		if(graphicsQueueFamily == UINT32_MAX || presentationQueueFamily == UINT32_MAX) {
			// missing graphics or presentation support
			incompatibleDevices.emplace_back(
				vk::getPhysicalDeviceProperties(pd).deviceName,
				static_cast<const char*>((graphicsQueueFamily == UINT32_MAX)
					? "no queue supporting graphics operations"
					: "no queue supporting presentation"));
			continue;
		}
		else
			// graphics and presentation operations are supported on the different queues
			compatibleDevices.emplace_back(pd, graphicsQueueFamily, presentationQueueFamily, vk::getPhysicalDeviceProperties(pd));

		nextDevice:;
	}

	// print device list
	cout << "List of devices:" << endl;
	for(size_t i=0, c=compatibleDevices.size(); i<c; i++) {
		auto& t = compatibleDevices[i];
		cout << "   " << i+1 << ": " << get<3>(t).deviceName
		     << "\n         type:  " << to_cstr(get<3>(t).deviceType)
		     << "\n         graphics queue family:  " << get<1>(t)
		     << "\n         presentation queue family: " << get<2>(t) << endl;
	}
	for(size_t i=0, c=incompatibleDevices.size(); i<c; i++) {
		auto& [name, reason] = incompatibleDevices[i];
		cout << "   incompatible: " << name
		     << "\n      reason: " << reason << endl;
	}

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
	int bestScore = deviceTypeScore[clamp(int(get<3>(*bestDevice).deviceType), 0, int(deviceTypeScore.size())-1)];
	if(get<1>(*bestDevice) == get<2>(*bestDevice))
		bestScore++;
	for(auto it=compatibleDevices.begin()+1; it!=compatibleDevices.end(); it++) {
		int score = deviceTypeScore[clamp(int(get<3>(*it).deviceType), 0, int(deviceTypeScore.size())-1)];
		if(get<1>(*it) == get<2>(*it))
			score++;
		if(score > bestScore) {
			bestDevice = it;
			bestScore = score;
		}
	}
	cout << "Using device:\n"
	        "   " << get<3>(*bestDevice).deviceName << endl;
	vk::PhysicalDevice physicalDevice = get<0>(*bestDevice);
	graphicsQueueFamily = get<1>(*bestDevice);
	presentationQueueFamily = get<2>(*bestDevice);

	// create device
	vk::initDevice(
		physicalDevice,  // physicalDevice
		vk::DeviceCreateInfo{  // pCreateInfo
			.flags = {},
			.queueCreateInfoCount =
				graphicsQueueFamily==presentationQueueFamily ? uint32_t(1) : uint32_t(2),
			.pQueueCreateInfos =
				array{
					vk::DeviceQueueCreateInfo{
						.flags = vk::DeviceQueueCreateFlags(),
						.queueFamilyIndex = graphicsQueueFamily,
						.queueCount = 1,
						.pQueuePriorities = &(const float&)1.f,
					},
					vk::DeviceQueueCreateInfo{
						.flags = vk::DeviceQueueCreateFlags(),
						.queueFamilyIndex = presentationQueueFamily,
						.queueCount = 1,
						.pQueuePriorities = &(const float&)1.f,
					},
				}.data(),
			.enabledLayerCount = 0,  // no enabled layers
			.ppEnabledLayerNames = nullptr,
			.enabledExtensionCount = 1,  // number of enabled extensions
			.ppEnabledExtensionNames =
				array<const char*, 1>{ "VK_KHR_swapchain" }.data(),  // enabled extension names
			.pEnabledFeatures = nullptr,  // enabled features
		}
	);

	// get queues
	graphicsQueue = vk::getDeviceQueue(graphicsQueueFamily, 0);
	presentationQueue = vk::getDeviceQueue(presentationQueueFamily, 0);

	// print surface formats
	cout << "Surface formats:" << endl;
	vk::vector<vk::SurfaceFormatKHR> availableSurfaceFormats = vk::getPhysicalDeviceSurfaceFormatsKHR(surface);
	for(vk::SurfaceFormatKHR sf : availableSurfaceFormats)
		cout << "   " << vk::to_cstr(sf.format) << ", color space: " << vk::to_cstr(sf.colorSpace) << endl;

	// choose surface format
	constexpr const array allowedSurfaceFormats{
		vk::SurfaceFormatKHR{ vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear },
		vk::SurfaceFormatKHR{ vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear },
		vk::SurfaceFormatKHR{ vk::Format::eA8B8G8R8SrgbPack32, vk::ColorSpaceKHR::eSrgbNonlinear },
	};
	if(availableSurfaceFormats.size()==1 && availableSurfaceFormats[0].format==vk::Format::eUndefined)
		// Vulkan spec allowed single eUndefined value until 1.1.111 (2019-06-10)
		// with the meaning you can use any valid vk::Format value.
		// Now, it is forbidden, but let's handle any old driver.
		surfaceFormat = allowedSurfaceFormats[0];
	else {
		for(vk::SurfaceFormatKHR sf : availableSurfaceFormats) {
			for(auto it=allowedSurfaceFormats.begin(), e=allowedSurfaceFormats.end(); it!=e; it++)
				if(it->format == sf.format && it->colorSpace == sf.colorSpace) {
					surfaceFormat = *it;
					goto surfaceFormatFound;
				}
		}
		if(availableSurfaceFormats.size() == 0)  // Vulkan must return at least one format (this is mandated since Vulkan 1.0.37 (2016-10-10), but was missing in the spec before probably because of omission)
			throw std::runtime_error("Vulkan error: vk::getPhysicalDeviceSurfaceFormatsKHR() returned empty list.");
		surfaceFormat = availableSurfaceFormats[0];
	surfaceFormatFound:;
	}
	cout << "Using format:\n"
	     << "   " << to_cstr(surfaceFormat.format) << ", color space: " << to_cstr(surfaceFormat.colorSpace) << endl;

	// render pass
	renderPass =
		vk::createRenderPassUnique(
			vk::RenderPassCreateInfo{
				.flags = vk::RenderPassCreateFlags(),
				.attachmentCount = 1,
				.pAttachments = array{
					vk::AttachmentDescription{
						.flags = vk::AttachmentDescriptionFlags(),
						.format = surfaceFormat.format,
						.samples = vk::SampleCountFlagBits::e1,
						.loadOp = vk::AttachmentLoadOp::eClear,
						.storeOp = vk::AttachmentStoreOp::eStore,
						.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
						.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
						.initialLayout = vk::ImageLayout::eUndefined,
						.finalLayout = vk::ImageLayout::ePresentSrcKHR,
					},
				}.data(),
				.subpassCount = 1,
				.pSubpasses = array{
					vk::SubpassDescription{
						.flags = vk::SubpassDescriptionFlags(),
						.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
						.inputAttachmentCount = 0,
						.pInputAttachments = nullptr,
						.colorAttachmentCount = 1,
						.pColorAttachments = array{
							vk::AttachmentReference{
								.attachment = 0,
								.layout = vk::ImageLayout::eColorAttachmentOptimal,
							},
						}.data(),
						.pResolveAttachments = nullptr,
						.pDepthStencilAttachment = nullptr,
						.preserveAttachmentCount = 0,
						.pPreserveAttachments = nullptr,
					},
				}.data(),
				.dependencyCount = 1,
				.pDependencies = array{
					vk::SubpassDependency{
						.srcSubpass = vk::SubpassExternal,
						.dstSubpass = 0,
						.srcStageMask = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput),
						.dstStageMask = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput),
						.srcAccessMask = vk::AccessFlags(),
						.dstAccessMask = vk::AccessFlags(vk::AccessFlagBits::eColorAttachmentWrite),
						.dependencyFlags = vk::DependencyFlags(),
					},
				}.data()
			}
		);

	// commandPool and commandBuffer
	commandPool =
		vk::createCommandPoolUnique(
			vk::CommandPoolCreateInfo{
				.flags = vk::CommandPoolCreateFlagBits::eTransient |
					vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				.queueFamilyIndex = graphicsQueueFamily,
			}
		);
	commandBuffer =
		vk::allocateCommandBuffer(
			vk::CommandBufferAllocateInfo{
				.commandPool = commandPool,
				.level = vk::CommandBufferLevel::ePrimary,
				.commandBufferCount = 1,
			}
		);

	// rendering fences
	imageAvailableFence =
		vk::createFenceUnique(
			vk::FenceCreateInfo{
				.flags = {},
			}
		);
	renderFinishedFence =
		vk::createFenceUnique(
			vk::FenceCreateInfo{
				.flags = vk::FenceCreateFlagBits::eSignaled,
			}
		);

	// create shader modules
	vsModule =
		vk::createShaderModuleUnique(
			vk::ShaderModuleCreateInfo{
				.flags = vk::ShaderModuleCreateFlags(),
				.codeSize = sizeof(vsSpirv),
				.pCode = vsSpirv,
			}
		);
	fsModule =
		vk::createShaderModuleUnique(
			vk::ShaderModuleCreateInfo{
				.flags = vk::ShaderModuleCreateFlags(),
				.codeSize = sizeof(fsSpirv),
				.pCode = fsSpirv,
			}
		);

	// pipeline layout
	pipelineLayout =
		vk::createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				.flags = vk::PipelineLayoutCreateFlags(),
				.setLayoutCount = 0,
				.pSetLayouts = nullptr,
				.pushConstantRangeCount = 0,
				.pPushConstantRanges = nullptr,
			}
		);
}


/** Recreate swapchain and pipeline callback.
 *  The function is usually called after the window resize and on the application start. */
void App::resize(VulkanWindow&, uint32_t& widthToBeSet, uint32_t& heightToBeSet)
{
	// make sure that we finished all the rendering
	// (this is necessary for swapchain re-creation)
	vk::deviceWaitIdle();

	// get surface capabilities
	// On Win32 and Xlib, currentExtent, minImageExtent and maxImageExtent of returned surfaceCapabilites are all equal.
	// It means that we can create a new swapchain only with imageExtent being equal to the window size.
	// The currentExtent might become 0,0 on Win32 and Xlib platform, for example, when the window is minimized.
	// If the currentExtent is not 0,0, both width and height must be greater than 0.
	// On Wayland, currentExtent might be 0xffffffff, 0xffffffff with the meaning that the window extent
	// will be determined by the extent of the swapchain.
	// Wayland's minImageExtent is 1,1 and maxImageExtent is the maximum supported surface size.
	vk::SurfaceCapabilitiesKHR surfaceCapabilities =
		vk::getPhysicalDeviceSurfaceCapabilitiesKHR(window.surface());

	// zero size swapchain is not allowed,
	// so we will ignore current resize and rendering attempt and wait for the next window resize
	// (zero size may happen, for example, on Win32 when shrinking window too much)
	if(surfaceCapabilities.currentExtent.width == 0 || surfaceCapabilities.currentExtent.height == 0) {
		widthToBeSet = surfaceCapabilities.currentExtent.width;
		heightToBeSet = surfaceCapabilities.currentExtent.height;
		return;  // new frame will be scheduled on the next window resize
	}

	// if currentExtent is unknown (f.ex. Wayland might return 0xffffffff before first window show)
	// use the size returned by window
	vk::Extent2D newSurfaceExtent;
	if(surfaceCapabilities.currentExtent.width == 0xffffffff || surfaceCapabilities.currentExtent.height == 0xffffffff)
		newSurfaceExtent = vk::Extent2D{ window.surfaceWidth(), window.surfaceHeight() };
	else
		newSurfaceExtent = surfaceCapabilities.currentExtent;

	// update VulkanWindow surface size
	widthToBeSet = newSurfaceExtent.width;
	heightToBeSet = newSurfaceExtent.height;

	// clear resources
	swapchainImageViews.clear();
	framebuffers.clear();
	pipeline = nullptr;

	// print info
	cout << "Recreating swapchain (extent: " << newSurfaceExtent.width << "x" << newSurfaceExtent.height
	     << ", extent by surfaceCapabilities: " << surfaceCapabilities.currentExtent.width << "x"
	     << surfaceCapabilities.currentExtent.height << ", minImageCount: " << surfaceCapabilities.minImageCount
	     << ", maxImageCount: " << surfaceCapabilities.maxImageCount << ")" << endl;

	// create new swapchain
	constexpr const uint32_t requestedImageCount = 2;
	swapchain =
		vk::createSwapchainKHRUnique(
			vk::SwapchainCreateInfoKHR{
				.flags = vk::SwapchainCreateFlagsKHR(),
				.surface = window.surface(),
				.minImageCount =
					surfaceCapabilities.maxImageCount==0
						? max(requestedImageCount, surfaceCapabilities.minImageCount)
						: clamp(requestedImageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount),
				.imageFormat = surfaceFormat.format,
				.imageColorSpace = surfaceFormat.colorSpace,
				.imageExtent = newSurfaceExtent,
				.imageArrayLayers = 1,
				.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
				.imageSharingMode =
					(graphicsQueueFamily==presentationQueueFamily)
						? vk::SharingMode::eExclusive
						: vk::SharingMode::eConcurrent,
				.queueFamilyIndexCount = 2,
				.pQueueFamilyIndices = array<uint32_t, 2>{ graphicsQueueFamily, presentationQueueFamily }.data(),
				.preTransform = surfaceCapabilities.currentTransform,
				.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
				.presentMode = vk::PresentModeKHR::eFifo,
				.clipped = vk::True,
				.oldSwapchain = swapchain,
			}
		);
	acquiredImageIndex = ~uint32_t(0);

	// swapchain images and image views
	vk::vector<vk::Image> swapchainImages = vk::getSwapchainImagesKHR(swapchain);
	swapchainImageViews.reserve(swapchainImages.size());
	for(vk::Image image : swapchainImages)
		swapchainImageViews.emplace_back(
			vk::createImageView(
				vk::ImageViewCreateInfo{
					.flags = vk::ImageViewCreateFlags(),
					.image = image,
					.viewType = vk::ImageViewType::e2D,
					.format = surfaceFormat.format,
					.components = {},
					.subresourceRange = vk::ImageSubresourceRange{
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.baseMipLevel = 0,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1,
					}
				}
			)
		);

	// framebuffers
	framebuffers.reserve(swapchainImages.size());
	for(size_t i=0, c=swapchainImages.size(); i<c; i++)
		framebuffers.emplace_back(
			vk::createFramebuffer(
				vk::FramebufferCreateInfo{
					.flags = vk::FramebufferCreateFlags(),
					.renderPass = renderPass,
					.attachmentCount = 1,
					.pAttachments = swapchainImageViews[i].getPtr(),
					.width = newSurfaceExtent.width,
					.height = newSurfaceExtent.height,
					.layers = 1,
				}
			)
		);

	// rendering finished semaphores
	if(renderingFinishedSemaphores.size() != swapchainImages.size())
	{
		renderingFinishedSemaphores.clear();
		renderingFinishedSemaphores.reserve(swapchainImages.size());
		vk::SemaphoreCreateInfo semaphoreCreateInfo{
			.flags = vk::SemaphoreCreateFlags(),
		};
		for(size_t i=0,c=swapchainImages.size(); i<c; i++)
			renderingFinishedSemaphores.emplace_back(
				vk::createSemaphore(semaphoreCreateInfo)
			);
	}

	// pipeline
	pipeline =
		vk::createGraphicsPipelineUnique(
			nullptr,  // pipelineCache
			vk::GraphicsPipelineCreateInfo{
				.flags = vk::PipelineCreateFlags(),

				// shader stages
				.stageCount = 2,
				.pStages =
					array{
						vk::PipelineShaderStageCreateInfo{
							.flags = vk::PipelineShaderStageCreateFlags(),
							.stage = vk::ShaderStageFlagBits::eVertex,
							.module = vsModule,
							.pName = "main",
							.pSpecializationInfo = nullptr,
						},
						vk::PipelineShaderStageCreateInfo{
							.flags = vk::PipelineShaderStageCreateFlags(),
							.stage = vk::ShaderStageFlagBits::eFragment,
							.module = fsModule,
							.pName = "main",
							.pSpecializationInfo = nullptr,
						},
					}.data(),

				// vertex input
				.pVertexInputState =
					&(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
						.flags = vk::PipelineVertexInputStateCreateFlags(),
						.vertexBindingDescriptionCount = 0,
						.pVertexBindingDescriptions = nullptr,
						.vertexAttributeDescriptionCount = 0,
						.pVertexAttributeDescriptions = nullptr,
					},

				// input assembly
				.pInputAssemblyState =
					&(const vk::PipelineInputAssemblyStateCreateInfo&)vk::PipelineInputAssemblyStateCreateInfo{
						.flags = vk::PipelineInputAssemblyStateCreateFlags(),
						.topology = vk::PrimitiveTopology::eTriangleList,
						.primitiveRestartEnable = vk::False,
					},

				// tessellation
				.pTessellationState = nullptr,

				// viewport
				.pViewportState =
					&(const vk::PipelineViewportStateCreateInfo&)vk::PipelineViewportStateCreateInfo{
						.flags = vk::PipelineViewportStateCreateFlags(),
						.viewportCount = 1,
						.pViewports = array{
							vk::Viewport{
								.x = 0.f,
								.y = 0.f,
								.width = float(newSurfaceExtent.width),
								.height = float(newSurfaceExtent.height),
								.minDepth = 0.f,
								.maxDepth = 1.f
							},
						}.data(),
						.scissorCount = 1,
						.pScissors = array{
							vk::Rect2D(vk::Offset2D(0, 0), newSurfaceExtent)
						}.data(),
					},

				// rasterization
				.pRasterizationState =
					&(const vk::PipelineRasterizationStateCreateInfo&)vk::PipelineRasterizationStateCreateInfo{
						.flags = vk::PipelineRasterizationStateCreateFlags(),
						.depthClampEnable = vk::False,
						.rasterizerDiscardEnable = vk::False,
						.polygonMode = vk::PolygonMode::eFill,
						.cullMode = vk::CullModeFlagBits::eNone,
						.frontFace = vk::FrontFace::eCounterClockwise,
						.depthBiasEnable = vk::False,
						.depthBiasConstantFactor = 0.f,
						.depthBiasClamp = 0.f,
						.depthBiasSlopeFactor = 0.f,
						.lineWidth = 1.f,
					},

				// multisampling
				.pMultisampleState =
					&(const vk::PipelineMultisampleStateCreateInfo&)vk::PipelineMultisampleStateCreateInfo{
						.flags = vk::PipelineMultisampleStateCreateFlags(),
						.rasterizationSamples = vk::SampleCountFlagBits::e1,
						.sampleShadingEnable = vk::False,
						.minSampleShading = 0.f,
						.pSampleMask = nullptr,
						.alphaToCoverageEnable = vk::False,
						.alphaToOneEnable = vk::False,
					},

				// depth and stencil
				.pDepthStencilState = nullptr,

				// blending
				.pColorBlendState =
					&(const vk::PipelineColorBlendStateCreateInfo&)vk::PipelineColorBlendStateCreateInfo{
						.flags = vk::PipelineColorBlendStateCreateFlags(),
						.logicOpEnable = vk::False,
						.logicOp = vk::LogicOp::eClear,
						.attachmentCount = 1,
						.pAttachments =
							array{
								vk::PipelineColorBlendAttachmentState{
									.blendEnable = vk::False,
									.srcColorBlendFactor = vk::BlendFactor::eZero,
									.dstColorBlendFactor = vk::BlendFactor::eZero,
									.colorBlendOp = vk::BlendOp::eAdd,
									.srcAlphaBlendFactor = vk::BlendFactor::eZero,
									.dstAlphaBlendFactor = vk::BlendFactor::eZero,
									.alphaBlendOp = vk::BlendOp::eAdd,
									.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
										vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
								},
							}.data(),
						.blendConstants = { 0.f, 0.f, 0.f, 0.f },
					},

				.pDynamicState = nullptr,
				.layout = pipelineLayout,
				.renderPass = renderPass,
				.subpass = 0,
				.basePipelineHandle = nullptr,
				.basePipelineIndex = -1,
			}
		);
}


void App::frame(VulkanWindow&)
{
	// acquire image
	if(acquiredImageIndex == ~uint32_t(0)) {

		vk::resetFences(imageAvailableFence);
		vk::Result r =
			vk::acquireNextImageKHR_noThrow(
				swapchain,            // swapchain
				uint64_t(1.5e9),      // timeout (1.5s)
				nullptr,              // semaphore to signal
				imageAvailableFence,  // fence to signal
				&acquiredImageIndex   // pImageIndex
			);

		// wait for previous frame rendering work and for imageAvailableFence
		if(r == vk::Result::eSuccess || r == vk::Result::eSuboptimalKHR)
		{
			vk::Result waitResult =
				vk::waitForFences_noThrow(
					array{  // fences
						renderFinishedFence.get(),
						imageAvailableFence.get(),
					},
					vk::True,  // waitAll
					uint64_t(1.5e9)  // timeout
				);
			if(waitResult != vk::Result::eSuccess) {
				if(waitResult == vk::Result::eTimeout)
					throw runtime_error("GPU timeout. Task is probably hanging on GPU.");
				throw runtime_error(string("Vulkan error: vkWaitForFences failed with error ") + vk::to_cstr(r) + ".");
			}

			// resize swapchain on suboptimal result
			if(r == vk::Result::eSuboptimalKHR) {
				window.scheduleResize();
				cout << "acquire result: Suboptimal" << endl;
				return;
			}
		}
		else
		{
			// handle errors
			if(r == vk::Result::eErrorOutOfDateKHR) {
				window.scheduleResize();
				cout << "acquire error: OutOfDate" << endl;
				return;
			} else
				throw runtime_error(string("Vulkan error: vkAcquireNextImageKHR failed with error ") + vk::to_cstr(r) + ".");
		}

	}

	// record command buffer
	vk::beginCommandBuffer(
		commandBuffer,
		vk::CommandBufferBeginInfo{
			.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
			.pInheritanceInfo = nullptr,
		}
	);
	vk::cmdBeginRenderPass(
		commandBuffer,
		vk::RenderPassBeginInfo{
			.renderPass = renderPass,
			.framebuffer = framebuffers[acquiredImageIndex],
			.renderArea = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(window.surfaceWidth(), window.surfaceHeight())),
			.clearValueCount = 1,
			.pClearValues = &(const vk::ClearValue&)vk::ClearValue{
				.color = vk::ClearColorValue{
					.float32 = { 0.0f, 0.0f, 0.0f, 1.f },
				}
			}
		},
		vk::SubpassContents::eInline
	);

	// rendering commands
	vk::cmdBindPipeline(commandBuffer, vk::PipelineBindPoint::eGraphics, pipeline);
	vk::cmdDraw(  // draw single triangle
		commandBuffer,
		3,  // vertexCount
		1,  // instanceCount
		0,  // firstVertex
		0   // firstInstance
	);

	// end render pass and command buffer
	vk::cmdEndRenderPass(commandBuffer);
	vk::endCommandBuffer(commandBuffer);

	// submit frame
	vk::resetFences(renderFinishedFence);
	vk::Semaphore renderingFinishedSemaphore = renderingFinishedSemaphores[acquiredImageIndex];
	vk::queueSubmit(
		graphicsQueue,  // queue
		vk::SubmitInfo{  // submits
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.pWaitDstStageMask = nullptr,
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffer,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &renderingFinishedSemaphore,
		},
		renderFinishedFence  // fence
	);

	// present
	vk::Result r =
		vk::queuePresentKHR_noThrow(
			presentationQueue,  // queue
			vk::PresentInfoKHR{
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &renderingFinishedSemaphore,
				.swapchainCount = 1,
				.pSwapchains = swapchain.getPtr(),
				.pImageIndices = &acquiredImageIndex,
				.pResults = nullptr,
			}
		);
	if(r == vk::Result::eSuccess)
		acquiredImageIndex = ~uint32_t(0);
	else {
		if(r == vk::Result::eSuboptimalKHR) {
			acquiredImageIndex = ~uint32_t(0);
			window.scheduleResize();
			cout << "present result: Suboptimal" << endl;
		} else if(r == vk::Result::eErrorOutOfDateKHR) {
			acquiredImageIndex = ~uint32_t(0);
			window.scheduleResize();
			cout << "present error: OutOfDate" << endl;
	#if _WIN32
		} else if(r == vk::Result(-1000255000)) {  // eErrorFullScreenExclusiveModeLostEXT
			acquiredImageIndex = ~uint32_t(0);
	#endif
		} else if(r == vk::Result::eErrorSurfaceLostKHR) {
			acquiredImageIndex = ~uint32_t(0);
			throw runtime_error(string("Vulkan error: vkQueuePresentKHR() failed with error ") + to_cstr(r) + ".");
		} else
			throw runtime_error(string("Vulkan error: vkQueuePresentKHR() failed with error ") + to_cstr(r) + ".");
	}
}


int main(int argc, char* argv[])
{
	// catch exceptions
	// (vulkan.hpp functions throw if they fail)
	try {

		App app(argc, argv);
		app.init();
		app.window.setResizeCallback(
			bind(
				&App::resize,
				&app,
				placeholders::_1,
				placeholders::_2,
				placeholders::_3
			)
		);
		app.window.setFrameCallback(
			bind(&App::frame, &app, placeholders::_1)
		);
		app.window.show();
		app.window.mainLoop();

	// catch exceptions
	} catch(vk::Error& e) {
		cout << "Failed because of Vulkan exception: " << e.what() << endl;
	} catch(exception& e) {
		cout << "Failed because of exception: " << e.what() << endl;
	} catch(...) {
		cout << "Failed because of unspecified exception." << endl;
	}

	return 0;
}
