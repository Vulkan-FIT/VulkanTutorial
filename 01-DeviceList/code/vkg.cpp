#include "vkg.h"
#include <cassert>
#include <memory>
#include <string>
#include <filesystem>
#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN  // this reduces win32 headers default namespace pollution
# include <windows.h>
#else
# include <dlfcn.h>
#endif

using namespace vk;
using namespace std;


// global variables
void* vk::detail::_library = nullptr;
Instance vk::detail::_instance = nullptr;
Device vk::detail::_device = nullptr;
Funcs vk::funcs;


// init and clean up functions
// author: PCJohn (peciva at fit.vut.cz)
void vk::loadLib()
{
#ifdef _WIN32
	loadLib("vulkan-1.dll");
#else
	loadLib("libvulkan.so.1");
#endif
}


void vk::loadLib(const std::filesystem::path& libPath)
{
	// avoid multiple initialization attempts
	if(detail::_library)
		throw VkgError("Vulkan error: Multiple initialization attempts.");

	// load library
	// and get vkGetInstanceProcAddr pointer
#ifdef _WIN32
	detail::_library = reinterpret_cast<void*>(LoadLibraryW(libPath.native().c_str()));
	if(detail::_library == nullptr)
		throw VkgError((string("Vulkan error: Can not open \"") + libPath.string() + "\".").c_str());
	funcs.vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(
		GetProcAddress(reinterpret_cast<HMODULE>(detail::_library), "vkGetInstanceProcAddr"));
#else
	detail::_library = dlopen(libPath.native().c_str(),RTLD_NOW);
	if(detail::_library == nullptr)
		throw VkgError((string("Vulkan error: Can not open \"") + libPath.native() + "\".").c_str());
	funcs.vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(dlsym(detail::_library, "vkGetInstanceProcAddr"));
#endif
	if(funcs.vkGetInstanceProcAddr == nullptr)
		throw VkgError((string("Vulkan error: Can not retrieve vkGetInstanceProcAddr function pointer out of \"") + libPath.string() + ".").c_str());

	// function pointers available without vk::Instance
	funcs.vkCreateInstance = getInstanceProcAddr<PFN_vkCreateInstance>("vkCreateInstance");
	if(funcs.vkCreateInstance == nullptr)
		throw VkgError((string("Vulkan error: Can not retrieve vkCreateInstance function pointer out of \"") + libPath.string() + "\".").c_str());
	funcs.vkEnumerateInstanceVersion = getInstanceProcAddr<PFN_vkEnumerateInstanceVersion>("vkEnumerateInstanceVersion");
	funcs.vkEnumerateInstanceExtensionProperties = getInstanceProcAddr<PFN_vkEnumerateInstanceExtensionProperties>("vkEnumerateInstanceExtensionProperties");
	funcs.vkEnumerateInstanceLayerProperties = getInstanceProcAddr<PFN_vkEnumerateInstanceLayerProperties>("vkEnumerateInstanceLayerProperties");

	// supply fallback function for vkEnumerateInstanceVersion on Vulkan 1.0
	if(funcs.vkEnumerateInstanceVersion == nullptr)
		funcs.vkEnumerateInstanceVersion = [](uint32_t* pApiVersion) -> Result { *pApiVersion = apiVersion1_0; return SUCCESS; };
}


void vk::createInstance(const vk::InstanceCreateInfo& pCreateInfo)
{
	assert(detail::_library && "vk::loadLib() must be called before vk::createInstance().");

	destroyInstance();

	// create instance
	Result r;
	Instance instance;
	if(enumerateInstanceVersion() != apiVersion1_0)
		r = funcs.vkCreateInstance(&pCreateInfo, nullptr, &instance);
	else
	{
		// (To avoid probably unintended exception, handle the special case of non-1.0 Vulkan version request
		// on Vulkan 1.0 system. See vkCreateInstance() documentation.)
		if(pCreateInfo.pApplicationInfo && pCreateInfo.pApplicationInfo->apiVersion != apiVersion1_0)
		{
			// replace requested Vulkan version by 1.0 to avoid throwing the exception
			ApplicationInfo appInfo2(*pCreateInfo.pApplicationInfo);
			appInfo2.apiVersion = apiVersion1_0;
			InstanceCreateInfo createInfo2(pCreateInfo);
			createInfo2.pApplicationInfo = &appInfo2;
			r = funcs.vkCreateInstance(&createInfo2, nullptr, &instance);
		}
		else
			r = funcs.vkCreateInstance(&pCreateInfo, nullptr, &instance);
	}

	initInstance(instance);
}


void vk::initInstance(Instance instance)
{
	assert(detail::_library && "vk::loadLib() must be called before vk::initInstance().");

	destroyInstance();

	detail::_instance = instance;

	funcs.vkDestroyInstance                          = getInstanceProcAddr<PFN_vkDestroyInstance                          >("vkDestroyInstance");
	funcs.vkEnumeratePhysicalDevices                 = getInstanceProcAddr<PFN_vkEnumeratePhysicalDevices                 >("vkEnumeratePhysicalDevices");
	funcs.vkGetPhysicalDeviceProperties              = getInstanceProcAddr<PFN_vkGetPhysicalDeviceProperties              >("vkGetPhysicalDeviceProperties");
	///funcs.vkGetPhysicalDeviceProperties2             = getInstanceProcAddr<PFN_vkGetPhysicalDeviceProperties2             >("vkGetPhysicalDeviceProperties2");
	funcs.vkCreateDevice                             = getInstanceProcAddr<PFN_vkCreateDevice                             >("vkCreateDevice");
	funcs.vkGetDeviceProcAddr                        = getInstanceProcAddr<PFN_vkGetDeviceProcAddr                        >("vkGetDeviceProcAddr");
	/*funcs.vkEnumerateDeviceExtensionProperties       = getInstanceProcAddr<PFN_vkEnumerateDeviceExtensionProperties       >("vkEnumerateDeviceExtensionProperties");
	//funcs.vkGetPhysicalDeviceSurfaceFormatsKHR       = getInstanceProcAddr<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR       >("vkGetPhysicalDeviceSurfaceFormatsKHR");
	funcs.vkGetPhysicalDeviceFormatProperties        = getInstanceProcAddr<PFN_vkGetPhysicalDeviceFormatProperties        >("vkGetPhysicalDeviceFormatProperties");
	funcs.vkGetPhysicalDeviceMemoryProperties        = getInstanceProcAddr<PFN_vkGetPhysicalDeviceMemoryProperties        >("vkGetPhysicalDeviceMemoryProperties");
	//funcs.vkGetPhysicalDeviceSurfacePresentModesKHR  = getInstanceProcAddr<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR  >("vkGetPhysicalDeviceSurfacePresentModesKHR");
	funcs.vkGetPhysicalDeviceQueueFamilyProperties   = getInstanceProcAddr<PFN_vkGetPhysicalDeviceQueueFamilyProperties   >("vkGetPhysicalDeviceQueueFamilyProperties");
	//funcs.vkGetPhysicalDeviceSurfaceSupportKHR       = getInstanceProcAddr<PFN_vkGetPhysicalDeviceSurfaceSupportKHR       >("vkGetPhysicalDeviceSurfaceSupportKHR");
	funcs.vkGetPhysicalDeviceFeatures                = getInstanceProcAddr<PFN_vkGetPhysicalDeviceFeatures                >("vkGetPhysicalDeviceFeatures");
	funcs.vkGetPhysicalDeviceFeatures2               = getInstanceProcAddr<PFN_vkGetPhysicalDeviceFeatures2               >("vkGetPhysicalDeviceFeatures2");
	//funcs.vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = getInstanceProcAddr<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>("vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");*/
}


void vk::initDevice(Device device)
{
	assert(detail::_instance && "vk::createInstance() or vk::initInstance() must be called before vk::initDevice().");

	destroyDevice();

	detail::_device = device;

	funcs.vkGetDeviceProcAddr  = getDeviceProcAddr<PFN_vkGetDeviceProcAddr  >("vkGetDeviceProcAddr");
	funcs.vkDestroyDevice      = getDeviceProcAddr<PFN_vkDestroyDevice      >("vkDestroyDevice");
	//funcs.vkGetDeviceQueue     = getDeviceProcAddr<PFN_vkGetDeviceQueue     >("vkGetDeviceQueue");
/*	vkCreateRenderPass   =getProcAddr<PFN_vkCreateRenderPass   >("vkCreateRenderPass");
	vkDestroyRenderPass  =getProcAddr<PFN_vkDestroyRenderPass  >("vkDestroyRenderPass");
	vkCreateBuffer       =getProcAddr<PFN_vkCreateBuffer       >("vkCreateBuffer");
	vkDestroyBuffer      =getProcAddr<PFN_vkDestroyBuffer      >("vkDestroyBuffer");
	vkGetBufferDeviceAddress=getProcAddr<PFN_vkGetBufferDeviceAddress>("vkGetBufferDeviceAddress");
	vkAllocateMemory     =getProcAddr<PFN_vkAllocateMemory     >("vkAllocateMemory");
	vkBindBufferMemory   =getProcAddr<PFN_vkBindBufferMemory   >("vkBindBufferMemory");
	vkBindImageMemory    =getProcAddr<PFN_vkBindImageMemory    >("vkBindImageMemory");
	vkFreeMemory         =getProcAddr<PFN_vkFreeMemory         >("vkFreeMemory");
	vkGetBufferMemoryRequirements =getProcAddr<PFN_vkGetBufferMemoryRequirements>("vkGetBufferMemoryRequirements");
	vkGetImageMemoryRequirements  =getProcAddr<PFN_vkGetImageMemoryRequirements >("vkGetImageMemoryRequirements");
	vkMapMemory          =getProcAddr<PFN_vkMapMemory          >("vkMapMemory");
	vkUnmapMemory        =getProcAddr<PFN_vkUnmapMemory        >("vkUnmapMemory");
	vkFlushMappedMemoryRanges=getProcAddr<PFN_vkFlushMappedMemoryRanges>("vkFlushMappedMemoryRanges");
	vkCreateImage        =getProcAddr<PFN_vkCreateImage        >("vkCreateImage");
	vkDestroyImage       =getProcAddr<PFN_vkDestroyImage       >("vkDestroyImage");
	vkCreateImageView    =getProcAddr<PFN_vkCreateImageView    >("vkCreateImageView");
	vkDestroyImageView   =getProcAddr<PFN_vkDestroyImageView   >("vkDestroyImageView");
	vkCreateSampler      =getProcAddr<PFN_vkCreateSampler      >("vkCreateSampler");
	vkDestroySampler     =getProcAddr<PFN_vkDestroySampler     >("vkDestroySampler");
	vkCreateFramebuffer  =getProcAddr<PFN_vkCreateFramebuffer  >("vkCreateFramebuffer");
	vkDestroyFramebuffer =getProcAddr<PFN_vkDestroyFramebuffer >("vkDestroyFramebuffer");
	vkCreateSwapchainKHR =getProcAddr<PFN_vkCreateSwapchainKHR >("vkCreateSwapchainKHR");
	vkDestroySwapchainKHR=getProcAddr<PFN_vkDestroySwapchainKHR>("vkDestroySwapchainKHR");
	vkGetSwapchainImagesKHR=getProcAddr<PFN_vkGetSwapchainImagesKHR>("vkGetSwapchainImagesKHR");
	vkAcquireNextImageKHR=getProcAddr<PFN_vkAcquireNextImageKHR>("vkAcquireNextImageKHR");
	vkQueuePresentKHR    =getProcAddr<PFN_vkQueuePresentKHR    >("vkQueuePresentKHR");
	vkCreateShaderModule =getProcAddr<PFN_vkCreateShaderModule >("vkCreateShaderModule");
	vkDestroyShaderModule=getProcAddr<PFN_vkDestroyShaderModule>("vkDestroyShaderModule");
	vkCreateDescriptorSetLayout=getProcAddr<PFN_vkCreateDescriptorSetLayout>("vkCreateDescriptorSetLayout");
	vkDestroyDescriptorSetLayout=getProcAddr<PFN_vkDestroyDescriptorSetLayout>("vkDestroyDescriptorSetLayout");
	vkCreateDescriptorPool=getProcAddr<PFN_vkCreateDescriptorPool>("vkCreateDescriptorPool");
	vkDestroyDescriptorPool=getProcAddr<PFN_vkDestroyDescriptorPool>("vkDestroyDescriptorPool");
	vkResetDescriptorPool=getProcAddr<PFN_vkResetDescriptorPool>("vkResetDescriptorPool");
	vkAllocateDescriptorSets=getProcAddr<PFN_vkAllocateDescriptorSets>("vkAllocateDescriptorSets");
	vkUpdateDescriptorSets=getProcAddr<PFN_vkUpdateDescriptorSets>("vkUpdateDescriptorSets");
	vkFreeDescriptorSets =getProcAddr<PFN_vkFreeDescriptorSets >("vkFreeDescriptorSets");
	vkCreatePipelineCache=getProcAddr<PFN_vkCreatePipelineCache>("vkCreatePipelineCache");
	vkDestroyPipelineCache=getProcAddr<PFN_vkDestroyPipelineCache>("vkDestroyPipelineCache");
	vkCreatePipelineLayout=getProcAddr<PFN_vkCreatePipelineLayout>("vkCreatePipelineLayout");
	vkDestroyPipelineLayout=getProcAddr<PFN_vkDestroyPipelineLayout>("vkDestroyPipelineLayout");
	vkCreateGraphicsPipelines=getProcAddr<PFN_vkCreateGraphicsPipelines>("vkCreateGraphicsPipelines");
	vkCreateComputePipelines=getProcAddr<PFN_vkCreateComputePipelines>("vkCreateComputePipelines");
	vkDestroyPipeline    =getProcAddr<PFN_vkDestroyPipeline    >("vkDestroyPipeline");
	vkCreateSemaphore    =getProcAddr<PFN_vkCreateSemaphore    >("vkCreateSemaphore");
	vkDestroySemaphore   =getProcAddr<PFN_vkDestroySemaphore   >("vkDestroySemaphore");
	vkCreateCommandPool  =getProcAddr<PFN_vkCreateCommandPool  >("vkCreateCommandPool");
	vkDestroyCommandPool =getProcAddr<PFN_vkDestroyCommandPool >("vkDestroyCommandPool");
	vkAllocateCommandBuffers=getProcAddr<PFN_vkAllocateCommandBuffers>("vkAllocateCommandBuffers");
	vkFreeCommandBuffers =getProcAddr<PFN_vkFreeCommandBuffers >("vkFreeCommandBuffers");
	vkBeginCommandBuffer =getProcAddr<PFN_vkBeginCommandBuffer >("vkBeginCommandBuffer");
	vkEndCommandBuffer   =getProcAddr<PFN_vkEndCommandBuffer   >("vkEndCommandBuffer");
	vkResetCommandPool   =getProcAddr<PFN_vkResetCommandPool   >("vkResetCommandPool");
	vkCmdPushConstants   =getProcAddr<PFN_vkCmdPushConstants   >("vkCmdPushConstants");
	vkCmdBeginRenderPass =getProcAddr<PFN_vkCmdBeginRenderPass >("vkCmdBeginRenderPass");
	vkCmdEndRenderPass   =getProcAddr<PFN_vkCmdEndRenderPass   >("vkCmdEndRenderPass");
	vkCmdExecuteCommands =getProcAddr<PFN_vkCmdExecuteCommands >("vkCmdExecuteCommands");
	vkCmdCopyBuffer      =getProcAddr<PFN_vkCmdCopyBuffer      >("vkCmdCopyBuffer");
	vkCreateFence        =getProcAddr<PFN_vkCreateFence        >("vkCreateFence");
	vkDestroyFence       =getProcAddr<PFN_vkDestroyFence       >("vkDestroyFence");
	vkCmdBindPipeline    =getProcAddr<PFN_vkCmdBindPipeline    >("vkCmdBindPipeline");
	vkCmdBindDescriptorSets=getProcAddr<PFN_vkCmdBindDescriptorSets>("vkCmdBindDescriptorSets");
	vkCmdBindIndexBuffer =getProcAddr<PFN_vkCmdBindIndexBuffer >("vkCmdBindIndexBuffer");
	vkCmdBindVertexBuffers=getProcAddr<PFN_vkCmdBindVertexBuffers>("vkCmdBindVertexBuffers");
	vkCmdDrawIndexedIndirect=getProcAddr<PFN_vkCmdDrawIndexedIndirect>("vkCmdDrawIndexedIndirect");
	vkCmdDrawIndexed     =getProcAddr<PFN_vkCmdDrawIndexed     >("vkCmdDrawIndexed");
	vkCmdDraw            =getProcAddr<PFN_vkCmdDraw            >("vkCmdDraw");
	vkCmdDrawIndirect    =getProcAddr<PFN_vkCmdDrawIndirect    >("vkCmdDrawIndirect");
	vkCmdDispatch        =getProcAddr<PFN_vkCmdDispatch        >("vkCmdDispatch");
	vkCmdDispatchIndirect=getProcAddr<PFN_vkCmdDispatchIndirect>("vkCmdDispatchIndirect");
	vkCmdDispatchBase    =getProcAddr<PFN_vkCmdDispatchBase    >("vkCmdDispatchBase");
	vkCmdPipelineBarrier =getProcAddr<PFN_vkCmdPipelineBarrier >("vkCmdPipelineBarrier");
	vkCmdSetDepthBias    =getProcAddr<PFN_vkCmdSetDepthBias    >("vkCmdSetDepthBias");
	vkCmdSetLineWidth    =getProcAddr<PFN_vkCmdSetLineWidth    >("vkCmdSetLineWidth");
	vkCmdSetLineStippleEXT=getProcAddr<PFN_vkCmdSetLineStippleEXT>("vkCmdSetLineStippleEXT");
	vkQueueSubmit        =getProcAddr<PFN_vkQueueSubmit        >("vkQueueSubmit");
	vkWaitForFences      =getProcAddr<PFN_vkWaitForFences      >("vkWaitForFences");
	vkResetFences        =getProcAddr<PFN_vkResetFences        >("vkResetFences");
	vkQueueWaitIdle      =getProcAddr<PFN_vkQueueWaitIdle      >("vkQueueWaitIdle");
	vkDeviceWaitIdle     =getProcAddr<PFN_vkDeviceWaitIdle     >("vkDeviceWaitIdle");
	vkCmdResetQueryPool  =getProcAddr<PFN_vkCmdResetQueryPool  >("vkCmdResetQueryPool");
	vkCmdWriteTimestamp  =getProcAddr<PFN_vkCmdWriteTimestamp  >("vkCmdWriteTimestamp");
	vkGetCalibratedTimestampsEXT=getProcAddr<PFN_vkGetCalibratedTimestampsEXT>("vkGetCalibratedTimestampsEXT");
	vkCreateQueryPool    =getProcAddr<PFN_vkCreateQueryPool    >("vkCreateQueryPool");
	vkDestroyQueryPool   =getProcAddr<PFN_vkDestroyQueryPool   >("vkDestroyQueryPool");
	vkGetQueryPoolResults=getProcAddr<PFN_vkGetQueryPoolResults>("vkGetQueryPoolResults");*/
}


void vk::destroyDevice() noexcept
{
	if(detail::_device) {
		funcs.vkDestroyDevice(detail::_device, nullptr);
		detail::_device = nullptr;
	}
}


void vk::destroyInstance() noexcept
{
	if(detail::_instance) {
		funcs.vkDestroyInstance(detail::_instance, nullptr);
		detail::_instance = nullptr;
	}
}


void vk::unloadLib() noexcept
{
	if(detail::_library) {

		// release Vulkan library
#ifdef _WIN32
		FreeLibrary(reinterpret_cast<HMODULE>(detail::_library));
#else
		dlclose(detail::_library);
#endif
		detail::_library = nullptr;
	}
}


void vk::cleanUp() noexcept
{
	destroyDevice();
	destroyInstance();
	unloadLib();
}


// nifty counter / Schwarz counter
// author: PCJohn (peciva at fit.vut.cz)
static int niftyCounter = 0;

vk::VkgInitAndCleanUp::VkgInitAndCleanUp()
{
	niftyCounter++;
}

vk::VkgInitAndCleanUp::~VkgInitAndCleanUp()
{
	if(--niftyCounter == 0)
		vk::cleanUp();
}


template<typename Type>
Array<Type>::Array(const Array& other)
	: _data(new Type[other._size])
	, _size(other._size)
{
	try {
		std::uninitialized_copy_n(other._data, other._size, _data);
	} catch(...) {
		::operator delete[](_data);
		_data = nullptr;
		throw;
	}
}


template<typename Type>
Array<Type>& Array<Type>::operator=(const Array& rhs)
{
	if(_size == rhs._size)
		std::copy_n(rhs._data, _size, _data);
	else {
		delete[] _data;
		_size = rhs._size;
		_data = ::operator new(sizeof(Type) * _size);
		try {
			std::uninitialized_copy_n(rhs._data, rhs._size, _data);
		} catch(...) {
			::operator delete[](_data);
			_data = nullptr;
			throw;
		}
	}
}


template<typename Type>
void Array<Type>::resize(size_t newSize)
{
	if(newSize > size()) {
		Type* m = reinterpret_cast<Type*>(::operator new(sizeof(Type) * newSize));
		try {
			std::uninitialized_move(_data, _data+_size, m);
		} catch(...) {
			::operator delete[](m);
			throw;
		}
		try {
			std::uninitialized_default_construct(m+_size, m+newSize);
		} catch(...) {
			std::destroy(m, m+_size);
			::operator delete[](m);
			throw;
		}
		Type* tmp = _data;
		_data = m;
		delete[] tmp;
	}
	if(newSize < size()) {
		Type* m = reinterpret_cast<Type*>(::operator new(sizeof(Type) * newSize));
		try {
			std::uninitialized_move(_data, _data+newSize, m);
		} catch(...) {
			::operator delete[](m);
			throw;
		}
		Type* tmp = _data;
		_data = m;
		delete[] _data;
	}
}


// convert VkResult to string
// author: PCJohn (peciva at fit.vut.cz)
constexpr const char* outOfHostMemoryString = "OutOfHostMemory"; 
constexpr const char* outOfDeviceMemoryString = "OutOfDeviceMemory"; 
constexpr const char* initializationFailedString = "InitializationFailed"; 
constexpr const char* unknownVkResultValueString = "UnknownVkResultValue"; 


Span<const char> vk::resultToString(Result result)
{
	switch(result) {
	case ERROR_OUT_OF_HOST_MEMORY: return { outOfHostMemoryString, 15 };
	case ERROR_OUT_OF_DEVICE_MEMORY: return { outOfDeviceMemoryString, 17 };
	case ERROR_INITIALIZATION_FAILED: return { initializationFailedString, 20 };
	default: return { unknownVkResultValueString, 20 };
	/*case ERROR_DEVICE_LOST = -4,
	case ERROR_MEMORY_MAP_FAILED = -5,
	case ERROR_LAYER_NOT_PRESENT = -6,
	case ERROR_EXTENSION_NOT_PRESENT = -7,
	case ERROR_FEATURE_NOT_PRESENT = -8,
	case ERROR_INCOMPATIBLE_DRIVER = -9,
	case ERROR_TOO_MANY_OBJECTS = -10,
	case ERROR_FORMAT_NOT_SUPPORTED = -11,
	case ERROR_FRAGMENTED_POOL = -12,
	case ERROR_UNKNOWN = -13,
	// Provided by VK_VERSION_1_1
	case ERROR_OUT_OF_POOL_MEMORY = -1000069000,
	// Provided by VK_VERSION_1_1
	case ERROR_INVALID_EXTERNAL_HANDLE = -1000072003,
	// Provided by VK_VERSION_1_2
	case ERROR_FRAGMENTATION = -1000161000,
	// Provided by VK_VERSION_1_2
	case ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS = -1000257000,
	*/}
}


size_t vk::int32ToString(int32_t value, char* buffer)
{
	// print '-' for negative numbers
	size_t len;
	if(value < 0) {
		buffer[0] = '-';
		len = 1;
	}
	else
		len = 0;

	// print numbers
	do {
		buffer[len] = value % 10 + '0';
		value = value / 10;
		len++;
	} while(value != 0);

	// terminating zero
	buffer[len] = 0;
	return len;
}


// exceptions
// author: PCJohn (peciva at fit.vut.cz)
void vk::throwResultException(const char* funcName, Result result)
{
	switch(result) {
	case ERROR_OUT_OF_HOST_MEMORY: throw OutOfHostMemoryError(funcName, result);
	case ERROR_OUT_OF_DEVICE_MEMORY: throw OutOfDeviceMemoryError(funcName, result);
	case ERROR_INITIALIZATION_FAILED: throw InitializationFailedError(funcName, result);
	}
}

void vk::throwResultException(Result result, const char* message)
{
	switch(result) {
	case ERROR_OUT_OF_HOST_MEMORY: throw OutOfHostMemoryError(message);
	case ERROR_OUT_OF_DEVICE_MEMORY: throw OutOfDeviceMemoryError(message);
	case ERROR_INITIALIZATION_FAILED: throw InitializationFailedError(message);
	/*case ERROR_DEVICE_LOST = -4,
	case ERROR_MEMORY_MAP_FAILED = -5,
	case ERROR_LAYER_NOT_PRESENT = -6,
	case ERROR_EXTENSION_NOT_PRESENT = -7,
	case ERROR_FEATURE_NOT_PRESENT = -8,
	case ERROR_INCOMPATIBLE_DRIVER = -9,
	case ERROR_TOO_MANY_OBJECTS = -10,
	case ERROR_FORMAT_NOT_SUPPORTED = -11,
	case ERROR_FRAGMENTED_POOL = -12,
	case ERROR_UNKNOWN = -13,
	// Provided by VK_VERSION_1_1
	case ERROR_OUT_OF_POOL_MEMORY = -1000069000,
	// Provided by VK_VERSION_1_1
	case ERROR_INVALID_EXTERNAL_HANDLE = -1000072003,
	// Provided by VK_VERSION_1_2
	case ERROR_FRAGMENTATION = -1000161000,
	// Provided by VK_VERSION_1_2
	case ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS = -1000257000,
	*/}
}

Error::Error(const char* msgHeader, const char* msgBody) noexcept
{
	size_t l1 = strlen(msgHeader);
	size_t l2 = strlen(msgBody);
	_msg = new char[l1+l2+1];
	memcpy(&_msg[0], msgHeader, l1);
	memcpy(&_msg[l1], msgBody, l2+1);
}

Error::Error(const char* funcName, Result result) noexcept
{
	// get components of the message
	// (VkResult converted to string, funcName length, VkResult converted to integer string)
	Span<const char> resultText = resultToString(result);
	size_t funcNameLength = funcName ? strlen(funcName) : 0;
	char codeText[12];
	size_t codeLength = int32ToString(result, codeText);
	size_t n = 14 + funcNameLength + 21 + resultText.size() + 17 + codeLength + 2 + 1;

	// construct message
	// example string: "Vulkan error: vkCreateInstance() failed with error InitializationFailed (VkResult code -3)."
	_msg = new char[n];
	memcpy(&_msg[0], "Vulkan error: ", 14);
	memcpy(&_msg[14], funcName, funcNameLength);
	size_t i = 14 + funcNameLength;
	memcpy(&_msg[i], "() failed with error ", 21);
	i += 21;
	memcpy(&_msg[i], resultText.data(), resultText.size());
	i += resultText.size();
	memcpy(&_msg[i], " (VkResult code ", 17);
	i += 17;
	memcpy(&_msg[i], codeText, codeLength);
	i += codeLength;
	memcpy(&_msg[i], ").", 3);
}


uint32_t vk::enumerateInstanceVersion()
{
	uint32_t version;
	Result r = funcs.vkEnumerateInstanceVersion(&version);
	if(r != SUCCESS)
		throwResultException("vkEnumerateInstanceVersion", r);
	return version;
}


Array<PhysicalDevice> vk::enumeratePhysicalDevices()
{
	Array<PhysicalDevice> a;
	uint32_t n;
	Result r;
	do {
		// get num devices
		r = funcs.vkEnumeratePhysicalDevices(instance(), &n, nullptr);
		if(r != SUCCESS)
			throwResultException("vkEnumeratePhysicalDevices", r);
		
		// enumerate physical devices
		a.alloc(n);
		r = funcs.vkEnumeratePhysicalDevices(instance(), &n, a.data());
		if(r < 0)
			throwResultException("vkEnumeratePhysicalDevices", r);

	} while(r == INCOMPLETE);

	// number of devices might change between two successive vkEnumeratePhysicalDevices() calls
	if(n != a.size())
		a.resize(n);

	return a;
}