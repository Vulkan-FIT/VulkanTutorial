#include "vkg.hpp"
#include <cassert>
#include <cstdlib>
#include <string>
#include <vector>
#include <filesystem>
#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN  // this reduces win32 headers default namespace pollution
# include <windows.h>
#else
# include <dlfcn.h>
#endif
namespace vk {

void* detail::_library = nullptr;
Instance detail::_instance = nullptr;
PhysicalDevice detail::_physicalDevice = nullptr;
Device detail::_device = nullptr;
uint32_t detail::_instanceVersion = 0;
const AllocationCallbacks* detail::_allocator = nullptr;
Funcs detail::_funcs;
int Context::_contextCounter{0};
struct DestroyCallbackData {
    Context::CallbackType callbackType;
    void (*func)(void*);
    void* data;
};
static std::vector<DestroyCallbackData> atDestroyList;

Context::Context() {
    ++_contextCounter;
}
void Context::atDestroy(CallbackType callbackType, void(*func)(void*), void* data)
{
    atDestroyList.emplace_back(callbackType, func, data);
}
Context::~Context() noexcept {
    auto callAtDestroyCallbacks =
        [](CallbackType callbackType) {
            for(auto cb : atDestroyList)
                if(cb.callbackType == callbackType)
                    cb.func(cb.data);
        };
    if(--_contextCounter == 0) {
        callAtDestroyCallbacks(CallbackType::eBeforeDeviceDestroy);
        destroyDevice();
        callAtDestroyCallbacks(CallbackType::eBeforeInstanceDestroy);
        destroyInstance();
        callAtDestroyCallbacks(CallbackType::eBeforeUnloadLib);
        unloadLib();
        callAtDestroyCallbacks(CallbackType::eAfterCleanUp);
        atDestroyList.clear();
    }
}

Error::Error(const char* msgHeader, const char* msgBody) noexcept {
    size_t l1 = strlen(msgHeader); size_t l2 = strlen(msgBody);
    _msg = reinterpret_cast<char*>(malloc(l1 + l2 + 1));
    if (_msg) { memcpy(_msg, msgHeader, l1); memcpy(_msg + l1, msgBody, l2 + 1); }
}
Error::Error(const char* funcName, Result) noexcept {
    if (funcName) { size_t n = strlen(funcName) + 1; _msg = reinterpret_cast<char*>(malloc(n)); if (_msg) strncpy(_msg, funcName, n); }
}

void throwResultException(Result result, const char* funcName) {
    switch (result) {
    case Result::eSuccess: throw SuccessResult(funcName, result);
    case Result::eNotReady: throw NotReadyResult(funcName, result);
    case Result::eTimeout: throw TimeoutResult(funcName, result);
    case Result::eEventSet: throw EventSetResult(funcName, result);
    case Result::eEventReset: throw EventResetResult(funcName, result);
    case Result::eIncomplete: throw IncompleteResult(funcName, result);
    case Result::eErrorOutOfHostMemory: throw OutOfHostMemoryError(funcName, result);
    case Result::eErrorOutOfDeviceMemory: throw OutOfDeviceMemoryError(funcName, result);
    case Result::eErrorInitializationFailed: throw InitializationFailedError(funcName, result);
    case Result::eErrorDeviceLost: throw DeviceLostError(funcName, result);
    case Result::eErrorMemoryMapFailed: throw MemoryMapFailedError(funcName, result);
    case Result::eErrorLayerNotPresent: throw LayerNotPresentError(funcName, result);
    case Result::eErrorExtensionNotPresent: throw ExtensionNotPresentError(funcName, result);
    case Result::eErrorFeatureNotPresent: throw FeatureNotPresentError(funcName, result);
    case Result::eErrorIncompatibleDriver: throw IncompatibleDriverError(funcName, result);
    case Result::eErrorTooManyObjects: throw TooManyObjectsError(funcName, result);
    case Result::eErrorFormatNotSupported: throw FormatNotSupportedError(funcName, result);
    case Result::eErrorFragmentedPool: throw FragmentedPoolError(funcName, result);
    case Result::eErrorUnknown: throw UnknownError(funcName, result);
    case Result::eErrorSurfaceLostKHR: throw SurfaceLostKHRError(funcName, result);
    case Result::eErrorNativeWindowInUseKHR: throw NativeWindowInUseKHRError(funcName, result);
    case Result::eSuboptimalKHR: throw SuboptimalKHRResult(funcName, result);
    case Result::eErrorOutOfDateKHR: throw OutOfDateKHRError(funcName, result);
    case Result::eErrorIncompatibleDisplayKHR: throw IncompatibleDisplayKHRError(funcName, result);
    case Result::eErrorValidationFailedEXT: throw ValidationFailedEXTError(funcName, result);
    case Result::eErrorInvalidShaderNV: throw InvalidShaderNVError(funcName, result);
    case Result::eErrorOutOfPoolMemory: throw OutOfPoolMemoryError(funcName, result);
    case Result::eErrorInvalidExternalHandle: throw InvalidExternalHandleError(funcName, result);
    case Result::eErrorInvalidDrmFormatModifierPlaneLayoutEXT: throw InvalidDrmFormatModifierPlaneLayoutEXTError(funcName, result);
    case Result::eErrorFragmentation: throw FragmentationError(funcName, result);
    case Result::eErrorNotPermitted: throw NotPermittedError(funcName, result);
    case Result::eErrorInvalidOpaqueCaptureAddress: throw InvalidOpaqueCaptureAddressError(funcName, result);
    case Result::eThreadIdleKHR: throw ThreadIdleKHRResult(funcName, result);
    case Result::eThreadDoneKHR: throw ThreadDoneKHRResult(funcName, result);
    case Result::eOperationDeferredKHR: throw OperationDeferredKHRResult(funcName, result);
    case Result::eOperationNotDeferredKHR: throw OperationNotDeferredKHRResult(funcName, result);
    case Result::ePipelineCompileRequired: throw PipelineCompileRequiredResult(funcName, result);
    case Result::eErrorCompressionExhaustedEXT: throw CompressionExhaustedEXTError(funcName, result);
    case Result::eIncompatibleShaderBinaryEXT: throw IncompatibleShaderBinaryEXTResult(funcName, result);
    case Result::ePipelineBinaryMissingKHR: throw PipelineBinaryMissingKHRResult(funcName, result);
    case Result::eErrorNotEnoughSpaceKHR: throw NotEnoughSpaceKHRError(funcName, result);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    case Result::eErrorFullScreenExclusiveModeLostEXT: throw FullScreenExclusiveModeLostEXTError(funcName, result);
#endif // VK_USE_PLATFORM_WIN32_KHR
    default: throw VkgError("throwResultException", result);
    }
}

void throwResultExceptionWithMessage(Result result, const char* message) {
    switch (result) {
    case Result::eSuccess: throw SuccessResult(message);
    case Result::eNotReady: throw NotReadyResult(message);
    case Result::eTimeout: throw TimeoutResult(message);
    case Result::eEventSet: throw EventSetResult(message);
    case Result::eEventReset: throw EventResetResult(message);
    case Result::eIncomplete: throw IncompleteResult(message);
    case Result::eErrorOutOfHostMemory: throw OutOfHostMemoryError(message);
    case Result::eErrorOutOfDeviceMemory: throw OutOfDeviceMemoryError(message);
    case Result::eErrorInitializationFailed: throw InitializationFailedError(message);
    case Result::eErrorDeviceLost: throw DeviceLostError(message);
    case Result::eErrorMemoryMapFailed: throw MemoryMapFailedError(message);
    case Result::eErrorLayerNotPresent: throw LayerNotPresentError(message);
    case Result::eErrorExtensionNotPresent: throw ExtensionNotPresentError(message);
    case Result::eErrorFeatureNotPresent: throw FeatureNotPresentError(message);
    case Result::eErrorIncompatibleDriver: throw IncompatibleDriverError(message);
    case Result::eErrorTooManyObjects: throw TooManyObjectsError(message);
    case Result::eErrorFormatNotSupported: throw FormatNotSupportedError(message);
    case Result::eErrorFragmentedPool: throw FragmentedPoolError(message);
    case Result::eErrorUnknown: throw UnknownError(message);
    case Result::eErrorSurfaceLostKHR: throw SurfaceLostKHRError(message);
    case Result::eErrorNativeWindowInUseKHR: throw NativeWindowInUseKHRError(message);
    case Result::eSuboptimalKHR: throw SuboptimalKHRResult(message);
    case Result::eErrorOutOfDateKHR: throw OutOfDateKHRError(message);
    case Result::eErrorIncompatibleDisplayKHR: throw IncompatibleDisplayKHRError(message);
    case Result::eErrorValidationFailedEXT: throw ValidationFailedEXTError(message);
    case Result::eErrorInvalidShaderNV: throw InvalidShaderNVError(message);
    case Result::eErrorOutOfPoolMemory: throw OutOfPoolMemoryError(message);
    case Result::eErrorInvalidExternalHandle: throw InvalidExternalHandleError(message);
    case Result::eErrorInvalidDrmFormatModifierPlaneLayoutEXT: throw InvalidDrmFormatModifierPlaneLayoutEXTError(message);
    case Result::eErrorFragmentation: throw FragmentationError(message);
    case Result::eErrorNotPermitted: throw NotPermittedError(message);
    case Result::eErrorInvalidOpaqueCaptureAddress: throw InvalidOpaqueCaptureAddressError(message);
    case Result::eThreadIdleKHR: throw ThreadIdleKHRResult(message);
    case Result::eThreadDoneKHR: throw ThreadDoneKHRResult(message);
    case Result::eOperationDeferredKHR: throw OperationDeferredKHRResult(message);
    case Result::eOperationNotDeferredKHR: throw OperationNotDeferredKHRResult(message);
    case Result::ePipelineCompileRequired: throw PipelineCompileRequiredResult(message);
    case Result::eErrorCompressionExhaustedEXT: throw CompressionExhaustedEXTError(message);
    case Result::eIncompatibleShaderBinaryEXT: throw IncompatibleShaderBinaryEXTResult(message);
    case Result::ePipelineBinaryMissingKHR: throw PipelineBinaryMissingKHRResult(message);
    case Result::eErrorNotEnoughSpaceKHR: throw NotEnoughSpaceKHRError(message);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    case Result::eErrorFullScreenExclusiveModeLostEXT: throw FullScreenExclusiveModeLostEXTError(message);
#endif // VK_USE_PLATFORM_WIN32_KHR
    default: throw VkgError("throwResultException", result);
    }
}

void loadLib_throw() {
#ifdef _WIN32
    loadLib_throw("vulkan-1.dll");
#else
    loadLib_throw("libvulkan.so.1");
#endif
}


Result loadLib_noThrow() noexcept {
#ifdef _WIN32
    return loadLib_noThrow("vulkan-1.dll");
#else
    return loadLib_noThrow("libvulkan.so.1");
#endif
}


void loadLib_throw(const char* libPath) {
    // avoid multiple initialization attempts
    if (detail::_library)
        throw VkgError("Vulkan error: Multiple initialization attempts.");

    // load library
    // and get vkGetInstanceProcAddr pointer
    std::filesystem::path p = std::filesystem::path(libPath);
#ifdef _WIN32
    detail::_library = reinterpret_cast<void*>(LoadLibraryW(p.native().c_str()));
    if (detail::_library == nullptr)
        throw VkgError((std::string("Vulkan error: Can not open \"") + p.string() + "\".").c_str());
    funcs.vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(
        GetProcAddress(reinterpret_cast<HMODULE>(detail::_library), "vkGetInstanceProcAddr"));
#else
    detail::_library = dlopen(p.native().c_str(), RTLD_NOW);
    if (detail::_library == nullptr)
        throw VkgError((std::string("Vulkan error: Can not open \"") + p.native() + "\".").c_str());
    funcs.vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(dlsym(detail::_library, "vkGetInstanceProcAddr"));
#endif
    if (funcs.vkGetInstanceProcAddr == nullptr) {
        unloadLib();
        throw VkgError((std::string("Vulkan error: Can not retrieve vkGetInstanceProcAddr function pointer out of \"") + p.string() + ".").c_str());
    }

    // function pointers available without vk::Instance
    funcs.vkEnumerateInstanceExtensionProperties = getInstanceProcAddr<PFN_vkEnumerateInstanceExtensionProperties>("vkEnumerateInstanceExtensionProperties");
    funcs.vkEnumerateInstanceLayerProperties = getInstanceProcAddr<PFN_vkEnumerateInstanceLayerProperties>("vkEnumerateInstanceLayerProperties");
    funcs.vkCreateInstance = getInstanceProcAddr<PFN_vkCreateInstance>("vkCreateInstance");
    PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion = getInstanceProcAddr<PFN_vkEnumerateInstanceVersion>("vkEnumerateInstanceVersion");

    // instance version
    if (vkEnumerateInstanceVersion) {
        uint32_t v;
        Result r = vkEnumerateInstanceVersion(&v);
        if (r != Result::eSuccess) {
            unloadLib();
            throwResultException(r, "vkEnumerateInstanceVersion");
        }
        detail::_instanceVersion = v;
    } else
        detail::_instanceVersion = ApiVersion10;
}


Result loadLib_noThrow(const char* libPath) noexcept {
    // avoid multiple initialization attempts
    if (detail::_library)
        return Result::eErrorUnknown;

    // load library
    // and get vkGetInstanceProcAddr pointer
    std::filesystem::path p = std::filesystem::path(libPath);
#ifdef _WIN32
    detail::_library = reinterpret_cast<void*>(LoadLibraryW(p.native().c_str()));
    if (detail::_library == nullptr)
        return Result::eErrorInitializationFailed;
    funcs.vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(
        GetProcAddress(reinterpret_cast<HMODULE>(detail::_library), "vkGetInstanceProcAddr"));
#else
    detail::_library = dlopen(p.native().c_str(), RTLD_NOW);
    if (detail::_library == nullptr)
        return Result::eErrorInitializationFailed;
    funcs.vkGetInstanceProcAddr = PFN_vkGetInstanceProcAddr(dlsym(detail::_library, "vkGetInstanceProcAddr"));
#endif
    if (funcs.vkGetInstanceProcAddr == nullptr) {
        unloadLib();
        return Result::eErrorIncompatibleDriver;
    }

    // function pointers available without vk::Instance
    funcs.vkEnumerateInstanceExtensionProperties = getInstanceProcAddr<PFN_vkEnumerateInstanceExtensionProperties>("vkEnumerateInstanceExtensionProperties");
    funcs.vkEnumerateInstanceLayerProperties = getInstanceProcAddr<PFN_vkEnumerateInstanceLayerProperties>("vkEnumerateInstanceLayerProperties");
    funcs.vkCreateInstance = getInstanceProcAddr<PFN_vkCreateInstance>("vkCreateInstance");
    PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion = getInstanceProcAddr<PFN_vkEnumerateInstanceVersion>("vkEnumerateInstanceVersion");

    // instance version
    if (vkEnumerateInstanceVersion) {
        uint32_t v;
        Result r = vkEnumerateInstanceVersion(&v);
        if (r != Result::eSuccess) {
            unloadLib();
            return r;
        }
        detail::_instanceVersion = v;
    } else
        detail::_instanceVersion = ApiVersion10;

    return Result::eSuccess;
}

void unloadLib() noexcept {
    if (detail::_library) {
#ifdef _WIN32
        FreeLibrary(reinterpret_cast<HMODULE>(detail::_library));
#else
        dlclose(detail::_library);
#endif
        detail::_library = nullptr;
    }
}

void cleanUp() noexcept {
    destroyDevice();
    destroyInstance();
    unloadLib();
}

void destroyDevice() noexcept {
    if (detail::_device) {
        funcs.vkDestroyDevice(detail::_device.handle(), detail::_allocator);
        detail::_device = nullptr;
    }
}

void destroyInstance() noexcept {
    if (detail::_instance) {
        funcs.vkDestroyInstance(detail::_instance.handle(), detail::_allocator);
        detail::_instance = nullptr;
    }
}

void initInstancePFNs() noexcept {
    funcs.vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetDeviceProcAddr");
    funcs.vkDestroyInstance = (PFN_vkDestroyInstance)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkDestroyInstance");
    funcs.vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkEnumeratePhysicalDevices");
    funcs.vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceFeatures");
    funcs.vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceFormatProperties");
    funcs.vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceImageFormatProperties");
    funcs.vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceProperties");
    funcs.vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceQueueFamilyProperties");
    funcs.vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceMemoryProperties");
    funcs.vkCreateDevice = (PFN_vkCreateDevice)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateDevice");
    funcs.vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkEnumerateDeviceExtensionProperties");
    funcs.vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkEnumerateDeviceLayerProperties");
    funcs.vkQueueSubmit = (PFN_vkQueueSubmit)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkQueueSubmit");
    funcs.vkQueueWaitIdle = (PFN_vkQueueWaitIdle)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkQueueWaitIdle");
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceSparseImageFormatProperties");
    funcs.vkQueueBindSparse = (PFN_vkQueueBindSparse)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkQueueBindSparse");
    funcs.vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkBeginCommandBuffer");
    funcs.vkEndCommandBuffer = (PFN_vkEndCommandBuffer)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkEndCommandBuffer");
    funcs.vkResetCommandBuffer = (PFN_vkResetCommandBuffer)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkResetCommandBuffer");
    funcs.vkCmdBindPipeline = (PFN_vkCmdBindPipeline)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindPipeline");
    funcs.vkCmdSetViewport = (PFN_vkCmdSetViewport)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetViewport");
    funcs.vkCmdSetScissor = (PFN_vkCmdSetScissor)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetScissor");
    funcs.vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetLineWidth");
    funcs.vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDepthBias");
    funcs.vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetBlendConstants");
    funcs.vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDepthBounds");
    funcs.vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetStencilCompareMask");
    funcs.vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetStencilWriteMask");
    funcs.vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetStencilReference");
    funcs.vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindDescriptorSets");
    funcs.vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindIndexBuffer");
    funcs.vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindVertexBuffers");
    funcs.vkCmdDraw = (PFN_vkCmdDraw)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDraw");
    funcs.vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawIndexed");
    funcs.vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawIndirect");
    funcs.vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawIndexedIndirect");
    funcs.vkCmdDispatch = (PFN_vkCmdDispatch)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDispatch");
    funcs.vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDispatchIndirect");
    funcs.vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyBuffer");
    funcs.vkCmdCopyImage = (PFN_vkCmdCopyImage)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyImage");
    funcs.vkCmdBlitImage = (PFN_vkCmdBlitImage)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBlitImage");
    funcs.vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyBufferToImage");
    funcs.vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyImageToBuffer");
    funcs.vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdUpdateBuffer");
    funcs.vkCmdFillBuffer = (PFN_vkCmdFillBuffer)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdFillBuffer");
    funcs.vkCmdClearColorImage = (PFN_vkCmdClearColorImage)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdClearColorImage");
    funcs.vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdClearDepthStencilImage");
    funcs.vkCmdClearAttachments = (PFN_vkCmdClearAttachments)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdClearAttachments");
    funcs.vkCmdResolveImage = (PFN_vkCmdResolveImage)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdResolveImage");
    funcs.vkCmdSetEvent = (PFN_vkCmdSetEvent)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetEvent");
    funcs.vkCmdResetEvent = (PFN_vkCmdResetEvent)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdResetEvent");
    funcs.vkCmdWaitEvents = (PFN_vkCmdWaitEvents)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdWaitEvents");
    funcs.vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdPipelineBarrier");
    funcs.vkCmdBeginQuery = (PFN_vkCmdBeginQuery)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBeginQuery");
    funcs.vkCmdEndQuery = (PFN_vkCmdEndQuery)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdEndQuery");
    funcs.vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdResetQueryPool");
    funcs.vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdWriteTimestamp");
    funcs.vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyQueryPoolResults");
    funcs.vkCmdPushConstants = (PFN_vkCmdPushConstants)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdPushConstants");
    funcs.vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBeginRenderPass");
    funcs.vkCmdNextSubpass = (PFN_vkCmdNextSubpass)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdNextSubpass");
    funcs.vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdEndRenderPass");
    funcs.vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdExecuteCommands");
    funcs.vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkEnumerateInstanceVersion");
    funcs.vkCmdSetDeviceMask = (PFN_vkCmdSetDeviceMask)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDeviceMask");
    funcs.vkCmdDispatchBase = (PFN_vkCmdDispatchBase)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDispatchBase");
    funcs.vkEnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkEnumeratePhysicalDeviceGroups");
    funcs.vkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceFeatures2");
    funcs.vkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceProperties2");
    funcs.vkGetPhysicalDeviceFormatProperties2 = (PFN_vkGetPhysicalDeviceFormatProperties2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceFormatProperties2");
    funcs.vkGetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceImageFormatProperties2");
    funcs.vkGetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceQueueFamilyProperties2");
    funcs.vkGetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceMemoryProperties2");
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties2 = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceSparseImageFormatProperties2");
    funcs.vkGetPhysicalDeviceExternalBufferProperties = (PFN_vkGetPhysicalDeviceExternalBufferProperties)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceExternalBufferProperties");
    funcs.vkGetPhysicalDeviceExternalFenceProperties = (PFN_vkGetPhysicalDeviceExternalFenceProperties)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceExternalFenceProperties");
    funcs.vkGetPhysicalDeviceExternalSemaphoreProperties = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceExternalSemaphoreProperties");
    funcs.vkCmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawIndirectCount");
    funcs.vkCmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawIndexedIndirectCount");
    funcs.vkCmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBeginRenderPass2");
    funcs.vkCmdNextSubpass2 = (PFN_vkCmdNextSubpass2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdNextSubpass2");
    funcs.vkCmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdEndRenderPass2");
    funcs.vkGetPhysicalDeviceToolProperties = (PFN_vkGetPhysicalDeviceToolProperties)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceToolProperties");
    funcs.vkCmdSetEvent2 = (PFN_vkCmdSetEvent2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetEvent2");
    funcs.vkCmdResetEvent2 = (PFN_vkCmdResetEvent2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdResetEvent2");
    funcs.vkCmdWaitEvents2 = (PFN_vkCmdWaitEvents2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdWaitEvents2");
    funcs.vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdPipelineBarrier2");
    funcs.vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdWriteTimestamp2");
    funcs.vkQueueSubmit2 = (PFN_vkQueueSubmit2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkQueueSubmit2");
    funcs.vkCmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyBuffer2");
    funcs.vkCmdCopyImage2 = (PFN_vkCmdCopyImage2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyImage2");
    funcs.vkCmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyBufferToImage2");
    funcs.vkCmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyImageToBuffer2");
    funcs.vkCmdBlitImage2 = (PFN_vkCmdBlitImage2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBlitImage2");
    funcs.vkCmdResolveImage2 = (PFN_vkCmdResolveImage2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdResolveImage2");
    funcs.vkCmdBeginRendering = (PFN_vkCmdBeginRendering)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBeginRendering");
    funcs.vkCmdEndRendering = (PFN_vkCmdEndRendering)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdEndRendering");
    funcs.vkCmdSetCullMode = (PFN_vkCmdSetCullMode)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetCullMode");
    funcs.vkCmdSetFrontFace = (PFN_vkCmdSetFrontFace)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetFrontFace");
    funcs.vkCmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetPrimitiveTopology");
    funcs.vkCmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetViewportWithCount");
    funcs.vkCmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetScissorWithCount");
    funcs.vkCmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindVertexBuffers2");
    funcs.vkCmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDepthTestEnable");
    funcs.vkCmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDepthWriteEnable");
    funcs.vkCmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDepthCompareOp");
    funcs.vkCmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDepthBoundsTestEnable");
    funcs.vkCmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetStencilTestEnable");
    funcs.vkCmdSetStencilOp = (PFN_vkCmdSetStencilOp)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetStencilOp");
    funcs.vkCmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetRasterizerDiscardEnable");
    funcs.vkCmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDepthBiasEnable");
    funcs.vkCmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetPrimitiveRestartEnable");
    funcs.vkCmdSetLineStipple = (PFN_vkCmdSetLineStipple)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetLineStipple");
    funcs.vkCmdBindIndexBuffer2 = (PFN_vkCmdBindIndexBuffer2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindIndexBuffer2");
    funcs.vkCmdPushDescriptorSet = (PFN_vkCmdPushDescriptorSet)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdPushDescriptorSet");
    funcs.vkCmdPushDescriptorSetWithTemplate = (PFN_vkCmdPushDescriptorSetWithTemplate)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdPushDescriptorSetWithTemplate");
    funcs.vkCmdSetRenderingAttachmentLocations = (PFN_vkCmdSetRenderingAttachmentLocations)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetRenderingAttachmentLocations");
    funcs.vkCmdSetRenderingInputAttachmentIndices = (PFN_vkCmdSetRenderingInputAttachmentIndices)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetRenderingInputAttachmentIndices");
    funcs.vkCmdBindDescriptorSets2 = (PFN_vkCmdBindDescriptorSets2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindDescriptorSets2");
    funcs.vkCmdPushConstants2 = (PFN_vkCmdPushConstants2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdPushConstants2");
    funcs.vkCmdPushDescriptorSet2 = (PFN_vkCmdPushDescriptorSet2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdPushDescriptorSet2");
    funcs.vkCmdPushDescriptorSetWithTemplate2 = (PFN_vkCmdPushDescriptorSetWithTemplate2)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdPushDescriptorSetWithTemplate2");
    funcs.vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkDestroySurfaceKHR");
    funcs.vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceSurfaceSupportKHR");
    funcs.vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    funcs.vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceSurfaceFormatsKHR");
    funcs.vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceSurfacePresentModesKHR");
    funcs.vkQueuePresentKHR = (PFN_vkQueuePresentKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkQueuePresentKHR");
    funcs.vkGetPhysicalDevicePresentRectanglesKHR = (PFN_vkGetPhysicalDevicePresentRectanglesKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDevicePresentRectanglesKHR");
    funcs.vkGetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceDisplayPropertiesKHR");
    funcs.vkGetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    funcs.vkGetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetDisplayPlaneSupportedDisplaysKHR");
    funcs.vkGetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetDisplayModePropertiesKHR");
    funcs.vkCreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateDisplayModeKHR");
    funcs.vkGetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetDisplayPlaneCapabilitiesKHR");
    funcs.vkCreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateDisplayPlaneSurfaceKHR");
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    funcs.vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateXlibSurfaceKHR");
    funcs.vkGetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif // VK_USE_PLATFORM_XLIB_KHR
#if defined(VK_USE_PLATFORM_XCB_KHR)
    funcs.vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateXcbSurfaceKHR");
    funcs.vkGetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif // VK_USE_PLATFORM_XCB_KHR
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    funcs.vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateWaylandSurfaceKHR");
    funcs.vkGetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    funcs.vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateAndroidSurfaceKHR");
#endif // VK_USE_PLATFORM_ANDROID_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    funcs.vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateWin32SurfaceKHR");
    funcs.vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
    funcs.vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateDebugReportCallbackEXT");
    funcs.vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkDestroyDebugReportCallbackEXT");
    funcs.vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkDebugReportMessageEXT");
    funcs.vkQueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkQueueBeginDebugUtilsLabelEXT");
    funcs.vkQueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkQueueEndDebugUtilsLabelEXT");
    funcs.vkQueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkQueueInsertDebugUtilsLabelEXT");
    funcs.vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBeginDebugUtilsLabelEXT");
    funcs.vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdEndDebugUtilsLabelEXT");
    funcs.vkCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdInsertDebugUtilsLabelEXT");
    funcs.vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateDebugUtilsMessengerEXT");
    funcs.vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkDestroyDebugUtilsMessengerEXT");
    funcs.vkSubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkSubmitDebugUtilsMessageEXT");
    funcs.vkCmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDebugMarkerBeginEXT");
    funcs.vkCmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDebugMarkerEndEXT");
    funcs.vkCmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDebugMarkerInsertEXT");
    funcs.vkCmdBindTransformFeedbackBuffersEXT = (PFN_vkCmdBindTransformFeedbackBuffersEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindTransformFeedbackBuffersEXT");
    funcs.vkCmdBeginTransformFeedbackEXT = (PFN_vkCmdBeginTransformFeedbackEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBeginTransformFeedbackEXT");
    funcs.vkCmdEndTransformFeedbackEXT = (PFN_vkCmdEndTransformFeedbackEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdEndTransformFeedbackEXT");
    funcs.vkCmdBeginQueryIndexedEXT = (PFN_vkCmdBeginQueryIndexedEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBeginQueryIndexedEXT");
    funcs.vkCmdEndQueryIndexedEXT = (PFN_vkCmdEndQueryIndexedEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdEndQueryIndexedEXT");
    funcs.vkCmdDrawIndirectByteCountEXT = (PFN_vkCmdDrawIndirectByteCountEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawIndirectByteCountEXT");
    funcs.vkCmdCuLaunchKernelNVX = (PFN_vkCmdCuLaunchKernelNVX)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCuLaunchKernelNVX");
#if defined(VK_USE_PLATFORM_GGP)
    funcs.vkCreateStreamDescriptorSurfaceGGP = (PFN_vkCreateStreamDescriptorSurfaceGGP)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateStreamDescriptorSurfaceGGP");
#endif // VK_USE_PLATFORM_GGP
    funcs.vkGetPhysicalDeviceExternalImageFormatPropertiesNV = (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
#if defined(VK_USE_PLATFORM_VI_NN)
    funcs.vkCreateViSurfaceNN = (PFN_vkCreateViSurfaceNN)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateViSurfaceNN");
#endif // VK_USE_PLATFORM_VI_NN
    funcs.vkCmdBeginConditionalRenderingEXT = (PFN_vkCmdBeginConditionalRenderingEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBeginConditionalRenderingEXT");
    funcs.vkCmdEndConditionalRenderingEXT = (PFN_vkCmdEndConditionalRenderingEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdEndConditionalRenderingEXT");
    funcs.vkCmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetViewportWScalingNV");
    funcs.vkReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkReleaseDisplayEXT");
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    funcs.vkAcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkAcquireXlibDisplayEXT");
    funcs.vkGetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetRandROutputDisplayEXT");
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
    funcs.vkGetPhysicalDeviceSurfaceCapabilities2EXT = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
    funcs.vkCmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDiscardRectangleEXT");
    funcs.vkCmdSetDiscardRectangleEnableEXT = (PFN_vkCmdSetDiscardRectangleEnableEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDiscardRectangleEnableEXT");
    funcs.vkCmdSetDiscardRectangleModeEXT = (PFN_vkCmdSetDiscardRectangleModeEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDiscardRectangleModeEXT");
    funcs.vkGetPhysicalDeviceSurfaceCapabilities2KHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    funcs.vkGetPhysicalDeviceSurfaceFormats2KHR = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceSurfaceFormats2KHR");
    funcs.vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR = (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    funcs.vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR = (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    funcs.vkGetPhysicalDeviceDisplayProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceDisplayProperties2KHR");
    funcs.vkGetPhysicalDeviceDisplayPlaneProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    funcs.vkGetDisplayModeProperties2KHR = (PFN_vkGetDisplayModeProperties2KHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetDisplayModeProperties2KHR");
    funcs.vkGetDisplayPlaneCapabilities2KHR = (PFN_vkGetDisplayPlaneCapabilities2KHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetDisplayPlaneCapabilities2KHR");
#if defined(VK_USE_PLATFORM_IOS_MVK)
    funcs.vkCreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateIOSSurfaceMVK");
#endif // VK_USE_PLATFORM_IOS_MVK
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    funcs.vkCreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateMacOSSurfaceMVK");
#endif // VK_USE_PLATFORM_MACOS_MVK
    funcs.vkCmdInitializeGraphScratchMemoryAMDX = (PFN_vkCmdInitializeGraphScratchMemoryAMDX)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdInitializeGraphScratchMemoryAMDX");
    funcs.vkCmdDispatchGraphAMDX = (PFN_vkCmdDispatchGraphAMDX)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDispatchGraphAMDX");
    funcs.vkCmdDispatchGraphIndirectAMDX = (PFN_vkCmdDispatchGraphIndirectAMDX)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDispatchGraphIndirectAMDX");
    funcs.vkCmdDispatchGraphIndirectCountAMDX = (PFN_vkCmdDispatchGraphIndirectCountAMDX)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDispatchGraphIndirectCountAMDX");
    funcs.vkCmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetSampleLocationsEXT");
    funcs.vkGetPhysicalDeviceMultisamplePropertiesEXT = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceMultisamplePropertiesEXT");
    funcs.vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBuildAccelerationStructuresKHR");
    funcs.vkCmdBuildAccelerationStructuresIndirectKHR = (PFN_vkCmdBuildAccelerationStructuresIndirectKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBuildAccelerationStructuresIndirectKHR");
    funcs.vkCmdCopyAccelerationStructureKHR = (PFN_vkCmdCopyAccelerationStructureKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyAccelerationStructureKHR");
    funcs.vkCmdCopyAccelerationStructureToMemoryKHR = (PFN_vkCmdCopyAccelerationStructureToMemoryKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyAccelerationStructureToMemoryKHR");
    funcs.vkCmdCopyMemoryToAccelerationStructureKHR = (PFN_vkCmdCopyMemoryToAccelerationStructureKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyMemoryToAccelerationStructureKHR");
    funcs.vkCmdWriteAccelerationStructuresPropertiesKHR = (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdWriteAccelerationStructuresPropertiesKHR");
    funcs.vkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdTraceRaysKHR");
    funcs.vkCmdTraceRaysIndirectKHR = (PFN_vkCmdTraceRaysIndirectKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdTraceRaysIndirectKHR");
    funcs.vkCmdSetRayTracingPipelineStackSizeKHR = (PFN_vkCmdSetRayTracingPipelineStackSizeKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetRayTracingPipelineStackSizeKHR");
    funcs.vkCmdBindShadingRateImageNV = (PFN_vkCmdBindShadingRateImageNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindShadingRateImageNV");
    funcs.vkCmdSetViewportShadingRatePaletteNV = (PFN_vkCmdSetViewportShadingRatePaletteNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetViewportShadingRatePaletteNV");
    funcs.vkCmdSetCoarseSampleOrderNV = (PFN_vkCmdSetCoarseSampleOrderNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetCoarseSampleOrderNV");
    funcs.vkCmdBuildAccelerationStructureNV = (PFN_vkCmdBuildAccelerationStructureNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBuildAccelerationStructureNV");
    funcs.vkCmdCopyAccelerationStructureNV = (PFN_vkCmdCopyAccelerationStructureNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyAccelerationStructureNV");
    funcs.vkCmdTraceRaysNV = (PFN_vkCmdTraceRaysNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdTraceRaysNV");
    funcs.vkCmdWriteAccelerationStructuresPropertiesNV = (PFN_vkCmdWriteAccelerationStructuresPropertiesNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdWriteAccelerationStructuresPropertiesNV");
    funcs.vkCmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdWriteBufferMarkerAMD");
    funcs.vkCmdWriteBufferMarker2AMD = (PFN_vkCmdWriteBufferMarker2AMD)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdWriteBufferMarker2AMD");
    funcs.vkCmdDrawMeshTasksNV = (PFN_vkCmdDrawMeshTasksNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawMeshTasksNV");
    funcs.vkCmdDrawMeshTasksIndirectNV = (PFN_vkCmdDrawMeshTasksIndirectNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawMeshTasksIndirectNV");
    funcs.vkCmdDrawMeshTasksIndirectCountNV = (PFN_vkCmdDrawMeshTasksIndirectCountNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawMeshTasksIndirectCountNV");
    funcs.vkCmdSetExclusiveScissorEnableNV = (PFN_vkCmdSetExclusiveScissorEnableNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetExclusiveScissorEnableNV");
    funcs.vkCmdSetExclusiveScissorNV = (PFN_vkCmdSetExclusiveScissorNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetExclusiveScissorNV");
    funcs.vkCmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetCheckpointNV");
    funcs.vkGetQueueCheckpointDataNV = (PFN_vkGetQueueCheckpointDataNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetQueueCheckpointDataNV");
    funcs.vkGetQueueCheckpointData2NV = (PFN_vkGetQueueCheckpointData2NV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetQueueCheckpointData2NV");
    funcs.vkCmdSetPerformanceMarkerINTEL = (PFN_vkCmdSetPerformanceMarkerINTEL)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetPerformanceMarkerINTEL");
    funcs.vkCmdSetPerformanceStreamMarkerINTEL = (PFN_vkCmdSetPerformanceStreamMarkerINTEL)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetPerformanceStreamMarkerINTEL");
    funcs.vkCmdSetPerformanceOverrideINTEL = (PFN_vkCmdSetPerformanceOverrideINTEL)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetPerformanceOverrideINTEL");
    funcs.vkQueueSetPerformanceConfigurationINTEL = (PFN_vkQueueSetPerformanceConfigurationINTEL)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkQueueSetPerformanceConfigurationINTEL");
#if defined(VK_USE_PLATFORM_FUCHSIA)
    funcs.vkCreateImagePipeSurfaceFUCHSIA = (PFN_vkCreateImagePipeSurfaceFUCHSIA)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateImagePipeSurfaceFUCHSIA");
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_METAL_EXT)
    funcs.vkCreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateMetalSurfaceEXT");
#endif // VK_USE_PLATFORM_METAL_EXT
    funcs.vkGetPhysicalDeviceFragmentShadingRatesKHR = (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceFragmentShadingRatesKHR");
    funcs.vkCmdSetFragmentShadingRateKHR = (PFN_vkCmdSetFragmentShadingRateKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetFragmentShadingRateKHR");
    funcs.vkGetPhysicalDeviceCooperativeMatrixPropertiesNV = (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV");
    funcs.vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV = (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    funcs.vkGetPhysicalDeviceSurfacePresentModes2EXT = (PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceSurfacePresentModes2EXT");
#endif // VK_USE_PLATFORM_WIN32_KHR
    funcs.vkCreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateHeadlessSurfaceEXT");
    funcs.vkCmdPreprocessGeneratedCommandsNV = (PFN_vkCmdPreprocessGeneratedCommandsNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdPreprocessGeneratedCommandsNV");
    funcs.vkCmdExecuteGeneratedCommandsNV = (PFN_vkCmdExecuteGeneratedCommandsNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdExecuteGeneratedCommandsNV");
    funcs.vkCmdBindPipelineShaderGroupNV = (PFN_vkCmdBindPipelineShaderGroupNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindPipelineShaderGroupNV");
    funcs.vkCmdSetDepthBias2EXT = (PFN_vkCmdSetDepthBias2EXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDepthBias2EXT");
    funcs.vkAcquireDrmDisplayEXT = (PFN_vkAcquireDrmDisplayEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkAcquireDrmDisplayEXT");
    funcs.vkGetDrmDisplayEXT = (PFN_vkGetDrmDisplayEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetDrmDisplayEXT");
    funcs.vkCmdCudaLaunchKernelNV = (PFN_vkCmdCudaLaunchKernelNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCudaLaunchKernelNV");
    funcs.vkCmdDispatchTileQCOM = (PFN_vkCmdDispatchTileQCOM)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDispatchTileQCOM");
    funcs.vkCmdBeginPerTileExecutionQCOM = (PFN_vkCmdBeginPerTileExecutionQCOM)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBeginPerTileExecutionQCOM");
    funcs.vkCmdEndPerTileExecutionQCOM = (PFN_vkCmdEndPerTileExecutionQCOM)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdEndPerTileExecutionQCOM");
    funcs.vkCmdBindDescriptorBuffersEXT = (PFN_vkCmdBindDescriptorBuffersEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindDescriptorBuffersEXT");
    funcs.vkCmdSetDescriptorBufferOffsetsEXT = (PFN_vkCmdSetDescriptorBufferOffsetsEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDescriptorBufferOffsetsEXT");
    funcs.vkCmdBindDescriptorBufferEmbeddedSamplersEXT = (PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindDescriptorBufferEmbeddedSamplersEXT");
    funcs.vkCmdSetFragmentShadingRateEnumNV = (PFN_vkCmdSetFragmentShadingRateEnumNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetFragmentShadingRateEnumNV");
    funcs.vkCmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawMeshTasksEXT");
    funcs.vkCmdDrawMeshTasksIndirectEXT = (PFN_vkCmdDrawMeshTasksIndirectEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawMeshTasksIndirectEXT");
    funcs.vkCmdDrawMeshTasksIndirectCountEXT = (PFN_vkCmdDrawMeshTasksIndirectCountEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawMeshTasksIndirectCountEXT");
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    funcs.vkAcquireWinrtDisplayNV = (PFN_vkAcquireWinrtDisplayNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkAcquireWinrtDisplayNV");
    funcs.vkGetWinrtDisplayNV = (PFN_vkGetWinrtDisplayNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetWinrtDisplayNV");
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    funcs.vkCreateDirectFBSurfaceEXT = (PFN_vkCreateDirectFBSurfaceEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateDirectFBSurfaceEXT");
    funcs.vkGetPhysicalDeviceDirectFBPresentationSupportEXT = (PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceDirectFBPresentationSupportEXT");
#endif // VK_USE_PLATFORM_DIRECTFB_EXT
    funcs.vkCmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetVertexInputEXT");
    funcs.vkCmdSubpassShadingHUAWEI = (PFN_vkCmdSubpassShadingHUAWEI)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSubpassShadingHUAWEI");
    funcs.vkCmdBindInvocationMaskHUAWEI = (PFN_vkCmdBindInvocationMaskHUAWEI)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindInvocationMaskHUAWEI");
    funcs.vkCmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetPatchControlPointsEXT");
    funcs.vkCmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetLogicOpEXT");
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    funcs.vkCreateScreenSurfaceQNX = (PFN_vkCreateScreenSurfaceQNX)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateScreenSurfaceQNX");
    funcs.vkGetPhysicalDeviceScreenPresentationSupportQNX = (PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceScreenPresentationSupportQNX");
#endif // VK_USE_PLATFORM_SCREEN_QNX
    funcs.vkCmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetColorWriteEnableEXT");
    funcs.vkCmdTraceRaysIndirect2KHR = (PFN_vkCmdTraceRaysIndirect2KHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdTraceRaysIndirect2KHR");
    funcs.vkCmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawMultiEXT");
    funcs.vkCmdDrawMultiIndexedEXT = (PFN_vkCmdDrawMultiIndexedEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawMultiIndexedEXT");
    funcs.vkCmdBuildMicromapsEXT = (PFN_vkCmdBuildMicromapsEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBuildMicromapsEXT");
    funcs.vkCmdCopyMicromapEXT = (PFN_vkCmdCopyMicromapEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyMicromapEXT");
    funcs.vkCmdCopyMicromapToMemoryEXT = (PFN_vkCmdCopyMicromapToMemoryEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyMicromapToMemoryEXT");
    funcs.vkCmdCopyMemoryToMicromapEXT = (PFN_vkCmdCopyMemoryToMicromapEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyMemoryToMicromapEXT");
    funcs.vkCmdWriteMicromapsPropertiesEXT = (PFN_vkCmdWriteMicromapsPropertiesEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdWriteMicromapsPropertiesEXT");
    funcs.vkCmdDrawClusterHUAWEI = (PFN_vkCmdDrawClusterHUAWEI)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawClusterHUAWEI");
    funcs.vkCmdDrawClusterIndirectHUAWEI = (PFN_vkCmdDrawClusterIndirectHUAWEI)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDrawClusterIndirectHUAWEI");
    funcs.vkCmdEndRendering2EXT = (PFN_vkCmdEndRendering2EXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdEndRendering2EXT");
    funcs.vkCmdCopyMemoryIndirectNV = (PFN_vkCmdCopyMemoryIndirectNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyMemoryIndirectNV");
    funcs.vkCmdCopyMemoryToImageIndirectNV = (PFN_vkCmdCopyMemoryToImageIndirectNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyMemoryToImageIndirectNV");
    funcs.vkCmdDecompressMemoryNV = (PFN_vkCmdDecompressMemoryNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDecompressMemoryNV");
    funcs.vkCmdDecompressMemoryIndirectCountNV = (PFN_vkCmdDecompressMemoryIndirectCountNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDecompressMemoryIndirectCountNV");
    funcs.vkCmdUpdatePipelineIndirectBufferNV = (PFN_vkCmdUpdatePipelineIndirectBufferNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdUpdatePipelineIndirectBufferNV");
    funcs.vkCmdSetDepthClampEnableEXT = (PFN_vkCmdSetDepthClampEnableEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDepthClampEnableEXT");
    funcs.vkCmdSetPolygonModeEXT = (PFN_vkCmdSetPolygonModeEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetPolygonModeEXT");
    funcs.vkCmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetRasterizationSamplesEXT");
    funcs.vkCmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetSampleMaskEXT");
    funcs.vkCmdSetAlphaToCoverageEnableEXT = (PFN_vkCmdSetAlphaToCoverageEnableEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetAlphaToCoverageEnableEXT");
    funcs.vkCmdSetAlphaToOneEnableEXT = (PFN_vkCmdSetAlphaToOneEnableEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetAlphaToOneEnableEXT");
    funcs.vkCmdSetLogicOpEnableEXT = (PFN_vkCmdSetLogicOpEnableEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetLogicOpEnableEXT");
    funcs.vkCmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetColorBlendEnableEXT");
    funcs.vkCmdSetColorBlendEquationEXT = (PFN_vkCmdSetColorBlendEquationEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetColorBlendEquationEXT");
    funcs.vkCmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetColorWriteMaskEXT");
    funcs.vkCmdSetTessellationDomainOriginEXT = (PFN_vkCmdSetTessellationDomainOriginEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetTessellationDomainOriginEXT");
    funcs.vkCmdSetRasterizationStreamEXT = (PFN_vkCmdSetRasterizationStreamEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetRasterizationStreamEXT");
    funcs.vkCmdSetConservativeRasterizationModeEXT = (PFN_vkCmdSetConservativeRasterizationModeEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetConservativeRasterizationModeEXT");
    funcs.vkCmdSetExtraPrimitiveOverestimationSizeEXT = (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetExtraPrimitiveOverestimationSizeEXT");
    funcs.vkCmdSetDepthClipEnableEXT = (PFN_vkCmdSetDepthClipEnableEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDepthClipEnableEXT");
    funcs.vkCmdSetSampleLocationsEnableEXT = (PFN_vkCmdSetSampleLocationsEnableEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetSampleLocationsEnableEXT");
    funcs.vkCmdSetColorBlendAdvancedEXT = (PFN_vkCmdSetColorBlendAdvancedEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetColorBlendAdvancedEXT");
    funcs.vkCmdSetProvokingVertexModeEXT = (PFN_vkCmdSetProvokingVertexModeEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetProvokingVertexModeEXT");
    funcs.vkCmdSetLineRasterizationModeEXT = (PFN_vkCmdSetLineRasterizationModeEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetLineRasterizationModeEXT");
    funcs.vkCmdSetLineStippleEnableEXT = (PFN_vkCmdSetLineStippleEnableEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetLineStippleEnableEXT");
    funcs.vkCmdSetDepthClipNegativeOneToOneEXT = (PFN_vkCmdSetDepthClipNegativeOneToOneEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDepthClipNegativeOneToOneEXT");
    funcs.vkCmdSetViewportWScalingEnableNV = (PFN_vkCmdSetViewportWScalingEnableNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetViewportWScalingEnableNV");
    funcs.vkCmdSetViewportSwizzleNV = (PFN_vkCmdSetViewportSwizzleNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetViewportSwizzleNV");
    funcs.vkCmdSetCoverageToColorEnableNV = (PFN_vkCmdSetCoverageToColorEnableNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetCoverageToColorEnableNV");
    funcs.vkCmdSetCoverageToColorLocationNV = (PFN_vkCmdSetCoverageToColorLocationNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetCoverageToColorLocationNV");
    funcs.vkCmdSetCoverageModulationModeNV = (PFN_vkCmdSetCoverageModulationModeNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetCoverageModulationModeNV");
    funcs.vkCmdSetCoverageModulationTableEnableNV = (PFN_vkCmdSetCoverageModulationTableEnableNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetCoverageModulationTableEnableNV");
    funcs.vkCmdSetCoverageModulationTableNV = (PFN_vkCmdSetCoverageModulationTableNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetCoverageModulationTableNV");
    funcs.vkCmdSetShadingRateImageEnableNV = (PFN_vkCmdSetShadingRateImageEnableNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetShadingRateImageEnableNV");
    funcs.vkCmdSetRepresentativeFragmentTestEnableNV = (PFN_vkCmdSetRepresentativeFragmentTestEnableNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetRepresentativeFragmentTestEnableNV");
    funcs.vkCmdSetCoverageReductionModeNV = (PFN_vkCmdSetCoverageReductionModeNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetCoverageReductionModeNV");
    funcs.vkCmdCopyTensorARM = (PFN_vkCmdCopyTensorARM)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdCopyTensorARM");
    funcs.vkGetPhysicalDeviceExternalTensorPropertiesARM = (PFN_vkGetPhysicalDeviceExternalTensorPropertiesARM)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceExternalTensorPropertiesARM");
    funcs.vkGetPhysicalDeviceOpticalFlowImageFormatsNV = (PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceOpticalFlowImageFormatsNV");
    funcs.vkCmdOpticalFlowExecuteNV = (PFN_vkCmdOpticalFlowExecuteNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdOpticalFlowExecuteNV");
    funcs.vkCmdBindShadersEXT = (PFN_vkCmdBindShadersEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindShadersEXT");
    funcs.vkCmdSetDepthClampRangeEXT = (PFN_vkCmdSetDepthClampRangeEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDepthClampRangeEXT");
    funcs.vkGetPhysicalDeviceCooperativeVectorPropertiesNV = (PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceCooperativeVectorPropertiesNV");
    funcs.vkCmdConvertCooperativeVectorMatrixNV = (PFN_vkCmdConvertCooperativeVectorMatrixNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdConvertCooperativeVectorMatrixNV");
    funcs.vkQueueNotifyOutOfBandNV = (PFN_vkQueueNotifyOutOfBandNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkQueueNotifyOutOfBandNV");
    funcs.vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR = (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR");
    funcs.vkCmdDispatchDataGraphARM = (PFN_vkCmdDispatchDataGraphARM)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdDispatchDataGraphARM");
    funcs.vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM = (PFN_vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM");
    funcs.vkGetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM = (PFN_vkGetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM");
    funcs.vkCmdSetDescriptorBufferOffsets2EXT = (PFN_vkCmdSetDescriptorBufferOffsets2EXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetDescriptorBufferOffsets2EXT");
    funcs.vkCmdBindDescriptorBufferEmbeddedSamplers2EXT = (PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindDescriptorBufferEmbeddedSamplers2EXT");
    funcs.vkCmdSetAttachmentFeedbackLoopEnableEXT = (PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdSetAttachmentFeedbackLoopEnableEXT");
    funcs.vkCmdBindTileMemoryQCOM = (PFN_vkCmdBindTileMemoryQCOM)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBindTileMemoryQCOM");
    funcs.vkCmdBuildClusterAccelerationStructureIndirectNV = (PFN_vkCmdBuildClusterAccelerationStructureIndirectNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBuildClusterAccelerationStructureIndirectNV");
    funcs.vkCmdBuildPartitionedAccelerationStructuresNV = (PFN_vkCmdBuildPartitionedAccelerationStructuresNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdBuildPartitionedAccelerationStructuresNV");
    funcs.vkCmdPreprocessGeneratedCommandsEXT = (PFN_vkCmdPreprocessGeneratedCommandsEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdPreprocessGeneratedCommandsEXT");
    funcs.vkCmdExecuteGeneratedCommandsEXT = (PFN_vkCmdExecuteGeneratedCommandsEXT)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCmdExecuteGeneratedCommandsEXT");
#if defined(VK_USE_PLATFORM_OHOS)
    funcs.vkCreateSurfaceOHOS = (PFN_vkCreateSurfaceOHOS)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkCreateSurfaceOHOS");
#endif // VK_USE_PLATFORM_OHOS
    funcs.vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV = (PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV)funcs.vkGetInstanceProcAddr(detail::_instance.handle(), "vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV");
}

void initDevicePFNs() noexcept {
    funcs.vkDestroyDevice = (PFN_vkDestroyDevice)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyDevice");
    funcs.vkGetDeviceQueue = (PFN_vkGetDeviceQueue)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceQueue");
    funcs.vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDeviceWaitIdle");
    funcs.vkAllocateMemory = (PFN_vkAllocateMemory)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkAllocateMemory");
    funcs.vkFreeMemory = (PFN_vkFreeMemory)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkFreeMemory");
    funcs.vkMapMemory = (PFN_vkMapMemory)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkMapMemory");
    funcs.vkUnmapMemory = (PFN_vkUnmapMemory)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkUnmapMemory");
    funcs.vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkFlushMappedMemoryRanges");
    funcs.vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkInvalidateMappedMemoryRanges");
    funcs.vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceMemoryCommitment");
    funcs.vkBindBufferMemory = (PFN_vkBindBufferMemory)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkBindBufferMemory");
    funcs.vkBindImageMemory = (PFN_vkBindImageMemory)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkBindImageMemory");
    funcs.vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetBufferMemoryRequirements");
    funcs.vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetImageMemoryRequirements");
    funcs.vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetImageSparseMemoryRequirements");
    funcs.vkCreateFence = (PFN_vkCreateFence)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateFence");
    funcs.vkDestroyFence = (PFN_vkDestroyFence)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyFence");
    funcs.vkResetFences = (PFN_vkResetFences)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkResetFences");
    funcs.vkGetFenceStatus = (PFN_vkGetFenceStatus)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetFenceStatus");
    funcs.vkWaitForFences = (PFN_vkWaitForFences)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkWaitForFences");
    funcs.vkCreateSemaphore = (PFN_vkCreateSemaphore)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateSemaphore");
    funcs.vkDestroySemaphore = (PFN_vkDestroySemaphore)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroySemaphore");
    funcs.vkCreateEvent = (PFN_vkCreateEvent)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateEvent");
    funcs.vkDestroyEvent = (PFN_vkDestroyEvent)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyEvent");
    funcs.vkGetEventStatus = (PFN_vkGetEventStatus)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetEventStatus");
    funcs.vkSetEvent = (PFN_vkSetEvent)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkSetEvent");
    funcs.vkResetEvent = (PFN_vkResetEvent)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkResetEvent");
    funcs.vkCreateQueryPool = (PFN_vkCreateQueryPool)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateQueryPool");
    funcs.vkDestroyQueryPool = (PFN_vkDestroyQueryPool)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyQueryPool");
    funcs.vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetQueryPoolResults");
    funcs.vkCreateBuffer = (PFN_vkCreateBuffer)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateBuffer");
    funcs.vkDestroyBuffer = (PFN_vkDestroyBuffer)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyBuffer");
    funcs.vkCreateBufferView = (PFN_vkCreateBufferView)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateBufferView");
    funcs.vkDestroyBufferView = (PFN_vkDestroyBufferView)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyBufferView");
    funcs.vkCreateImage = (PFN_vkCreateImage)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateImage");
    funcs.vkDestroyImage = (PFN_vkDestroyImage)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyImage");
    funcs.vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetImageSubresourceLayout");
    funcs.vkCreateImageView = (PFN_vkCreateImageView)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateImageView");
    funcs.vkDestroyImageView = (PFN_vkDestroyImageView)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyImageView");
    funcs.vkCreateShaderModule = (PFN_vkCreateShaderModule)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateShaderModule");
    funcs.vkDestroyShaderModule = (PFN_vkDestroyShaderModule)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyShaderModule");
    funcs.vkCreatePipelineCache = (PFN_vkCreatePipelineCache)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreatePipelineCache");
    funcs.vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyPipelineCache");
    funcs.vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPipelineCacheData");
    funcs.vkMergePipelineCaches = (PFN_vkMergePipelineCaches)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkMergePipelineCaches");
    funcs.vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateGraphicsPipelines");
    funcs.vkCreateComputePipelines = (PFN_vkCreateComputePipelines)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateComputePipelines");
    funcs.vkDestroyPipeline = (PFN_vkDestroyPipeline)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyPipeline");
    funcs.vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreatePipelineLayout");
    funcs.vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyPipelineLayout");
    funcs.vkCreateSampler = (PFN_vkCreateSampler)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateSampler");
    funcs.vkDestroySampler = (PFN_vkDestroySampler)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroySampler");
    funcs.vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateDescriptorSetLayout");
    funcs.vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyDescriptorSetLayout");
    funcs.vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateDescriptorPool");
    funcs.vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyDescriptorPool");
    funcs.vkResetDescriptorPool = (PFN_vkResetDescriptorPool)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkResetDescriptorPool");
    funcs.vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkAllocateDescriptorSets");
    funcs.vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkFreeDescriptorSets");
    funcs.vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkUpdateDescriptorSets");
    funcs.vkCreateFramebuffer = (PFN_vkCreateFramebuffer)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateFramebuffer");
    funcs.vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyFramebuffer");
    funcs.vkCreateRenderPass = (PFN_vkCreateRenderPass)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateRenderPass");
    funcs.vkDestroyRenderPass = (PFN_vkDestroyRenderPass)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyRenderPass");
    funcs.vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetRenderAreaGranularity");
    funcs.vkCreateCommandPool = (PFN_vkCreateCommandPool)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateCommandPool");
    funcs.vkDestroyCommandPool = (PFN_vkDestroyCommandPool)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyCommandPool");
    funcs.vkResetCommandPool = (PFN_vkResetCommandPool)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkResetCommandPool");
    funcs.vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkAllocateCommandBuffers");
    funcs.vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkFreeCommandBuffers");
    funcs.vkBindBufferMemory2 = (PFN_vkBindBufferMemory2)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkBindBufferMemory2");
    funcs.vkBindImageMemory2 = (PFN_vkBindImageMemory2)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkBindImageMemory2");
    funcs.vkGetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceGroupPeerMemoryFeatures");
    funcs.vkGetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetImageMemoryRequirements2");
    funcs.vkGetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetBufferMemoryRequirements2");
    funcs.vkGetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetImageSparseMemoryRequirements2");
    funcs.vkTrimCommandPool = (PFN_vkTrimCommandPool)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkTrimCommandPool");
    funcs.vkGetDeviceQueue2 = (PFN_vkGetDeviceQueue2)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceQueue2");
    funcs.vkCreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateSamplerYcbcrConversion");
    funcs.vkDestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroySamplerYcbcrConversion");
    funcs.vkCreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateDescriptorUpdateTemplate");
    funcs.vkDestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyDescriptorUpdateTemplate");
    funcs.vkUpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkUpdateDescriptorSetWithTemplate");
    funcs.vkGetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDescriptorSetLayoutSupport");
    funcs.vkCreateRenderPass2 = (PFN_vkCreateRenderPass2)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateRenderPass2");
    funcs.vkResetQueryPool = (PFN_vkResetQueryPool)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkResetQueryPool");
    funcs.vkGetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetSemaphoreCounterValue");
    funcs.vkWaitSemaphores = (PFN_vkWaitSemaphores)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkWaitSemaphores");
    funcs.vkSignalSemaphore = (PFN_vkSignalSemaphore)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkSignalSemaphore");
    funcs.vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetBufferDeviceAddress");
    funcs.vkGetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetBufferOpaqueCaptureAddress");
    funcs.vkGetDeviceMemoryOpaqueCaptureAddress = (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceMemoryOpaqueCaptureAddress");
    funcs.vkCreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreatePrivateDataSlot");
    funcs.vkDestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyPrivateDataSlot");
    funcs.vkSetPrivateData = (PFN_vkSetPrivateData)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkSetPrivateData");
    funcs.vkGetPrivateData = (PFN_vkGetPrivateData)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPrivateData");
    funcs.vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceBufferMemoryRequirements");
    funcs.vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceImageMemoryRequirements");
    funcs.vkGetDeviceImageSparseMemoryRequirements = (PFN_vkGetDeviceImageSparseMemoryRequirements)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceImageSparseMemoryRequirements");
    funcs.vkMapMemory2 = (PFN_vkMapMemory2)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkMapMemory2");
    funcs.vkUnmapMemory2 = (PFN_vkUnmapMemory2)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkUnmapMemory2");
    funcs.vkGetRenderingAreaGranularity = (PFN_vkGetRenderingAreaGranularity)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetRenderingAreaGranularity");
    funcs.vkGetDeviceImageSubresourceLayout = (PFN_vkGetDeviceImageSubresourceLayout)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceImageSubresourceLayout");
    funcs.vkGetImageSubresourceLayout2 = (PFN_vkGetImageSubresourceLayout2)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetImageSubresourceLayout2");
    funcs.vkCopyMemoryToImage = (PFN_vkCopyMemoryToImage)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCopyMemoryToImage");
    funcs.vkCopyImageToMemory = (PFN_vkCopyImageToMemory)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCopyImageToMemory");
    funcs.vkCopyImageToImage = (PFN_vkCopyImageToImage)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCopyImageToImage");
    funcs.vkTransitionImageLayout = (PFN_vkTransitionImageLayout)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkTransitionImageLayout");
    funcs.vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateSwapchainKHR");
    funcs.vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroySwapchainKHR");
    funcs.vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetSwapchainImagesKHR");
    funcs.vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkAcquireNextImageKHR");
    funcs.vkGetDeviceGroupPresentCapabilitiesKHR = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceGroupPresentCapabilitiesKHR");
    funcs.vkGetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceGroupSurfacePresentModesKHR");
    funcs.vkAcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkAcquireNextImage2KHR");
    funcs.vkCreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateSharedSwapchainsKHR");
    funcs.vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkSetDebugUtilsObjectNameEXT");
    funcs.vkSetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkSetDebugUtilsObjectTagEXT");
    funcs.vkDebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDebugMarkerSetObjectTagEXT");
    funcs.vkDebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDebugMarkerSetObjectNameEXT");
    funcs.vkCreateCuModuleNVX = (PFN_vkCreateCuModuleNVX)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateCuModuleNVX");
    funcs.vkCreateCuFunctionNVX = (PFN_vkCreateCuFunctionNVX)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateCuFunctionNVX");
    funcs.vkDestroyCuModuleNVX = (PFN_vkDestroyCuModuleNVX)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyCuModuleNVX");
    funcs.vkDestroyCuFunctionNVX = (PFN_vkDestroyCuFunctionNVX)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyCuFunctionNVX");
    funcs.vkGetImageViewHandleNVX = (PFN_vkGetImageViewHandleNVX)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetImageViewHandleNVX");
    funcs.vkGetImageViewHandle64NVX = (PFN_vkGetImageViewHandle64NVX)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetImageViewHandle64NVX");
    funcs.vkGetImageViewAddressNVX = (PFN_vkGetImageViewAddressNVX)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetImageViewAddressNVX");
    funcs.vkGetShaderInfoAMD = (PFN_vkGetShaderInfoAMD)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetShaderInfoAMD");
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    funcs.vkGetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMemoryWin32HandleNV");
    funcs.vkGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMemoryWin32HandleKHR");
    funcs.vkGetMemoryWin32HandlePropertiesKHR = (PFN_vkGetMemoryWin32HandlePropertiesKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMemoryWin32HandlePropertiesKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
    funcs.vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMemoryFdKHR");
    funcs.vkGetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMemoryFdPropertiesKHR");
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    funcs.vkImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkImportSemaphoreWin32HandleKHR");
    funcs.vkGetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetSemaphoreWin32HandleKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
    funcs.vkImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkImportSemaphoreFdKHR");
    funcs.vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetSemaphoreFdKHR");
    funcs.vkDisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDisplayPowerControlEXT");
    funcs.vkRegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkRegisterDeviceEventEXT");
    funcs.vkRegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkRegisterDisplayEventEXT");
    funcs.vkGetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetSwapchainCounterEXT");
    funcs.vkGetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetRefreshCycleDurationGOOGLE");
    funcs.vkGetPastPresentationTimingGOOGLE = (PFN_vkGetPastPresentationTimingGOOGLE)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPastPresentationTimingGOOGLE");
    funcs.vkSetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkSetHdrMetadataEXT");
    funcs.vkGetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetSwapchainStatusKHR");
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    funcs.vkImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkImportFenceWin32HandleKHR");
    funcs.vkGetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetFenceWin32HandleKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
    funcs.vkImportFenceFdKHR = (PFN_vkImportFenceFdKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkImportFenceFdKHR");
    funcs.vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetFenceFdKHR");
    funcs.vkAcquireProfilingLockKHR = (PFN_vkAcquireProfilingLockKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkAcquireProfilingLockKHR");
    funcs.vkReleaseProfilingLockKHR = (PFN_vkReleaseProfilingLockKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkReleaseProfilingLockKHR");
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    funcs.vkGetAndroidHardwareBufferPropertiesANDROID = (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetAndroidHardwareBufferPropertiesANDROID");
    funcs.vkGetMemoryAndroidHardwareBufferANDROID = (PFN_vkGetMemoryAndroidHardwareBufferANDROID)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMemoryAndroidHardwareBufferANDROID");
#endif // VK_USE_PLATFORM_ANDROID_KHR
    funcs.vkCreateExecutionGraphPipelinesAMDX = (PFN_vkCreateExecutionGraphPipelinesAMDX)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateExecutionGraphPipelinesAMDX");
    funcs.vkGetExecutionGraphPipelineScratchSizeAMDX = (PFN_vkGetExecutionGraphPipelineScratchSizeAMDX)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetExecutionGraphPipelineScratchSizeAMDX");
    funcs.vkGetExecutionGraphPipelineNodeIndexAMDX = (PFN_vkGetExecutionGraphPipelineNodeIndexAMDX)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetExecutionGraphPipelineNodeIndexAMDX");
    funcs.vkCreateDeferredOperationKHR = (PFN_vkCreateDeferredOperationKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateDeferredOperationKHR");
    funcs.vkDestroyDeferredOperationKHR = (PFN_vkDestroyDeferredOperationKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyDeferredOperationKHR");
    funcs.vkGetDeferredOperationMaxConcurrencyKHR = (PFN_vkGetDeferredOperationMaxConcurrencyKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeferredOperationMaxConcurrencyKHR");
    funcs.vkGetDeferredOperationResultKHR = (PFN_vkGetDeferredOperationResultKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeferredOperationResultKHR");
    funcs.vkDeferredOperationJoinKHR = (PFN_vkDeferredOperationJoinKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDeferredOperationJoinKHR");
    funcs.vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateAccelerationStructureKHR");
    funcs.vkDestroyAccelerationStructureKHR = (PFN_vkDestroyAccelerationStructureKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyAccelerationStructureKHR");
    funcs.vkBuildAccelerationStructuresKHR = (PFN_vkBuildAccelerationStructuresKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkBuildAccelerationStructuresKHR");
    funcs.vkCopyAccelerationStructureKHR = (PFN_vkCopyAccelerationStructureKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCopyAccelerationStructureKHR");
    funcs.vkCopyAccelerationStructureToMemoryKHR = (PFN_vkCopyAccelerationStructureToMemoryKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCopyAccelerationStructureToMemoryKHR");
    funcs.vkCopyMemoryToAccelerationStructureKHR = (PFN_vkCopyMemoryToAccelerationStructureKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCopyMemoryToAccelerationStructureKHR");
    funcs.vkWriteAccelerationStructuresPropertiesKHR = (PFN_vkWriteAccelerationStructuresPropertiesKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkWriteAccelerationStructuresPropertiesKHR");
    funcs.vkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetAccelerationStructureDeviceAddressKHR");
    funcs.vkGetDeviceAccelerationStructureCompatibilityKHR = (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceAccelerationStructureCompatibilityKHR");
    funcs.vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetAccelerationStructureBuildSizesKHR");
    funcs.vkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateRayTracingPipelinesKHR");
    funcs.vkGetRayTracingShaderGroupStackSizeKHR = (PFN_vkGetRayTracingShaderGroupStackSizeKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetRayTracingShaderGroupStackSizeKHR");
    funcs.vkGetImageDrmFormatModifierPropertiesEXT = (PFN_vkGetImageDrmFormatModifierPropertiesEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetImageDrmFormatModifierPropertiesEXT");
    funcs.vkCreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateValidationCacheEXT");
    funcs.vkDestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyValidationCacheEXT");
    funcs.vkMergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkMergeValidationCachesEXT");
    funcs.vkGetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetValidationCacheDataEXT");
    funcs.vkCreateAccelerationStructureNV = (PFN_vkCreateAccelerationStructureNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateAccelerationStructureNV");
    funcs.vkDestroyAccelerationStructureNV = (PFN_vkDestroyAccelerationStructureNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyAccelerationStructureNV");
    funcs.vkGetAccelerationStructureMemoryRequirementsNV = (PFN_vkGetAccelerationStructureMemoryRequirementsNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetAccelerationStructureMemoryRequirementsNV");
    funcs.vkBindAccelerationStructureMemoryNV = (PFN_vkBindAccelerationStructureMemoryNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkBindAccelerationStructureMemoryNV");
    funcs.vkCreateRayTracingPipelinesNV = (PFN_vkCreateRayTracingPipelinesNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateRayTracingPipelinesNV");
    funcs.vkGetAccelerationStructureHandleNV = (PFN_vkGetAccelerationStructureHandleNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetAccelerationStructureHandleNV");
    funcs.vkCompileDeferredNV = (PFN_vkCompileDeferredNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCompileDeferredNV");
    funcs.vkGetMemoryHostPointerPropertiesEXT = (PFN_vkGetMemoryHostPointerPropertiesEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMemoryHostPointerPropertiesEXT");
    funcs.vkInitializePerformanceApiINTEL = (PFN_vkInitializePerformanceApiINTEL)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkInitializePerformanceApiINTEL");
    funcs.vkUninitializePerformanceApiINTEL = (PFN_vkUninitializePerformanceApiINTEL)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkUninitializePerformanceApiINTEL");
    funcs.vkAcquirePerformanceConfigurationINTEL = (PFN_vkAcquirePerformanceConfigurationINTEL)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkAcquirePerformanceConfigurationINTEL");
    funcs.vkReleasePerformanceConfigurationINTEL = (PFN_vkReleasePerformanceConfigurationINTEL)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkReleasePerformanceConfigurationINTEL");
    funcs.vkGetPerformanceParameterINTEL = (PFN_vkGetPerformanceParameterINTEL)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPerformanceParameterINTEL");
    funcs.vkSetLocalDimmingAMD = (PFN_vkSetLocalDimmingAMD)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkSetLocalDimmingAMD");
    funcs.vkWaitForPresentKHR = (PFN_vkWaitForPresentKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkWaitForPresentKHR");
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    funcs.vkAcquireFullScreenExclusiveModeEXT = (PFN_vkAcquireFullScreenExclusiveModeEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkAcquireFullScreenExclusiveModeEXT");
    funcs.vkReleaseFullScreenExclusiveModeEXT = (PFN_vkReleaseFullScreenExclusiveModeEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkReleaseFullScreenExclusiveModeEXT");
    funcs.vkGetDeviceGroupSurfacePresentModes2EXT = (PFN_vkGetDeviceGroupSurfacePresentModes2EXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceGroupSurfacePresentModes2EXT");
#endif // VK_USE_PLATFORM_WIN32_KHR
    funcs.vkGetPipelineExecutablePropertiesKHR = (PFN_vkGetPipelineExecutablePropertiesKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPipelineExecutablePropertiesKHR");
    funcs.vkGetPipelineExecutableStatisticsKHR = (PFN_vkGetPipelineExecutableStatisticsKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPipelineExecutableStatisticsKHR");
    funcs.vkGetPipelineExecutableInternalRepresentationsKHR = (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPipelineExecutableInternalRepresentationsKHR");
    funcs.vkGetGeneratedCommandsMemoryRequirementsNV = (PFN_vkGetGeneratedCommandsMemoryRequirementsNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetGeneratedCommandsMemoryRequirementsNV");
    funcs.vkCreateIndirectCommandsLayoutNV = (PFN_vkCreateIndirectCommandsLayoutNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateIndirectCommandsLayoutNV");
    funcs.vkDestroyIndirectCommandsLayoutNV = (PFN_vkDestroyIndirectCommandsLayoutNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyIndirectCommandsLayoutNV");
    funcs.vkCreateCudaModuleNV = (PFN_vkCreateCudaModuleNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateCudaModuleNV");
    funcs.vkGetCudaModuleCacheNV = (PFN_vkGetCudaModuleCacheNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetCudaModuleCacheNV");
    funcs.vkCreateCudaFunctionNV = (PFN_vkCreateCudaFunctionNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateCudaFunctionNV");
    funcs.vkDestroyCudaModuleNV = (PFN_vkDestroyCudaModuleNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyCudaModuleNV");
    funcs.vkDestroyCudaFunctionNV = (PFN_vkDestroyCudaFunctionNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyCudaFunctionNV");
    funcs.vkGetFramebufferTilePropertiesQCOM = (PFN_vkGetFramebufferTilePropertiesQCOM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetFramebufferTilePropertiesQCOM");
    funcs.vkGetDynamicRenderingTilePropertiesQCOM = (PFN_vkGetDynamicRenderingTilePropertiesQCOM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDynamicRenderingTilePropertiesQCOM");
#if defined(VK_USE_PLATFORM_METAL_EXT)
    funcs.vkExportMetalObjectsEXT = (PFN_vkExportMetalObjectsEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkExportMetalObjectsEXT");
#endif // VK_USE_PLATFORM_METAL_EXT
    funcs.vkGetDescriptorSetLayoutSizeEXT = (PFN_vkGetDescriptorSetLayoutSizeEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDescriptorSetLayoutSizeEXT");
    funcs.vkGetDescriptorSetLayoutBindingOffsetEXT = (PFN_vkGetDescriptorSetLayoutBindingOffsetEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDescriptorSetLayoutBindingOffsetEXT");
    funcs.vkGetDescriptorEXT = (PFN_vkGetDescriptorEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDescriptorEXT");
    funcs.vkGetBufferOpaqueCaptureDescriptorDataEXT = (PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetBufferOpaqueCaptureDescriptorDataEXT");
    funcs.vkGetImageOpaqueCaptureDescriptorDataEXT = (PFN_vkGetImageOpaqueCaptureDescriptorDataEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetImageOpaqueCaptureDescriptorDataEXT");
    funcs.vkGetImageViewOpaqueCaptureDescriptorDataEXT = (PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetImageViewOpaqueCaptureDescriptorDataEXT");
    funcs.vkGetSamplerOpaqueCaptureDescriptorDataEXT = (PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetSamplerOpaqueCaptureDescriptorDataEXT");
    funcs.vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT = (PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT");
    funcs.vkGetDeviceFaultInfoEXT = (PFN_vkGetDeviceFaultInfoEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceFaultInfoEXT");
#if defined(VK_USE_PLATFORM_FUCHSIA)
    funcs.vkGetMemoryZirconHandleFUCHSIA = (PFN_vkGetMemoryZirconHandleFUCHSIA)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMemoryZirconHandleFUCHSIA");
    funcs.vkGetMemoryZirconHandlePropertiesFUCHSIA = (PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMemoryZirconHandlePropertiesFUCHSIA");
    funcs.vkImportSemaphoreZirconHandleFUCHSIA = (PFN_vkImportSemaphoreZirconHandleFUCHSIA)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkImportSemaphoreZirconHandleFUCHSIA");
    funcs.vkGetSemaphoreZirconHandleFUCHSIA = (PFN_vkGetSemaphoreZirconHandleFUCHSIA)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetSemaphoreZirconHandleFUCHSIA");
    funcs.vkCreateBufferCollectionFUCHSIA = (PFN_vkCreateBufferCollectionFUCHSIA)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateBufferCollectionFUCHSIA");
    funcs.vkSetBufferCollectionImageConstraintsFUCHSIA = (PFN_vkSetBufferCollectionImageConstraintsFUCHSIA)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkSetBufferCollectionImageConstraintsFUCHSIA");
    funcs.vkSetBufferCollectionBufferConstraintsFUCHSIA = (PFN_vkSetBufferCollectionBufferConstraintsFUCHSIA)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkSetBufferCollectionBufferConstraintsFUCHSIA");
    funcs.vkDestroyBufferCollectionFUCHSIA = (PFN_vkDestroyBufferCollectionFUCHSIA)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyBufferCollectionFUCHSIA");
    funcs.vkGetBufferCollectionPropertiesFUCHSIA = (PFN_vkGetBufferCollectionPropertiesFUCHSIA)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetBufferCollectionPropertiesFUCHSIA");
#endif // VK_USE_PLATFORM_FUCHSIA
    funcs.vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI = (PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI");
    funcs.vkGetMemoryRemoteAddressNV = (PFN_vkGetMemoryRemoteAddressNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMemoryRemoteAddressNV");
    funcs.vkGetPipelinePropertiesEXT = (PFN_vkGetPipelinePropertiesEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPipelinePropertiesEXT");
    funcs.vkCreateMicromapEXT = (PFN_vkCreateMicromapEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateMicromapEXT");
    funcs.vkDestroyMicromapEXT = (PFN_vkDestroyMicromapEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyMicromapEXT");
    funcs.vkBuildMicromapsEXT = (PFN_vkBuildMicromapsEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkBuildMicromapsEXT");
    funcs.vkCopyMicromapEXT = (PFN_vkCopyMicromapEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCopyMicromapEXT");
    funcs.vkCopyMicromapToMemoryEXT = (PFN_vkCopyMicromapToMemoryEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCopyMicromapToMemoryEXT");
    funcs.vkCopyMemoryToMicromapEXT = (PFN_vkCopyMemoryToMicromapEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCopyMemoryToMicromapEXT");
    funcs.vkWriteMicromapsPropertiesEXT = (PFN_vkWriteMicromapsPropertiesEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkWriteMicromapsPropertiesEXT");
    funcs.vkGetDeviceMicromapCompatibilityEXT = (PFN_vkGetDeviceMicromapCompatibilityEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceMicromapCompatibilityEXT");
    funcs.vkGetMicromapBuildSizesEXT = (PFN_vkGetMicromapBuildSizesEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMicromapBuildSizesEXT");
    funcs.vkSetDeviceMemoryPriorityEXT = (PFN_vkSetDeviceMemoryPriorityEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkSetDeviceMemoryPriorityEXT");
    funcs.vkGetDescriptorSetLayoutHostMappingInfoVALVE = (PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDescriptorSetLayoutHostMappingInfoVALVE");
    funcs.vkGetDescriptorSetHostMappingVALVE = (PFN_vkGetDescriptorSetHostMappingVALVE)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDescriptorSetHostMappingVALVE");
    funcs.vkGetPipelineIndirectMemoryRequirementsNV = (PFN_vkGetPipelineIndirectMemoryRequirementsNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPipelineIndirectMemoryRequirementsNV");
    funcs.vkGetPipelineIndirectDeviceAddressNV = (PFN_vkGetPipelineIndirectDeviceAddressNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPipelineIndirectDeviceAddressNV");
    funcs.vkCreateTensorARM = (PFN_vkCreateTensorARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateTensorARM");
    funcs.vkDestroyTensorARM = (PFN_vkDestroyTensorARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyTensorARM");
    funcs.vkCreateTensorViewARM = (PFN_vkCreateTensorViewARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateTensorViewARM");
    funcs.vkDestroyTensorViewARM = (PFN_vkDestroyTensorViewARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyTensorViewARM");
    funcs.vkGetTensorMemoryRequirementsARM = (PFN_vkGetTensorMemoryRequirementsARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetTensorMemoryRequirementsARM");
    funcs.vkBindTensorMemoryARM = (PFN_vkBindTensorMemoryARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkBindTensorMemoryARM");
    funcs.vkGetDeviceTensorMemoryRequirementsARM = (PFN_vkGetDeviceTensorMemoryRequirementsARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDeviceTensorMemoryRequirementsARM");
    funcs.vkGetTensorOpaqueCaptureDescriptorDataARM = (PFN_vkGetTensorOpaqueCaptureDescriptorDataARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetTensorOpaqueCaptureDescriptorDataARM");
    funcs.vkGetTensorViewOpaqueCaptureDescriptorDataARM = (PFN_vkGetTensorViewOpaqueCaptureDescriptorDataARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetTensorViewOpaqueCaptureDescriptorDataARM");
    funcs.vkGetShaderModuleIdentifierEXT = (PFN_vkGetShaderModuleIdentifierEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetShaderModuleIdentifierEXT");
    funcs.vkGetShaderModuleCreateInfoIdentifierEXT = (PFN_vkGetShaderModuleCreateInfoIdentifierEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetShaderModuleCreateInfoIdentifierEXT");
    funcs.vkCreateOpticalFlowSessionNV = (PFN_vkCreateOpticalFlowSessionNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateOpticalFlowSessionNV");
    funcs.vkDestroyOpticalFlowSessionNV = (PFN_vkDestroyOpticalFlowSessionNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyOpticalFlowSessionNV");
    funcs.vkBindOpticalFlowSessionImageNV = (PFN_vkBindOpticalFlowSessionImageNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkBindOpticalFlowSessionImageNV");
    funcs.vkAntiLagUpdateAMD = (PFN_vkAntiLagUpdateAMD)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkAntiLagUpdateAMD");
    funcs.vkWaitForPresent2KHR = (PFN_vkWaitForPresent2KHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkWaitForPresent2KHR");
    funcs.vkCreateShadersEXT = (PFN_vkCreateShadersEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateShadersEXT");
    funcs.vkDestroyShaderEXT = (PFN_vkDestroyShaderEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyShaderEXT");
    funcs.vkGetShaderBinaryDataEXT = (PFN_vkGetShaderBinaryDataEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetShaderBinaryDataEXT");
    funcs.vkCreatePipelineBinariesKHR = (PFN_vkCreatePipelineBinariesKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreatePipelineBinariesKHR");
    funcs.vkDestroyPipelineBinaryKHR = (PFN_vkDestroyPipelineBinaryKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyPipelineBinaryKHR");
    funcs.vkGetPipelineKeyKHR = (PFN_vkGetPipelineKeyKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPipelineKeyKHR");
    funcs.vkGetPipelineBinaryDataKHR = (PFN_vkGetPipelineBinaryDataKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPipelineBinaryDataKHR");
    funcs.vkReleaseCapturedPipelineDataKHR = (PFN_vkReleaseCapturedPipelineDataKHR)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkReleaseCapturedPipelineDataKHR");
    funcs.vkConvertCooperativeVectorMatrixNV = (PFN_vkConvertCooperativeVectorMatrixNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkConvertCooperativeVectorMatrixNV");
    funcs.vkSetLatencySleepModeNV = (PFN_vkSetLatencySleepModeNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkSetLatencySleepModeNV");
    funcs.vkLatencySleepNV = (PFN_vkLatencySleepNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkLatencySleepNV");
    funcs.vkSetLatencyMarkerNV = (PFN_vkSetLatencyMarkerNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkSetLatencyMarkerNV");
    funcs.vkGetLatencyTimingsNV = (PFN_vkGetLatencyTimingsNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetLatencyTimingsNV");
    funcs.vkCreateDataGraphPipelinesARM = (PFN_vkCreateDataGraphPipelinesARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateDataGraphPipelinesARM");
    funcs.vkCreateDataGraphPipelineSessionARM = (PFN_vkCreateDataGraphPipelineSessionARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateDataGraphPipelineSessionARM");
    funcs.vkGetDataGraphPipelineSessionBindPointRequirementsARM = (PFN_vkGetDataGraphPipelineSessionBindPointRequirementsARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDataGraphPipelineSessionBindPointRequirementsARM");
    funcs.vkGetDataGraphPipelineSessionMemoryRequirementsARM = (PFN_vkGetDataGraphPipelineSessionMemoryRequirementsARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDataGraphPipelineSessionMemoryRequirementsARM");
    funcs.vkBindDataGraphPipelineSessionMemoryARM = (PFN_vkBindDataGraphPipelineSessionMemoryARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkBindDataGraphPipelineSessionMemoryARM");
    funcs.vkDestroyDataGraphPipelineSessionARM = (PFN_vkDestroyDataGraphPipelineSessionARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyDataGraphPipelineSessionARM");
    funcs.vkGetDataGraphPipelineAvailablePropertiesARM = (PFN_vkGetDataGraphPipelineAvailablePropertiesARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDataGraphPipelineAvailablePropertiesARM");
    funcs.vkGetDataGraphPipelinePropertiesARM = (PFN_vkGetDataGraphPipelinePropertiesARM)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetDataGraphPipelinePropertiesARM");
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    funcs.vkGetScreenBufferPropertiesQNX = (PFN_vkGetScreenBufferPropertiesQNX)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetScreenBufferPropertiesQNX");
#endif // VK_USE_PLATFORM_SCREEN_QNX
    funcs.vkCreateExternalComputeQueueNV = (PFN_vkCreateExternalComputeQueueNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateExternalComputeQueueNV");
    funcs.vkDestroyExternalComputeQueueNV = (PFN_vkDestroyExternalComputeQueueNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyExternalComputeQueueNV");
    funcs.vkGetExternalComputeQueueDataNV = (PFN_vkGetExternalComputeQueueDataNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetExternalComputeQueueDataNV");
    funcs.vkGetClusterAccelerationStructureBuildSizesNV = (PFN_vkGetClusterAccelerationStructureBuildSizesNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetClusterAccelerationStructureBuildSizesNV");
    funcs.vkGetPartitionedAccelerationStructuresBuildSizesNV = (PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetPartitionedAccelerationStructuresBuildSizesNV");
    funcs.vkGetGeneratedCommandsMemoryRequirementsEXT = (PFN_vkGetGeneratedCommandsMemoryRequirementsEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetGeneratedCommandsMemoryRequirementsEXT");
    funcs.vkCreateIndirectCommandsLayoutEXT = (PFN_vkCreateIndirectCommandsLayoutEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateIndirectCommandsLayoutEXT");
    funcs.vkDestroyIndirectCommandsLayoutEXT = (PFN_vkDestroyIndirectCommandsLayoutEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyIndirectCommandsLayoutEXT");
    funcs.vkCreateIndirectExecutionSetEXT = (PFN_vkCreateIndirectExecutionSetEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkCreateIndirectExecutionSetEXT");
    funcs.vkDestroyIndirectExecutionSetEXT = (PFN_vkDestroyIndirectExecutionSetEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkDestroyIndirectExecutionSetEXT");
    funcs.vkUpdateIndirectExecutionSetPipelineEXT = (PFN_vkUpdateIndirectExecutionSetPipelineEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkUpdateIndirectExecutionSetPipelineEXT");
    funcs.vkUpdateIndirectExecutionSetShaderEXT = (PFN_vkUpdateIndirectExecutionSetShaderEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkUpdateIndirectExecutionSetShaderEXT");
#if defined(VK_USE_PLATFORM_METAL_EXT)
    funcs.vkGetMemoryMetalHandleEXT = (PFN_vkGetMemoryMetalHandleEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMemoryMetalHandleEXT");
    funcs.vkGetMemoryMetalHandlePropertiesEXT = (PFN_vkGetMemoryMetalHandlePropertiesEXT)funcs.vkGetDeviceProcAddr(detail::_device.handle(), "vkGetMemoryMetalHandlePropertiesEXT");
#endif // VK_USE_PLATFORM_METAL_EXT
}

void initInstance_throw(const InstanceCreateInfo& createInfo) {
    assert(detail::_library && "vk::loadLib() must be called before vk::initInstance().");
    destroyInstance();
    Instance::HandleType instanceHandle;
    Result r = funcs.vkCreateInstance(&createInfo, detail::_allocator, &instanceHandle);
    checkForSuccessValue(r, "vkCreateInstance");
    initInstance(instanceHandle);
}

Result initInstance_noThrow(const InstanceCreateInfo& createInfo) noexcept {
    assert(detail::_library && "vk::loadLib() must be called before vk::initInstance().");
    destroyInstance();
    Instance::HandleType instanceHandle;
    Result r = funcs.vkCreateInstance(&createInfo, detail::_allocator, &instanceHandle);
    if (r != Result::eSuccess) return r;
    initInstance(instanceHandle);
    return Result::eSuccess;
}

void initInstance(Instance instance) noexcept {
    assert(detail::_library && "vk::loadLib() must be called before vk::initInstance().");
    destroyInstance();
    detail::_instance = instance;
    initInstancePFNs();
}

void initDevice_throw(PhysicalDevice pd, const DeviceCreateInfo& createInfo) {
    assert(detail::_instance && "vk::initInstance() must be called before vk::initDevice().");
    destroyDevice();
    Device::HandleType deviceHandle;
    Result r = funcs.vkCreateDevice(pd.handle(), &createInfo, detail::_allocator, &deviceHandle);
    checkForSuccessValue(r, "vkCreateDevice");
    initDevice(pd, deviceHandle);
}

Result initDevice_noThrow(PhysicalDevice pd, const DeviceCreateInfo& createInfo) noexcept {
    assert(detail::_instance && "vk::initInstance() must be called before vk::initDevice().");
    destroyDevice();
    Device::HandleType deviceHandle;
    Result r = funcs.vkCreateDevice(pd.handle(), &createInfo, detail::_allocator, &deviceHandle);
    if (r != Result::eSuccess) return r;
    initDevice(pd, deviceHandle);
    return Result::eSuccess;
}

void initDevice(PhysicalDevice physicalDevice, Device device) noexcept {
    assert(detail::_instance && "vk::initInstance() must be called before vk::initDevice().");
    destroyDevice();
    detail::_physicalDevice = physicalDevice;
    detail::_device = device;
    initDevicePFNs();
}

void setAllocator(const AllocationCallbacks* a) noexcept {
    assert(!detail::_instance && "vk::setAllocator() must be called before vk::initInstance().");
    detail::_allocator = a;
}

const char* to_cstr(Result v) {
    switch (v) {
    case Result::eSuccess: return "Success";
    case Result::eNotReady: return "NotReady";
    case Result::eTimeout: return "Timeout";
    case Result::eEventSet: return "EventSet";
    case Result::eEventReset: return "EventReset";
    case Result::eIncomplete: return "Incomplete";
    case Result::eErrorOutOfHostMemory: return "ErrorOutOfHostMemory";
    case Result::eErrorOutOfDeviceMemory: return "ErrorOutOfDeviceMemory";
    case Result::eErrorInitializationFailed: return "ErrorInitializationFailed";
    case Result::eErrorDeviceLost: return "ErrorDeviceLost";
    case Result::eErrorMemoryMapFailed: return "ErrorMemoryMapFailed";
    case Result::eErrorLayerNotPresent: return "ErrorLayerNotPresent";
    case Result::eErrorExtensionNotPresent: return "ErrorExtensionNotPresent";
    case Result::eErrorFeatureNotPresent: return "ErrorFeatureNotPresent";
    case Result::eErrorIncompatibleDriver: return "ErrorIncompatibleDriver";
    case Result::eErrorTooManyObjects: return "ErrorTooManyObjects";
    case Result::eErrorFormatNotSupported: return "ErrorFormatNotSupported";
    case Result::eErrorFragmentedPool: return "ErrorFragmentedPool";
    case Result::eErrorUnknown: return "ErrorUnknown";
    case Result::eErrorSurfaceLostKHR: return "ErrorSurfaceLostKHR";
    case Result::eErrorNativeWindowInUseKHR: return "ErrorNativeWindowInUseKHR";
    case Result::eSuboptimalKHR: return "SuboptimalKHR";
    case Result::eErrorOutOfDateKHR: return "ErrorOutOfDateKHR";
    case Result::eErrorIncompatibleDisplayKHR: return "ErrorIncompatibleDisplayKHR";
    case Result::eErrorValidationFailedEXT: return "ErrorValidationFailedEXT";
    case Result::eErrorInvalidShaderNV: return "ErrorInvalidShaderNV";
    case Result::eErrorOutOfPoolMemory: return "ErrorOutOfPoolMemory";
    case Result::eErrorInvalidExternalHandle: return "ErrorInvalidExternalHandle";
    case Result::eErrorInvalidDrmFormatModifierPlaneLayoutEXT: return "ErrorInvalidDrmFormatModifierPlaneLayoutEXT";
    case Result::eErrorFragmentation: return "ErrorFragmentation";
    case Result::eErrorNotPermitted: return "ErrorNotPermitted";
    case Result::eErrorInvalidOpaqueCaptureAddress: return "ErrorInvalidOpaqueCaptureAddress";
    case Result::eThreadIdleKHR: return "ThreadIdleKHR";
    case Result::eThreadDoneKHR: return "ThreadDoneKHR";
    case Result::eOperationDeferredKHR: return "OperationDeferredKHR";
    case Result::eOperationNotDeferredKHR: return "OperationNotDeferredKHR";
    case Result::ePipelineCompileRequired: return "PipelineCompileRequired";
    case Result::eErrorCompressionExhaustedEXT: return "ErrorCompressionExhaustedEXT";
    case Result::eIncompatibleShaderBinaryEXT: return "IncompatibleShaderBinaryEXT";
    case Result::ePipelineBinaryMissingKHR: return "PipelineBinaryMissingKHR";
    case Result::eErrorNotEnoughSpaceKHR: return "ErrorNotEnoughSpaceKHR";
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    case Result::eErrorFullScreenExclusiveModeLostEXT: return "ErrorFullScreenExclusiveModeLostEXT";
#endif // VK_USE_PLATFORM_WIN32_KHR
    default: return "Unknown";
    }
}

const char* to_cstr(StructureType v) {
    switch (v) {
    case StructureType::eApplicationInfo: return "ApplicationInfo";
    case StructureType::eInstanceCreateInfo: return "InstanceCreateInfo";
    case StructureType::eDeviceQueueCreateInfo: return "DeviceQueueCreateInfo";
    case StructureType::eDeviceCreateInfo: return "DeviceCreateInfo";
    case StructureType::eSubmitInfo: return "SubmitInfo";
    case StructureType::eMemoryAllocateInfo: return "MemoryAllocateInfo";
    case StructureType::eMappedMemoryRange: return "MappedMemoryRange";
    case StructureType::eBindSparseInfo: return "BindSparseInfo";
    case StructureType::eFenceCreateInfo: return "FenceCreateInfo";
    case StructureType::eSemaphoreCreateInfo: return "SemaphoreCreateInfo";
    case StructureType::eEventCreateInfo: return "EventCreateInfo";
    case StructureType::eQueryPoolCreateInfo: return "QueryPoolCreateInfo";
    case StructureType::eBufferCreateInfo: return "BufferCreateInfo";
    case StructureType::eBufferViewCreateInfo: return "BufferViewCreateInfo";
    case StructureType::eImageCreateInfo: return "ImageCreateInfo";
    case StructureType::eImageViewCreateInfo: return "ImageViewCreateInfo";
    case StructureType::eShaderModuleCreateInfo: return "ShaderModuleCreateInfo";
    case StructureType::ePipelineCacheCreateInfo: return "PipelineCacheCreateInfo";
    case StructureType::ePipelineShaderStageCreateInfo: return "PipelineShaderStageCreateInfo";
    case StructureType::ePipelineVertexInputStateCreateInfo: return "PipelineVertexInputStateCreateInfo";
    case StructureType::ePipelineInputAssemblyStateCreateInfo: return "PipelineInputAssemblyStateCreateInfo";
    case StructureType::ePipelineTessellationStateCreateInfo: return "PipelineTessellationStateCreateInfo";
    case StructureType::ePipelineViewportStateCreateInfo: return "PipelineViewportStateCreateInfo";
    case StructureType::ePipelineRasterizationStateCreateInfo: return "PipelineRasterizationStateCreateInfo";
    case StructureType::ePipelineMultisampleStateCreateInfo: return "PipelineMultisampleStateCreateInfo";
    case StructureType::ePipelineDepthStencilStateCreateInfo: return "PipelineDepthStencilStateCreateInfo";
    case StructureType::ePipelineColorBlendStateCreateInfo: return "PipelineColorBlendStateCreateInfo";
    case StructureType::ePipelineDynamicStateCreateInfo: return "PipelineDynamicStateCreateInfo";
    case StructureType::eGraphicsPipelineCreateInfo: return "GraphicsPipelineCreateInfo";
    case StructureType::eComputePipelineCreateInfo: return "ComputePipelineCreateInfo";
    case StructureType::ePipelineLayoutCreateInfo: return "PipelineLayoutCreateInfo";
    case StructureType::eSamplerCreateInfo: return "SamplerCreateInfo";
    case StructureType::eDescriptorSetLayoutCreateInfo: return "DescriptorSetLayoutCreateInfo";
    case StructureType::eDescriptorPoolCreateInfo: return "DescriptorPoolCreateInfo";
    case StructureType::eDescriptorSetAllocateInfo: return "DescriptorSetAllocateInfo";
    case StructureType::eWriteDescriptorSet: return "WriteDescriptorSet";
    case StructureType::eCopyDescriptorSet: return "CopyDescriptorSet";
    case StructureType::eFramebufferCreateInfo: return "FramebufferCreateInfo";
    case StructureType::eRenderPassCreateInfo: return "RenderPassCreateInfo";
    case StructureType::eCommandPoolCreateInfo: return "CommandPoolCreateInfo";
    case StructureType::eCommandBufferAllocateInfo: return "CommandBufferAllocateInfo";
    case StructureType::eCommandBufferInheritanceInfo: return "CommandBufferInheritanceInfo";
    case StructureType::eCommandBufferBeginInfo: return "CommandBufferBeginInfo";
    case StructureType::eRenderPassBeginInfo: return "RenderPassBeginInfo";
    case StructureType::eBufferMemoryBarrier: return "BufferMemoryBarrier";
    case StructureType::eImageMemoryBarrier: return "ImageMemoryBarrier";
    case StructureType::eMemoryBarrier: return "MemoryBarrier";
    case StructureType::eLoaderInstanceCreateInfo: return "LoaderInstanceCreateInfo";
    case StructureType::eLoaderDeviceCreateInfo: return "LoaderDeviceCreateInfo";
    case StructureType::ePhysicalDeviceVulkan11Features: return "PhysicalDeviceVulkan11Features";
    case StructureType::ePhysicalDeviceVulkan11Properties: return "PhysicalDeviceVulkan11Properties";
    case StructureType::ePhysicalDeviceVulkan12Features: return "PhysicalDeviceVulkan12Features";
    case StructureType::ePhysicalDeviceVulkan12Properties: return "PhysicalDeviceVulkan12Properties";
    case StructureType::ePhysicalDeviceVulkan13Features: return "PhysicalDeviceVulkan13Features";
    case StructureType::ePhysicalDeviceVulkan13Properties: return "PhysicalDeviceVulkan13Properties";
    case StructureType::ePhysicalDeviceVulkan14Features: return "PhysicalDeviceVulkan14Features";
    case StructureType::ePhysicalDeviceVulkan14Properties: return "PhysicalDeviceVulkan14Properties";
    case StructureType::eSwapchainCreateInfoKHR: return "SwapchainCreateInfoKHR";
    case StructureType::ePresentInfoKHR: return "PresentInfoKHR";
    case StructureType::eDisplayModeCreateInfoKHR: return "DisplayModeCreateInfoKHR";
    case StructureType::eDisplaySurfaceCreateInfoKHR: return "DisplaySurfaceCreateInfoKHR";
    case StructureType::eDisplayPresentInfoKHR: return "DisplayPresentInfoKHR";
    case StructureType::eDebugReportCallbackCreateInfoEXT: return "DebugReportCallbackCreateInfoEXT";
    case StructureType::ePipelineRasterizationStateRasterizationOrderAMD: return "PipelineRasterizationStateRasterizationOrderAMD";
    case StructureType::eDebugMarkerObjectNameInfoEXT: return "DebugMarkerObjectNameInfoEXT";
    case StructureType::eDebugMarkerObjectTagInfoEXT: return "DebugMarkerObjectTagInfoEXT";
    case StructureType::eDebugMarkerMarkerInfoEXT: return "DebugMarkerMarkerInfoEXT";
    case StructureType::eDedicatedAllocationImageCreateInfoNV: return "DedicatedAllocationImageCreateInfoNV";
    case StructureType::eDedicatedAllocationBufferCreateInfoNV: return "DedicatedAllocationBufferCreateInfoNV";
    case StructureType::eDedicatedAllocationMemoryAllocateInfoNV: return "DedicatedAllocationMemoryAllocateInfoNV";
    case StructureType::ePhysicalDeviceTransformFeedbackFeaturesEXT: return "PhysicalDeviceTransformFeedbackFeaturesEXT";
    case StructureType::ePhysicalDeviceTransformFeedbackPropertiesEXT: return "PhysicalDeviceTransformFeedbackPropertiesEXT";
    case StructureType::ePipelineRasterizationStateStreamCreateInfoEXT: return "PipelineRasterizationStateStreamCreateInfoEXT";
    case StructureType::eCuModuleCreateInfoNVX: return "CuModuleCreateInfoNVX";
    case StructureType::eCuFunctionCreateInfoNVX: return "CuFunctionCreateInfoNVX";
    case StructureType::eCuLaunchInfoNVX: return "CuLaunchInfoNVX";
    case StructureType::eCuModuleTexturingModeCreateInfoNVX: return "CuModuleTexturingModeCreateInfoNVX";
    case StructureType::eImageViewHandleInfoNVX: return "ImageViewHandleInfoNVX";
    case StructureType::eImageViewAddressPropertiesNVX: return "ImageViewAddressPropertiesNVX";
    case StructureType::eTextureLodGatherFormatPropertiesAMD: return "TextureLodGatherFormatPropertiesAMD";
    case StructureType::eRenderingInfo: return "RenderingInfo";
    case StructureType::eRenderingAttachmentInfo: return "RenderingAttachmentInfo";
    case StructureType::ePipelineRenderingCreateInfo: return "PipelineRenderingCreateInfo";
    case StructureType::ePhysicalDeviceDynamicRenderingFeatures: return "PhysicalDeviceDynamicRenderingFeatures";
    case StructureType::eCommandBufferInheritanceRenderingInfo: return "CommandBufferInheritanceRenderingInfo";
    case StructureType::eMultiviewPerViewAttributesInfoNVX: return "MultiviewPerViewAttributesInfoNVX";
    case StructureType::eAttachmentSampleCountInfoAMD: return "AttachmentSampleCountInfoAMD";
    case StructureType::eRenderingFragmentDensityMapAttachmentInfoEXT: return "RenderingFragmentDensityMapAttachmentInfoEXT";
    case StructureType::eRenderingFragmentShadingRateAttachmentInfoKHR: return "RenderingFragmentShadingRateAttachmentInfoKHR";
    case StructureType::ePhysicalDeviceCornerSampledImageFeaturesNV: return "PhysicalDeviceCornerSampledImageFeaturesNV";
    case StructureType::eRenderPassMultiviewCreateInfo: return "RenderPassMultiviewCreateInfo";
    case StructureType::ePhysicalDeviceMultiviewFeatures: return "PhysicalDeviceMultiviewFeatures";
    case StructureType::ePhysicalDeviceMultiviewProperties: return "PhysicalDeviceMultiviewProperties";
    case StructureType::eExternalMemoryImageCreateInfoNV: return "ExternalMemoryImageCreateInfoNV";
    case StructureType::eExportMemoryAllocateInfoNV: return "ExportMemoryAllocateInfoNV";
    case StructureType::ePhysicalDeviceFeatures2: return "PhysicalDeviceFeatures2";
    case StructureType::ePhysicalDeviceProperties2: return "PhysicalDeviceProperties2";
    case StructureType::eFormatProperties2: return "FormatProperties2";
    case StructureType::eImageFormatProperties2: return "ImageFormatProperties2";
    case StructureType::ePhysicalDeviceImageFormatInfo2: return "PhysicalDeviceImageFormatInfo2";
    case StructureType::eQueueFamilyProperties2: return "QueueFamilyProperties2";
    case StructureType::ePhysicalDeviceMemoryProperties2: return "PhysicalDeviceMemoryProperties2";
    case StructureType::eSparseImageFormatProperties2: return "SparseImageFormatProperties2";
    case StructureType::ePhysicalDeviceSparseImageFormatInfo2: return "PhysicalDeviceSparseImageFormatInfo2";
    case StructureType::eMemoryAllocateFlagsInfo: return "MemoryAllocateFlagsInfo";
    case StructureType::eDeviceGroupRenderPassBeginInfo: return "DeviceGroupRenderPassBeginInfo";
    case StructureType::eDeviceGroupCommandBufferBeginInfo: return "DeviceGroupCommandBufferBeginInfo";
    case StructureType::eDeviceGroupSubmitInfo: return "DeviceGroupSubmitInfo";
    case StructureType::eDeviceGroupBindSparseInfo: return "DeviceGroupBindSparseInfo";
    case StructureType::eBindBufferMemoryDeviceGroupInfo: return "BindBufferMemoryDeviceGroupInfo";
    case StructureType::eBindImageMemoryDeviceGroupInfo: return "BindImageMemoryDeviceGroupInfo";
    case StructureType::eDeviceGroupPresentCapabilitiesKHR: return "DeviceGroupPresentCapabilitiesKHR";
    case StructureType::eImageSwapchainCreateInfoKHR: return "ImageSwapchainCreateInfoKHR";
    case StructureType::eBindImageMemorySwapchainInfoKHR: return "BindImageMemorySwapchainInfoKHR";
    case StructureType::eAcquireNextImageInfoKHR: return "AcquireNextImageInfoKHR";
    case StructureType::eDeviceGroupPresentInfoKHR: return "DeviceGroupPresentInfoKHR";
    case StructureType::eDeviceGroupSwapchainCreateInfoKHR: return "DeviceGroupSwapchainCreateInfoKHR";
    case StructureType::eValidationFlagsEXT: return "ValidationFlagsEXT";
    case StructureType::ePhysicalDeviceShaderDrawParametersFeatures: return "PhysicalDeviceShaderDrawParametersFeatures";
    case StructureType::ePhysicalDeviceTextureCompressionAstcHdrFeatures: return "PhysicalDeviceTextureCompressionAstcHdrFeatures";
    case StructureType::eImageViewAstcDecodeModeEXT: return "ImageViewAstcDecodeModeEXT";
    case StructureType::ePhysicalDeviceAstcDecodeFeaturesEXT: return "PhysicalDeviceAstcDecodeFeaturesEXT";
    case StructureType::ePipelineRobustnessCreateInfo: return "PipelineRobustnessCreateInfo";
    case StructureType::ePhysicalDevicePipelineRobustnessFeatures: return "PhysicalDevicePipelineRobustnessFeatures";
    case StructureType::ePhysicalDevicePipelineRobustnessProperties: return "PhysicalDevicePipelineRobustnessProperties";
    case StructureType::ePhysicalDeviceGroupProperties: return "PhysicalDeviceGroupProperties";
    case StructureType::eDeviceGroupDeviceCreateInfo: return "DeviceGroupDeviceCreateInfo";
    case StructureType::ePhysicalDeviceExternalImageFormatInfo: return "PhysicalDeviceExternalImageFormatInfo";
    case StructureType::eExternalImageFormatProperties: return "ExternalImageFormatProperties";
    case StructureType::ePhysicalDeviceExternalBufferInfo: return "PhysicalDeviceExternalBufferInfo";
    case StructureType::eExternalBufferProperties: return "ExternalBufferProperties";
    case StructureType::ePhysicalDeviceIdProperties: return "PhysicalDeviceIdProperties";
    case StructureType::eExternalMemoryBufferCreateInfo: return "ExternalMemoryBufferCreateInfo";
    case StructureType::eExternalMemoryImageCreateInfo: return "ExternalMemoryImageCreateInfo";
    case StructureType::eExportMemoryAllocateInfo: return "ExportMemoryAllocateInfo";
    case StructureType::eImportMemoryFdInfoKHR: return "ImportMemoryFdInfoKHR";
    case StructureType::eMemoryFdPropertiesKHR: return "MemoryFdPropertiesKHR";
    case StructureType::eMemoryGetFdInfoKHR: return "MemoryGetFdInfoKHR";
    case StructureType::ePhysicalDeviceExternalSemaphoreInfo: return "PhysicalDeviceExternalSemaphoreInfo";
    case StructureType::eExternalSemaphoreProperties: return "ExternalSemaphoreProperties";
    case StructureType::eExportSemaphoreCreateInfo: return "ExportSemaphoreCreateInfo";
    case StructureType::eImportSemaphoreFdInfoKHR: return "ImportSemaphoreFdInfoKHR";
    case StructureType::eSemaphoreGetFdInfoKHR: return "SemaphoreGetFdInfoKHR";
    case StructureType::ePhysicalDevicePushDescriptorProperties: return "PhysicalDevicePushDescriptorProperties";
    case StructureType::eCommandBufferInheritanceConditionalRenderingInfoEXT: return "CommandBufferInheritanceConditionalRenderingInfoEXT";
    case StructureType::ePhysicalDeviceConditionalRenderingFeaturesEXT: return "PhysicalDeviceConditionalRenderingFeaturesEXT";
    case StructureType::eConditionalRenderingBeginInfoEXT: return "ConditionalRenderingBeginInfoEXT";
    case StructureType::ePhysicalDeviceShaderFloat16Int8Features: return "PhysicalDeviceShaderFloat16Int8Features";
    case StructureType::ePhysicalDevice16BitStorageFeatures: return "PhysicalDevice16BitStorageFeatures";
    case StructureType::ePresentRegionsKHR: return "PresentRegionsKHR";
    case StructureType::eDescriptorUpdateTemplateCreateInfo: return "DescriptorUpdateTemplateCreateInfo";
    case StructureType::ePipelineViewportWScalingStateCreateInfoNV: return "PipelineViewportWScalingStateCreateInfoNV";
    case StructureType::eSurfaceCapabilities2EXT: return "SurfaceCapabilities2EXT";
    case StructureType::eDisplayPowerInfoEXT: return "DisplayPowerInfoEXT";
    case StructureType::eDeviceEventInfoEXT: return "DeviceEventInfoEXT";
    case StructureType::eDisplayEventInfoEXT: return "DisplayEventInfoEXT";
    case StructureType::eSwapchainCounterCreateInfoEXT: return "SwapchainCounterCreateInfoEXT";
    case StructureType::ePresentTimesInfoGOOGLE: return "PresentTimesInfoGOOGLE";
    case StructureType::ePhysicalDeviceSubgroupProperties: return "PhysicalDeviceSubgroupProperties";
    case StructureType::ePhysicalDeviceMultiviewPerViewAttributesPropertiesNVX: return "PhysicalDeviceMultiviewPerViewAttributesPropertiesNVX";
    case StructureType::ePipelineViewportSwizzleStateCreateInfoNV: return "PipelineViewportSwizzleStateCreateInfoNV";
    case StructureType::ePhysicalDeviceDiscardRectanglePropertiesEXT: return "PhysicalDeviceDiscardRectanglePropertiesEXT";
    case StructureType::ePipelineDiscardRectangleStateCreateInfoEXT: return "PipelineDiscardRectangleStateCreateInfoEXT";
    case StructureType::ePhysicalDeviceConservativeRasterizationPropertiesEXT: return "PhysicalDeviceConservativeRasterizationPropertiesEXT";
    case StructureType::ePipelineRasterizationConservativeStateCreateInfoEXT: return "PipelineRasterizationConservativeStateCreateInfoEXT";
    case StructureType::ePhysicalDeviceDepthClipEnableFeaturesEXT: return "PhysicalDeviceDepthClipEnableFeaturesEXT";
    case StructureType::ePipelineRasterizationDepthClipStateCreateInfoEXT: return "PipelineRasterizationDepthClipStateCreateInfoEXT";
    case StructureType::eHdrMetadataEXT: return "HdrMetadataEXT";
    case StructureType::ePhysicalDeviceImagelessFramebufferFeatures: return "PhysicalDeviceImagelessFramebufferFeatures";
    case StructureType::eFramebufferAttachmentsCreateInfo: return "FramebufferAttachmentsCreateInfo";
    case StructureType::eFramebufferAttachmentImageInfo: return "FramebufferAttachmentImageInfo";
    case StructureType::eRenderPassAttachmentBeginInfo: return "RenderPassAttachmentBeginInfo";
    case StructureType::eAttachmentDescription2: return "AttachmentDescription2";
    case StructureType::eAttachmentReference2: return "AttachmentReference2";
    case StructureType::eSubpassDescription2: return "SubpassDescription2";
    case StructureType::eSubpassDependency2: return "SubpassDependency2";
    case StructureType::eRenderPassCreateInfo2: return "RenderPassCreateInfo2";
    case StructureType::eSubpassBeginInfo: return "SubpassBeginInfo";
    case StructureType::eSubpassEndInfo: return "SubpassEndInfo";
    case StructureType::ePhysicalDeviceRelaxedLineRasterizationFeaturesIMG: return "PhysicalDeviceRelaxedLineRasterizationFeaturesIMG";
    case StructureType::eSharedPresentSurfaceCapabilitiesKHR: return "SharedPresentSurfaceCapabilitiesKHR";
    case StructureType::ePhysicalDeviceExternalFenceInfo: return "PhysicalDeviceExternalFenceInfo";
    case StructureType::eExternalFenceProperties: return "ExternalFenceProperties";
    case StructureType::eExportFenceCreateInfo: return "ExportFenceCreateInfo";
    case StructureType::eImportFenceFdInfoKHR: return "ImportFenceFdInfoKHR";
    case StructureType::eFenceGetFdInfoKHR: return "FenceGetFdInfoKHR";
    case StructureType::ePhysicalDevicePerformanceQueryFeaturesKHR: return "PhysicalDevicePerformanceQueryFeaturesKHR";
    case StructureType::ePhysicalDevicePerformanceQueryPropertiesKHR: return "PhysicalDevicePerformanceQueryPropertiesKHR";
    case StructureType::eQueryPoolPerformanceCreateInfoKHR: return "QueryPoolPerformanceCreateInfoKHR";
    case StructureType::ePerformanceQuerySubmitInfoKHR: return "PerformanceQuerySubmitInfoKHR";
    case StructureType::eAcquireProfilingLockInfoKHR: return "AcquireProfilingLockInfoKHR";
    case StructureType::ePerformanceCounterKHR: return "PerformanceCounterKHR";
    case StructureType::ePerformanceCounterDescriptionKHR: return "PerformanceCounterDescriptionKHR";
    case StructureType::ePerformanceQueryReservationInfoKHR: return "PerformanceQueryReservationInfoKHR";
    case StructureType::ePhysicalDevicePointClippingProperties: return "PhysicalDevicePointClippingProperties";
    case StructureType::eRenderPassInputAttachmentAspectCreateInfo: return "RenderPassInputAttachmentAspectCreateInfo";
    case StructureType::eImageViewUsageCreateInfo: return "ImageViewUsageCreateInfo";
    case StructureType::ePipelineTessellationDomainOriginStateCreateInfo: return "PipelineTessellationDomainOriginStateCreateInfo";
    case StructureType::ePhysicalDeviceSurfaceInfo2KHR: return "PhysicalDeviceSurfaceInfo2KHR";
    case StructureType::eSurfaceCapabilities2KHR: return "SurfaceCapabilities2KHR";
    case StructureType::eSurfaceFormat2KHR: return "SurfaceFormat2KHR";
    case StructureType::ePhysicalDeviceVariablePointersFeatures: return "PhysicalDeviceVariablePointersFeatures";
    case StructureType::eDisplayProperties2KHR: return "DisplayProperties2KHR";
    case StructureType::eDisplayPlaneProperties2KHR: return "DisplayPlaneProperties2KHR";
    case StructureType::eDisplayModeProperties2KHR: return "DisplayModeProperties2KHR";
    case StructureType::eDisplayPlaneInfo2KHR: return "DisplayPlaneInfo2KHR";
    case StructureType::eDisplayPlaneCapabilities2KHR: return "DisplayPlaneCapabilities2KHR";
    case StructureType::eMemoryDedicatedRequirements: return "MemoryDedicatedRequirements";
    case StructureType::eMemoryDedicatedAllocateInfo: return "MemoryDedicatedAllocateInfo";
    case StructureType::eDebugUtilsObjectNameInfoEXT: return "DebugUtilsObjectNameInfoEXT";
    case StructureType::eDebugUtilsObjectTagInfoEXT: return "DebugUtilsObjectTagInfoEXT";
    case StructureType::eDebugUtilsLabelEXT: return "DebugUtilsLabelEXT";
    case StructureType::eDebugUtilsMessengerCallbackDataEXT: return "DebugUtilsMessengerCallbackDataEXT";
    case StructureType::eDebugUtilsMessengerCreateInfoEXT: return "DebugUtilsMessengerCreateInfoEXT";
    case StructureType::ePhysicalDeviceSamplerFilterMinmaxProperties: return "PhysicalDeviceSamplerFilterMinmaxProperties";
    case StructureType::eSamplerReductionModeCreateInfo: return "SamplerReductionModeCreateInfo";
    case StructureType::ePhysicalDeviceInlineUniformBlockFeatures: return "PhysicalDeviceInlineUniformBlockFeatures";
    case StructureType::ePhysicalDeviceInlineUniformBlockProperties: return "PhysicalDeviceInlineUniformBlockProperties";
    case StructureType::eWriteDescriptorSetInlineUniformBlock: return "WriteDescriptorSetInlineUniformBlock";
    case StructureType::eDescriptorPoolInlineUniformBlockCreateInfo: return "DescriptorPoolInlineUniformBlockCreateInfo";
    case StructureType::ePhysicalDeviceShaderBfloat16FeaturesKHR: return "PhysicalDeviceShaderBfloat16FeaturesKHR";
    case StructureType::eSampleLocationsInfoEXT: return "SampleLocationsInfoEXT";
    case StructureType::eRenderPassSampleLocationsBeginInfoEXT: return "RenderPassSampleLocationsBeginInfoEXT";
    case StructureType::ePipelineSampleLocationsStateCreateInfoEXT: return "PipelineSampleLocationsStateCreateInfoEXT";
    case StructureType::ePhysicalDeviceSampleLocationsPropertiesEXT: return "PhysicalDeviceSampleLocationsPropertiesEXT";
    case StructureType::eMultisamplePropertiesEXT: return "MultisamplePropertiesEXT";
    case StructureType::eProtectedSubmitInfo: return "ProtectedSubmitInfo";
    case StructureType::ePhysicalDeviceProtectedMemoryFeatures: return "PhysicalDeviceProtectedMemoryFeatures";
    case StructureType::ePhysicalDeviceProtectedMemoryProperties: return "PhysicalDeviceProtectedMemoryProperties";
    case StructureType::eDeviceQueueInfo2: return "DeviceQueueInfo2";
    case StructureType::eBufferMemoryRequirementsInfo2: return "BufferMemoryRequirementsInfo2";
    case StructureType::eImageMemoryRequirementsInfo2: return "ImageMemoryRequirementsInfo2";
    case StructureType::eImageSparseMemoryRequirementsInfo2: return "ImageSparseMemoryRequirementsInfo2";
    case StructureType::eMemoryRequirements2: return "MemoryRequirements2";
    case StructureType::eSparseImageMemoryRequirements2: return "SparseImageMemoryRequirements2";
    case StructureType::eImageFormatListCreateInfo: return "ImageFormatListCreateInfo";
    case StructureType::ePhysicalDeviceBlendOperationAdvancedFeaturesEXT: return "PhysicalDeviceBlendOperationAdvancedFeaturesEXT";
    case StructureType::ePhysicalDeviceBlendOperationAdvancedPropertiesEXT: return "PhysicalDeviceBlendOperationAdvancedPropertiesEXT";
    case StructureType::ePipelineColorBlendAdvancedStateCreateInfoEXT: return "PipelineColorBlendAdvancedStateCreateInfoEXT";
    case StructureType::ePipelineCoverageToColorStateCreateInfoNV: return "PipelineCoverageToColorStateCreateInfoNV";
    case StructureType::eWriteDescriptorSetAccelerationStructureKHR: return "WriteDescriptorSetAccelerationStructureKHR";
    case StructureType::eAccelerationStructureBuildGeometryInfoKHR: return "AccelerationStructureBuildGeometryInfoKHR";
    case StructureType::eAccelerationStructureDeviceAddressInfoKHR: return "AccelerationStructureDeviceAddressInfoKHR";
    case StructureType::eAccelerationStructureGeometryAabbsDataKHR: return "AccelerationStructureGeometryAabbsDataKHR";
    case StructureType::eAccelerationStructureGeometryInstancesDataKHR: return "AccelerationStructureGeometryInstancesDataKHR";
    case StructureType::eAccelerationStructureGeometryTrianglesDataKHR: return "AccelerationStructureGeometryTrianglesDataKHR";
    case StructureType::eAccelerationStructureGeometryKHR: return "AccelerationStructureGeometryKHR";
    case StructureType::eAccelerationStructureVersionInfoKHR: return "AccelerationStructureVersionInfoKHR";
    case StructureType::eCopyAccelerationStructureInfoKHR: return "CopyAccelerationStructureInfoKHR";
    case StructureType::eCopyAccelerationStructureToMemoryInfoKHR: return "CopyAccelerationStructureToMemoryInfoKHR";
    case StructureType::eCopyMemoryToAccelerationStructureInfoKHR: return "CopyMemoryToAccelerationStructureInfoKHR";
    case StructureType::ePhysicalDeviceAccelerationStructureFeaturesKHR: return "PhysicalDeviceAccelerationStructureFeaturesKHR";
    case StructureType::ePhysicalDeviceAccelerationStructurePropertiesKHR: return "PhysicalDeviceAccelerationStructurePropertiesKHR";
    case StructureType::eAccelerationStructureCreateInfoKHR: return "AccelerationStructureCreateInfoKHR";
    case StructureType::eAccelerationStructureBuildSizesInfoKHR: return "AccelerationStructureBuildSizesInfoKHR";
    case StructureType::eRayTracingPipelineCreateInfoKHR: return "RayTracingPipelineCreateInfoKHR";
    case StructureType::eRayTracingShaderGroupCreateInfoKHR: return "RayTracingShaderGroupCreateInfoKHR";
    case StructureType::eRayTracingPipelineInterfaceCreateInfoKHR: return "RayTracingPipelineInterfaceCreateInfoKHR";
    case StructureType::ePipelineCoverageModulationStateCreateInfoNV: return "PipelineCoverageModulationStateCreateInfoNV";
    case StructureType::ePhysicalDeviceShaderSmBuiltinsFeaturesNV: return "PhysicalDeviceShaderSmBuiltinsFeaturesNV";
    case StructureType::ePhysicalDeviceShaderSmBuiltinsPropertiesNV: return "PhysicalDeviceShaderSmBuiltinsPropertiesNV";
    case StructureType::eSamplerYcbcrConversionCreateInfo: return "SamplerYcbcrConversionCreateInfo";
    case StructureType::eSamplerYcbcrConversionInfo: return "SamplerYcbcrConversionInfo";
    case StructureType::eBindImagePlaneMemoryInfo: return "BindImagePlaneMemoryInfo";
    case StructureType::eImagePlaneMemoryRequirementsInfo: return "ImagePlaneMemoryRequirementsInfo";
    case StructureType::ePhysicalDeviceSamplerYcbcrConversionFeatures: return "PhysicalDeviceSamplerYcbcrConversionFeatures";
    case StructureType::eSamplerYcbcrConversionImageFormatProperties: return "SamplerYcbcrConversionImageFormatProperties";
    case StructureType::eBindBufferMemoryInfo: return "BindBufferMemoryInfo";
    case StructureType::eBindImageMemoryInfo: return "BindImageMemoryInfo";
    case StructureType::eDrmFormatModifierPropertiesListEXT: return "DrmFormatModifierPropertiesListEXT";
    case StructureType::ePhysicalDeviceImageDrmFormatModifierInfoEXT: return "PhysicalDeviceImageDrmFormatModifierInfoEXT";
    case StructureType::eImageDrmFormatModifierListCreateInfoEXT: return "ImageDrmFormatModifierListCreateInfoEXT";
    case StructureType::eImageDrmFormatModifierExplicitCreateInfoEXT: return "ImageDrmFormatModifierExplicitCreateInfoEXT";
    case StructureType::eImageDrmFormatModifierPropertiesEXT: return "ImageDrmFormatModifierPropertiesEXT";
    case StructureType::eDrmFormatModifierPropertiesList2EXT: return "DrmFormatModifierPropertiesList2EXT";
    case StructureType::eValidationCacheCreateInfoEXT: return "ValidationCacheCreateInfoEXT";
    case StructureType::eShaderModuleValidationCacheCreateInfoEXT: return "ShaderModuleValidationCacheCreateInfoEXT";
    case StructureType::eDescriptorSetLayoutBindingFlagsCreateInfo: return "DescriptorSetLayoutBindingFlagsCreateInfo";
    case StructureType::ePhysicalDeviceDescriptorIndexingFeatures: return "PhysicalDeviceDescriptorIndexingFeatures";
    case StructureType::ePhysicalDeviceDescriptorIndexingProperties: return "PhysicalDeviceDescriptorIndexingProperties";
    case StructureType::eDescriptorSetVariableDescriptorCountAllocateInfo: return "DescriptorSetVariableDescriptorCountAllocateInfo";
    case StructureType::eDescriptorSetVariableDescriptorCountLayoutSupport: return "DescriptorSetVariableDescriptorCountLayoutSupport";
    case StructureType::ePipelineViewportShadingRateImageStateCreateInfoNV: return "PipelineViewportShadingRateImageStateCreateInfoNV";
    case StructureType::ePhysicalDeviceShadingRateImageFeaturesNV: return "PhysicalDeviceShadingRateImageFeaturesNV";
    case StructureType::ePhysicalDeviceShadingRateImagePropertiesNV: return "PhysicalDeviceShadingRateImagePropertiesNV";
    case StructureType::ePipelineViewportCoarseSampleOrderStateCreateInfoNV: return "PipelineViewportCoarseSampleOrderStateCreateInfoNV";
    case StructureType::eRayTracingPipelineCreateInfoNV: return "RayTracingPipelineCreateInfoNV";
    case StructureType::eAccelerationStructureCreateInfoNV: return "AccelerationStructureCreateInfoNV";
    case StructureType::eGeometryNV: return "GeometryNV";
    case StructureType::eGeometryTrianglesNV: return "GeometryTrianglesNV";
    case StructureType::eGeometryAabbNV: return "GeometryAabbNV";
    case StructureType::eBindAccelerationStructureMemoryInfoNV: return "BindAccelerationStructureMemoryInfoNV";
    case StructureType::eWriteDescriptorSetAccelerationStructureNV: return "WriteDescriptorSetAccelerationStructureNV";
    case StructureType::eAccelerationStructureMemoryRequirementsInfoNV: return "AccelerationStructureMemoryRequirementsInfoNV";
    case StructureType::ePhysicalDeviceRayTracingPropertiesNV: return "PhysicalDeviceRayTracingPropertiesNV";
    case StructureType::eRayTracingShaderGroupCreateInfoNV: return "RayTracingShaderGroupCreateInfoNV";
    case StructureType::eAccelerationStructureInfoNV: return "AccelerationStructureInfoNV";
    case StructureType::ePhysicalDeviceRepresentativeFragmentTestFeaturesNV: return "PhysicalDeviceRepresentativeFragmentTestFeaturesNV";
    case StructureType::ePipelineRepresentativeFragmentTestStateCreateInfoNV: return "PipelineRepresentativeFragmentTestStateCreateInfoNV";
    case StructureType::ePhysicalDeviceMaintenance3Properties: return "PhysicalDeviceMaintenance3Properties";
    case StructureType::eDescriptorSetLayoutSupport: return "DescriptorSetLayoutSupport";
    case StructureType::ePhysicalDeviceImageViewImageFormatInfoEXT: return "PhysicalDeviceImageViewImageFormatInfoEXT";
    case StructureType::eFilterCubicImageViewImageFormatPropertiesEXT: return "FilterCubicImageViewImageFormatPropertiesEXT";
    case StructureType::eDeviceQueueGlobalPriorityCreateInfo: return "DeviceQueueGlobalPriorityCreateInfo";
    case StructureType::ePhysicalDeviceShaderSubgroupExtendedTypesFeatures: return "PhysicalDeviceShaderSubgroupExtendedTypesFeatures";
    case StructureType::ePhysicalDevice8BitStorageFeatures: return "PhysicalDevice8BitStorageFeatures";
    case StructureType::eImportMemoryHostPointerInfoEXT: return "ImportMemoryHostPointerInfoEXT";
    case StructureType::eMemoryHostPointerPropertiesEXT: return "MemoryHostPointerPropertiesEXT";
    case StructureType::ePhysicalDeviceExternalMemoryHostPropertiesEXT: return "PhysicalDeviceExternalMemoryHostPropertiesEXT";
    case StructureType::ePhysicalDeviceShaderAtomicInt64Features: return "PhysicalDeviceShaderAtomicInt64Features";
    case StructureType::ePhysicalDeviceShaderClockFeaturesKHR: return "PhysicalDeviceShaderClockFeaturesKHR";
    case StructureType::ePipelineCompilerControlCreateInfoAMD: return "PipelineCompilerControlCreateInfoAMD";
    case StructureType::eCalibratedTimestampInfoKHR: return "CalibratedTimestampInfoKHR";
    case StructureType::ePhysicalDeviceShaderCorePropertiesAMD: return "PhysicalDeviceShaderCorePropertiesAMD";
    case StructureType::eDeviceMemoryOverallocationCreateInfoAMD: return "DeviceMemoryOverallocationCreateInfoAMD";
    case StructureType::ePipelineVertexInputDivisorStateCreateInfo: return "PipelineVertexInputDivisorStateCreateInfo";
    case StructureType::ePhysicalDeviceVertexAttributeDivisorFeatures: return "PhysicalDeviceVertexAttributeDivisorFeatures";
    case StructureType::ePhysicalDeviceVertexAttributeDivisorPropertiesEXT: return "PhysicalDeviceVertexAttributeDivisorPropertiesEXT";
    case StructureType::ePipelineCreationFeedbackCreateInfo: return "PipelineCreationFeedbackCreateInfo";
    case StructureType::ePhysicalDeviceDriverProperties: return "PhysicalDeviceDriverProperties";
    case StructureType::ePhysicalDeviceFloatControlsProperties: return "PhysicalDeviceFloatControlsProperties";
    case StructureType::ePhysicalDeviceDepthStencilResolveProperties: return "PhysicalDeviceDepthStencilResolveProperties";
    case StructureType::eSubpassDescriptionDepthStencilResolve: return "SubpassDescriptionDepthStencilResolve";
    case StructureType::ePhysicalDeviceComputeShaderDerivativesFeaturesKHR: return "PhysicalDeviceComputeShaderDerivativesFeaturesKHR";
    case StructureType::ePhysicalDeviceMeshShaderFeaturesNV: return "PhysicalDeviceMeshShaderFeaturesNV";
    case StructureType::ePhysicalDeviceMeshShaderPropertiesNV: return "PhysicalDeviceMeshShaderPropertiesNV";
    case StructureType::ePhysicalDeviceFragmentShaderBarycentricFeaturesKHR: return "PhysicalDeviceFragmentShaderBarycentricFeaturesKHR";
    case StructureType::ePhysicalDeviceShaderImageFootprintFeaturesNV: return "PhysicalDeviceShaderImageFootprintFeaturesNV";
    case StructureType::ePipelineViewportExclusiveScissorStateCreateInfoNV: return "PipelineViewportExclusiveScissorStateCreateInfoNV";
    case StructureType::ePhysicalDeviceExclusiveScissorFeaturesNV: return "PhysicalDeviceExclusiveScissorFeaturesNV";
    case StructureType::eCheckpointDataNV: return "CheckpointDataNV";
    case StructureType::eQueueFamilyCheckpointPropertiesNV: return "QueueFamilyCheckpointPropertiesNV";
    case StructureType::ePhysicalDeviceTimelineSemaphoreFeatures: return "PhysicalDeviceTimelineSemaphoreFeatures";
    case StructureType::ePhysicalDeviceTimelineSemaphoreProperties: return "PhysicalDeviceTimelineSemaphoreProperties";
    case StructureType::eSemaphoreTypeCreateInfo: return "SemaphoreTypeCreateInfo";
    case StructureType::eTimelineSemaphoreSubmitInfo: return "TimelineSemaphoreSubmitInfo";
    case StructureType::eSemaphoreWaitInfo: return "SemaphoreWaitInfo";
    case StructureType::eSemaphoreSignalInfo: return "SemaphoreSignalInfo";
    case StructureType::ePhysicalDeviceShaderIntegerFunctions2FeaturesINTEL: return "PhysicalDeviceShaderIntegerFunctions2FeaturesINTEL";
    case StructureType::eQueryPoolPerformanceQueryCreateInfoINTEL: return "QueryPoolPerformanceQueryCreateInfoINTEL";
    case StructureType::eInitializePerformanceApiInfoINTEL: return "InitializePerformanceApiInfoINTEL";
    case StructureType::ePerformanceMarkerInfoINTEL: return "PerformanceMarkerInfoINTEL";
    case StructureType::ePerformanceStreamMarkerInfoINTEL: return "PerformanceStreamMarkerInfoINTEL";
    case StructureType::ePerformanceOverrideInfoINTEL: return "PerformanceOverrideInfoINTEL";
    case StructureType::ePerformanceConfigurationAcquireInfoINTEL: return "PerformanceConfigurationAcquireInfoINTEL";
    case StructureType::ePhysicalDeviceVulkanMemoryModelFeatures: return "PhysicalDeviceVulkanMemoryModelFeatures";
    case StructureType::ePhysicalDevicePciBusInfoPropertiesEXT: return "PhysicalDevicePciBusInfoPropertiesEXT";
    case StructureType::eDisplayNativeHdrSurfaceCapabilitiesAMD: return "DisplayNativeHdrSurfaceCapabilitiesAMD";
    case StructureType::eSwapchainDisplayNativeHdrCreateInfoAMD: return "SwapchainDisplayNativeHdrCreateInfoAMD";
    case StructureType::ePhysicalDeviceShaderTerminateInvocationFeatures: return "PhysicalDeviceShaderTerminateInvocationFeatures";
    case StructureType::ePhysicalDeviceFragmentDensityMapFeaturesEXT: return "PhysicalDeviceFragmentDensityMapFeaturesEXT";
    case StructureType::ePhysicalDeviceFragmentDensityMapPropertiesEXT: return "PhysicalDeviceFragmentDensityMapPropertiesEXT";
    case StructureType::eRenderPassFragmentDensityMapCreateInfoEXT: return "RenderPassFragmentDensityMapCreateInfoEXT";
    case StructureType::ePhysicalDeviceScalarBlockLayoutFeatures: return "PhysicalDeviceScalarBlockLayoutFeatures";
    case StructureType::ePhysicalDeviceSubgroupSizeControlProperties: return "PhysicalDeviceSubgroupSizeControlProperties";
    case StructureType::ePipelineShaderStageRequiredSubgroupSizeCreateInfo: return "PipelineShaderStageRequiredSubgroupSizeCreateInfo";
    case StructureType::ePhysicalDeviceSubgroupSizeControlFeatures: return "PhysicalDeviceSubgroupSizeControlFeatures";
    case StructureType::eFragmentShadingRateAttachmentInfoKHR: return "FragmentShadingRateAttachmentInfoKHR";
    case StructureType::ePipelineFragmentShadingRateStateCreateInfoKHR: return "PipelineFragmentShadingRateStateCreateInfoKHR";
    case StructureType::ePhysicalDeviceFragmentShadingRatePropertiesKHR: return "PhysicalDeviceFragmentShadingRatePropertiesKHR";
    case StructureType::ePhysicalDeviceFragmentShadingRateFeaturesKHR: return "PhysicalDeviceFragmentShadingRateFeaturesKHR";
    case StructureType::ePhysicalDeviceFragmentShadingRateKHR: return "PhysicalDeviceFragmentShadingRateKHR";
    case StructureType::ePhysicalDeviceShaderCoreProperties2AMD: return "PhysicalDeviceShaderCoreProperties2AMD";
    case StructureType::ePhysicalDeviceCoherentMemoryFeaturesAMD: return "PhysicalDeviceCoherentMemoryFeaturesAMD";
    case StructureType::ePhysicalDeviceDynamicRenderingLocalReadFeatures: return "PhysicalDeviceDynamicRenderingLocalReadFeatures";
    case StructureType::eRenderingAttachmentLocationInfo: return "RenderingAttachmentLocationInfo";
    case StructureType::eRenderingInputAttachmentIndexInfo: return "RenderingInputAttachmentIndexInfo";
    case StructureType::ePhysicalDeviceShaderImageAtomicInt64FeaturesEXT: return "PhysicalDeviceShaderImageAtomicInt64FeaturesEXT";
    case StructureType::ePhysicalDeviceShaderQuadControlFeaturesKHR: return "PhysicalDeviceShaderQuadControlFeaturesKHR";
    case StructureType::ePhysicalDeviceMemoryBudgetPropertiesEXT: return "PhysicalDeviceMemoryBudgetPropertiesEXT";
    case StructureType::ePhysicalDeviceMemoryPriorityFeaturesEXT: return "PhysicalDeviceMemoryPriorityFeaturesEXT";
    case StructureType::eMemoryPriorityAllocateInfoEXT: return "MemoryPriorityAllocateInfoEXT";
    case StructureType::eSurfaceProtectedCapabilitiesKHR: return "SurfaceProtectedCapabilitiesKHR";
    case StructureType::ePhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV: return "PhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV";
    case StructureType::ePhysicalDeviceSeparateDepthStencilLayoutsFeatures: return "PhysicalDeviceSeparateDepthStencilLayoutsFeatures";
    case StructureType::eAttachmentReferenceStencilLayout: return "AttachmentReferenceStencilLayout";
    case StructureType::eAttachmentDescriptionStencilLayout: return "AttachmentDescriptionStencilLayout";
    case StructureType::eBufferDeviceAddressInfo: return "BufferDeviceAddressInfo";
    case StructureType::ePhysicalDeviceBufferDeviceAddressFeaturesEXT: return "PhysicalDeviceBufferDeviceAddressFeaturesEXT";
    case StructureType::eBufferDeviceAddressCreateInfoEXT: return "BufferDeviceAddressCreateInfoEXT";
    case StructureType::ePhysicalDeviceToolProperties: return "PhysicalDeviceToolProperties";
    case StructureType::eImageStencilUsageCreateInfo: return "ImageStencilUsageCreateInfo";
    case StructureType::eValidationFeaturesEXT: return "ValidationFeaturesEXT";
    case StructureType::ePhysicalDevicePresentWaitFeaturesKHR: return "PhysicalDevicePresentWaitFeaturesKHR";
    case StructureType::ePhysicalDeviceCooperativeMatrixFeaturesNV: return "PhysicalDeviceCooperativeMatrixFeaturesNV";
    case StructureType::eCooperativeMatrixPropertiesNV: return "CooperativeMatrixPropertiesNV";
    case StructureType::ePhysicalDeviceCooperativeMatrixPropertiesNV: return "PhysicalDeviceCooperativeMatrixPropertiesNV";
    case StructureType::ePhysicalDeviceCoverageReductionModeFeaturesNV: return "PhysicalDeviceCoverageReductionModeFeaturesNV";
    case StructureType::ePipelineCoverageReductionStateCreateInfoNV: return "PipelineCoverageReductionStateCreateInfoNV";
    case StructureType::eFramebufferMixedSamplesCombinationNV: return "FramebufferMixedSamplesCombinationNV";
    case StructureType::ePhysicalDeviceFragmentShaderInterlockFeaturesEXT: return "PhysicalDeviceFragmentShaderInterlockFeaturesEXT";
    case StructureType::ePhysicalDeviceYcbcrImageArraysFeaturesEXT: return "PhysicalDeviceYcbcrImageArraysFeaturesEXT";
    case StructureType::ePhysicalDeviceUniformBufferStandardLayoutFeatures: return "PhysicalDeviceUniformBufferStandardLayoutFeatures";
    case StructureType::ePhysicalDeviceProvokingVertexFeaturesEXT: return "PhysicalDeviceProvokingVertexFeaturesEXT";
    case StructureType::ePipelineRasterizationProvokingVertexStateCreateInfoEXT: return "PipelineRasterizationProvokingVertexStateCreateInfoEXT";
    case StructureType::ePhysicalDeviceProvokingVertexPropertiesEXT: return "PhysicalDeviceProvokingVertexPropertiesEXT";
    case StructureType::eHeadlessSurfaceCreateInfoEXT: return "HeadlessSurfaceCreateInfoEXT";
    case StructureType::ePhysicalDeviceBufferDeviceAddressFeatures: return "PhysicalDeviceBufferDeviceAddressFeatures";
    case StructureType::eBufferOpaqueCaptureAddressCreateInfo: return "BufferOpaqueCaptureAddressCreateInfo";
    case StructureType::eMemoryOpaqueCaptureAddressAllocateInfo: return "MemoryOpaqueCaptureAddressAllocateInfo";
    case StructureType::eDeviceMemoryOpaqueCaptureAddressInfo: return "DeviceMemoryOpaqueCaptureAddressInfo";
    case StructureType::ePhysicalDeviceLineRasterizationFeatures: return "PhysicalDeviceLineRasterizationFeatures";
    case StructureType::ePipelineRasterizationLineStateCreateInfo: return "PipelineRasterizationLineStateCreateInfo";
    case StructureType::ePhysicalDeviceLineRasterizationProperties: return "PhysicalDeviceLineRasterizationProperties";
    case StructureType::ePhysicalDeviceShaderAtomicFloatFeaturesEXT: return "PhysicalDeviceShaderAtomicFloatFeaturesEXT";
    case StructureType::ePhysicalDeviceHostQueryResetFeatures: return "PhysicalDeviceHostQueryResetFeatures";
    case StructureType::ePhysicalDeviceIndexTypeUint8Features: return "PhysicalDeviceIndexTypeUint8Features";
    case StructureType::ePhysicalDeviceExtendedDynamicStateFeaturesEXT: return "PhysicalDeviceExtendedDynamicStateFeaturesEXT";
    case StructureType::ePhysicalDevicePipelineExecutablePropertiesFeaturesKHR: return "PhysicalDevicePipelineExecutablePropertiesFeaturesKHR";
    case StructureType::ePipelineInfoKHR: return "PipelineInfoKHR";
    case StructureType::ePipelineExecutablePropertiesKHR: return "PipelineExecutablePropertiesKHR";
    case StructureType::ePipelineExecutableInfoKHR: return "PipelineExecutableInfoKHR";
    case StructureType::ePipelineExecutableStatisticKHR: return "PipelineExecutableStatisticKHR";
    case StructureType::ePipelineExecutableInternalRepresentationKHR: return "PipelineExecutableInternalRepresentationKHR";
    case StructureType::ePhysicalDeviceHostImageCopyFeatures: return "PhysicalDeviceHostImageCopyFeatures";
    case StructureType::ePhysicalDeviceHostImageCopyProperties: return "PhysicalDeviceHostImageCopyProperties";
    case StructureType::eMemoryToImageCopy: return "MemoryToImageCopy";
    case StructureType::eImageToMemoryCopy: return "ImageToMemoryCopy";
    case StructureType::eCopyImageToMemoryInfo: return "CopyImageToMemoryInfo";
    case StructureType::eCopyMemoryToImageInfo: return "CopyMemoryToImageInfo";
    case StructureType::eHostImageLayoutTransitionInfo: return "HostImageLayoutTransitionInfo";
    case StructureType::eCopyImageToImageInfo: return "CopyImageToImageInfo";
    case StructureType::eSubresourceHostMemcpySize: return "SubresourceHostMemcpySize";
    case StructureType::eHostImageCopyDevicePerformanceQuery: return "HostImageCopyDevicePerformanceQuery";
    case StructureType::eMemoryMapInfo: return "MemoryMapInfo";
    case StructureType::eMemoryUnmapInfo: return "MemoryUnmapInfo";
    case StructureType::ePhysicalDeviceMapMemoryPlacedFeaturesEXT: return "PhysicalDeviceMapMemoryPlacedFeaturesEXT";
    case StructureType::ePhysicalDeviceMapMemoryPlacedPropertiesEXT: return "PhysicalDeviceMapMemoryPlacedPropertiesEXT";
    case StructureType::eMemoryMapPlacedInfoEXT: return "MemoryMapPlacedInfoEXT";
    case StructureType::ePhysicalDeviceShaderAtomicFloat2FeaturesEXT: return "PhysicalDeviceShaderAtomicFloat2FeaturesEXT";
    case StructureType::eSurfacePresentModeKHR: return "SurfacePresentModeKHR";
    case StructureType::eSurfacePresentScalingCapabilitiesKHR: return "SurfacePresentScalingCapabilitiesKHR";
    case StructureType::eSurfacePresentModeCompatibilityKHR: return "SurfacePresentModeCompatibilityKHR";
    case StructureType::ePhysicalDeviceSwapchainMaintenance1FeaturesKHR: return "PhysicalDeviceSwapchainMaintenance1FeaturesKHR";
    case StructureType::eSwapchainPresentFenceInfoKHR: return "SwapchainPresentFenceInfoKHR";
    case StructureType::eSwapchainPresentModesCreateInfoKHR: return "SwapchainPresentModesCreateInfoKHR";
    case StructureType::eSwapchainPresentModeInfoKHR: return "SwapchainPresentModeInfoKHR";
    case StructureType::eSwapchainPresentScalingCreateInfoKHR: return "SwapchainPresentScalingCreateInfoKHR";
    case StructureType::eReleaseSwapchainImagesInfoKHR: return "ReleaseSwapchainImagesInfoKHR";
    case StructureType::ePhysicalDeviceShaderDemoteToHelperInvocationFeatures: return "PhysicalDeviceShaderDemoteToHelperInvocationFeatures";
    case StructureType::ePhysicalDeviceDeviceGeneratedCommandsPropertiesNV: return "PhysicalDeviceDeviceGeneratedCommandsPropertiesNV";
    case StructureType::eGraphicsShaderGroupCreateInfoNV: return "GraphicsShaderGroupCreateInfoNV";
    case StructureType::eGraphicsPipelineShaderGroupsCreateInfoNV: return "GraphicsPipelineShaderGroupsCreateInfoNV";
    case StructureType::eIndirectCommandsLayoutTokenNV: return "IndirectCommandsLayoutTokenNV";
    case StructureType::eIndirectCommandsLayoutCreateInfoNV: return "IndirectCommandsLayoutCreateInfoNV";
    case StructureType::eGeneratedCommandsInfoNV: return "GeneratedCommandsInfoNV";
    case StructureType::eGeneratedCommandsMemoryRequirementsInfoNV: return "GeneratedCommandsMemoryRequirementsInfoNV";
    case StructureType::ePhysicalDeviceDeviceGeneratedCommandsFeaturesNV: return "PhysicalDeviceDeviceGeneratedCommandsFeaturesNV";
    case StructureType::ePhysicalDeviceInheritedViewportScissorFeaturesNV: return "PhysicalDeviceInheritedViewportScissorFeaturesNV";
    case StructureType::eCommandBufferInheritanceViewportScissorInfoNV: return "CommandBufferInheritanceViewportScissorInfoNV";
    case StructureType::ePhysicalDeviceShaderIntegerDotProductFeatures: return "PhysicalDeviceShaderIntegerDotProductFeatures";
    case StructureType::ePhysicalDeviceShaderIntegerDotProductProperties: return "PhysicalDeviceShaderIntegerDotProductProperties";
    case StructureType::ePhysicalDeviceTexelBufferAlignmentProperties: return "PhysicalDeviceTexelBufferAlignmentProperties";
    case StructureType::ePhysicalDeviceTexelBufferAlignmentFeaturesEXT: return "PhysicalDeviceTexelBufferAlignmentFeaturesEXT";
    case StructureType::eCommandBufferInheritanceRenderPassTransformInfoQCOM: return "CommandBufferInheritanceRenderPassTransformInfoQCOM";
    case StructureType::eRenderPassTransformBeginInfoQCOM: return "RenderPassTransformBeginInfoQCOM";
    case StructureType::ePhysicalDeviceDepthBiasControlFeaturesEXT: return "PhysicalDeviceDepthBiasControlFeaturesEXT";
    case StructureType::eDepthBiasInfoEXT: return "DepthBiasInfoEXT";
    case StructureType::eDepthBiasRepresentationInfoEXT: return "DepthBiasRepresentationInfoEXT";
    case StructureType::ePhysicalDeviceDeviceMemoryReportFeaturesEXT: return "PhysicalDeviceDeviceMemoryReportFeaturesEXT";
    case StructureType::eDeviceDeviceMemoryReportCreateInfoEXT: return "DeviceDeviceMemoryReportCreateInfoEXT";
    case StructureType::eDeviceMemoryReportCallbackDataEXT: return "DeviceMemoryReportCallbackDataEXT";
    case StructureType::ePhysicalDeviceRobustness2FeaturesKHR: return "PhysicalDeviceRobustness2FeaturesKHR";
    case StructureType::ePhysicalDeviceRobustness2PropertiesKHR: return "PhysicalDeviceRobustness2PropertiesKHR";
    case StructureType::eSamplerCustomBorderColorCreateInfoEXT: return "SamplerCustomBorderColorCreateInfoEXT";
    case StructureType::ePhysicalDeviceCustomBorderColorPropertiesEXT: return "PhysicalDeviceCustomBorderColorPropertiesEXT";
    case StructureType::ePhysicalDeviceCustomBorderColorFeaturesEXT: return "PhysicalDeviceCustomBorderColorFeaturesEXT";
    case StructureType::ePipelineLibraryCreateInfoKHR: return "PipelineLibraryCreateInfoKHR";
    case StructureType::ePhysicalDevicePresentBarrierFeaturesNV: return "PhysicalDevicePresentBarrierFeaturesNV";
    case StructureType::eSurfaceCapabilitiesPresentBarrierNV: return "SurfaceCapabilitiesPresentBarrierNV";
    case StructureType::eSwapchainPresentBarrierCreateInfoNV: return "SwapchainPresentBarrierCreateInfoNV";
    case StructureType::ePresentIdKHR: return "PresentIdKHR";
    case StructureType::ePhysicalDevicePresentIdFeaturesKHR: return "PhysicalDevicePresentIdFeaturesKHR";
    case StructureType::ePhysicalDevicePrivateDataFeatures: return "PhysicalDevicePrivateDataFeatures";
    case StructureType::eDevicePrivateDataCreateInfo: return "DevicePrivateDataCreateInfo";
    case StructureType::ePrivateDataSlotCreateInfo: return "PrivateDataSlotCreateInfo";
    case StructureType::ePhysicalDevicePipelineCreationCacheControlFeatures: return "PhysicalDevicePipelineCreationCacheControlFeatures";
    case StructureType::ePhysicalDeviceDiagnosticsConfigFeaturesNV: return "PhysicalDeviceDiagnosticsConfigFeaturesNV";
    case StructureType::eDeviceDiagnosticsConfigCreateInfoNV: return "DeviceDiagnosticsConfigCreateInfoNV";
    case StructureType::ePhysicalDeviceTileShadingFeaturesQCOM: return "PhysicalDeviceTileShadingFeaturesQCOM";
    case StructureType::ePhysicalDeviceTileShadingPropertiesQCOM: return "PhysicalDeviceTileShadingPropertiesQCOM";
    case StructureType::eRenderPassTileShadingCreateInfoQCOM: return "RenderPassTileShadingCreateInfoQCOM";
    case StructureType::ePerTileBeginInfoQCOM: return "PerTileBeginInfoQCOM";
    case StructureType::ePerTileEndInfoQCOM: return "PerTileEndInfoQCOM";
    case StructureType::eDispatchTileInfoQCOM: return "DispatchTileInfoQCOM";
    case StructureType::eQueryLowLatencySupportNV: return "QueryLowLatencySupportNV";
    case StructureType::eMemoryBarrier2: return "MemoryBarrier2";
    case StructureType::eBufferMemoryBarrier2: return "BufferMemoryBarrier2";
    case StructureType::eImageMemoryBarrier2: return "ImageMemoryBarrier2";
    case StructureType::eDependencyInfo: return "DependencyInfo";
    case StructureType::eSubmitInfo2: return "SubmitInfo2";
    case StructureType::eSemaphoreSubmitInfo: return "SemaphoreSubmitInfo";
    case StructureType::eCommandBufferSubmitInfo: return "CommandBufferSubmitInfo";
    case StructureType::ePhysicalDeviceSynchronization2Features: return "PhysicalDeviceSynchronization2Features";
    case StructureType::eQueueFamilyCheckpointProperties2NV: return "QueueFamilyCheckpointProperties2NV";
    case StructureType::eCheckpointData2NV: return "CheckpointData2NV";
    case StructureType::ePhysicalDeviceDescriptorBufferPropertiesEXT: return "PhysicalDeviceDescriptorBufferPropertiesEXT";
    case StructureType::ePhysicalDeviceDescriptorBufferDensityMapPropertiesEXT: return "PhysicalDeviceDescriptorBufferDensityMapPropertiesEXT";
    case StructureType::ePhysicalDeviceDescriptorBufferFeaturesEXT: return "PhysicalDeviceDescriptorBufferFeaturesEXT";
    case StructureType::eDescriptorAddressInfoEXT: return "DescriptorAddressInfoEXT";
    case StructureType::eDescriptorGetInfoEXT: return "DescriptorGetInfoEXT";
    case StructureType::eBufferCaptureDescriptorDataInfoEXT: return "BufferCaptureDescriptorDataInfoEXT";
    case StructureType::eImageCaptureDescriptorDataInfoEXT: return "ImageCaptureDescriptorDataInfoEXT";
    case StructureType::eImageViewCaptureDescriptorDataInfoEXT: return "ImageViewCaptureDescriptorDataInfoEXT";
    case StructureType::eSamplerCaptureDescriptorDataInfoEXT: return "SamplerCaptureDescriptorDataInfoEXT";
    case StructureType::eOpaqueCaptureDescriptorDataCreateInfoEXT: return "OpaqueCaptureDescriptorDataCreateInfoEXT";
    case StructureType::eDescriptorBufferBindingInfoEXT: return "DescriptorBufferBindingInfoEXT";
    case StructureType::eDescriptorBufferBindingPushDescriptorBufferHandleEXT: return "DescriptorBufferBindingPushDescriptorBufferHandleEXT";
    case StructureType::eAccelerationStructureCaptureDescriptorDataInfoEXT: return "AccelerationStructureCaptureDescriptorDataInfoEXT";
    case StructureType::ePhysicalDeviceGraphicsPipelineLibraryFeaturesEXT: return "PhysicalDeviceGraphicsPipelineLibraryFeaturesEXT";
    case StructureType::ePhysicalDeviceGraphicsPipelineLibraryPropertiesEXT: return "PhysicalDeviceGraphicsPipelineLibraryPropertiesEXT";
    case StructureType::eGraphicsPipelineLibraryCreateInfoEXT: return "GraphicsPipelineLibraryCreateInfoEXT";
    case StructureType::ePhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD: return "PhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD";
    case StructureType::ePhysicalDeviceFragmentShaderBarycentricPropertiesKHR: return "PhysicalDeviceFragmentShaderBarycentricPropertiesKHR";
    case StructureType::ePhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR: return "PhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR";
    case StructureType::ePhysicalDeviceZeroInitializeWorkgroupMemoryFeatures: return "PhysicalDeviceZeroInitializeWorkgroupMemoryFeatures";
    case StructureType::ePhysicalDeviceFragmentShadingRateEnumsPropertiesNV: return "PhysicalDeviceFragmentShadingRateEnumsPropertiesNV";
    case StructureType::ePhysicalDeviceFragmentShadingRateEnumsFeaturesNV: return "PhysicalDeviceFragmentShadingRateEnumsFeaturesNV";
    case StructureType::ePipelineFragmentShadingRateEnumStateCreateInfoNV: return "PipelineFragmentShadingRateEnumStateCreateInfoNV";
    case StructureType::eAccelerationStructureGeometryMotionTrianglesDataNV: return "AccelerationStructureGeometryMotionTrianglesDataNV";
    case StructureType::ePhysicalDeviceRayTracingMotionBlurFeaturesNV: return "PhysicalDeviceRayTracingMotionBlurFeaturesNV";
    case StructureType::eAccelerationStructureMotionInfoNV: return "AccelerationStructureMotionInfoNV";
    case StructureType::ePhysicalDeviceMeshShaderFeaturesEXT: return "PhysicalDeviceMeshShaderFeaturesEXT";
    case StructureType::ePhysicalDeviceMeshShaderPropertiesEXT: return "PhysicalDeviceMeshShaderPropertiesEXT";
    case StructureType::ePhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT: return "PhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT";
    case StructureType::ePhysicalDeviceFragmentDensityMap2FeaturesEXT: return "PhysicalDeviceFragmentDensityMap2FeaturesEXT";
    case StructureType::ePhysicalDeviceFragmentDensityMap2PropertiesEXT: return "PhysicalDeviceFragmentDensityMap2PropertiesEXT";
    case StructureType::eCopyCommandTransformInfoQCOM: return "CopyCommandTransformInfoQCOM";
    case StructureType::ePhysicalDeviceImageRobustnessFeatures: return "PhysicalDeviceImageRobustnessFeatures";
    case StructureType::ePhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR: return "PhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR";
    case StructureType::eCopyBufferInfo2: return "CopyBufferInfo2";
    case StructureType::eCopyImageInfo2: return "CopyImageInfo2";
    case StructureType::eCopyBufferToImageInfo2: return "CopyBufferToImageInfo2";
    case StructureType::eCopyImageToBufferInfo2: return "CopyImageToBufferInfo2";
    case StructureType::eBlitImageInfo2: return "BlitImageInfo2";
    case StructureType::eResolveImageInfo2: return "ResolveImageInfo2";
    case StructureType::eBufferCopy2: return "BufferCopy2";
    case StructureType::eImageCopy2: return "ImageCopy2";
    case StructureType::eImageBlit2: return "ImageBlit2";
    case StructureType::eBufferImageCopy2: return "BufferImageCopy2";
    case StructureType::eImageResolve2: return "ImageResolve2";
    case StructureType::eSubresourceLayout2: return "SubresourceLayout2";
    case StructureType::eImageSubresource2: return "ImageSubresource2";
    case StructureType::ePhysicalDeviceImageCompressionControlFeaturesEXT: return "PhysicalDeviceImageCompressionControlFeaturesEXT";
    case StructureType::eImageCompressionControlEXT: return "ImageCompressionControlEXT";
    case StructureType::eImageCompressionPropertiesEXT: return "ImageCompressionPropertiesEXT";
    case StructureType::ePhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT: return "PhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT";
    case StructureType::ePhysicalDevice4444FormatsFeaturesEXT: return "PhysicalDevice4444FormatsFeaturesEXT";
    case StructureType::ePhysicalDeviceFaultFeaturesEXT: return "PhysicalDeviceFaultFeaturesEXT";
    case StructureType::eDeviceFaultCountsEXT: return "DeviceFaultCountsEXT";
    case StructureType::eDeviceFaultInfoEXT: return "DeviceFaultInfoEXT";
    case StructureType::ePhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT: return "PhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT";
    case StructureType::ePhysicalDeviceRgba10X6FormatsFeaturesEXT: return "PhysicalDeviceRgba10X6FormatsFeaturesEXT";
    case StructureType::ePhysicalDeviceRayTracingPipelineFeaturesKHR: return "PhysicalDeviceRayTracingPipelineFeaturesKHR";
    case StructureType::ePhysicalDeviceRayTracingPipelinePropertiesKHR: return "PhysicalDeviceRayTracingPipelinePropertiesKHR";
    case StructureType::ePhysicalDeviceRayQueryFeaturesKHR: return "PhysicalDeviceRayQueryFeaturesKHR";
    case StructureType::ePhysicalDeviceMutableDescriptorTypeFeaturesEXT: return "PhysicalDeviceMutableDescriptorTypeFeaturesEXT";
    case StructureType::eMutableDescriptorTypeCreateInfoEXT: return "MutableDescriptorTypeCreateInfoEXT";
    case StructureType::ePhysicalDeviceVertexInputDynamicStateFeaturesEXT: return "PhysicalDeviceVertexInputDynamicStateFeaturesEXT";
    case StructureType::eVertexInputBindingDescription2EXT: return "VertexInputBindingDescription2EXT";
    case StructureType::eVertexInputAttributeDescription2EXT: return "VertexInputAttributeDescription2EXT";
    case StructureType::ePhysicalDeviceDrmPropertiesEXT: return "PhysicalDeviceDrmPropertiesEXT";
    case StructureType::ePhysicalDeviceAddressBindingReportFeaturesEXT: return "PhysicalDeviceAddressBindingReportFeaturesEXT";
    case StructureType::eDeviceAddressBindingCallbackDataEXT: return "DeviceAddressBindingCallbackDataEXT";
    case StructureType::ePhysicalDeviceDepthClipControlFeaturesEXT: return "PhysicalDeviceDepthClipControlFeaturesEXT";
    case StructureType::ePipelineViewportDepthClipControlCreateInfoEXT: return "PipelineViewportDepthClipControlCreateInfoEXT";
    case StructureType::ePhysicalDevicePrimitiveTopologyListRestartFeaturesEXT: return "PhysicalDevicePrimitiveTopologyListRestartFeaturesEXT";
    case StructureType::eFormatProperties3: return "FormatProperties3";
    case StructureType::ePhysicalDevicePresentModeFifoLatestReadyFeaturesKHR: return "PhysicalDevicePresentModeFifoLatestReadyFeaturesKHR";
    case StructureType::eSubpassShadingPipelineCreateInfoHUAWEI: return "SubpassShadingPipelineCreateInfoHUAWEI";
    case StructureType::ePhysicalDeviceSubpassShadingFeaturesHUAWEI: return "PhysicalDeviceSubpassShadingFeaturesHUAWEI";
    case StructureType::ePhysicalDeviceSubpassShadingPropertiesHUAWEI: return "PhysicalDeviceSubpassShadingPropertiesHUAWEI";
    case StructureType::ePhysicalDeviceInvocationMaskFeaturesHUAWEI: return "PhysicalDeviceInvocationMaskFeaturesHUAWEI";
    case StructureType::eMemoryGetRemoteAddressInfoNV: return "MemoryGetRemoteAddressInfoNV";
    case StructureType::ePhysicalDeviceExternalMemoryRdmaFeaturesNV: return "PhysicalDeviceExternalMemoryRdmaFeaturesNV";
    case StructureType::ePipelinePropertiesIdentifierEXT: return "PipelinePropertiesIdentifierEXT";
    case StructureType::ePhysicalDevicePipelinePropertiesFeaturesEXT: return "PhysicalDevicePipelinePropertiesFeaturesEXT";
    case StructureType::ePhysicalDeviceFrameBoundaryFeaturesEXT: return "PhysicalDeviceFrameBoundaryFeaturesEXT";
    case StructureType::eFrameBoundaryEXT: return "FrameBoundaryEXT";
    case StructureType::ePhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT: return "PhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT";
    case StructureType::eSubpassResolvePerformanceQueryEXT: return "SubpassResolvePerformanceQueryEXT";
    case StructureType::eMultisampledRenderToSingleSampledInfoEXT: return "MultisampledRenderToSingleSampledInfoEXT";
    case StructureType::ePhysicalDeviceExtendedDynamicState2FeaturesEXT: return "PhysicalDeviceExtendedDynamicState2FeaturesEXT";
    case StructureType::ePhysicalDeviceColorWriteEnableFeaturesEXT: return "PhysicalDeviceColorWriteEnableFeaturesEXT";
    case StructureType::ePipelineColorWriteCreateInfoEXT: return "PipelineColorWriteCreateInfoEXT";
    case StructureType::ePhysicalDevicePrimitivesGeneratedQueryFeaturesEXT: return "PhysicalDevicePrimitivesGeneratedQueryFeaturesEXT";
    case StructureType::ePhysicalDeviceRayTracingMaintenance1FeaturesKHR: return "PhysicalDeviceRayTracingMaintenance1FeaturesKHR";
    case StructureType::ePhysicalDeviceGlobalPriorityQueryFeatures: return "PhysicalDeviceGlobalPriorityQueryFeatures";
    case StructureType::eQueueFamilyGlobalPriorityProperties: return "QueueFamilyGlobalPriorityProperties";
    case StructureType::ePhysicalDeviceImageViewMinLodFeaturesEXT: return "PhysicalDeviceImageViewMinLodFeaturesEXT";
    case StructureType::eImageViewMinLodCreateInfoEXT: return "ImageViewMinLodCreateInfoEXT";
    case StructureType::ePhysicalDeviceMultiDrawFeaturesEXT: return "PhysicalDeviceMultiDrawFeaturesEXT";
    case StructureType::ePhysicalDeviceMultiDrawPropertiesEXT: return "PhysicalDeviceMultiDrawPropertiesEXT";
    case StructureType::ePhysicalDeviceImage2DViewOf3DFeaturesEXT: return "PhysicalDeviceImage2DViewOf3DFeaturesEXT";
    case StructureType::ePhysicalDeviceShaderTileImageFeaturesEXT: return "PhysicalDeviceShaderTileImageFeaturesEXT";
    case StructureType::ePhysicalDeviceShaderTileImagePropertiesEXT: return "PhysicalDeviceShaderTileImagePropertiesEXT";
    case StructureType::eMicromapBuildInfoEXT: return "MicromapBuildInfoEXT";
    case StructureType::eMicromapVersionInfoEXT: return "MicromapVersionInfoEXT";
    case StructureType::eCopyMicromapInfoEXT: return "CopyMicromapInfoEXT";
    case StructureType::eCopyMicromapToMemoryInfoEXT: return "CopyMicromapToMemoryInfoEXT";
    case StructureType::eCopyMemoryToMicromapInfoEXT: return "CopyMemoryToMicromapInfoEXT";
    case StructureType::ePhysicalDeviceOpacityMicromapFeaturesEXT: return "PhysicalDeviceOpacityMicromapFeaturesEXT";
    case StructureType::ePhysicalDeviceOpacityMicromapPropertiesEXT: return "PhysicalDeviceOpacityMicromapPropertiesEXT";
    case StructureType::eMicromapCreateInfoEXT: return "MicromapCreateInfoEXT";
    case StructureType::eMicromapBuildSizesInfoEXT: return "MicromapBuildSizesInfoEXT";
    case StructureType::eAccelerationStructureTrianglesOpacityMicromapEXT: return "AccelerationStructureTrianglesOpacityMicromapEXT";
    case StructureType::ePhysicalDeviceClusterCullingShaderFeaturesHUAWEI: return "PhysicalDeviceClusterCullingShaderFeaturesHUAWEI";
    case StructureType::ePhysicalDeviceClusterCullingShaderPropertiesHUAWEI: return "PhysicalDeviceClusterCullingShaderPropertiesHUAWEI";
    case StructureType::ePhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI: return "PhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI";
    case StructureType::ePhysicalDeviceBorderColorSwizzleFeaturesEXT: return "PhysicalDeviceBorderColorSwizzleFeaturesEXT";
    case StructureType::eSamplerBorderColorComponentMappingCreateInfoEXT: return "SamplerBorderColorComponentMappingCreateInfoEXT";
    case StructureType::ePhysicalDevicePageableDeviceLocalMemoryFeaturesEXT: return "PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT";
    case StructureType::ePhysicalDeviceMaintenance4Features: return "PhysicalDeviceMaintenance4Features";
    case StructureType::ePhysicalDeviceMaintenance4Properties: return "PhysicalDeviceMaintenance4Properties";
    case StructureType::eDeviceBufferMemoryRequirements: return "DeviceBufferMemoryRequirements";
    case StructureType::eDeviceImageMemoryRequirements: return "DeviceImageMemoryRequirements";
    case StructureType::ePhysicalDeviceShaderCorePropertiesARM: return "PhysicalDeviceShaderCorePropertiesARM";
    case StructureType::ePhysicalDeviceShaderSubgroupRotateFeatures: return "PhysicalDeviceShaderSubgroupRotateFeatures";
    case StructureType::eDeviceQueueShaderCoreControlCreateInfoARM: return "DeviceQueueShaderCoreControlCreateInfoARM";
    case StructureType::ePhysicalDeviceSchedulingControlsFeaturesARM: return "PhysicalDeviceSchedulingControlsFeaturesARM";
    case StructureType::ePhysicalDeviceSchedulingControlsPropertiesARM: return "PhysicalDeviceSchedulingControlsPropertiesARM";
    case StructureType::ePhysicalDeviceImageSlicedViewOf3DFeaturesEXT: return "PhysicalDeviceImageSlicedViewOf3DFeaturesEXT";
    case StructureType::eImageViewSlicedCreateInfoEXT: return "ImageViewSlicedCreateInfoEXT";
    case StructureType::ePhysicalDeviceDescriptorSetHostMappingFeaturesVALVE: return "PhysicalDeviceDescriptorSetHostMappingFeaturesVALVE";
    case StructureType::eDescriptorSetBindingReferenceVALVE: return "DescriptorSetBindingReferenceVALVE";
    case StructureType::eDescriptorSetLayoutHostMappingInfoVALVE: return "DescriptorSetLayoutHostMappingInfoVALVE";
    case StructureType::ePhysicalDeviceDepthClampZeroOneFeaturesKHR: return "PhysicalDeviceDepthClampZeroOneFeaturesKHR";
    case StructureType::ePhysicalDeviceNonSeamlessCubeMapFeaturesEXT: return "PhysicalDeviceNonSeamlessCubeMapFeaturesEXT";
    case StructureType::ePhysicalDeviceRenderPassStripedFeaturesARM: return "PhysicalDeviceRenderPassStripedFeaturesARM";
    case StructureType::ePhysicalDeviceRenderPassStripedPropertiesARM: return "PhysicalDeviceRenderPassStripedPropertiesARM";
    case StructureType::eRenderPassStripeBeginInfoARM: return "RenderPassStripeBeginInfoARM";
    case StructureType::eRenderPassStripeInfoARM: return "RenderPassStripeInfoARM";
    case StructureType::eRenderPassStripeSubmitInfoARM: return "RenderPassStripeSubmitInfoARM";
    case StructureType::ePhysicalDeviceFragmentDensityMapOffsetFeaturesEXT: return "PhysicalDeviceFragmentDensityMapOffsetFeaturesEXT";
    case StructureType::ePhysicalDeviceFragmentDensityMapOffsetPropertiesEXT: return "PhysicalDeviceFragmentDensityMapOffsetPropertiesEXT";
    case StructureType::eRenderPassFragmentDensityMapOffsetEndInfoEXT: return "RenderPassFragmentDensityMapOffsetEndInfoEXT";
    case StructureType::ePhysicalDeviceCopyMemoryIndirectFeaturesNV: return "PhysicalDeviceCopyMemoryIndirectFeaturesNV";
    case StructureType::ePhysicalDeviceCopyMemoryIndirectPropertiesNV: return "PhysicalDeviceCopyMemoryIndirectPropertiesNV";
    case StructureType::ePhysicalDeviceMemoryDecompressionFeaturesNV: return "PhysicalDeviceMemoryDecompressionFeaturesNV";
    case StructureType::ePhysicalDeviceMemoryDecompressionPropertiesNV: return "PhysicalDeviceMemoryDecompressionPropertiesNV";
    case StructureType::ePhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV: return "PhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV";
    case StructureType::eComputePipelineIndirectBufferInfoNV: return "ComputePipelineIndirectBufferInfoNV";
    case StructureType::ePipelineIndirectDeviceAddressInfoNV: return "PipelineIndirectDeviceAddressInfoNV";
    case StructureType::ePhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV: return "PhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV";
    case StructureType::eAccelerationStructureGeometryLinearSweptSpheresDataNV: return "AccelerationStructureGeometryLinearSweptSpheresDataNV";
    case StructureType::eAccelerationStructureGeometrySpheresDataNV: return "AccelerationStructureGeometrySpheresDataNV";
    case StructureType::ePhysicalDeviceLinearColorAttachmentFeaturesNV: return "PhysicalDeviceLinearColorAttachmentFeaturesNV";
    case StructureType::ePhysicalDeviceShaderMaximalReconvergenceFeaturesKHR: return "PhysicalDeviceShaderMaximalReconvergenceFeaturesKHR";
    case StructureType::ePhysicalDeviceImageCompressionControlSwapchainFeaturesEXT: return "PhysicalDeviceImageCompressionControlSwapchainFeaturesEXT";
    case StructureType::ePhysicalDeviceImageProcessingFeaturesQCOM: return "PhysicalDeviceImageProcessingFeaturesQCOM";
    case StructureType::ePhysicalDeviceImageProcessingPropertiesQCOM: return "PhysicalDeviceImageProcessingPropertiesQCOM";
    case StructureType::eImageViewSampleWeightCreateInfoQCOM: return "ImageViewSampleWeightCreateInfoQCOM";
    case StructureType::ePhysicalDeviceNestedCommandBufferFeaturesEXT: return "PhysicalDeviceNestedCommandBufferFeaturesEXT";
    case StructureType::ePhysicalDeviceNestedCommandBufferPropertiesEXT: return "PhysicalDeviceNestedCommandBufferPropertiesEXT";
    case StructureType::eExternalMemoryAcquireUnmodifiedEXT: return "ExternalMemoryAcquireUnmodifiedEXT";
    case StructureType::ePhysicalDeviceExtendedDynamicState3FeaturesEXT: return "PhysicalDeviceExtendedDynamicState3FeaturesEXT";
    case StructureType::ePhysicalDeviceExtendedDynamicState3PropertiesEXT: return "PhysicalDeviceExtendedDynamicState3PropertiesEXT";
    case StructureType::ePhysicalDeviceSubpassMergeFeedbackFeaturesEXT: return "PhysicalDeviceSubpassMergeFeedbackFeaturesEXT";
    case StructureType::eRenderPassCreationControlEXT: return "RenderPassCreationControlEXT";
    case StructureType::eRenderPassCreationFeedbackCreateInfoEXT: return "RenderPassCreationFeedbackCreateInfoEXT";
    case StructureType::eRenderPassSubpassFeedbackCreateInfoEXT: return "RenderPassSubpassFeedbackCreateInfoEXT";
    case StructureType::eDirectDriverLoadingInfoLUNARG: return "DirectDriverLoadingInfoLUNARG";
    case StructureType::eDirectDriverLoadingListLUNARG: return "DirectDriverLoadingListLUNARG";
    case StructureType::eTensorCreateInfoARM: return "TensorCreateInfoARM";
    case StructureType::eTensorViewCreateInfoARM: return "TensorViewCreateInfoARM";
    case StructureType::eBindTensorMemoryInfoARM: return "BindTensorMemoryInfoARM";
    case StructureType::eWriteDescriptorSetTensorARM: return "WriteDescriptorSetTensorARM";
    case StructureType::ePhysicalDeviceTensorPropertiesARM: return "PhysicalDeviceTensorPropertiesARM";
    case StructureType::eTensorFormatPropertiesARM: return "TensorFormatPropertiesARM";
    case StructureType::eTensorDescriptionARM: return "TensorDescriptionARM";
    case StructureType::eTensorMemoryRequirementsInfoARM: return "TensorMemoryRequirementsInfoARM";
    case StructureType::eTensorMemoryBarrierARM: return "TensorMemoryBarrierARM";
    case StructureType::ePhysicalDeviceTensorFeaturesARM: return "PhysicalDeviceTensorFeaturesARM";
    case StructureType::eDeviceTensorMemoryRequirementsARM: return "DeviceTensorMemoryRequirementsARM";
    case StructureType::eCopyTensorInfoARM: return "CopyTensorInfoARM";
    case StructureType::eTensorCopyARM: return "TensorCopyARM";
    case StructureType::eTensorDependencyInfoARM: return "TensorDependencyInfoARM";
    case StructureType::eMemoryDedicatedAllocateInfoTensorARM: return "MemoryDedicatedAllocateInfoTensorARM";
    case StructureType::ePhysicalDeviceExternalTensorInfoARM: return "PhysicalDeviceExternalTensorInfoARM";
    case StructureType::eExternalTensorPropertiesARM: return "ExternalTensorPropertiesARM";
    case StructureType::eExternalMemoryTensorCreateInfoARM: return "ExternalMemoryTensorCreateInfoARM";
    case StructureType::ePhysicalDeviceDescriptorBufferTensorFeaturesARM: return "PhysicalDeviceDescriptorBufferTensorFeaturesARM";
    case StructureType::ePhysicalDeviceDescriptorBufferTensorPropertiesARM: return "PhysicalDeviceDescriptorBufferTensorPropertiesARM";
    case StructureType::eDescriptorGetTensorInfoARM: return "DescriptorGetTensorInfoARM";
    case StructureType::eTensorCaptureDescriptorDataInfoARM: return "TensorCaptureDescriptorDataInfoARM";
    case StructureType::eTensorViewCaptureDescriptorDataInfoARM: return "TensorViewCaptureDescriptorDataInfoARM";
    case StructureType::eFrameBoundaryTensorsARM: return "FrameBoundaryTensorsARM";
    case StructureType::ePhysicalDeviceShaderModuleIdentifierFeaturesEXT: return "PhysicalDeviceShaderModuleIdentifierFeaturesEXT";
    case StructureType::ePhysicalDeviceShaderModuleIdentifierPropertiesEXT: return "PhysicalDeviceShaderModuleIdentifierPropertiesEXT";
    case StructureType::ePipelineShaderStageModuleIdentifierCreateInfoEXT: return "PipelineShaderStageModuleIdentifierCreateInfoEXT";
    case StructureType::eShaderModuleIdentifierEXT: return "ShaderModuleIdentifierEXT";
    case StructureType::ePhysicalDeviceOpticalFlowFeaturesNV: return "PhysicalDeviceOpticalFlowFeaturesNV";
    case StructureType::ePhysicalDeviceOpticalFlowPropertiesNV: return "PhysicalDeviceOpticalFlowPropertiesNV";
    case StructureType::eOpticalFlowImageFormatInfoNV: return "OpticalFlowImageFormatInfoNV";
    case StructureType::eOpticalFlowImageFormatPropertiesNV: return "OpticalFlowImageFormatPropertiesNV";
    case StructureType::eOpticalFlowSessionCreateInfoNV: return "OpticalFlowSessionCreateInfoNV";
    case StructureType::eOpticalFlowExecuteInfoNV: return "OpticalFlowExecuteInfoNV";
    case StructureType::eOpticalFlowSessionCreatePrivateDataInfoNV: return "OpticalFlowSessionCreatePrivateDataInfoNV";
    case StructureType::ePhysicalDeviceLegacyDitheringFeaturesEXT: return "PhysicalDeviceLegacyDitheringFeaturesEXT";
    case StructureType::ePhysicalDevicePipelineProtectedAccessFeatures: return "PhysicalDevicePipelineProtectedAccessFeatures";
    case StructureType::ePhysicalDeviceMaintenance5Features: return "PhysicalDeviceMaintenance5Features";
    case StructureType::ePhysicalDeviceMaintenance5Properties: return "PhysicalDeviceMaintenance5Properties";
    case StructureType::eRenderingAreaInfo: return "RenderingAreaInfo";
    case StructureType::eDeviceImageSubresourceInfo: return "DeviceImageSubresourceInfo";
    case StructureType::ePipelineCreateFlags2CreateInfo: return "PipelineCreateFlags2CreateInfo";
    case StructureType::eBufferUsageFlags2CreateInfo: return "BufferUsageFlags2CreateInfo";
    case StructureType::ePhysicalDeviceAntiLagFeaturesAMD: return "PhysicalDeviceAntiLagFeaturesAMD";
    case StructureType::eAntiLagDataAMD: return "AntiLagDataAMD";
    case StructureType::eAntiLagPresentationInfoAMD: return "AntiLagPresentationInfoAMD";
    case StructureType::eSurfaceCapabilitiesPresentId2KHR: return "SurfaceCapabilitiesPresentId2KHR";
    case StructureType::ePresentId2KHR: return "PresentId2KHR";
    case StructureType::ePhysicalDevicePresentId2FeaturesKHR: return "PhysicalDevicePresentId2FeaturesKHR";
    case StructureType::eSurfaceCapabilitiesPresentWait2KHR: return "SurfaceCapabilitiesPresentWait2KHR";
    case StructureType::ePhysicalDevicePresentWait2FeaturesKHR: return "PhysicalDevicePresentWait2FeaturesKHR";
    case StructureType::ePresentWait2InfoKHR: return "PresentWait2InfoKHR";
    case StructureType::ePhysicalDeviceRayTracingPositionFetchFeaturesKHR: return "PhysicalDeviceRayTracingPositionFetchFeaturesKHR";
    case StructureType::ePhysicalDeviceShaderObjectFeaturesEXT: return "PhysicalDeviceShaderObjectFeaturesEXT";
    case StructureType::ePhysicalDeviceShaderObjectPropertiesEXT: return "PhysicalDeviceShaderObjectPropertiesEXT";
    case StructureType::eShaderCreateInfoEXT: return "ShaderCreateInfoEXT";
    case StructureType::ePhysicalDevicePipelineBinaryFeaturesKHR: return "PhysicalDevicePipelineBinaryFeaturesKHR";
    case StructureType::ePipelineBinaryCreateInfoKHR: return "PipelineBinaryCreateInfoKHR";
    case StructureType::ePipelineBinaryInfoKHR: return "PipelineBinaryInfoKHR";
    case StructureType::ePipelineBinaryKeyKHR: return "PipelineBinaryKeyKHR";
    case StructureType::ePhysicalDevicePipelineBinaryPropertiesKHR: return "PhysicalDevicePipelineBinaryPropertiesKHR";
    case StructureType::eReleaseCapturedPipelineDataInfoKHR: return "ReleaseCapturedPipelineDataInfoKHR";
    case StructureType::ePipelineBinaryDataInfoKHR: return "PipelineBinaryDataInfoKHR";
    case StructureType::ePipelineCreateInfoKHR: return "PipelineCreateInfoKHR";
    case StructureType::eDevicePipelineBinaryInternalCacheControlKHR: return "DevicePipelineBinaryInternalCacheControlKHR";
    case StructureType::ePipelineBinaryHandlesInfoKHR: return "PipelineBinaryHandlesInfoKHR";
    case StructureType::ePhysicalDeviceTilePropertiesFeaturesQCOM: return "PhysicalDeviceTilePropertiesFeaturesQCOM";
    case StructureType::eTilePropertiesQCOM: return "TilePropertiesQCOM";
    case StructureType::ePhysicalDeviceAmigoProfilingFeaturesSEC: return "PhysicalDeviceAmigoProfilingFeaturesSEC";
    case StructureType::eAmigoProfilingSubmitInfoSEC: return "AmigoProfilingSubmitInfoSEC";
    case StructureType::ePhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM: return "PhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM";
    case StructureType::ePhysicalDeviceRayTracingInvocationReorderFeaturesNV: return "PhysicalDeviceRayTracingInvocationReorderFeaturesNV";
    case StructureType::ePhysicalDeviceRayTracingInvocationReorderPropertiesNV: return "PhysicalDeviceRayTracingInvocationReorderPropertiesNV";
    case StructureType::ePhysicalDeviceCooperativeVectorFeaturesNV: return "PhysicalDeviceCooperativeVectorFeaturesNV";
    case StructureType::ePhysicalDeviceCooperativeVectorPropertiesNV: return "PhysicalDeviceCooperativeVectorPropertiesNV";
    case StructureType::eCooperativeVectorPropertiesNV: return "CooperativeVectorPropertiesNV";
    case StructureType::eConvertCooperativeVectorMatrixInfoNV: return "ConvertCooperativeVectorMatrixInfoNV";
    case StructureType::ePhysicalDeviceExtendedSparseAddressSpaceFeaturesNV: return "PhysicalDeviceExtendedSparseAddressSpaceFeaturesNV";
    case StructureType::ePhysicalDeviceExtendedSparseAddressSpacePropertiesNV: return "PhysicalDeviceExtendedSparseAddressSpacePropertiesNV";
    case StructureType::ePhysicalDeviceLegacyVertexAttributesFeaturesEXT: return "PhysicalDeviceLegacyVertexAttributesFeaturesEXT";
    case StructureType::ePhysicalDeviceLegacyVertexAttributesPropertiesEXT: return "PhysicalDeviceLegacyVertexAttributesPropertiesEXT";
    case StructureType::eLayerSettingsCreateInfoEXT: return "LayerSettingsCreateInfoEXT";
    case StructureType::ePhysicalDeviceShaderCoreBuiltinsFeaturesARM: return "PhysicalDeviceShaderCoreBuiltinsFeaturesARM";
    case StructureType::ePhysicalDeviceShaderCoreBuiltinsPropertiesARM: return "PhysicalDeviceShaderCoreBuiltinsPropertiesARM";
    case StructureType::ePhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT: return "PhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT";
    case StructureType::ePhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT: return "PhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT";
    case StructureType::eLatencySleepModeInfoNV: return "LatencySleepModeInfoNV";
    case StructureType::eLatencySleepInfoNV: return "LatencySleepInfoNV";
    case StructureType::eSetLatencyMarkerInfoNV: return "SetLatencyMarkerInfoNV";
    case StructureType::eGetLatencyMarkerInfoNV: return "GetLatencyMarkerInfoNV";
    case StructureType::eLatencyTimingsFrameReportNV: return "LatencyTimingsFrameReportNV";
    case StructureType::eLatencySubmissionPresentIdNV: return "LatencySubmissionPresentIdNV";
    case StructureType::eOutOfBandQueueTypeInfoNV: return "OutOfBandQueueTypeInfoNV";
    case StructureType::eSwapchainLatencyCreateInfoNV: return "SwapchainLatencyCreateInfoNV";
    case StructureType::eLatencySurfaceCapabilitiesNV: return "LatencySurfaceCapabilitiesNV";
    case StructureType::ePhysicalDeviceCooperativeMatrixFeaturesKHR: return "PhysicalDeviceCooperativeMatrixFeaturesKHR";
    case StructureType::eCooperativeMatrixPropertiesKHR: return "CooperativeMatrixPropertiesKHR";
    case StructureType::ePhysicalDeviceCooperativeMatrixPropertiesKHR: return "PhysicalDeviceCooperativeMatrixPropertiesKHR";
    case StructureType::eDataGraphPipelineCreateInfoARM: return "DataGraphPipelineCreateInfoARM";
    case StructureType::eDataGraphPipelineSessionCreateInfoARM: return "DataGraphPipelineSessionCreateInfoARM";
    case StructureType::eDataGraphPipelineResourceInfoARM: return "DataGraphPipelineResourceInfoARM";
    case StructureType::eDataGraphPipelineConstantARM: return "DataGraphPipelineConstantARM";
    case StructureType::eDataGraphPipelineSessionMemoryRequirementsInfoARM: return "DataGraphPipelineSessionMemoryRequirementsInfoARM";
    case StructureType::eBindDataGraphPipelineSessionMemoryInfoARM: return "BindDataGraphPipelineSessionMemoryInfoARM";
    case StructureType::ePhysicalDeviceDataGraphFeaturesARM: return "PhysicalDeviceDataGraphFeaturesARM";
    case StructureType::eDataGraphPipelineShaderModuleCreateInfoARM: return "DataGraphPipelineShaderModuleCreateInfoARM";
    case StructureType::eDataGraphPipelinePropertyQueryResultARM: return "DataGraphPipelinePropertyQueryResultARM";
    case StructureType::eDataGraphPipelineInfoARM: return "DataGraphPipelineInfoARM";
    case StructureType::eDataGraphPipelineCompilerControlCreateInfoARM: return "DataGraphPipelineCompilerControlCreateInfoARM";
    case StructureType::eDataGraphPipelineSessionBindPointRequirementsInfoARM: return "DataGraphPipelineSessionBindPointRequirementsInfoARM";
    case StructureType::eDataGraphPipelineSessionBindPointRequirementARM: return "DataGraphPipelineSessionBindPointRequirementARM";
    case StructureType::eDataGraphPipelineIdentifierCreateInfoARM: return "DataGraphPipelineIdentifierCreateInfoARM";
    case StructureType::eDataGraphPipelineDispatchInfoARM: return "DataGraphPipelineDispatchInfoARM";
    case StructureType::eDataGraphProcessingEngineCreateInfoARM: return "DataGraphProcessingEngineCreateInfoARM";
    case StructureType::eQueueFamilyDataGraphProcessingEnginePropertiesARM: return "QueueFamilyDataGraphProcessingEnginePropertiesARM";
    case StructureType::eQueueFamilyDataGraphPropertiesARM: return "QueueFamilyDataGraphPropertiesARM";
    case StructureType::ePhysicalDeviceQueueFamilyDataGraphProcessingEngineInfoARM: return "PhysicalDeviceQueueFamilyDataGraphProcessingEngineInfoARM";
    case StructureType::eDataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM: return "DataGraphPipelineConstantTensorSemiStructuredSparsityInfoARM";
    case StructureType::ePhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM: return "PhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM";
    case StructureType::eMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM: return "MultiviewPerViewRenderAreasRenderPassBeginInfoQCOM";
    case StructureType::ePhysicalDeviceComputeShaderDerivativesPropertiesKHR: return "PhysicalDeviceComputeShaderDerivativesPropertiesKHR";
    case StructureType::ePhysicalDevicePerStageDescriptorSetFeaturesNV: return "PhysicalDevicePerStageDescriptorSetFeaturesNV";
    case StructureType::ePhysicalDeviceImageProcessing2FeaturesQCOM: return "PhysicalDeviceImageProcessing2FeaturesQCOM";
    case StructureType::ePhysicalDeviceImageProcessing2PropertiesQCOM: return "PhysicalDeviceImageProcessing2PropertiesQCOM";
    case StructureType::eSamplerBlockMatchWindowCreateInfoQCOM: return "SamplerBlockMatchWindowCreateInfoQCOM";
    case StructureType::eSamplerCubicWeightsCreateInfoQCOM: return "SamplerCubicWeightsCreateInfoQCOM";
    case StructureType::ePhysicalDeviceCubicWeightsFeaturesQCOM: return "PhysicalDeviceCubicWeightsFeaturesQCOM";
    case StructureType::eBlitImageCubicWeightsInfoQCOM: return "BlitImageCubicWeightsInfoQCOM";
    case StructureType::ePhysicalDeviceYcbcrDegammaFeaturesQCOM: return "PhysicalDeviceYcbcrDegammaFeaturesQCOM";
    case StructureType::eSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM: return "SamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM";
    case StructureType::ePhysicalDeviceCubicClampFeaturesQCOM: return "PhysicalDeviceCubicClampFeaturesQCOM";
    case StructureType::ePhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT: return "PhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT";
    case StructureType::ePhysicalDeviceVertexAttributeDivisorProperties: return "PhysicalDeviceVertexAttributeDivisorProperties";
    case StructureType::ePhysicalDeviceUnifiedImageLayoutsFeaturesKHR: return "PhysicalDeviceUnifiedImageLayoutsFeaturesKHR";
    case StructureType::eAttachmentFeedbackLoopInfoEXT: return "AttachmentFeedbackLoopInfoEXT";
    case StructureType::ePhysicalDeviceShaderFloatControls2Features: return "PhysicalDeviceShaderFloatControls2Features";
    case StructureType::ePhysicalDeviceLayeredDriverPropertiesMSFT: return "PhysicalDeviceLayeredDriverPropertiesMSFT";
    case StructureType::ePhysicalDeviceShaderExpectAssumeFeatures: return "PhysicalDeviceShaderExpectAssumeFeatures";
    case StructureType::ePhysicalDeviceMaintenance6Features: return "PhysicalDeviceMaintenance6Features";
    case StructureType::ePhysicalDeviceMaintenance6Properties: return "PhysicalDeviceMaintenance6Properties";
    case StructureType::eBindMemoryStatus: return "BindMemoryStatus";
    case StructureType::eBindDescriptorSetsInfo: return "BindDescriptorSetsInfo";
    case StructureType::ePushConstantsInfo: return "PushConstantsInfo";
    case StructureType::ePushDescriptorSetInfo: return "PushDescriptorSetInfo";
    case StructureType::ePushDescriptorSetWithTemplateInfo: return "PushDescriptorSetWithTemplateInfo";
    case StructureType::eSetDescriptorBufferOffsetsInfoEXT: return "SetDescriptorBufferOffsetsInfoEXT";
    case StructureType::eBindDescriptorBufferEmbeddedSamplersInfoEXT: return "BindDescriptorBufferEmbeddedSamplersInfoEXT";
    case StructureType::ePhysicalDeviceDescriptorPoolOverallocationFeaturesNV: return "PhysicalDeviceDescriptorPoolOverallocationFeaturesNV";
    case StructureType::ePhysicalDeviceTileMemoryHeapFeaturesQCOM: return "PhysicalDeviceTileMemoryHeapFeaturesQCOM";
    case StructureType::ePhysicalDeviceTileMemoryHeapPropertiesQCOM: return "PhysicalDeviceTileMemoryHeapPropertiesQCOM";
    case StructureType::eTileMemoryRequirementsQCOM: return "TileMemoryRequirementsQCOM";
    case StructureType::eTileMemoryBindInfoQCOM: return "TileMemoryBindInfoQCOM";
    case StructureType::eTileMemorySizeInfoQCOM: return "TileMemorySizeInfoQCOM";
    case StructureType::eDisplaySurfaceStereoCreateInfoNV: return "DisplaySurfaceStereoCreateInfoNV";
    case StructureType::eDisplayModeStereoPropertiesNV: return "DisplayModeStereoPropertiesNV";
    case StructureType::ePhysicalDeviceRawAccessChainsFeaturesNV: return "PhysicalDeviceRawAccessChainsFeaturesNV";
    case StructureType::eExternalComputeQueueDeviceCreateInfoNV: return "ExternalComputeQueueDeviceCreateInfoNV";
    case StructureType::eExternalComputeQueueCreateInfoNV: return "ExternalComputeQueueCreateInfoNV";
    case StructureType::eExternalComputeQueueDataParamsNV: return "ExternalComputeQueueDataParamsNV";
    case StructureType::ePhysicalDeviceExternalComputeQueuePropertiesNV: return "PhysicalDeviceExternalComputeQueuePropertiesNV";
    case StructureType::ePhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR: return "PhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR";
    case StructureType::ePhysicalDeviceCommandBufferInheritanceFeaturesNV: return "PhysicalDeviceCommandBufferInheritanceFeaturesNV";
    case StructureType::ePhysicalDeviceMaintenance7FeaturesKHR: return "PhysicalDeviceMaintenance7FeaturesKHR";
    case StructureType::ePhysicalDeviceMaintenance7PropertiesKHR: return "PhysicalDeviceMaintenance7PropertiesKHR";
    case StructureType::ePhysicalDeviceLayeredApiPropertiesListKHR: return "PhysicalDeviceLayeredApiPropertiesListKHR";
    case StructureType::ePhysicalDeviceLayeredApiPropertiesKHR: return "PhysicalDeviceLayeredApiPropertiesKHR";
    case StructureType::ePhysicalDeviceLayeredApiVulkanPropertiesKHR: return "PhysicalDeviceLayeredApiVulkanPropertiesKHR";
    case StructureType::ePhysicalDeviceShaderAtomicFloat16VectorFeaturesNV: return "PhysicalDeviceShaderAtomicFloat16VectorFeaturesNV";
    case StructureType::ePhysicalDeviceShaderReplicatedCompositesFeaturesEXT: return "PhysicalDeviceShaderReplicatedCompositesFeaturesEXT";
    case StructureType::ePhysicalDeviceShaderFloat8FeaturesEXT: return "PhysicalDeviceShaderFloat8FeaturesEXT";
    case StructureType::ePhysicalDeviceRayTracingValidationFeaturesNV: return "PhysicalDeviceRayTracingValidationFeaturesNV";
    case StructureType::ePhysicalDeviceClusterAccelerationStructureFeaturesNV: return "PhysicalDeviceClusterAccelerationStructureFeaturesNV";
    case StructureType::ePhysicalDeviceClusterAccelerationStructurePropertiesNV: return "PhysicalDeviceClusterAccelerationStructurePropertiesNV";
    case StructureType::eClusterAccelerationStructureClustersBottomLevelInputNV: return "ClusterAccelerationStructureClustersBottomLevelInputNV";
    case StructureType::eClusterAccelerationStructureTriangleClusterInputNV: return "ClusterAccelerationStructureTriangleClusterInputNV";
    case StructureType::eClusterAccelerationStructureMoveObjectsInputNV: return "ClusterAccelerationStructureMoveObjectsInputNV";
    case StructureType::eClusterAccelerationStructureInputInfoNV: return "ClusterAccelerationStructureInputInfoNV";
    case StructureType::eClusterAccelerationStructureCommandsInfoNV: return "ClusterAccelerationStructureCommandsInfoNV";
    case StructureType::eRayTracingPipelineClusterAccelerationStructureCreateInfoNV: return "RayTracingPipelineClusterAccelerationStructureCreateInfoNV";
    case StructureType::ePhysicalDevicePartitionedAccelerationStructureFeaturesNV: return "PhysicalDevicePartitionedAccelerationStructureFeaturesNV";
    case StructureType::ePhysicalDevicePartitionedAccelerationStructurePropertiesNV: return "PhysicalDevicePartitionedAccelerationStructurePropertiesNV";
    case StructureType::eWriteDescriptorSetPartitionedAccelerationStructureNV: return "WriteDescriptorSetPartitionedAccelerationStructureNV";
    case StructureType::ePartitionedAccelerationStructureInstancesInputNV: return "PartitionedAccelerationStructureInstancesInputNV";
    case StructureType::eBuildPartitionedAccelerationStructureInfoNV: return "BuildPartitionedAccelerationStructureInfoNV";
    case StructureType::ePartitionedAccelerationStructureFlagsNV: return "PartitionedAccelerationStructureFlagsNV";
    case StructureType::ePhysicalDeviceDeviceGeneratedCommandsFeaturesEXT: return "PhysicalDeviceDeviceGeneratedCommandsFeaturesEXT";
    case StructureType::ePhysicalDeviceDeviceGeneratedCommandsPropertiesEXT: return "PhysicalDeviceDeviceGeneratedCommandsPropertiesEXT";
    case StructureType::eGeneratedCommandsMemoryRequirementsInfoEXT: return "GeneratedCommandsMemoryRequirementsInfoEXT";
    case StructureType::eIndirectExecutionSetCreateInfoEXT: return "IndirectExecutionSetCreateInfoEXT";
    case StructureType::eGeneratedCommandsInfoEXT: return "GeneratedCommandsInfoEXT";
    case StructureType::eIndirectCommandsLayoutCreateInfoEXT: return "IndirectCommandsLayoutCreateInfoEXT";
    case StructureType::eIndirectCommandsLayoutTokenEXT: return "IndirectCommandsLayoutTokenEXT";
    case StructureType::eWriteIndirectExecutionSetPipelineEXT: return "WriteIndirectExecutionSetPipelineEXT";
    case StructureType::eWriteIndirectExecutionSetShaderEXT: return "WriteIndirectExecutionSetShaderEXT";
    case StructureType::eIndirectExecutionSetPipelineInfoEXT: return "IndirectExecutionSetPipelineInfoEXT";
    case StructureType::eIndirectExecutionSetShaderInfoEXT: return "IndirectExecutionSetShaderInfoEXT";
    case StructureType::eIndirectExecutionSetShaderLayoutInfoEXT: return "IndirectExecutionSetShaderLayoutInfoEXT";
    case StructureType::eGeneratedCommandsPipelineInfoEXT: return "GeneratedCommandsPipelineInfoEXT";
    case StructureType::eGeneratedCommandsShaderInfoEXT: return "GeneratedCommandsShaderInfoEXT";
    case StructureType::ePhysicalDeviceMaintenance8FeaturesKHR: return "PhysicalDeviceMaintenance8FeaturesKHR";
    case StructureType::eMemoryBarrierAccessFlags3KHR: return "MemoryBarrierAccessFlags3KHR";
    case StructureType::ePhysicalDeviceImageAlignmentControlFeaturesMESA: return "PhysicalDeviceImageAlignmentControlFeaturesMESA";
    case StructureType::ePhysicalDeviceImageAlignmentControlPropertiesMESA: return "PhysicalDeviceImageAlignmentControlPropertiesMESA";
    case StructureType::eImageAlignmentControlCreateInfoMESA: return "ImageAlignmentControlCreateInfoMESA";
    case StructureType::ePhysicalDeviceDepthClampControlFeaturesEXT: return "PhysicalDeviceDepthClampControlFeaturesEXT";
    case StructureType::ePipelineViewportDepthClampControlCreateInfoEXT: return "PipelineViewportDepthClampControlCreateInfoEXT";
    case StructureType::ePhysicalDeviceMaintenance9FeaturesKHR: return "PhysicalDeviceMaintenance9FeaturesKHR";
    case StructureType::ePhysicalDeviceMaintenance9PropertiesKHR: return "PhysicalDeviceMaintenance9PropertiesKHR";
    case StructureType::eQueueFamilyOwnershipTransferPropertiesKHR: return "QueueFamilyOwnershipTransferPropertiesKHR";
    case StructureType::ePhysicalDeviceHdrVividFeaturesHUAWEI: return "PhysicalDeviceHdrVividFeaturesHUAWEI";
    case StructureType::eHdrVividDynamicMetadataHUAWEI: return "HdrVividDynamicMetadataHUAWEI";
    case StructureType::ePhysicalDeviceCooperativeMatrix2FeaturesNV: return "PhysicalDeviceCooperativeMatrix2FeaturesNV";
    case StructureType::eCooperativeMatrixFlexibleDimensionsPropertiesNV: return "CooperativeMatrixFlexibleDimensionsPropertiesNV";
    case StructureType::ePhysicalDeviceCooperativeMatrix2PropertiesNV: return "PhysicalDeviceCooperativeMatrix2PropertiesNV";
    case StructureType::ePhysicalDevicePipelineOpacityMicromapFeaturesARM: return "PhysicalDevicePipelineOpacityMicromapFeaturesARM";
    case StructureType::ePhysicalDeviceVertexAttributeRobustnessFeaturesEXT: return "PhysicalDeviceVertexAttributeRobustnessFeaturesEXT";
    case StructureType::ePhysicalDeviceFormatPackFeaturesARM: return "PhysicalDeviceFormatPackFeaturesARM";
    case StructureType::ePhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE: return "PhysicalDeviceFragmentDensityMapLayeredFeaturesVALVE";
    case StructureType::ePhysicalDeviceFragmentDensityMapLayeredPropertiesVALVE: return "PhysicalDeviceFragmentDensityMapLayeredPropertiesVALVE";
    case StructureType::ePipelineFragmentDensityMapLayeredCreateInfoVALVE: return "PipelineFragmentDensityMapLayeredCreateInfoVALVE";
    case StructureType::eRenderingEndInfoEXT: return "RenderingEndInfoEXT";
    case StructureType::ePhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT: return "PhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT";
    case StructureType::ePhysicalDevicePipelineCacheIncrementalModeFeaturesSEC: return "PhysicalDevicePipelineCacheIncrementalModeFeaturesSEC";
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    case StructureType::eXlibSurfaceCreateInfoKHR: return "XlibSurfaceCreateInfoKHR";
#endif // VK_USE_PLATFORM_XLIB_KHR
#if defined(VK_USE_PLATFORM_XCB_KHR)
    case StructureType::eXcbSurfaceCreateInfoKHR: return "XcbSurfaceCreateInfoKHR";
#endif // VK_USE_PLATFORM_XCB_KHR
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    case StructureType::eWaylandSurfaceCreateInfoKHR: return "WaylandSurfaceCreateInfoKHR";
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    case StructureType::eAndroidSurfaceCreateInfoKHR: return "AndroidSurfaceCreateInfoKHR";
#endif // VK_USE_PLATFORM_ANDROID_KHR
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    case StructureType::eWin32SurfaceCreateInfoKHR: return "Win32SurfaceCreateInfoKHR";
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_GGP)
    case StructureType::eStreamDescriptorSurfaceCreateInfoGGP: return "StreamDescriptorSurfaceCreateInfoGGP";
#endif // VK_USE_PLATFORM_GGP
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    case StructureType::eImportMemoryWin32HandleInfoNV: return "ImportMemoryWin32HandleInfoNV";
    case StructureType::eExportMemoryWin32HandleInfoNV: return "ExportMemoryWin32HandleInfoNV";
    case StructureType::eWin32KeyedMutexAcquireReleaseInfoNV: return "Win32KeyedMutexAcquireReleaseInfoNV";
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_VI_NN)
    case StructureType::eViSurfaceCreateInfoNN: return "ViSurfaceCreateInfoNN";
#endif // VK_USE_PLATFORM_VI_NN
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    case StructureType::eImportMemoryWin32HandleInfoKHR: return "ImportMemoryWin32HandleInfoKHR";
    case StructureType::eExportMemoryWin32HandleInfoKHR: return "ExportMemoryWin32HandleInfoKHR";
    case StructureType::eMemoryWin32HandlePropertiesKHR: return "MemoryWin32HandlePropertiesKHR";
    case StructureType::eMemoryGetWin32HandleInfoKHR: return "MemoryGetWin32HandleInfoKHR";
    case StructureType::eWin32KeyedMutexAcquireReleaseInfoKHR: return "Win32KeyedMutexAcquireReleaseInfoKHR";
    case StructureType::eImportSemaphoreWin32HandleInfoKHR: return "ImportSemaphoreWin32HandleInfoKHR";
    case StructureType::eExportSemaphoreWin32HandleInfoKHR: return "ExportSemaphoreWin32HandleInfoKHR";
    case StructureType::eD3D12FenceSubmitInfoKHR: return "D3D12FenceSubmitInfoKHR";
    case StructureType::eSemaphoreGetWin32HandleInfoKHR: return "SemaphoreGetWin32HandleInfoKHR";
    case StructureType::eImportFenceWin32HandleInfoKHR: return "ImportFenceWin32HandleInfoKHR";
    case StructureType::eExportFenceWin32HandleInfoKHR: return "ExportFenceWin32HandleInfoKHR";
    case StructureType::eFenceGetWin32HandleInfoKHR: return "FenceGetWin32HandleInfoKHR";
#endif // VK_USE_PLATFORM_WIN32_KHR
#if defined(VK_USE_PLATFORM_IOS_MVK)
    case StructureType::eIosSurfaceCreateInfoMVK: return "IosSurfaceCreateInfoMVK";
#endif // VK_USE_PLATFORM_IOS_MVK
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    case StructureType::eMacosSurfaceCreateInfoMVK: return "MacosSurfaceCreateInfoMVK";
#endif // VK_USE_PLATFORM_MACOS_MVK
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    case StructureType::eAndroidHardwareBufferUsageANDROID: return "AndroidHardwareBufferUsageANDROID";
    case StructureType::eAndroidHardwareBufferPropertiesANDROID: return "AndroidHardwareBufferPropertiesANDROID";
    case StructureType::eAndroidHardwareBufferFormatPropertiesANDROID: return "AndroidHardwareBufferFormatPropertiesANDROID";
    case StructureType::eImportAndroidHardwareBufferInfoANDROID: return "ImportAndroidHardwareBufferInfoANDROID";
    case StructureType::eMemoryGetAndroidHardwareBufferInfoANDROID: return "MemoryGetAndroidHardwareBufferInfoANDROID";
    case StructureType::eExternalFormatANDROID: return "ExternalFormatANDROID";
    case StructureType::eAndroidHardwareBufferFormatProperties2ANDROID: return "AndroidHardwareBufferFormatProperties2ANDROID";
#endif // VK_USE_PLATFORM_ANDROID_KHR
    case StructureType::ePhysicalDeviceShaderEnqueueFeaturesAMDX: return "PhysicalDeviceShaderEnqueueFeaturesAMDX";
    case StructureType::ePhysicalDeviceShaderEnqueuePropertiesAMDX: return "PhysicalDeviceShaderEnqueuePropertiesAMDX";
    case StructureType::eExecutionGraphPipelineScratchSizeAMDX: return "ExecutionGraphPipelineScratchSizeAMDX";
    case StructureType::eExecutionGraphPipelineCreateInfoAMDX: return "ExecutionGraphPipelineCreateInfoAMDX";
    case StructureType::ePipelineShaderStageNodeCreateInfoAMDX: return "PipelineShaderStageNodeCreateInfoAMDX";
    case StructureType::ePhysicalDevicePortabilitySubsetFeaturesKHR: return "PhysicalDevicePortabilitySubsetFeaturesKHR";
    case StructureType::ePhysicalDevicePortabilitySubsetPropertiesKHR: return "PhysicalDevicePortabilitySubsetPropertiesKHR";
#if defined(VK_USE_PLATFORM_GGP)
    case StructureType::ePresentFrameTokenGGP: return "PresentFrameTokenGGP";
#endif // VK_USE_PLATFORM_GGP
#if defined(VK_USE_PLATFORM_FUCHSIA)
    case StructureType::eImagepipeSurfaceCreateInfoFUCHSIA: return "ImagepipeSurfaceCreateInfoFUCHSIA";
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_METAL_EXT)
    case StructureType::eMetalSurfaceCreateInfoEXT: return "MetalSurfaceCreateInfoEXT";
#endif // VK_USE_PLATFORM_METAL_EXT
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    case StructureType::eSurfaceFullScreenExclusiveInfoEXT: return "SurfaceFullScreenExclusiveInfoEXT";
    case StructureType::eSurfaceCapabilitiesFullScreenExclusiveEXT: return "SurfaceCapabilitiesFullScreenExclusiveEXT";
    case StructureType::eSurfaceFullScreenExclusiveWin32InfoEXT: return "SurfaceFullScreenExclusiveWin32InfoEXT";
#endif // VK_USE_PLATFORM_WIN32_KHR
    case StructureType::eCudaModuleCreateInfoNV: return "CudaModuleCreateInfoNV";
    case StructureType::eCudaFunctionCreateInfoNV: return "CudaFunctionCreateInfoNV";
    case StructureType::eCudaLaunchInfoNV: return "CudaLaunchInfoNV";
    case StructureType::ePhysicalDeviceCudaKernelLaunchFeaturesNV: return "PhysicalDeviceCudaKernelLaunchFeaturesNV";
    case StructureType::ePhysicalDeviceCudaKernelLaunchPropertiesNV: return "PhysicalDeviceCudaKernelLaunchPropertiesNV";
#if defined(VK_USE_PLATFORM_METAL_EXT)
    case StructureType::eExportMetalObjectCreateInfoEXT: return "ExportMetalObjectCreateInfoEXT";
    case StructureType::eExportMetalObjectsInfoEXT: return "ExportMetalObjectsInfoEXT";
    case StructureType::eExportMetalDeviceInfoEXT: return "ExportMetalDeviceInfoEXT";
    case StructureType::eExportMetalCommandQueueInfoEXT: return "ExportMetalCommandQueueInfoEXT";
    case StructureType::eExportMetalBufferInfoEXT: return "ExportMetalBufferInfoEXT";
    case StructureType::eImportMetalBufferInfoEXT: return "ImportMetalBufferInfoEXT";
    case StructureType::eExportMetalTextureInfoEXT: return "ExportMetalTextureInfoEXT";
    case StructureType::eImportMetalTextureInfoEXT: return "ImportMetalTextureInfoEXT";
    case StructureType::eExportMetalIoSurfaceInfoEXT: return "ExportMetalIoSurfaceInfoEXT";
    case StructureType::eImportMetalIoSurfaceInfoEXT: return "ImportMetalIoSurfaceInfoEXT";
    case StructureType::eExportMetalSharedEventInfoEXT: return "ExportMetalSharedEventInfoEXT";
    case StructureType::eImportMetalSharedEventInfoEXT: return "ImportMetalSharedEventInfoEXT";
#endif // VK_USE_PLATFORM_METAL_EXT
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    case StructureType::eDirectfbSurfaceCreateInfoEXT: return "DirectfbSurfaceCreateInfoEXT";
#endif // VK_USE_PLATFORM_DIRECTFB_EXT
#if defined(VK_USE_PLATFORM_FUCHSIA)
    case StructureType::eImportMemoryZirconHandleInfoFUCHSIA: return "ImportMemoryZirconHandleInfoFUCHSIA";
    case StructureType::eMemoryZirconHandlePropertiesFUCHSIA: return "MemoryZirconHandlePropertiesFUCHSIA";
    case StructureType::eMemoryGetZirconHandleInfoFUCHSIA: return "MemoryGetZirconHandleInfoFUCHSIA";
    case StructureType::eImportSemaphoreZirconHandleInfoFUCHSIA: return "ImportSemaphoreZirconHandleInfoFUCHSIA";
    case StructureType::eSemaphoreGetZirconHandleInfoFUCHSIA: return "SemaphoreGetZirconHandleInfoFUCHSIA";
    case StructureType::eBufferCollectionCreateInfoFUCHSIA: return "BufferCollectionCreateInfoFUCHSIA";
    case StructureType::eImportMemoryBufferCollectionFUCHSIA: return "ImportMemoryBufferCollectionFUCHSIA";
    case StructureType::eBufferCollectionImageCreateInfoFUCHSIA: return "BufferCollectionImageCreateInfoFUCHSIA";
    case StructureType::eBufferCollectionPropertiesFUCHSIA: return "BufferCollectionPropertiesFUCHSIA";
    case StructureType::eBufferConstraintsInfoFUCHSIA: return "BufferConstraintsInfoFUCHSIA";
    case StructureType::eBufferCollectionBufferCreateInfoFUCHSIA: return "BufferCollectionBufferCreateInfoFUCHSIA";
    case StructureType::eImageConstraintsInfoFUCHSIA: return "ImageConstraintsInfoFUCHSIA";
    case StructureType::eImageFormatConstraintsInfoFUCHSIA: return "ImageFormatConstraintsInfoFUCHSIA";
    case StructureType::eSysmemColorSpaceFUCHSIA: return "SysmemColorSpaceFUCHSIA";
    case StructureType::eBufferCollectionConstraintsInfoFUCHSIA: return "BufferCollectionConstraintsInfoFUCHSIA";
#endif // VK_USE_PLATFORM_FUCHSIA
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    case StructureType::eScreenSurfaceCreateInfoQNX: return "ScreenSurfaceCreateInfoQNX";
#endif // VK_USE_PLATFORM_SCREEN_QNX
    case StructureType::ePhysicalDeviceDisplacementMicromapFeaturesNV: return "PhysicalDeviceDisplacementMicromapFeaturesNV";
    case StructureType::ePhysicalDeviceDisplacementMicromapPropertiesNV: return "PhysicalDeviceDisplacementMicromapPropertiesNV";
    case StructureType::eAccelerationStructureTrianglesDisplacementMicromapNV: return "AccelerationStructureTrianglesDisplacementMicromapNV";
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    case StructureType::ePhysicalDeviceExternalFormatResolveFeaturesANDROID: return "PhysicalDeviceExternalFormatResolveFeaturesANDROID";
    case StructureType::ePhysicalDeviceExternalFormatResolvePropertiesANDROID: return "PhysicalDeviceExternalFormatResolvePropertiesANDROID";
    case StructureType::eAndroidHardwareBufferFormatResolvePropertiesANDROID: return "AndroidHardwareBufferFormatResolvePropertiesANDROID";
#endif // VK_USE_PLATFORM_ANDROID_KHR
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    case StructureType::eScreenBufferPropertiesQNX: return "ScreenBufferPropertiesQNX";
    case StructureType::eScreenBufferFormatPropertiesQNX: return "ScreenBufferFormatPropertiesQNX";
    case StructureType::eImportScreenBufferInfoQNX: return "ImportScreenBufferInfoQNX";
    case StructureType::eExternalFormatQNX: return "ExternalFormatQNX";
    case StructureType::ePhysicalDeviceExternalMemoryScreenBufferFeaturesQNX: return "PhysicalDeviceExternalMemoryScreenBufferFeaturesQNX";
#endif // VK_USE_PLATFORM_SCREEN_QNX
#if defined(VK_USE_PLATFORM_OHOS)
    case StructureType::eOhSurfaceCreateInfoOHOS: return "OhSurfaceCreateInfoOHOS";
#endif // VK_USE_PLATFORM_OHOS
#if defined(VK_USE_PLATFORM_METAL_EXT)
    case StructureType::eImportMemoryMetalHandleInfoEXT: return "ImportMemoryMetalHandleInfoEXT";
    case StructureType::eMemoryMetalHandlePropertiesEXT: return "MemoryMetalHandlePropertiesEXT";
    case StructureType::eMemoryGetMetalHandleInfoEXT: return "MemoryGetMetalHandleInfoEXT";
#endif // VK_USE_PLATFORM_METAL_EXT
    case StructureType::eSetPresentConfigNV: return "SetPresentConfigNV";
    case StructureType::ePhysicalDevicePresentMeteringFeaturesNV: return "PhysicalDevicePresentMeteringFeaturesNV";
    default: return "Unknown";
    }
}

const char* to_cstr(PipelineCacheHeaderVersion v) {
    switch (v) {
    case PipelineCacheHeaderVersion::eOne: return "One";
    default: return "Unknown";
    }
}

const char* to_cstr(ImageLayout v) {
    switch (v) {
    case ImageLayout::eUndefined: return "Undefined";
    case ImageLayout::eGeneral: return "General";
    case ImageLayout::eColorAttachmentOptimal: return "ColorAttachmentOptimal";
    case ImageLayout::eDepthStencilAttachmentOptimal: return "DepthStencilAttachmentOptimal";
    case ImageLayout::eDepthStencilReadOnlyOptimal: return "DepthStencilReadOnlyOptimal";
    case ImageLayout::eShaderReadOnlyOptimal: return "ShaderReadOnlyOptimal";
    case ImageLayout::eTransferSrcOptimal: return "TransferSrcOptimal";
    case ImageLayout::eTransferDstOptimal: return "TransferDstOptimal";
    case ImageLayout::ePreinitialized: return "Preinitialized";
    case ImageLayout::ePresentSrcKHR: return "PresentSrcKHR";
    case ImageLayout::eSharedPresentKHR: return "SharedPresentKHR";
    case ImageLayout::eDepthReadOnlyStencilAttachmentOptimal: return "DepthReadOnlyStencilAttachmentOptimal";
    case ImageLayout::eDepthAttachmentStencilReadOnlyOptimal: return "DepthAttachmentStencilReadOnlyOptimal";
    case ImageLayout::eFragmentShadingRateAttachmentOptimalKHR: return "FragmentShadingRateAttachmentOptimalKHR";
    case ImageLayout::eFragmentDensityMapOptimalEXT: return "FragmentDensityMapOptimalEXT";
    case ImageLayout::eRenderingLocalRead: return "RenderingLocalRead";
    case ImageLayout::eDepthAttachmentOptimal: return "DepthAttachmentOptimal";
    case ImageLayout::eDepthReadOnlyOptimal: return "DepthReadOnlyOptimal";
    case ImageLayout::eStencilAttachmentOptimal: return "StencilAttachmentOptimal";
    case ImageLayout::eStencilReadOnlyOptimal: return "StencilReadOnlyOptimal";
    case ImageLayout::eReadOnlyOptimal: return "ReadOnlyOptimal";
    case ImageLayout::eAttachmentOptimal: return "AttachmentOptimal";
    case ImageLayout::eAttachmentFeedbackLoopOptimalEXT: return "AttachmentFeedbackLoopOptimalEXT";
    case ImageLayout::eTensorAliasingARM: return "TensorAliasingARM";
    case ImageLayout::eZeroInitializedEXT: return "ZeroInitializedEXT";
    default: return "Unknown";
    }
}

const char* to_cstr(ObjectType v) {
    switch (v) {
    case ObjectType::eUnknown: return "Unknown";
    case ObjectType::eInstance: return "Instance";
    case ObjectType::ePhysicalDevice: return "PhysicalDevice";
    case ObjectType::eDevice: return "Device";
    case ObjectType::eQueue: return "Queue";
    case ObjectType::eSemaphore: return "Semaphore";
    case ObjectType::eCommandBuffer: return "CommandBuffer";
    case ObjectType::eFence: return "Fence";
    case ObjectType::eDeviceMemory: return "DeviceMemory";
    case ObjectType::eBuffer: return "Buffer";
    case ObjectType::eImage: return "Image";
    case ObjectType::eEvent: return "Event";
    case ObjectType::eQueryPool: return "QueryPool";
    case ObjectType::eBufferView: return "BufferView";
    case ObjectType::eImageView: return "ImageView";
    case ObjectType::eShaderModule: return "ShaderModule";
    case ObjectType::ePipelineCache: return "PipelineCache";
    case ObjectType::ePipelineLayout: return "PipelineLayout";
    case ObjectType::eRenderPass: return "RenderPass";
    case ObjectType::ePipeline: return "Pipeline";
    case ObjectType::eDescriptorSetLayout: return "DescriptorSetLayout";
    case ObjectType::eSampler: return "Sampler";
    case ObjectType::eDescriptorPool: return "DescriptorPool";
    case ObjectType::eDescriptorSet: return "DescriptorSet";
    case ObjectType::eFramebuffer: return "Framebuffer";
    case ObjectType::eCommandPool: return "CommandPool";
    case ObjectType::eSurfaceKHR: return "SurfaceKHR";
    case ObjectType::eSwapchainKHR: return "SwapchainKHR";
    case ObjectType::eDisplayKHR: return "DisplayKHR";
    case ObjectType::eDisplayModeKHR: return "DisplayModeKHR";
    case ObjectType::eDebugReportCallbackEXT: return "DebugReportCallbackEXT";
    case ObjectType::eCuModuleNVX: return "CuModuleNVX";
    case ObjectType::eCuFunctionNVX: return "CuFunctionNVX";
    case ObjectType::eDescriptorUpdateTemplate: return "DescriptorUpdateTemplate";
    case ObjectType::eDebugUtilsMessengerEXT: return "DebugUtilsMessengerEXT";
    case ObjectType::eAccelerationStructureKHR: return "AccelerationStructureKHR";
    case ObjectType::eSamplerYcbcrConversion: return "SamplerYcbcrConversion";
    case ObjectType::eValidationCacheEXT: return "ValidationCacheEXT";
    case ObjectType::eAccelerationStructureNV: return "AccelerationStructureNV";
    case ObjectType::ePerformanceConfigurationINTEL: return "PerformanceConfigurationINTEL";
    case ObjectType::eDeferredOperationKHR: return "DeferredOperationKHR";
    case ObjectType::eIndirectCommandsLayoutNV: return "IndirectCommandsLayoutNV";
    case ObjectType::ePrivateDataSlot: return "PrivateDataSlot";
    case ObjectType::eMicromapEXT: return "MicromapEXT";
    case ObjectType::eTensorARM: return "TensorARM";
    case ObjectType::eTensorViewARM: return "TensorViewARM";
    case ObjectType::eOpticalFlowSessionNV: return "OpticalFlowSessionNV";
    case ObjectType::eShaderEXT: return "ShaderEXT";
    case ObjectType::ePipelineBinaryKHR: return "PipelineBinaryKHR";
    case ObjectType::eDataGraphPipelineSessionARM: return "DataGraphPipelineSessionARM";
    case ObjectType::eExternalComputeQueueNV: return "ExternalComputeQueueNV";
    case ObjectType::eIndirectCommandsLayoutEXT: return "IndirectCommandsLayoutEXT";
    case ObjectType::eIndirectExecutionSetEXT: return "IndirectExecutionSetEXT";
    case ObjectType::eCudaModuleNV: return "CudaModuleNV";
    case ObjectType::eCudaFunctionNV: return "CudaFunctionNV";
#if defined(VK_USE_PLATFORM_FUCHSIA)
    case ObjectType::eBufferCollectionFUCHSIA: return "BufferCollectionFUCHSIA";
#endif // VK_USE_PLATFORM_FUCHSIA
    default: return "Unknown";
    }
}

const char* to_cstr(VendorId v) {
    switch (v) {
    case VendorId::eKhronos: return "Khronos";
    case VendorId::eViv: return "Viv";
    case VendorId::eVsi: return "Vsi";
    case VendorId::eKazan: return "Kazan";
    case VendorId::eCodeplay: return "Codeplay";
    case VendorId::eMesa: return "Mesa";
    case VendorId::ePocl: return "Pocl";
    case VendorId::eMobileye: return "Mobileye";
    default: return "Unknown";
    }
}

const char* to_cstr(SystemAllocationScope v) {
    switch (v) {
    case SystemAllocationScope::eCommand: return "Command";
    case SystemAllocationScope::eObject: return "Object";
    case SystemAllocationScope::eCache: return "Cache";
    case SystemAllocationScope::eDevice: return "Device";
    case SystemAllocationScope::eInstance: return "Instance";
    default: return "Unknown";
    }
}

const char* to_cstr(InternalAllocationType v) {
    switch (v) {
    case InternalAllocationType::eExecutable: return "Executable";
    default: return "Unknown";
    }
}

const char* to_cstr(Format v) {
    switch (v) {
    case Format::eUndefined: return "Undefined";
    case Format::eR4G4UnormPack8: return "R4G4UnormPack8";
    case Format::eR4G4B4A4UnormPack16: return "R4G4B4A4UnormPack16";
    case Format::eB4G4R4A4UnormPack16: return "B4G4R4A4UnormPack16";
    case Format::eR5G6B5UnormPack16: return "R5G6B5UnormPack16";
    case Format::eB5G6R5UnormPack16: return "B5G6R5UnormPack16";
    case Format::eR5G5B5A1UnormPack16: return "R5G5B5A1UnormPack16";
    case Format::eB5G5R5A1UnormPack16: return "B5G5R5A1UnormPack16";
    case Format::eA1R5G5B5UnormPack16: return "A1R5G5B5UnormPack16";
    case Format::eR8Unorm: return "R8Unorm";
    case Format::eR8Snorm: return "R8Snorm";
    case Format::eR8Uscaled: return "R8Uscaled";
    case Format::eR8Sscaled: return "R8Sscaled";
    case Format::eR8Uint: return "R8Uint";
    case Format::eR8Sint: return "R8Sint";
    case Format::eR8Srgb: return "R8Srgb";
    case Format::eR8G8Unorm: return "R8G8Unorm";
    case Format::eR8G8Snorm: return "R8G8Snorm";
    case Format::eR8G8Uscaled: return "R8G8Uscaled";
    case Format::eR8G8Sscaled: return "R8G8Sscaled";
    case Format::eR8G8Uint: return "R8G8Uint";
    case Format::eR8G8Sint: return "R8G8Sint";
    case Format::eR8G8Srgb: return "R8G8Srgb";
    case Format::eR8G8B8Unorm: return "R8G8B8Unorm";
    case Format::eR8G8B8Snorm: return "R8G8B8Snorm";
    case Format::eR8G8B8Uscaled: return "R8G8B8Uscaled";
    case Format::eR8G8B8Sscaled: return "R8G8B8Sscaled";
    case Format::eR8G8B8Uint: return "R8G8B8Uint";
    case Format::eR8G8B8Sint: return "R8G8B8Sint";
    case Format::eR8G8B8Srgb: return "R8G8B8Srgb";
    case Format::eB8G8R8Unorm: return "B8G8R8Unorm";
    case Format::eB8G8R8Snorm: return "B8G8R8Snorm";
    case Format::eB8G8R8Uscaled: return "B8G8R8Uscaled";
    case Format::eB8G8R8Sscaled: return "B8G8R8Sscaled";
    case Format::eB8G8R8Uint: return "B8G8R8Uint";
    case Format::eB8G8R8Sint: return "B8G8R8Sint";
    case Format::eB8G8R8Srgb: return "B8G8R8Srgb";
    case Format::eR8G8B8A8Unorm: return "R8G8B8A8Unorm";
    case Format::eR8G8B8A8Snorm: return "R8G8B8A8Snorm";
    case Format::eR8G8B8A8Uscaled: return "R8G8B8A8Uscaled";
    case Format::eR8G8B8A8Sscaled: return "R8G8B8A8Sscaled";
    case Format::eR8G8B8A8Uint: return "R8G8B8A8Uint";
    case Format::eR8G8B8A8Sint: return "R8G8B8A8Sint";
    case Format::eR8G8B8A8Srgb: return "R8G8B8A8Srgb";
    case Format::eB8G8R8A8Unorm: return "B8G8R8A8Unorm";
    case Format::eB8G8R8A8Snorm: return "B8G8R8A8Snorm";
    case Format::eB8G8R8A8Uscaled: return "B8G8R8A8Uscaled";
    case Format::eB8G8R8A8Sscaled: return "B8G8R8A8Sscaled";
    case Format::eB8G8R8A8Uint: return "B8G8R8A8Uint";
    case Format::eB8G8R8A8Sint: return "B8G8R8A8Sint";
    case Format::eB8G8R8A8Srgb: return "B8G8R8A8Srgb";
    case Format::eA8B8G8R8UnormPack32: return "A8B8G8R8UnormPack32";
    case Format::eA8B8G8R8SnormPack32: return "A8B8G8R8SnormPack32";
    case Format::eA8B8G8R8UscaledPack32: return "A8B8G8R8UscaledPack32";
    case Format::eA8B8G8R8SscaledPack32: return "A8B8G8R8SscaledPack32";
    case Format::eA8B8G8R8UintPack32: return "A8B8G8R8UintPack32";
    case Format::eA8B8G8R8SintPack32: return "A8B8G8R8SintPack32";
    case Format::eA8B8G8R8SrgbPack32: return "A8B8G8R8SrgbPack32";
    case Format::eA2R10G10B10UnormPack32: return "A2R10G10B10UnormPack32";
    case Format::eA2R10G10B10SnormPack32: return "A2R10G10B10SnormPack32";
    case Format::eA2R10G10B10UscaledPack32: return "A2R10G10B10UscaledPack32";
    case Format::eA2R10G10B10SscaledPack32: return "A2R10G10B10SscaledPack32";
    case Format::eA2R10G10B10UintPack32: return "A2R10G10B10UintPack32";
    case Format::eA2R10G10B10SintPack32: return "A2R10G10B10SintPack32";
    case Format::eA2B10G10R10UnormPack32: return "A2B10G10R10UnormPack32";
    case Format::eA2B10G10R10SnormPack32: return "A2B10G10R10SnormPack32";
    case Format::eA2B10G10R10UscaledPack32: return "A2B10G10R10UscaledPack32";
    case Format::eA2B10G10R10SscaledPack32: return "A2B10G10R10SscaledPack32";
    case Format::eA2B10G10R10UintPack32: return "A2B10G10R10UintPack32";
    case Format::eA2B10G10R10SintPack32: return "A2B10G10R10SintPack32";
    case Format::eR16Unorm: return "R16Unorm";
    case Format::eR16Snorm: return "R16Snorm";
    case Format::eR16Uscaled: return "R16Uscaled";
    case Format::eR16Sscaled: return "R16Sscaled";
    case Format::eR16Uint: return "R16Uint";
    case Format::eR16Sint: return "R16Sint";
    case Format::eR16Sfloat: return "R16Sfloat";
    case Format::eR16G16Unorm: return "R16G16Unorm";
    case Format::eR16G16Snorm: return "R16G16Snorm";
    case Format::eR16G16Uscaled: return "R16G16Uscaled";
    case Format::eR16G16Sscaled: return "R16G16Sscaled";
    case Format::eR16G16Uint: return "R16G16Uint";
    case Format::eR16G16Sint: return "R16G16Sint";
    case Format::eR16G16Sfloat: return "R16G16Sfloat";
    case Format::eR16G16B16Unorm: return "R16G16B16Unorm";
    case Format::eR16G16B16Snorm: return "R16G16B16Snorm";
    case Format::eR16G16B16Uscaled: return "R16G16B16Uscaled";
    case Format::eR16G16B16Sscaled: return "R16G16B16Sscaled";
    case Format::eR16G16B16Uint: return "R16G16B16Uint";
    case Format::eR16G16B16Sint: return "R16G16B16Sint";
    case Format::eR16G16B16Sfloat: return "R16G16B16Sfloat";
    case Format::eR16G16B16A16Unorm: return "R16G16B16A16Unorm";
    case Format::eR16G16B16A16Snorm: return "R16G16B16A16Snorm";
    case Format::eR16G16B16A16Uscaled: return "R16G16B16A16Uscaled";
    case Format::eR16G16B16A16Sscaled: return "R16G16B16A16Sscaled";
    case Format::eR16G16B16A16Uint: return "R16G16B16A16Uint";
    case Format::eR16G16B16A16Sint: return "R16G16B16A16Sint";
    case Format::eR16G16B16A16Sfloat: return "R16G16B16A16Sfloat";
    case Format::eR32Uint: return "R32Uint";
    case Format::eR32Sint: return "R32Sint";
    case Format::eR32Sfloat: return "R32Sfloat";
    case Format::eR32G32Uint: return "R32G32Uint";
    case Format::eR32G32Sint: return "R32G32Sint";
    case Format::eR32G32Sfloat: return "R32G32Sfloat";
    case Format::eR32G32B32Uint: return "R32G32B32Uint";
    case Format::eR32G32B32Sint: return "R32G32B32Sint";
    case Format::eR32G32B32Sfloat: return "R32G32B32Sfloat";
    case Format::eR32G32B32A32Uint: return "R32G32B32A32Uint";
    case Format::eR32G32B32A32Sint: return "R32G32B32A32Sint";
    case Format::eR32G32B32A32Sfloat: return "R32G32B32A32Sfloat";
    case Format::eR64Uint: return "R64Uint";
    case Format::eR64Sint: return "R64Sint";
    case Format::eR64Sfloat: return "R64Sfloat";
    case Format::eR64G64Uint: return "R64G64Uint";
    case Format::eR64G64Sint: return "R64G64Sint";
    case Format::eR64G64Sfloat: return "R64G64Sfloat";
    case Format::eR64G64B64Uint: return "R64G64B64Uint";
    case Format::eR64G64B64Sint: return "R64G64B64Sint";
    case Format::eR64G64B64Sfloat: return "R64G64B64Sfloat";
    case Format::eR64G64B64A64Uint: return "R64G64B64A64Uint";
    case Format::eR64G64B64A64Sint: return "R64G64B64A64Sint";
    case Format::eR64G64B64A64Sfloat: return "R64G64B64A64Sfloat";
    case Format::eB10G11R11UfloatPack32: return "B10G11R11UfloatPack32";
    case Format::eE5B9G9R9UfloatPack32: return "E5B9G9R9UfloatPack32";
    case Format::eD16Unorm: return "D16Unorm";
    case Format::eX8D24UnormPack32: return "X8D24UnormPack32";
    case Format::eD32Sfloat: return "D32Sfloat";
    case Format::eS8Uint: return "S8Uint";
    case Format::eD16UnormS8Uint: return "D16UnormS8Uint";
    case Format::eD24UnormS8Uint: return "D24UnormS8Uint";
    case Format::eD32SfloatS8Uint: return "D32SfloatS8Uint";
    case Format::eBc1RgbUnormBlock: return "Bc1RgbUnormBlock";
    case Format::eBc1RgbSrgbBlock: return "Bc1RgbSrgbBlock";
    case Format::eBc1RgbaUnormBlock: return "Bc1RgbaUnormBlock";
    case Format::eBc1RgbaSrgbBlock: return "Bc1RgbaSrgbBlock";
    case Format::eBc2UnormBlock: return "Bc2UnormBlock";
    case Format::eBc2SrgbBlock: return "Bc2SrgbBlock";
    case Format::eBc3UnormBlock: return "Bc3UnormBlock";
    case Format::eBc3SrgbBlock: return "Bc3SrgbBlock";
    case Format::eBc4UnormBlock: return "Bc4UnormBlock";
    case Format::eBc4SnormBlock: return "Bc4SnormBlock";
    case Format::eBc5UnormBlock: return "Bc5UnormBlock";
    case Format::eBc5SnormBlock: return "Bc5SnormBlock";
    case Format::eBc6hUfloatBlock: return "Bc6hUfloatBlock";
    case Format::eBc6hSfloatBlock: return "Bc6hSfloatBlock";
    case Format::eBc7UnormBlock: return "Bc7UnormBlock";
    case Format::eBc7SrgbBlock: return "Bc7SrgbBlock";
    case Format::eEtc2R8G8B8UnormBlock: return "Etc2R8G8B8UnormBlock";
    case Format::eEtc2R8G8B8SrgbBlock: return "Etc2R8G8B8SrgbBlock";
    case Format::eEtc2R8G8B8A1UnormBlock: return "Etc2R8G8B8A1UnormBlock";
    case Format::eEtc2R8G8B8A1SrgbBlock: return "Etc2R8G8B8A1SrgbBlock";
    case Format::eEtc2R8G8B8A8UnormBlock: return "Etc2R8G8B8A8UnormBlock";
    case Format::eEtc2R8G8B8A8SrgbBlock: return "Etc2R8G8B8A8SrgbBlock";
    case Format::eEacR11UnormBlock: return "EacR11UnormBlock";
    case Format::eEacR11SnormBlock: return "EacR11SnormBlock";
    case Format::eEacR11G11UnormBlock: return "EacR11G11UnormBlock";
    case Format::eEacR11G11SnormBlock: return "EacR11G11SnormBlock";
    case Format::eAstc4x4UnormBlock: return "Astc4x4UnormBlock";
    case Format::eAstc4x4SrgbBlock: return "Astc4x4SrgbBlock";
    case Format::eAstc5x4UnormBlock: return "Astc5x4UnormBlock";
    case Format::eAstc5x4SrgbBlock: return "Astc5x4SrgbBlock";
    case Format::eAstc5x5UnormBlock: return "Astc5x5UnormBlock";
    case Format::eAstc5x5SrgbBlock: return "Astc5x5SrgbBlock";
    case Format::eAstc6x5UnormBlock: return "Astc6x5UnormBlock";
    case Format::eAstc6x5SrgbBlock: return "Astc6x5SrgbBlock";
    case Format::eAstc6x6UnormBlock: return "Astc6x6UnormBlock";
    case Format::eAstc6x6SrgbBlock: return "Astc6x6SrgbBlock";
    case Format::eAstc8x5UnormBlock: return "Astc8x5UnormBlock";
    case Format::eAstc8x5SrgbBlock: return "Astc8x5SrgbBlock";
    case Format::eAstc8x6UnormBlock: return "Astc8x6UnormBlock";
    case Format::eAstc8x6SrgbBlock: return "Astc8x6SrgbBlock";
    case Format::eAstc8x8UnormBlock: return "Astc8x8UnormBlock";
    case Format::eAstc8x8SrgbBlock: return "Astc8x8SrgbBlock";
    case Format::eAstc10x5UnormBlock: return "Astc10x5UnormBlock";
    case Format::eAstc10x5SrgbBlock: return "Astc10x5SrgbBlock";
    case Format::eAstc10x6UnormBlock: return "Astc10x6UnormBlock";
    case Format::eAstc10x6SrgbBlock: return "Astc10x6SrgbBlock";
    case Format::eAstc10x8UnormBlock: return "Astc10x8UnormBlock";
    case Format::eAstc10x8SrgbBlock: return "Astc10x8SrgbBlock";
    case Format::eAstc10x10UnormBlock: return "Astc10x10UnormBlock";
    case Format::eAstc10x10SrgbBlock: return "Astc10x10SrgbBlock";
    case Format::eAstc12x10UnormBlock: return "Astc12x10UnormBlock";
    case Format::eAstc12x10SrgbBlock: return "Astc12x10SrgbBlock";
    case Format::eAstc12x12UnormBlock: return "Astc12x12UnormBlock";
    case Format::eAstc12x12SrgbBlock: return "Astc12x12SrgbBlock";
    case Format::ePvrtc12BppUnormBlockIMG: return "Pvrtc12BppUnormBlockIMG";
    case Format::ePvrtc14BppUnormBlockIMG: return "Pvrtc14BppUnormBlockIMG";
    case Format::ePvrtc22BppUnormBlockIMG: return "Pvrtc22BppUnormBlockIMG";
    case Format::ePvrtc24BppUnormBlockIMG: return "Pvrtc24BppUnormBlockIMG";
    case Format::ePvrtc12BppSrgbBlockIMG: return "Pvrtc12BppSrgbBlockIMG";
    case Format::ePvrtc14BppSrgbBlockIMG: return "Pvrtc14BppSrgbBlockIMG";
    case Format::ePvrtc22BppSrgbBlockIMG: return "Pvrtc22BppSrgbBlockIMG";
    case Format::ePvrtc24BppSrgbBlockIMG: return "Pvrtc24BppSrgbBlockIMG";
    case Format::eAstc4x4SfloatBlock: return "Astc4x4SfloatBlock";
    case Format::eAstc5x4SfloatBlock: return "Astc5x4SfloatBlock";
    case Format::eAstc5x5SfloatBlock: return "Astc5x5SfloatBlock";
    case Format::eAstc6x5SfloatBlock: return "Astc6x5SfloatBlock";
    case Format::eAstc6x6SfloatBlock: return "Astc6x6SfloatBlock";
    case Format::eAstc8x5SfloatBlock: return "Astc8x5SfloatBlock";
    case Format::eAstc8x6SfloatBlock: return "Astc8x6SfloatBlock";
    case Format::eAstc8x8SfloatBlock: return "Astc8x8SfloatBlock";
    case Format::eAstc10x5SfloatBlock: return "Astc10x5SfloatBlock";
    case Format::eAstc10x6SfloatBlock: return "Astc10x6SfloatBlock";
    case Format::eAstc10x8SfloatBlock: return "Astc10x8SfloatBlock";
    case Format::eAstc10x10SfloatBlock: return "Astc10x10SfloatBlock";
    case Format::eAstc12x10SfloatBlock: return "Astc12x10SfloatBlock";
    case Format::eAstc12x12SfloatBlock: return "Astc12x12SfloatBlock";
    case Format::eG8B8G8R8422Unorm: return "G8B8G8R8422Unorm";
    case Format::eB8G8R8G8422Unorm: return "B8G8R8G8422Unorm";
    case Format::eG8B8R83Plane420Unorm: return "G8B8R83Plane420Unorm";
    case Format::eG8B8R82Plane420Unorm: return "G8B8R82Plane420Unorm";
    case Format::eG8B8R83Plane422Unorm: return "G8B8R83Plane422Unorm";
    case Format::eG8B8R82Plane422Unorm: return "G8B8R82Plane422Unorm";
    case Format::eG8B8R83Plane444Unorm: return "G8B8R83Plane444Unorm";
    case Format::eR10X6UnormPack16: return "R10X6UnormPack16";
    case Format::eR10X6G10X6Unorm2Pack16: return "R10X6G10X6Unorm2Pack16";
    case Format::eR10X6G10X6B10X6A10X6Unorm4Pack16: return "R10X6G10X6B10X6A10X6Unorm4Pack16";
    case Format::eG10X6B10X6G10X6R10X6422Unorm4Pack16: return "G10X6B10X6G10X6R10X6422Unorm4Pack16";
    case Format::eB10X6G10X6R10X6G10X6422Unorm4Pack16: return "B10X6G10X6R10X6G10X6422Unorm4Pack16";
    case Format::eG10X6B10X6R10X63Plane420Unorm3Pack16: return "G10X6B10X6R10X63Plane420Unorm3Pack16";
    case Format::eG10X6B10X6R10X62Plane420Unorm3Pack16: return "G10X6B10X6R10X62Plane420Unorm3Pack16";
    case Format::eG10X6B10X6R10X63Plane422Unorm3Pack16: return "G10X6B10X6R10X63Plane422Unorm3Pack16";
    case Format::eG10X6B10X6R10X62Plane422Unorm3Pack16: return "G10X6B10X6R10X62Plane422Unorm3Pack16";
    case Format::eG10X6B10X6R10X63Plane444Unorm3Pack16: return "G10X6B10X6R10X63Plane444Unorm3Pack16";
    case Format::eR12X4UnormPack16: return "R12X4UnormPack16";
    case Format::eR12X4G12X4Unorm2Pack16: return "R12X4G12X4Unorm2Pack16";
    case Format::eR12X4G12X4B12X4A12X4Unorm4Pack16: return "R12X4G12X4B12X4A12X4Unorm4Pack16";
    case Format::eG12X4B12X4G12X4R12X4422Unorm4Pack16: return "G12X4B12X4G12X4R12X4422Unorm4Pack16";
    case Format::eB12X4G12X4R12X4G12X4422Unorm4Pack16: return "B12X4G12X4R12X4G12X4422Unorm4Pack16";
    case Format::eG12X4B12X4R12X43Plane420Unorm3Pack16: return "G12X4B12X4R12X43Plane420Unorm3Pack16";
    case Format::eG12X4B12X4R12X42Plane420Unorm3Pack16: return "G12X4B12X4R12X42Plane420Unorm3Pack16";
    case Format::eG12X4B12X4R12X43Plane422Unorm3Pack16: return "G12X4B12X4R12X43Plane422Unorm3Pack16";
    case Format::eG12X4B12X4R12X42Plane422Unorm3Pack16: return "G12X4B12X4R12X42Plane422Unorm3Pack16";
    case Format::eG12X4B12X4R12X43Plane444Unorm3Pack16: return "G12X4B12X4R12X43Plane444Unorm3Pack16";
    case Format::eG16B16G16R16422Unorm: return "G16B16G16R16422Unorm";
    case Format::eB16G16R16G16422Unorm: return "B16G16R16G16422Unorm";
    case Format::eG16B16R163Plane420Unorm: return "G16B16R163Plane420Unorm";
    case Format::eG16B16R162Plane420Unorm: return "G16B16R162Plane420Unorm";
    case Format::eG16B16R163Plane422Unorm: return "G16B16R163Plane422Unorm";
    case Format::eG16B16R162Plane422Unorm: return "G16B16R162Plane422Unorm";
    case Format::eG16B16R163Plane444Unorm: return "G16B16R163Plane444Unorm";
    case Format::eG8B8R82Plane444Unorm: return "G8B8R82Plane444Unorm";
    case Format::eG10X6B10X6R10X62Plane444Unorm3Pack16: return "G10X6B10X6R10X62Plane444Unorm3Pack16";
    case Format::eG12X4B12X4R12X42Plane444Unorm3Pack16: return "G12X4B12X4R12X42Plane444Unorm3Pack16";
    case Format::eG16B16R162Plane444Unorm: return "G16B16R162Plane444Unorm";
    case Format::eA4R4G4B4UnormPack16: return "A4R4G4B4UnormPack16";
    case Format::eA4B4G4R4UnormPack16: return "A4B4G4R4UnormPack16";
    case Format::eR8BoolARM: return "R8BoolARM";
    case Format::eR16G16Sfixed5NV: return "R16G16Sfixed5NV";
    case Format::eA1B5G5R5UnormPack16: return "A1B5G5R5UnormPack16";
    case Format::eA8Unorm: return "A8Unorm";
    case Format::eR10X6UintPack16ARM: return "R10X6UintPack16ARM";
    case Format::eR10X6G10X6Uint2Pack16ARM: return "R10X6G10X6Uint2Pack16ARM";
    case Format::eR10X6G10X6B10X6A10X6Uint4Pack16ARM: return "R10X6G10X6B10X6A10X6Uint4Pack16ARM";
    case Format::eR12X4UintPack16ARM: return "R12X4UintPack16ARM";
    case Format::eR12X4G12X4Uint2Pack16ARM: return "R12X4G12X4Uint2Pack16ARM";
    case Format::eR12X4G12X4B12X4A12X4Uint4Pack16ARM: return "R12X4G12X4B12X4A12X4Uint4Pack16ARM";
    case Format::eR14X2UintPack16ARM: return "R14X2UintPack16ARM";
    case Format::eR14X2G14X2Uint2Pack16ARM: return "R14X2G14X2Uint2Pack16ARM";
    case Format::eR14X2G14X2B14X2A14X2Uint4Pack16ARM: return "R14X2G14X2B14X2A14X2Uint4Pack16ARM";
    case Format::eR14X2UnormPack16ARM: return "R14X2UnormPack16ARM";
    case Format::eR14X2G14X2Unorm2Pack16ARM: return "R14X2G14X2Unorm2Pack16ARM";
    case Format::eR14X2G14X2B14X2A14X2Unorm4Pack16ARM: return "R14X2G14X2B14X2A14X2Unorm4Pack16ARM";
    case Format::eG14X2B14X2R14X22Plane420Unorm3Pack16ARM: return "G14X2B14X2R14X22Plane420Unorm3Pack16ARM";
    case Format::eG14X2B14X2R14X22Plane422Unorm3Pack16ARM: return "G14X2B14X2R14X22Plane422Unorm3Pack16ARM";
    default: return "Unknown";
    }
}

const char* to_cstr(ImageTiling v) {
    switch (v) {
    case ImageTiling::eOptimal: return "Optimal";
    case ImageTiling::eLinear: return "Linear";
    case ImageTiling::eDrmFormatModifierEXT: return "DrmFormatModifierEXT";
    default: return "Unknown";
    }
}

const char* to_cstr(ImageType v) {
    switch (v) {
    case ImageType::e1D: return "1D";
    case ImageType::e2D: return "2D";
    case ImageType::e3D: return "3D";
    default: return "Unknown";
    }
}

const char* to_cstr(PhysicalDeviceType v) {
    switch (v) {
    case PhysicalDeviceType::eOther: return "Other";
    case PhysicalDeviceType::eIntegratedGpu: return "IntegratedGpu";
    case PhysicalDeviceType::eDiscreteGpu: return "DiscreteGpu";
    case PhysicalDeviceType::eVirtualGpu: return "VirtualGpu";
    case PhysicalDeviceType::eCpu: return "Cpu";
    default: return "Unknown";
    }
}

const char* to_cstr(QueryType v) {
    switch (v) {
    case QueryType::eOcclusion: return "Occlusion";
    case QueryType::ePipelineStatistics: return "PipelineStatistics";
    case QueryType::eTimestamp: return "Timestamp";
    case QueryType::eTransformFeedbackStreamEXT: return "TransformFeedbackStreamEXT";
    case QueryType::ePerformanceQueryKHR: return "PerformanceQueryKHR";
    case QueryType::eAccelerationStructureCompactedSizeKHR: return "AccelerationStructureCompactedSizeKHR";
    case QueryType::eAccelerationStructureSerializationSizeKHR: return "AccelerationStructureSerializationSizeKHR";
    case QueryType::eAccelerationStructureCompactedSizeNV: return "AccelerationStructureCompactedSizeNV";
    case QueryType::ePerformanceQueryINTEL: return "PerformanceQueryINTEL";
    case QueryType::eMeshPrimitivesGeneratedEXT: return "MeshPrimitivesGeneratedEXT";
    case QueryType::ePrimitivesGeneratedEXT: return "PrimitivesGeneratedEXT";
    case QueryType::eAccelerationStructureSerializationBottomLevelPointersKHR: return "AccelerationStructureSerializationBottomLevelPointersKHR";
    case QueryType::eAccelerationStructureSizeKHR: return "AccelerationStructureSizeKHR";
    case QueryType::eMicromapSerializationSizeEXT: return "MicromapSerializationSizeEXT";
    case QueryType::eMicromapCompactedSizeEXT: return "MicromapCompactedSizeEXT";
    default: return "Unknown";
    }
}

const char* to_cstr(SharingMode v) {
    switch (v) {
    case SharingMode::eExclusive: return "Exclusive";
    case SharingMode::eConcurrent: return "Concurrent";
    default: return "Unknown";
    }
}

const char* to_cstr(ComponentSwizzle v) {
    switch (v) {
    case ComponentSwizzle::eIdentity: return "Identity";
    case ComponentSwizzle::eZero: return "Zero";
    case ComponentSwizzle::eOne: return "One";
    case ComponentSwizzle::eR: return "R";
    case ComponentSwizzle::eG: return "G";
    case ComponentSwizzle::eB: return "B";
    case ComponentSwizzle::eA: return "A";
    default: return "Unknown";
    }
}

const char* to_cstr(ImageViewType v) {
    switch (v) {
    case ImageViewType::e1D: return "1D";
    case ImageViewType::e2D: return "2D";
    case ImageViewType::e3D: return "3D";
    case ImageViewType::eCube: return "Cube";
    case ImageViewType::e1DArray: return "1DArray";
    case ImageViewType::e2DArray: return "2DArray";
    case ImageViewType::eCubeArray: return "CubeArray";
    default: return "Unknown";
    }
}

const char* to_cstr(BlendFactor v) {
    switch (v) {
    case BlendFactor::eZero: return "Zero";
    case BlendFactor::eOne: return "One";
    case BlendFactor::eSrcColor: return "SrcColor";
    case BlendFactor::eOneMinusSrcColor: return "OneMinusSrcColor";
    case BlendFactor::eDstColor: return "DstColor";
    case BlendFactor::eOneMinusDstColor: return "OneMinusDstColor";
    case BlendFactor::eSrcAlpha: return "SrcAlpha";
    case BlendFactor::eOneMinusSrcAlpha: return "OneMinusSrcAlpha";
    case BlendFactor::eDstAlpha: return "DstAlpha";
    case BlendFactor::eOneMinusDstAlpha: return "OneMinusDstAlpha";
    case BlendFactor::eConstantColor: return "ConstantColor";
    case BlendFactor::eOneMinusConstantColor: return "OneMinusConstantColor";
    case BlendFactor::eConstantAlpha: return "ConstantAlpha";
    case BlendFactor::eOneMinusConstantAlpha: return "OneMinusConstantAlpha";
    case BlendFactor::eSrcAlphaSaturate: return "SrcAlphaSaturate";
    case BlendFactor::eSrc1Color: return "Src1Color";
    case BlendFactor::eOneMinusSrc1Color: return "OneMinusSrc1Color";
    case BlendFactor::eSrc1Alpha: return "Src1Alpha";
    case BlendFactor::eOneMinusSrc1Alpha: return "OneMinusSrc1Alpha";
    default: return "Unknown";
    }
}

const char* to_cstr(BlendOp v) {
    switch (v) {
    case BlendOp::eAdd: return "Add";
    case BlendOp::eSubtract: return "Subtract";
    case BlendOp::eReverseSubtract: return "ReverseSubtract";
    case BlendOp::eMin: return "Min";
    case BlendOp::eMax: return "Max";
    case BlendOp::eZeroEXT: return "ZeroEXT";
    case BlendOp::eSrcEXT: return "SrcEXT";
    case BlendOp::eDstEXT: return "DstEXT";
    case BlendOp::eSrcOverEXT: return "SrcOverEXT";
    case BlendOp::eDstOverEXT: return "DstOverEXT";
    case BlendOp::eSrcInEXT: return "SrcInEXT";
    case BlendOp::eDstInEXT: return "DstInEXT";
    case BlendOp::eSrcOutEXT: return "SrcOutEXT";
    case BlendOp::eDstOutEXT: return "DstOutEXT";
    case BlendOp::eSrcAtopEXT: return "SrcAtopEXT";
    case BlendOp::eDstAtopEXT: return "DstAtopEXT";
    case BlendOp::eXorEXT: return "XorEXT";
    case BlendOp::eMultiplyEXT: return "MultiplyEXT";
    case BlendOp::eScreenEXT: return "ScreenEXT";
    case BlendOp::eOverlayEXT: return "OverlayEXT";
    case BlendOp::eDarkenEXT: return "DarkenEXT";
    case BlendOp::eLightenEXT: return "LightenEXT";
    case BlendOp::eColordodgeEXT: return "ColordodgeEXT";
    case BlendOp::eColorburnEXT: return "ColorburnEXT";
    case BlendOp::eHardlightEXT: return "HardlightEXT";
    case BlendOp::eSoftlightEXT: return "SoftlightEXT";
    case BlendOp::eDifferenceEXT: return "DifferenceEXT";
    case BlendOp::eExclusionEXT: return "ExclusionEXT";
    case BlendOp::eInvertEXT: return "InvertEXT";
    case BlendOp::eInvertRgbEXT: return "InvertRgbEXT";
    case BlendOp::eLineardodgeEXT: return "LineardodgeEXT";
    case BlendOp::eLinearburnEXT: return "LinearburnEXT";
    case BlendOp::eVividlightEXT: return "VividlightEXT";
    case BlendOp::eLinearlightEXT: return "LinearlightEXT";
    case BlendOp::ePinlightEXT: return "PinlightEXT";
    case BlendOp::eHardmixEXT: return "HardmixEXT";
    case BlendOp::eHslHueEXT: return "HslHueEXT";
    case BlendOp::eHslSaturationEXT: return "HslSaturationEXT";
    case BlendOp::eHslColorEXT: return "HslColorEXT";
    case BlendOp::eHslLuminosityEXT: return "HslLuminosityEXT";
    case BlendOp::ePlusEXT: return "PlusEXT";
    case BlendOp::ePlusClampedEXT: return "PlusClampedEXT";
    case BlendOp::ePlusClampedAlphaEXT: return "PlusClampedAlphaEXT";
    case BlendOp::ePlusDarkerEXT: return "PlusDarkerEXT";
    case BlendOp::eMinusEXT: return "MinusEXT";
    case BlendOp::eMinusClampedEXT: return "MinusClampedEXT";
    case BlendOp::eContrastEXT: return "ContrastEXT";
    case BlendOp::eInvertOvgEXT: return "InvertOvgEXT";
    case BlendOp::eRedEXT: return "RedEXT";
    case BlendOp::eGreenEXT: return "GreenEXT";
    case BlendOp::eBlueEXT: return "BlueEXT";
    default: return "Unknown";
    }
}

const char* to_cstr(CompareOp v) {
    switch (v) {
    case CompareOp::eNever: return "Never";
    case CompareOp::eLess: return "Less";
    case CompareOp::eEqual: return "Equal";
    case CompareOp::eLessOrEqual: return "LessOrEqual";
    case CompareOp::eGreater: return "Greater";
    case CompareOp::eNotEqual: return "NotEqual";
    case CompareOp::eGreaterOrEqual: return "GreaterOrEqual";
    case CompareOp::eAlways: return "Always";
    default: return "Unknown";
    }
}

const char* to_cstr(DynamicState v) {
    switch (v) {
    case DynamicState::eViewport: return "Viewport";
    case DynamicState::eScissor: return "Scissor";
    case DynamicState::eLineWidth: return "LineWidth";
    case DynamicState::eDepthBias: return "DepthBias";
    case DynamicState::eBlendConstants: return "BlendConstants";
    case DynamicState::eDepthBounds: return "DepthBounds";
    case DynamicState::eStencilCompareMask: return "StencilCompareMask";
    case DynamicState::eStencilWriteMask: return "StencilWriteMask";
    case DynamicState::eStencilReference: return "StencilReference";
    case DynamicState::eViewportWScalingNV: return "ViewportWScalingNV";
    case DynamicState::eDiscardRectangleEXT: return "DiscardRectangleEXT";
    case DynamicState::eDiscardRectangleEnableEXT: return "DiscardRectangleEnableEXT";
    case DynamicState::eDiscardRectangleModeEXT: return "DiscardRectangleModeEXT";
    case DynamicState::eSampleLocationsEXT: return "SampleLocationsEXT";
    case DynamicState::eViewportShadingRatePaletteNV: return "ViewportShadingRatePaletteNV";
    case DynamicState::eViewportCoarseSampleOrderNV: return "ViewportCoarseSampleOrderNV";
    case DynamicState::eExclusiveScissorEnableNV: return "ExclusiveScissorEnableNV";
    case DynamicState::eExclusiveScissorNV: return "ExclusiveScissorNV";
    case DynamicState::eFragmentShadingRateKHR: return "FragmentShadingRateKHR";
    case DynamicState::eLineStipple: return "LineStipple";
    case DynamicState::eCullMode: return "CullMode";
    case DynamicState::eFrontFace: return "FrontFace";
    case DynamicState::ePrimitiveTopology: return "PrimitiveTopology";
    case DynamicState::eViewportWithCount: return "ViewportWithCount";
    case DynamicState::eScissorWithCount: return "ScissorWithCount";
    case DynamicState::eVertexInputBindingStride: return "VertexInputBindingStride";
    case DynamicState::eDepthTestEnable: return "DepthTestEnable";
    case DynamicState::eDepthWriteEnable: return "DepthWriteEnable";
    case DynamicState::eDepthCompareOp: return "DepthCompareOp";
    case DynamicState::eDepthBoundsTestEnable: return "DepthBoundsTestEnable";
    case DynamicState::eStencilTestEnable: return "StencilTestEnable";
    case DynamicState::eStencilOp: return "StencilOp";
    case DynamicState::eRayTracingPipelineStackSizeKHR: return "RayTracingPipelineStackSizeKHR";
    case DynamicState::eVertexInputEXT: return "VertexInputEXT";
    case DynamicState::eRasterizerDiscardEnable: return "RasterizerDiscardEnable";
    case DynamicState::eDepthBiasEnable: return "DepthBiasEnable";
    case DynamicState::ePrimitiveRestartEnable: return "PrimitiveRestartEnable";
    case DynamicState::ePatchControlPointsEXT: return "PatchControlPointsEXT";
    case DynamicState::eLogicOpEXT: return "LogicOpEXT";
    case DynamicState::eColorWriteEnableEXT: return "ColorWriteEnableEXT";
    case DynamicState::eDepthClampEnableEXT: return "DepthClampEnableEXT";
    case DynamicState::ePolygonModeEXT: return "PolygonModeEXT";
    case DynamicState::eRasterizationSamplesEXT: return "RasterizationSamplesEXT";
    case DynamicState::eSampleMaskEXT: return "SampleMaskEXT";
    case DynamicState::eAlphaToCoverageEnableEXT: return "AlphaToCoverageEnableEXT";
    case DynamicState::eAlphaToOneEnableEXT: return "AlphaToOneEnableEXT";
    case DynamicState::eLogicOpEnableEXT: return "LogicOpEnableEXT";
    case DynamicState::eColorBlendEnableEXT: return "ColorBlendEnableEXT";
    case DynamicState::eColorBlendEquationEXT: return "ColorBlendEquationEXT";
    case DynamicState::eColorWriteMaskEXT: return "ColorWriteMaskEXT";
    case DynamicState::eTessellationDomainOriginEXT: return "TessellationDomainOriginEXT";
    case DynamicState::eRasterizationStreamEXT: return "RasterizationStreamEXT";
    case DynamicState::eConservativeRasterizationModeEXT: return "ConservativeRasterizationModeEXT";
    case DynamicState::eExtraPrimitiveOverestimationSizeEXT: return "ExtraPrimitiveOverestimationSizeEXT";
    case DynamicState::eDepthClipEnableEXT: return "DepthClipEnableEXT";
    case DynamicState::eSampleLocationsEnableEXT: return "SampleLocationsEnableEXT";
    case DynamicState::eColorBlendAdvancedEXT: return "ColorBlendAdvancedEXT";
    case DynamicState::eProvokingVertexModeEXT: return "ProvokingVertexModeEXT";
    case DynamicState::eLineRasterizationModeEXT: return "LineRasterizationModeEXT";
    case DynamicState::eLineStippleEnableEXT: return "LineStippleEnableEXT";
    case DynamicState::eDepthClipNegativeOneToOneEXT: return "DepthClipNegativeOneToOneEXT";
    case DynamicState::eViewportWScalingEnableNV: return "ViewportWScalingEnableNV";
    case DynamicState::eViewportSwizzleNV: return "ViewportSwizzleNV";
    case DynamicState::eCoverageToColorEnableNV: return "CoverageToColorEnableNV";
    case DynamicState::eCoverageToColorLocationNV: return "CoverageToColorLocationNV";
    case DynamicState::eCoverageModulationModeNV: return "CoverageModulationModeNV";
    case DynamicState::eCoverageModulationTableEnableNV: return "CoverageModulationTableEnableNV";
    case DynamicState::eCoverageModulationTableNV: return "CoverageModulationTableNV";
    case DynamicState::eShadingRateImageEnableNV: return "ShadingRateImageEnableNV";
    case DynamicState::eRepresentativeFragmentTestEnableNV: return "RepresentativeFragmentTestEnableNV";
    case DynamicState::eCoverageReductionModeNV: return "CoverageReductionModeNV";
    case DynamicState::eAttachmentFeedbackLoopEnableEXT: return "AttachmentFeedbackLoopEnableEXT";
    case DynamicState::eDepthClampRangeEXT: return "DepthClampRangeEXT";
    default: return "Unknown";
    }
}

const char* to_cstr(FrontFace v) {
    switch (v) {
    case FrontFace::eCounterClockwise: return "CounterClockwise";
    case FrontFace::eClockwise: return "Clockwise";
    default: return "Unknown";
    }
}

const char* to_cstr(VertexInputRate v) {
    switch (v) {
    case VertexInputRate::eVertex: return "Vertex";
    case VertexInputRate::eInstance: return "Instance";
    default: return "Unknown";
    }
}

const char* to_cstr(PrimitiveTopology v) {
    switch (v) {
    case PrimitiveTopology::ePointList: return "PointList";
    case PrimitiveTopology::eLineList: return "LineList";
    case PrimitiveTopology::eLineStrip: return "LineStrip";
    case PrimitiveTopology::eTriangleList: return "TriangleList";
    case PrimitiveTopology::eTriangleStrip: return "TriangleStrip";
    case PrimitiveTopology::eTriangleFan: return "TriangleFan";
    case PrimitiveTopology::eLineListWithAdjacency: return "LineListWithAdjacency";
    case PrimitiveTopology::eLineStripWithAdjacency: return "LineStripWithAdjacency";
    case PrimitiveTopology::eTriangleListWithAdjacency: return "TriangleListWithAdjacency";
    case PrimitiveTopology::eTriangleStripWithAdjacency: return "TriangleStripWithAdjacency";
    case PrimitiveTopology::ePatchList: return "PatchList";
    default: return "Unknown";
    }
}

const char* to_cstr(PolygonMode v) {
    switch (v) {
    case PolygonMode::eFill: return "Fill";
    case PolygonMode::eLine: return "Line";
    case PolygonMode::ePoint: return "Point";
    case PolygonMode::eFillRectangleNV: return "FillRectangleNV";
    default: return "Unknown";
    }
}

const char* to_cstr(StencilOp v) {
    switch (v) {
    case StencilOp::eKeep: return "Keep";
    case StencilOp::eZero: return "Zero";
    case StencilOp::eReplace: return "Replace";
    case StencilOp::eIncrementAndClamp: return "IncrementAndClamp";
    case StencilOp::eDecrementAndClamp: return "DecrementAndClamp";
    case StencilOp::eInvert: return "Invert";
    case StencilOp::eIncrementAndWrap: return "IncrementAndWrap";
    case StencilOp::eDecrementAndWrap: return "DecrementAndWrap";
    default: return "Unknown";
    }
}

const char* to_cstr(LogicOp v) {
    switch (v) {
    case LogicOp::eClear: return "Clear";
    case LogicOp::eAnd: return "And";
    case LogicOp::eAndReverse: return "AndReverse";
    case LogicOp::eCopy: return "Copy";
    case LogicOp::eAndInverted: return "AndInverted";
    case LogicOp::eNoOp: return "NoOp";
    case LogicOp::eXor: return "Xor";
    case LogicOp::eOr: return "Or";
    case LogicOp::eNor: return "Nor";
    case LogicOp::eEquivalent: return "Equivalent";
    case LogicOp::eInvert: return "Invert";
    case LogicOp::eOrReverse: return "OrReverse";
    case LogicOp::eCopyInverted: return "CopyInverted";
    case LogicOp::eOrInverted: return "OrInverted";
    case LogicOp::eNand: return "Nand";
    case LogicOp::eSet: return "Set";
    default: return "Unknown";
    }
}

const char* to_cstr(BorderColor v) {
    switch (v) {
    case BorderColor::eFloatTransparentBlack: return "FloatTransparentBlack";
    case BorderColor::eIntTransparentBlack: return "IntTransparentBlack";
    case BorderColor::eFloatOpaqueBlack: return "FloatOpaqueBlack";
    case BorderColor::eIntOpaqueBlack: return "IntOpaqueBlack";
    case BorderColor::eFloatOpaqueWhite: return "FloatOpaqueWhite";
    case BorderColor::eIntOpaqueWhite: return "IntOpaqueWhite";
    case BorderColor::eFloatCustomEXT: return "FloatCustomEXT";
    case BorderColor::eIntCustomEXT: return "IntCustomEXT";
    default: return "Unknown";
    }
}

const char* to_cstr(Filter v) {
    switch (v) {
    case Filter::eNearest: return "Nearest";
    case Filter::eLinear: return "Linear";
    case Filter::eCubicEXT: return "CubicEXT";
    default: return "Unknown";
    }
}

const char* to_cstr(SamplerAddressMode v) {
    switch (v) {
    case SamplerAddressMode::eRepeat: return "Repeat";
    case SamplerAddressMode::eMirroredRepeat: return "MirroredRepeat";
    case SamplerAddressMode::eClampToEdge: return "ClampToEdge";
    case SamplerAddressMode::eClampToBorder: return "ClampToBorder";
    case SamplerAddressMode::eMirrorClampToEdge: return "MirrorClampToEdge";
    default: return "Unknown";
    }
}

const char* to_cstr(SamplerMipmapMode v) {
    switch (v) {
    case SamplerMipmapMode::eNearest: return "Nearest";
    case SamplerMipmapMode::eLinear: return "Linear";
    default: return "Unknown";
    }
}

const char* to_cstr(DescriptorType v) {
    switch (v) {
    case DescriptorType::eSampler: return "Sampler";
    case DescriptorType::eCombinedImageSampler: return "CombinedImageSampler";
    case DescriptorType::eSampledImage: return "SampledImage";
    case DescriptorType::eStorageImage: return "StorageImage";
    case DescriptorType::eUniformTexelBuffer: return "UniformTexelBuffer";
    case DescriptorType::eStorageTexelBuffer: return "StorageTexelBuffer";
    case DescriptorType::eUniformBuffer: return "UniformBuffer";
    case DescriptorType::eStorageBuffer: return "StorageBuffer";
    case DescriptorType::eUniformBufferDynamic: return "UniformBufferDynamic";
    case DescriptorType::eStorageBufferDynamic: return "StorageBufferDynamic";
    case DescriptorType::eInputAttachment: return "InputAttachment";
    case DescriptorType::eInlineUniformBlock: return "InlineUniformBlock";
    case DescriptorType::eAccelerationStructureKHR: return "AccelerationStructureKHR";
    case DescriptorType::eAccelerationStructureNV: return "AccelerationStructureNV";
    case DescriptorType::eMutableEXT: return "MutableEXT";
    case DescriptorType::eSampleWeightImageQCOM: return "SampleWeightImageQCOM";
    case DescriptorType::eBlockMatchImageQCOM: return "BlockMatchImageQCOM";
    case DescriptorType::eTensorARM: return "TensorARM";
    case DescriptorType::ePartitionedAccelerationStructureNV: return "PartitionedAccelerationStructureNV";
    default: return "Unknown";
    }
}

const char* to_cstr(AttachmentLoadOp v) {
    switch (v) {
    case AttachmentLoadOp::eLoad: return "Load";
    case AttachmentLoadOp::eClear: return "Clear";
    case AttachmentLoadOp::eDontCare: return "DontCare";
    case AttachmentLoadOp::eNone: return "None";
    default: return "Unknown";
    }
}

const char* to_cstr(AttachmentStoreOp v) {
    switch (v) {
    case AttachmentStoreOp::eStore: return "Store";
    case AttachmentStoreOp::eDontCare: return "DontCare";
    case AttachmentStoreOp::eNone: return "None";
    default: return "Unknown";
    }
}

const char* to_cstr(PipelineBindPoint v) {
    switch (v) {
    case PipelineBindPoint::eGraphics: return "Graphics";
    case PipelineBindPoint::eCompute: return "Compute";
    case PipelineBindPoint::eRayTracingKHR: return "RayTracingKHR";
    case PipelineBindPoint::eSubpassShadingHUAWEI: return "SubpassShadingHUAWEI";
    case PipelineBindPoint::eDataGraphARM: return "DataGraphARM";
    case PipelineBindPoint::eExecutionGraphAMDX: return "ExecutionGraphAMDX";
    default: return "Unknown";
    }
}

const char* to_cstr(CommandBufferLevel v) {
    switch (v) {
    case CommandBufferLevel::ePrimary: return "Primary";
    case CommandBufferLevel::eSecondary: return "Secondary";
    default: return "Unknown";
    }
}

const char* to_cstr(IndexType v) {
    switch (v) {
    case IndexType::eUint16: return "Uint16";
    case IndexType::eUint32: return "Uint32";
    case IndexType::eNoneKHR: return "NoneKHR";
    case IndexType::eUint8: return "Uint8";
    default: return "Unknown";
    }
}

const char* to_cstr(SubpassContents v) {
    switch (v) {
    case SubpassContents::eInline: return "Inline";
    case SubpassContents::eSecondaryCommandBuffers: return "SecondaryCommandBuffers";
    case SubpassContents::eInlineAndSecondaryCommandBuffersKHR: return "InlineAndSecondaryCommandBuffersKHR";
    default: return "Unknown";
    }
}

const char* to_cstr(PointClippingBehavior v) {
    switch (v) {
    case PointClippingBehavior::eAllClipPlanes: return "AllClipPlanes";
    case PointClippingBehavior::eUserClipPlanesOnly: return "UserClipPlanesOnly";
    default: return "Unknown";
    }
}

const char* to_cstr(TessellationDomainOrigin v) {
    switch (v) {
    case TessellationDomainOrigin::eUpperLeft: return "UpperLeft";
    case TessellationDomainOrigin::eLowerLeft: return "LowerLeft";
    default: return "Unknown";
    }
}

const char* to_cstr(SamplerYcbcrModelConversion v) {
    switch (v) {
    case SamplerYcbcrModelConversion::eRgbIdentity: return "RgbIdentity";
    case SamplerYcbcrModelConversion::eYcbcrIdentity: return "YcbcrIdentity";
    case SamplerYcbcrModelConversion::eYcbcr709: return "Ycbcr709";
    case SamplerYcbcrModelConversion::eYcbcr601: return "Ycbcr601";
    case SamplerYcbcrModelConversion::eYcbcr2020: return "Ycbcr2020";
    default: return "Unknown";
    }
}

const char* to_cstr(SamplerYcbcrRange v) {
    switch (v) {
    case SamplerYcbcrRange::eItuFull: return "ItuFull";
    case SamplerYcbcrRange::eItuNarrow: return "ItuNarrow";
    default: return "Unknown";
    }
}

const char* to_cstr(ChromaLocation v) {
    switch (v) {
    case ChromaLocation::eCositedEven: return "CositedEven";
    case ChromaLocation::eMidpoint: return "Midpoint";
    default: return "Unknown";
    }
}

const char* to_cstr(DescriptorUpdateTemplateType v) {
    switch (v) {
    case DescriptorUpdateTemplateType::eDescriptorSet: return "DescriptorSet";
    case DescriptorUpdateTemplateType::ePushDescriptors: return "PushDescriptors";
    default: return "Unknown";
    }
}

const char* to_cstr(DriverId v) {
    switch (v) {
    case DriverId::eAmdProprietary: return "AmdProprietary";
    case DriverId::eAmdOpenSource: return "AmdOpenSource";
    case DriverId::eMesaRadv: return "MesaRadv";
    case DriverId::eNvidiaProprietary: return "NvidiaProprietary";
    case DriverId::eIntelProprietaryWindows: return "IntelProprietaryWindows";
    case DriverId::eIntelOpenSourceMESA: return "IntelOpenSourceMESA";
    case DriverId::eImaginationProprietary: return "ImaginationProprietary";
    case DriverId::eQualcommProprietary: return "QualcommProprietary";
    case DriverId::eArmProprietary: return "ArmProprietary";
    case DriverId::eGoogleSwiftshader: return "GoogleSwiftshader";
    case DriverId::eGgpProprietary: return "GgpProprietary";
    case DriverId::eBroadcomProprietary: return "BroadcomProprietary";
    case DriverId::eMesaLlvmpipe: return "MesaLlvmpipe";
    case DriverId::eMoltenvk: return "Moltenvk";
    case DriverId::eCoreaviProprietary: return "CoreaviProprietary";
    case DriverId::eJuiceProprietary: return "JuiceProprietary";
    case DriverId::eVerisiliconProprietary: return "VerisiliconProprietary";
    case DriverId::eMesaTurnip: return "MesaTurnip";
    case DriverId::eMesaV3dv: return "MesaV3dv";
    case DriverId::eMesaPanvk: return "MesaPanvk";
    case DriverId::eSamsungProprietary: return "SamsungProprietary";
    case DriverId::eMesaVenus: return "MesaVenus";
    case DriverId::eMesaDozen: return "MesaDozen";
    case DriverId::eMesaNvk: return "MesaNvk";
    case DriverId::eImaginationOpenSourceMESA: return "ImaginationOpenSourceMESA";
    case DriverId::eMesaHoneykrisp: return "MesaHoneykrisp";
    case DriverId::eVulkanScEmulationOnVulkan: return "VulkanScEmulationOnVulkan";
    default: return "Unknown";
    }
}

const char* to_cstr(ShaderFloatControlsIndependence v) {
    switch (v) {
    case ShaderFloatControlsIndependence::e32BitOnly: return "32BitOnly";
    case ShaderFloatControlsIndependence::eAll: return "All";
    case ShaderFloatControlsIndependence::eNone: return "None";
    default: return "Unknown";
    }
}

const char* to_cstr(SamplerReductionMode v) {
    switch (v) {
    case SamplerReductionMode::eWeightedAverage: return "WeightedAverage";
    case SamplerReductionMode::eMin: return "Min";
    case SamplerReductionMode::eMax: return "Max";
    case SamplerReductionMode::eWeightedAverageRangeclampQCOM: return "WeightedAverageRangeclampQCOM";
    default: return "Unknown";
    }
}

const char* to_cstr(SemaphoreType v) {
    switch (v) {
    case SemaphoreType::eBinary: return "Binary";
    case SemaphoreType::eTimeline: return "Timeline";
    default: return "Unknown";
    }
}

const char* to_cstr(PipelineRobustnessBufferBehavior v) {
    switch (v) {
    case PipelineRobustnessBufferBehavior::eDeviceDefault: return "DeviceDefault";
    case PipelineRobustnessBufferBehavior::eDisabled: return "Disabled";
    case PipelineRobustnessBufferBehavior::eRobustBufferAccess: return "RobustBufferAccess";
    case PipelineRobustnessBufferBehavior::eRobustBufferAccess2: return "RobustBufferAccess2";
    default: return "Unknown";
    }
}

const char* to_cstr(PipelineRobustnessImageBehavior v) {
    switch (v) {
    case PipelineRobustnessImageBehavior::eDeviceDefault: return "DeviceDefault";
    case PipelineRobustnessImageBehavior::eDisabled: return "Disabled";
    case PipelineRobustnessImageBehavior::eRobustImageAccess: return "RobustImageAccess";
    case PipelineRobustnessImageBehavior::eRobustImageAccess2: return "RobustImageAccess2";
    default: return "Unknown";
    }
}

const char* to_cstr(QueueGlobalPriority v) {
    switch (v) {
    case QueueGlobalPriority::eLow: return "Low";
    case QueueGlobalPriority::eMedium: return "Medium";
    case QueueGlobalPriority::eHigh: return "High";
    case QueueGlobalPriority::eRealtime: return "Realtime";
    default: return "Unknown";
    }
}

const char* to_cstr(LineRasterizationMode v) {
    switch (v) {
    case LineRasterizationMode::eDefault: return "Default";
    case LineRasterizationMode::eRectangular: return "Rectangular";
    case LineRasterizationMode::eBresenham: return "Bresenham";
    case LineRasterizationMode::eRectangularSmooth: return "RectangularSmooth";
    default: return "Unknown";
    }
}

const char* to_cstr(PresentModeKHR v) {
    switch (v) {
    case PresentModeKHR::eImmediate: return "Immediate";
    case PresentModeKHR::eMailbox: return "Mailbox";
    case PresentModeKHR::eFifo: return "Fifo";
    case PresentModeKHR::eFifoRelaxed: return "FifoRelaxed";
    case PresentModeKHR::eSharedDemandRefresh: return "SharedDemandRefresh";
    case PresentModeKHR::eSharedContinuousRefresh: return "SharedContinuousRefresh";
    case PresentModeKHR::eFifoLatestReady: return "FifoLatestReady";
    default: return "Unknown";
    }
}

const char* to_cstr(ColorSpaceKHR v) {
    switch (v) {
    case ColorSpaceKHR::eSrgbNonlinear: return "SrgbNonlinear";
    case ColorSpaceKHR::eDisplayP3NonlinearEXT: return "DisplayP3NonlinearEXT";
    case ColorSpaceKHR::eExtendedSrgbLinearEXT: return "ExtendedSrgbLinearEXT";
    case ColorSpaceKHR::eDisplayP3LinearEXT: return "DisplayP3LinearEXT";
    case ColorSpaceKHR::eDciP3NonlinearEXT: return "DciP3NonlinearEXT";
    case ColorSpaceKHR::eBt709LinearEXT: return "Bt709LinearEXT";
    case ColorSpaceKHR::eBt709NonlinearEXT: return "Bt709NonlinearEXT";
    case ColorSpaceKHR::eBt2020LinearEXT: return "Bt2020LinearEXT";
    case ColorSpaceKHR::eHdr10St2084EXT: return "Hdr10St2084EXT";
    case ColorSpaceKHR::eDolbyvisionEXT: return "DolbyvisionEXT";
    case ColorSpaceKHR::eHdr10HlgEXT: return "Hdr10HlgEXT";
    case ColorSpaceKHR::eAdobergbLinearEXT: return "AdobergbLinearEXT";
    case ColorSpaceKHR::eAdobergbNonlinearEXT: return "AdobergbNonlinearEXT";
    case ColorSpaceKHR::ePassThroughEXT: return "PassThroughEXT";
    case ColorSpaceKHR::eExtendedSrgbNonlinearEXT: return "ExtendedSrgbNonlinearEXT";
    case ColorSpaceKHR::eDisplayNativeAMD: return "DisplayNativeAMD";
    default: return "Unknown";
    }
}

const char* to_cstr(DebugReportObjectTypeEXT v) {
    switch (v) {
    case DebugReportObjectTypeEXT::eUnknown: return "Unknown";
    case DebugReportObjectTypeEXT::eInstance: return "Instance";
    case DebugReportObjectTypeEXT::ePhysicalDevice: return "PhysicalDevice";
    case DebugReportObjectTypeEXT::eDevice: return "Device";
    case DebugReportObjectTypeEXT::eQueue: return "Queue";
    case DebugReportObjectTypeEXT::eSemaphore: return "Semaphore";
    case DebugReportObjectTypeEXT::eCommandBuffer: return "CommandBuffer";
    case DebugReportObjectTypeEXT::eFence: return "Fence";
    case DebugReportObjectTypeEXT::eDeviceMemory: return "DeviceMemory";
    case DebugReportObjectTypeEXT::eBuffer: return "Buffer";
    case DebugReportObjectTypeEXT::eImage: return "Image";
    case DebugReportObjectTypeEXT::eEvent: return "Event";
    case DebugReportObjectTypeEXT::eQueryPool: return "QueryPool";
    case DebugReportObjectTypeEXT::eBufferView: return "BufferView";
    case DebugReportObjectTypeEXT::eImageView: return "ImageView";
    case DebugReportObjectTypeEXT::eShaderModule: return "ShaderModule";
    case DebugReportObjectTypeEXT::ePipelineCache: return "PipelineCache";
    case DebugReportObjectTypeEXT::ePipelineLayout: return "PipelineLayout";
    case DebugReportObjectTypeEXT::eRenderPass: return "RenderPass";
    case DebugReportObjectTypeEXT::ePipeline: return "Pipeline";
    case DebugReportObjectTypeEXT::eDescriptorSetLayout: return "DescriptorSetLayout";
    case DebugReportObjectTypeEXT::eSampler: return "Sampler";
    case DebugReportObjectTypeEXT::eDescriptorPool: return "DescriptorPool";
    case DebugReportObjectTypeEXT::eDescriptorSet: return "DescriptorSet";
    case DebugReportObjectTypeEXT::eFramebuffer: return "Framebuffer";
    case DebugReportObjectTypeEXT::eCommandPool: return "CommandPool";
    case DebugReportObjectTypeEXT::eSurfaceKHR: return "SurfaceKHR";
    case DebugReportObjectTypeEXT::eSwapchainKHR: return "SwapchainKHR";
    case DebugReportObjectTypeEXT::eDebugReportCallbackEXT: return "DebugReportCallbackEXT";
    case DebugReportObjectTypeEXT::eDisplayKHR: return "DisplayKHR";
    case DebugReportObjectTypeEXT::eDisplayModeKHR: return "DisplayModeKHR";
    case DebugReportObjectTypeEXT::eValidationCacheEXT: return "ValidationCacheEXT";
    case DebugReportObjectTypeEXT::eCuModuleNVX: return "CuModuleNVX";
    case DebugReportObjectTypeEXT::eCuFunctionNVX: return "CuFunctionNVX";
    case DebugReportObjectTypeEXT::eDescriptorUpdateTemplate: return "DescriptorUpdateTemplate";
    case DebugReportObjectTypeEXT::eAccelerationStructureKHR: return "AccelerationStructureKHR";
    case DebugReportObjectTypeEXT::eSamplerYcbcrConversion: return "SamplerYcbcrConversion";
    case DebugReportObjectTypeEXT::eAccelerationStructureNV: return "AccelerationStructureNV";
    case DebugReportObjectTypeEXT::eCudaModuleNV: return "CudaModuleNV";
    case DebugReportObjectTypeEXT::eCudaFunctionNV: return "CudaFunctionNV";
#if defined(VK_USE_PLATFORM_FUCHSIA)
    case DebugReportObjectTypeEXT::eBufferCollectionFUCHSIA: return "BufferCollectionFUCHSIA";
#endif // VK_USE_PLATFORM_FUCHSIA
    default: return "Unknown";
    }
}

const char* to_cstr(RasterizationOrderAMD v) {
    switch (v) {
    case RasterizationOrderAMD::eStrict: return "Strict";
    case RasterizationOrderAMD::eRelaxed: return "Relaxed";
    default: return "Unknown";
    }
}

const char* to_cstr(ShaderInfoTypeAMD v) {
    switch (v) {
    case ShaderInfoTypeAMD::eStatistics: return "Statistics";
    case ShaderInfoTypeAMD::eBinary: return "Binary";
    case ShaderInfoTypeAMD::eDisassembly: return "Disassembly";
    default: return "Unknown";
    }
}

const char* to_cstr(ValidationCheckEXT v) {
    switch (v) {
    case ValidationCheckEXT::eAll: return "All";
    case ValidationCheckEXT::eShaders: return "Shaders";
    default: return "Unknown";
    }
}

const char* to_cstr(DisplayPowerStateEXT v) {
    switch (v) {
    case DisplayPowerStateEXT::eOff: return "Off";
    case DisplayPowerStateEXT::eSuspend: return "Suspend";
    case DisplayPowerStateEXT::eOn: return "On";
    default: return "Unknown";
    }
}

const char* to_cstr(DeviceEventTypeEXT v) {
    switch (v) {
    case DeviceEventTypeEXT::eDisplayHotplug: return "DisplayHotplug";
    default: return "Unknown";
    }
}

const char* to_cstr(DisplayEventTypeEXT v) {
    switch (v) {
    case DisplayEventTypeEXT::eFirstPixelOut: return "FirstPixelOut";
    default: return "Unknown";
    }
}

const char* to_cstr(ViewportCoordinateSwizzleNV v) {
    switch (v) {
    case ViewportCoordinateSwizzleNV::ePositiveX: return "PositiveX";
    case ViewportCoordinateSwizzleNV::eNegativeX: return "NegativeX";
    case ViewportCoordinateSwizzleNV::ePositiveY: return "PositiveY";
    case ViewportCoordinateSwizzleNV::eNegativeY: return "NegativeY";
    case ViewportCoordinateSwizzleNV::ePositiveZ: return "PositiveZ";
    case ViewportCoordinateSwizzleNV::eNegativeZ: return "NegativeZ";
    case ViewportCoordinateSwizzleNV::ePositiveW: return "PositiveW";
    case ViewportCoordinateSwizzleNV::eNegativeW: return "NegativeW";
    default: return "Unknown";
    }
}

const char* to_cstr(DiscardRectangleModeEXT v) {
    switch (v) {
    case DiscardRectangleModeEXT::eInclusive: return "Inclusive";
    case DiscardRectangleModeEXT::eExclusive: return "Exclusive";
    default: return "Unknown";
    }
}

const char* to_cstr(ConservativeRasterizationModeEXT v) {
    switch (v) {
    case ConservativeRasterizationModeEXT::eDisabled: return "Disabled";
    case ConservativeRasterizationModeEXT::eOverestimate: return "Overestimate";
    case ConservativeRasterizationModeEXT::eUnderestimate: return "Underestimate";
    default: return "Unknown";
    }
}

const char* to_cstr(PerformanceCounterUnitKHR v) {
    switch (v) {
    case PerformanceCounterUnitKHR::eGeneric: return "Generic";
    case PerformanceCounterUnitKHR::ePercentage: return "Percentage";
    case PerformanceCounterUnitKHR::eNanoseconds: return "Nanoseconds";
    case PerformanceCounterUnitKHR::eBytes: return "Bytes";
    case PerformanceCounterUnitKHR::eBytesPerSecond: return "BytesPerSecond";
    case PerformanceCounterUnitKHR::eKelvin: return "Kelvin";
    case PerformanceCounterUnitKHR::eWatts: return "Watts";
    case PerformanceCounterUnitKHR::eVolts: return "Volts";
    case PerformanceCounterUnitKHR::eAmps: return "Amps";
    case PerformanceCounterUnitKHR::eHertz: return "Hertz";
    case PerformanceCounterUnitKHR::eCycles: return "Cycles";
    default: return "Unknown";
    }
}

const char* to_cstr(PerformanceCounterScopeKHR v) {
    switch (v) {
    case PerformanceCounterScopeKHR::eCommandBuffer: return "CommandBuffer";
    case PerformanceCounterScopeKHR::eRenderPass: return "RenderPass";
    case PerformanceCounterScopeKHR::eCommand: return "Command";
    default: return "Unknown";
    }
}

const char* to_cstr(PerformanceCounterStorageKHR v) {
    switch (v) {
    case PerformanceCounterStorageKHR::eInt32: return "Int32";
    case PerformanceCounterStorageKHR::eInt64: return "Int64";
    case PerformanceCounterStorageKHR::eUint32: return "Uint32";
    case PerformanceCounterStorageKHR::eUint64: return "Uint64";
    case PerformanceCounterStorageKHR::eFloat32: return "Float32";
    case PerformanceCounterStorageKHR::eFloat64: return "Float64";
    default: return "Unknown";
    }
}

const char* to_cstr(ComponentTypeKHR v) {
    switch (v) {
    case ComponentTypeKHR::eFloat16: return "Float16";
    case ComponentTypeKHR::eFloat32: return "Float32";
    case ComponentTypeKHR::eFloat64: return "Float64";
    case ComponentTypeKHR::eSint8: return "Sint8";
    case ComponentTypeKHR::eSint16: return "Sint16";
    case ComponentTypeKHR::eSint32: return "Sint32";
    case ComponentTypeKHR::eSint64: return "Sint64";
    case ComponentTypeKHR::eUint8: return "Uint8";
    case ComponentTypeKHR::eUint16: return "Uint16";
    case ComponentTypeKHR::eUint32: return "Uint32";
    case ComponentTypeKHR::eUint64: return "Uint64";
    case ComponentTypeKHR::eBfloat16: return "Bfloat16";
    case ComponentTypeKHR::eSint8PackedNV: return "Sint8PackedNV";
    case ComponentTypeKHR::eUint8PackedNV: return "Uint8PackedNV";
    case ComponentTypeKHR::eFloat8E4M3EXT: return "Float8E4M3EXT";
    case ComponentTypeKHR::eFloat8E5M2EXT: return "Float8E5M2EXT";
    default: return "Unknown";
    }
}

const char* to_cstr(BlendOverlapEXT v) {
    switch (v) {
    case BlendOverlapEXT::eUncorrelated: return "Uncorrelated";
    case BlendOverlapEXT::eDisjoint: return "Disjoint";
    case BlendOverlapEXT::eConjoint: return "Conjoint";
    default: return "Unknown";
    }
}

const char* to_cstr(CopyAccelerationStructureModeKHR v) {
    switch (v) {
    case CopyAccelerationStructureModeKHR::eClone: return "Clone";
    case CopyAccelerationStructureModeKHR::eCompact: return "Compact";
    case CopyAccelerationStructureModeKHR::eSerialize: return "Serialize";
    case CopyAccelerationStructureModeKHR::eDeserialize: return "Deserialize";
    default: return "Unknown";
    }
}

const char* to_cstr(AccelerationStructureTypeKHR v) {
    switch (v) {
    case AccelerationStructureTypeKHR::eTopLevel: return "TopLevel";
    case AccelerationStructureTypeKHR::eBottomLevel: return "BottomLevel";
    case AccelerationStructureTypeKHR::eGeneric: return "Generic";
    default: return "Unknown";
    }
}

const char* to_cstr(BuildAccelerationStructureModeKHR v) {
    switch (v) {
    case BuildAccelerationStructureModeKHR::eBuild: return "Build";
    case BuildAccelerationStructureModeKHR::eUpdate: return "Update";
    default: return "Unknown";
    }
}

const char* to_cstr(GeometryTypeKHR v) {
    switch (v) {
    case GeometryTypeKHR::eTriangles: return "Triangles";
    case GeometryTypeKHR::eAabbs: return "Aabbs";
    case GeometryTypeKHR::eInstances: return "Instances";
    case GeometryTypeKHR::eSpheresNV: return "SpheresNV";
    case GeometryTypeKHR::eLinearSweptSpheresNV: return "LinearSweptSpheresNV";
    default: return "Unknown";
    }
}

const char* to_cstr(AccelerationStructureBuildTypeKHR v) {
    switch (v) {
    case AccelerationStructureBuildTypeKHR::eHost: return "Host";
    case AccelerationStructureBuildTypeKHR::eDevice: return "Device";
    case AccelerationStructureBuildTypeKHR::eHostOrDevice: return "HostOrDevice";
    default: return "Unknown";
    }
}

const char* to_cstr(AccelerationStructureCompatibilityKHR v) {
    switch (v) {
    case AccelerationStructureCompatibilityKHR::eCompatible: return "Compatible";
    case AccelerationStructureCompatibilityKHR::eIncompatible: return "Incompatible";
    default: return "Unknown";
    }
}

const char* to_cstr(RayTracingShaderGroupTypeKHR v) {
    switch (v) {
    case RayTracingShaderGroupTypeKHR::eGeneral: return "General";
    case RayTracingShaderGroupTypeKHR::eTrianglesHitGroup: return "TrianglesHitGroup";
    case RayTracingShaderGroupTypeKHR::eProceduralHitGroup: return "ProceduralHitGroup";
    default: return "Unknown";
    }
}

const char* to_cstr(ShaderGroupShaderKHR v) {
    switch (v) {
    case ShaderGroupShaderKHR::eGeneral: return "General";
    case ShaderGroupShaderKHR::eClosestHit: return "ClosestHit";
    case ShaderGroupShaderKHR::eAnyHit: return "AnyHit";
    case ShaderGroupShaderKHR::eIntersection: return "Intersection";
    default: return "Unknown";
    }
}

const char* to_cstr(CoverageModulationModeNV v) {
    switch (v) {
    case CoverageModulationModeNV::eNone: return "None";
    case CoverageModulationModeNV::eRgb: return "Rgb";
    case CoverageModulationModeNV::eAlpha: return "Alpha";
    case CoverageModulationModeNV::eRgba: return "Rgba";
    default: return "Unknown";
    }
}

const char* to_cstr(ValidationCacheHeaderVersionEXT v) {
    switch (v) {
    case ValidationCacheHeaderVersionEXT::eOne: return "One";
    default: return "Unknown";
    }
}

const char* to_cstr(ShadingRatePaletteEntryNV v) {
    switch (v) {
    case ShadingRatePaletteEntryNV::eNoInvocations: return "NoInvocations";
    case ShadingRatePaletteEntryNV::e16InvocationsPerPixel: return "16InvocationsPerPixel";
    case ShadingRatePaletteEntryNV::e8InvocationsPerPixel: return "8InvocationsPerPixel";
    case ShadingRatePaletteEntryNV::e4InvocationsPerPixel: return "4InvocationsPerPixel";
    case ShadingRatePaletteEntryNV::e2InvocationsPerPixel: return "2InvocationsPerPixel";
    case ShadingRatePaletteEntryNV::e1InvocationPerPixel: return "1InvocationPerPixel";
    case ShadingRatePaletteEntryNV::e1InvocationPer2X1Pixels: return "1InvocationPer2X1Pixels";
    case ShadingRatePaletteEntryNV::e1InvocationPer1X2Pixels: return "1InvocationPer1X2Pixels";
    case ShadingRatePaletteEntryNV::e1InvocationPer2X2Pixels: return "1InvocationPer2X2Pixels";
    case ShadingRatePaletteEntryNV::e1InvocationPer4X2Pixels: return "1InvocationPer4X2Pixels";
    case ShadingRatePaletteEntryNV::e1InvocationPer2X4Pixels: return "1InvocationPer2X4Pixels";
    case ShadingRatePaletteEntryNV::e1InvocationPer4X4Pixels: return "1InvocationPer4X4Pixels";
    default: return "Unknown";
    }
}

const char* to_cstr(CoarseSampleOrderTypeNV v) {
    switch (v) {
    case CoarseSampleOrderTypeNV::eDefault: return "Default";
    case CoarseSampleOrderTypeNV::eCustom: return "Custom";
    case CoarseSampleOrderTypeNV::ePixelMajor: return "PixelMajor";
    case CoarseSampleOrderTypeNV::eSampleMajor: return "SampleMajor";
    default: return "Unknown";
    }
}

const char* to_cstr(AccelerationStructureMemoryRequirementsTypeNV v) {
    switch (v) {
    case AccelerationStructureMemoryRequirementsTypeNV::eObject: return "Object";
    case AccelerationStructureMemoryRequirementsTypeNV::eBuildScratch: return "BuildScratch";
    case AccelerationStructureMemoryRequirementsTypeNV::eUpdateScratch: return "UpdateScratch";
    default: return "Unknown";
    }
}

const char* to_cstr(TimeDomainKHR v) {
    switch (v) {
    case TimeDomainKHR::eDevice: return "Device";
    case TimeDomainKHR::eClockMonotonic: return "ClockMonotonic";
    case TimeDomainKHR::eClockMonotonicRaw: return "ClockMonotonicRaw";
    case TimeDomainKHR::eQueryPerformanceCounter: return "QueryPerformanceCounter";
    default: return "Unknown";
    }
}

const char* to_cstr(MemoryOverallocationBehaviorAMD v) {
    switch (v) {
    case MemoryOverallocationBehaviorAMD::eDefault: return "Default";
    case MemoryOverallocationBehaviorAMD::eAllowed: return "Allowed";
    case MemoryOverallocationBehaviorAMD::eDisallowed: return "Disallowed";
    default: return "Unknown";
    }
}

const char* to_cstr(IndirectCommandsTokenTypeEXT v) {
    switch (v) {
    case IndirectCommandsTokenTypeEXT::eExecutionSet: return "ExecutionSet";
    case IndirectCommandsTokenTypeEXT::ePushConstant: return "PushConstant";
    case IndirectCommandsTokenTypeEXT::eSequenceIndex: return "SequenceIndex";
    case IndirectCommandsTokenTypeEXT::eIndexBuffer: return "IndexBuffer";
    case IndirectCommandsTokenTypeEXT::eVertexBuffer: return "VertexBuffer";
    case IndirectCommandsTokenTypeEXT::eDrawIndexed: return "DrawIndexed";
    case IndirectCommandsTokenTypeEXT::eDraw: return "Draw";
    case IndirectCommandsTokenTypeEXT::eDrawIndexedCount: return "DrawIndexedCount";
    case IndirectCommandsTokenTypeEXT::eDrawCount: return "DrawCount";
    case IndirectCommandsTokenTypeEXT::eDispatch: return "Dispatch";
    case IndirectCommandsTokenTypeEXT::eDrawMeshTasksNv: return "DrawMeshTasksNv";
    case IndirectCommandsTokenTypeEXT::eDrawMeshTasksCountNv: return "DrawMeshTasksCountNv";
    case IndirectCommandsTokenTypeEXT::eDrawMeshTasks: return "DrawMeshTasks";
    case IndirectCommandsTokenTypeEXT::eDrawMeshTasksCount: return "DrawMeshTasksCount";
    case IndirectCommandsTokenTypeEXT::eTraceRays2: return "TraceRays2";
    default: return "Unknown";
    }
}

const char* to_cstr(PerformanceConfigurationTypeINTEL v) {
    switch (v) {
    case PerformanceConfigurationTypeINTEL::eCommandQueueMetricsDiscoveryActivated: return "CommandQueueMetricsDiscoveryActivated";
    default: return "Unknown";
    }
}

const char* to_cstr(QueryPoolSamplingModeINTEL v) {
    switch (v) {
    case QueryPoolSamplingModeINTEL::eManual: return "Manual";
    default: return "Unknown";
    }
}

const char* to_cstr(PerformanceOverrideTypeINTEL v) {
    switch (v) {
    case PerformanceOverrideTypeINTEL::eNullHardware: return "NullHardware";
    case PerformanceOverrideTypeINTEL::eFlushGpuCaches: return "FlushGpuCaches";
    default: return "Unknown";
    }
}

const char* to_cstr(PerformanceParameterTypeINTEL v) {
    switch (v) {
    case PerformanceParameterTypeINTEL::eHwCountersSupported: return "HwCountersSupported";
    case PerformanceParameterTypeINTEL::eStreamMarkerValidBits: return "StreamMarkerValidBits";
    default: return "Unknown";
    }
}

const char* to_cstr(PerformanceValueTypeINTEL v) {
    switch (v) {
    case PerformanceValueTypeINTEL::eUint32: return "Uint32";
    case PerformanceValueTypeINTEL::eUint64: return "Uint64";
    case PerformanceValueTypeINTEL::eFloat: return "Float";
    case PerformanceValueTypeINTEL::eBool: return "Bool";
    case PerformanceValueTypeINTEL::eString: return "String";
    default: return "Unknown";
    }
}

const char* to_cstr(FragmentShadingRateCombinerOpKHR v) {
    switch (v) {
    case FragmentShadingRateCombinerOpKHR::eKeep: return "Keep";
    case FragmentShadingRateCombinerOpKHR::eReplace: return "Replace";
    case FragmentShadingRateCombinerOpKHR::eMin: return "Min";
    case FragmentShadingRateCombinerOpKHR::eMax: return "Max";
    case FragmentShadingRateCombinerOpKHR::eMul: return "Mul";
    default: return "Unknown";
    }
}

const char* to_cstr(ValidationFeatureEnableEXT v) {
    switch (v) {
    case ValidationFeatureEnableEXT::eGpuAssisted: return "GpuAssisted";
    case ValidationFeatureEnableEXT::eGpuAssistedReserveBindingSlot: return "GpuAssistedReserveBindingSlot";
    case ValidationFeatureEnableEXT::eBestPractices: return "BestPractices";
    case ValidationFeatureEnableEXT::eDebugPrintf: return "DebugPrintf";
    case ValidationFeatureEnableEXT::eSynchronizationValidation: return "SynchronizationValidation";
    default: return "Unknown";
    }
}

const char* to_cstr(ValidationFeatureDisableEXT v) {
    switch (v) {
    case ValidationFeatureDisableEXT::eAll: return "All";
    case ValidationFeatureDisableEXT::eShaders: return "Shaders";
    case ValidationFeatureDisableEXT::eThreadSafety: return "ThreadSafety";
    case ValidationFeatureDisableEXT::eApiParameters: return "ApiParameters";
    case ValidationFeatureDisableEXT::eObjectLifetimes: return "ObjectLifetimes";
    case ValidationFeatureDisableEXT::eCoreChecks: return "CoreChecks";
    case ValidationFeatureDisableEXT::eUniqueHandles: return "UniqueHandles";
    case ValidationFeatureDisableEXT::eShaderValidationCache: return "ShaderValidationCache";
    default: return "Unknown";
    }
}

const char* to_cstr(ScopeKHR v) {
    switch (v) {
    case ScopeKHR::eDevice: return "Device";
    case ScopeKHR::eWorkgroup: return "Workgroup";
    case ScopeKHR::eSubgroup: return "Subgroup";
    case ScopeKHR::eQueueFamily: return "QueueFamily";
    default: return "Unknown";
    }
}

const char* to_cstr(CoverageReductionModeNV v) {
    switch (v) {
    case CoverageReductionModeNV::eMerge: return "Merge";
    case CoverageReductionModeNV::eTruncate: return "Truncate";
    default: return "Unknown";
    }
}

const char* to_cstr(ProvokingVertexModeEXT v) {
    switch (v) {
    case ProvokingVertexModeEXT::eFirstVertex: return "FirstVertex";
    case ProvokingVertexModeEXT::eLastVertex: return "LastVertex";
    default: return "Unknown";
    }
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
const char* to_cstr(FullScreenExclusiveEXT v) {
    switch (v) {
    case FullScreenExclusiveEXT::eDefault: return "Default";
    case FullScreenExclusiveEXT::eAllowed: return "Allowed";
    case FullScreenExclusiveEXT::eDisallowed: return "Disallowed";
    case FullScreenExclusiveEXT::eApplicationControlled: return "ApplicationControlled";
    default: return "Unknown";
    }
}

#endif // VK_USE_PLATFORM_WIN32_KHR
const char* to_cstr(PipelineExecutableStatisticFormatKHR v) {
    switch (v) {
    case PipelineExecutableStatisticFormatKHR::eBool32: return "Bool32";
    case PipelineExecutableStatisticFormatKHR::eInt64: return "Int64";
    case PipelineExecutableStatisticFormatKHR::eUint64: return "Uint64";
    case PipelineExecutableStatisticFormatKHR::eFloat64: return "Float64";
    default: return "Unknown";
    }
}

const char* to_cstr(IndirectCommandsTokenTypeNV v) {
    switch (v) {
    case IndirectCommandsTokenTypeNV::eShaderGroup: return "ShaderGroup";
    case IndirectCommandsTokenTypeNV::eStateFlags: return "StateFlags";
    case IndirectCommandsTokenTypeNV::eIndexBuffer: return "IndexBuffer";
    case IndirectCommandsTokenTypeNV::eVertexBuffer: return "VertexBuffer";
    case IndirectCommandsTokenTypeNV::ePushConstant: return "PushConstant";
    case IndirectCommandsTokenTypeNV::eDrawIndexed: return "DrawIndexed";
    case IndirectCommandsTokenTypeNV::eDraw: return "Draw";
    case IndirectCommandsTokenTypeNV::eDrawTasks: return "DrawTasks";
    case IndirectCommandsTokenTypeNV::eDrawMeshTasks: return "DrawMeshTasks";
    case IndirectCommandsTokenTypeNV::ePipeline: return "Pipeline";
    case IndirectCommandsTokenTypeNV::eDispatch: return "Dispatch";
    default: return "Unknown";
    }
}

const char* to_cstr(DepthBiasRepresentationEXT v) {
    switch (v) {
    case DepthBiasRepresentationEXT::eLeastRepresentableValueFormat: return "LeastRepresentableValueFormat";
    case DepthBiasRepresentationEXT::eLeastRepresentableValueForceUnorm: return "LeastRepresentableValueForceUnorm";
    case DepthBiasRepresentationEXT::eFloat: return "Float";
    default: return "Unknown";
    }
}

const char* to_cstr(DeviceMemoryReportEventTypeEXT v) {
    switch (v) {
    case DeviceMemoryReportEventTypeEXT::eAllocate: return "Allocate";
    case DeviceMemoryReportEventTypeEXT::eFree: return "Free";
    case DeviceMemoryReportEventTypeEXT::eImport: return "Import";
    case DeviceMemoryReportEventTypeEXT::eUnimport: return "Unimport";
    case DeviceMemoryReportEventTypeEXT::eAllocationFailed: return "AllocationFailed";
    default: return "Unknown";
    }
}

const char* to_cstr(FragmentShadingRateTypeNV v) {
    switch (v) {
    case FragmentShadingRateTypeNV::eFragmentSize: return "FragmentSize";
    case FragmentShadingRateTypeNV::eEnums: return "Enums";
    default: return "Unknown";
    }
}

const char* to_cstr(FragmentShadingRateNV v) {
    switch (v) {
    case FragmentShadingRateNV::e1InvocationPerPixel: return "1InvocationPerPixel";
    case FragmentShadingRateNV::e1InvocationPer1X2Pixels: return "1InvocationPer1X2Pixels";
    case FragmentShadingRateNV::e1InvocationPer2X1Pixels: return "1InvocationPer2X1Pixels";
    case FragmentShadingRateNV::e1InvocationPer2X2Pixels: return "1InvocationPer2X2Pixels";
    case FragmentShadingRateNV::e1InvocationPer2X4Pixels: return "1InvocationPer2X4Pixels";
    case FragmentShadingRateNV::e1InvocationPer4X2Pixels: return "1InvocationPer4X2Pixels";
    case FragmentShadingRateNV::e1InvocationPer4X4Pixels: return "1InvocationPer4X4Pixels";
    case FragmentShadingRateNV::e2InvocationsPerPixel: return "2InvocationsPerPixel";
    case FragmentShadingRateNV::e4InvocationsPerPixel: return "4InvocationsPerPixel";
    case FragmentShadingRateNV::e8InvocationsPerPixel: return "8InvocationsPerPixel";
    case FragmentShadingRateNV::e16InvocationsPerPixel: return "16InvocationsPerPixel";
    case FragmentShadingRateNV::eNoInvocations: return "NoInvocations";
    default: return "Unknown";
    }
}

const char* to_cstr(AccelerationStructureMotionInstanceTypeNV v) {
    switch (v) {
    case AccelerationStructureMotionInstanceTypeNV::eStatic: return "Static";
    case AccelerationStructureMotionInstanceTypeNV::eMatrixMotion: return "MatrixMotion";
    case AccelerationStructureMotionInstanceTypeNV::eSrtMotion: return "SrtMotion";
    default: return "Unknown";
    }
}

const char* to_cstr(DeviceFaultAddressTypeEXT v) {
    switch (v) {
    case DeviceFaultAddressTypeEXT::eNone: return "None";
    case DeviceFaultAddressTypeEXT::eReadInvalid: return "ReadInvalid";
    case DeviceFaultAddressTypeEXT::eWriteInvalid: return "WriteInvalid";
    case DeviceFaultAddressTypeEXT::eExecuteInvalid: return "ExecuteInvalid";
    case DeviceFaultAddressTypeEXT::eInstructionPointerUnknown: return "InstructionPointerUnknown";
    case DeviceFaultAddressTypeEXT::eInstructionPointerInvalid: return "InstructionPointerInvalid";
    case DeviceFaultAddressTypeEXT::eInstructionPointerFault: return "InstructionPointerFault";
    default: return "Unknown";
    }
}

const char* to_cstr(DeviceFaultVendorBinaryHeaderVersionEXT v) {
    switch (v) {
    case DeviceFaultVendorBinaryHeaderVersionEXT::eOne: return "One";
    default: return "Unknown";
    }
}

const char* to_cstr(DeviceAddressBindingTypeEXT v) {
    switch (v) {
    case DeviceAddressBindingTypeEXT::eBind: return "Bind";
    case DeviceAddressBindingTypeEXT::eUnbind: return "Unbind";
    default: return "Unknown";
    }
}

const char* to_cstr(MicromapTypeEXT v) {
    switch (v) {
    case MicromapTypeEXT::eOpacityMicromap: return "OpacityMicromap";
    case MicromapTypeEXT::eDisplacementMicromapNV: return "DisplacementMicromapNV";
    default: return "Unknown";
    }
}

const char* to_cstr(BuildMicromapModeEXT v) {
    switch (v) {
    case BuildMicromapModeEXT::eBuild: return "Build";
    default: return "Unknown";
    }
}

const char* to_cstr(CopyMicromapModeEXT v) {
    switch (v) {
    case CopyMicromapModeEXT::eClone: return "Clone";
    case CopyMicromapModeEXT::eSerialize: return "Serialize";
    case CopyMicromapModeEXT::eDeserialize: return "Deserialize";
    case CopyMicromapModeEXT::eCompact: return "Compact";
    default: return "Unknown";
    }
}

const char* to_cstr(OpacityMicromapFormatEXT v) {
    switch (v) {
    case OpacityMicromapFormatEXT::e2State: return "2State";
    case OpacityMicromapFormatEXT::e4State: return "4State";
    default: return "Unknown";
    }
}

const char* to_cstr(OpacityMicromapSpecialIndexEXT v) {
    switch (v) {
    case OpacityMicromapSpecialIndexEXT::eFullyTransparent: return "FullyTransparent";
    case OpacityMicromapSpecialIndexEXT::eFullyOpaque: return "FullyOpaque";
    case OpacityMicromapSpecialIndexEXT::eFullyUnknownTransparent: return "FullyUnknownTransparent";
    case OpacityMicromapSpecialIndexEXT::eFullyUnknownOpaque: return "FullyUnknownOpaque";
    case OpacityMicromapSpecialIndexEXT::eClusterGeometryDisableOpacityMicromapNV: return "ClusterGeometryDisableOpacityMicromapNV";
    default: return "Unknown";
    }
}

const char* to_cstr(DisplacementMicromapFormatNV v) {
    switch (v) {
    case DisplacementMicromapFormatNV::e64Triangles64Bytes: return "64Triangles64Bytes";
    case DisplacementMicromapFormatNV::e256Triangles128Bytes: return "256Triangles128Bytes";
    case DisplacementMicromapFormatNV::e1024Triangles128Bytes: return "1024Triangles128Bytes";
    default: return "Unknown";
    }
}

const char* to_cstr(RayTracingLssIndexingModeNV v) {
    switch (v) {
    case RayTracingLssIndexingModeNV::eList: return "List";
    case RayTracingLssIndexingModeNV::eSuccessive: return "Successive";
    default: return "Unknown";
    }
}

const char* to_cstr(RayTracingLssPrimitiveEndCapsModeNV v) {
    switch (v) {
    case RayTracingLssPrimitiveEndCapsModeNV::eNone: return "None";
    case RayTracingLssPrimitiveEndCapsModeNV::eChained: return "Chained";
    default: return "Unknown";
    }
}

const char* to_cstr(SubpassMergeStatusEXT v) {
    switch (v) {
    case SubpassMergeStatusEXT::eMerged: return "Merged";
    case SubpassMergeStatusEXT::eDisallowed: return "Disallowed";
    case SubpassMergeStatusEXT::eNotMergedSideEffects: return "NotMergedSideEffects";
    case SubpassMergeStatusEXT::eNotMergedSamplesMismatch: return "NotMergedSamplesMismatch";
    case SubpassMergeStatusEXT::eNotMergedViewsMismatch: return "NotMergedViewsMismatch";
    case SubpassMergeStatusEXT::eNotMergedAliasing: return "NotMergedAliasing";
    case SubpassMergeStatusEXT::eNotMergedDependencies: return "NotMergedDependencies";
    case SubpassMergeStatusEXT::eNotMergedIncompatibleInputAttachment: return "NotMergedIncompatibleInputAttachment";
    case SubpassMergeStatusEXT::eNotMergedTooManyAttachments: return "NotMergedTooManyAttachments";
    case SubpassMergeStatusEXT::eNotMergedInsufficientStorage: return "NotMergedInsufficientStorage";
    case SubpassMergeStatusEXT::eNotMergedDepthStencilCount: return "NotMergedDepthStencilCount";
    case SubpassMergeStatusEXT::eNotMergedResolveAttachmentReuse: return "NotMergedResolveAttachmentReuse";
    case SubpassMergeStatusEXT::eNotMergedSingleSubpass: return "NotMergedSingleSubpass";
    case SubpassMergeStatusEXT::eNotMergedUnspecified: return "NotMergedUnspecified";
    default: return "Unknown";
    }
}

const char* to_cstr(DirectDriverLoadingModeLUNARG v) {
    switch (v) {
    case DirectDriverLoadingModeLUNARG::eExclusive: return "Exclusive";
    case DirectDriverLoadingModeLUNARG::eInclusive: return "Inclusive";
    default: return "Unknown";
    }
}

const char* to_cstr(TensorTilingARM v) {
    switch (v) {
    case TensorTilingARM::eOptimal: return "Optimal";
    case TensorTilingARM::eLinear: return "Linear";
    default: return "Unknown";
    }
}

const char* to_cstr(OpticalFlowPerformanceLevelNV v) {
    switch (v) {
    case OpticalFlowPerformanceLevelNV::eUnknown: return "Unknown";
    case OpticalFlowPerformanceLevelNV::eSlow: return "Slow";
    case OpticalFlowPerformanceLevelNV::eMedium: return "Medium";
    case OpticalFlowPerformanceLevelNV::eFast: return "Fast";
    default: return "Unknown";
    }
}

const char* to_cstr(OpticalFlowSessionBindingPointNV v) {
    switch (v) {
    case OpticalFlowSessionBindingPointNV::eUnknown: return "Unknown";
    case OpticalFlowSessionBindingPointNV::eInput: return "Input";
    case OpticalFlowSessionBindingPointNV::eReference: return "Reference";
    case OpticalFlowSessionBindingPointNV::eHint: return "Hint";
    case OpticalFlowSessionBindingPointNV::eFlowVector: return "FlowVector";
    case OpticalFlowSessionBindingPointNV::eBackwardFlowVector: return "BackwardFlowVector";
    case OpticalFlowSessionBindingPointNV::eCost: return "Cost";
    case OpticalFlowSessionBindingPointNV::eBackwardCost: return "BackwardCost";
    case OpticalFlowSessionBindingPointNV::eGlobalFlow: return "GlobalFlow";
    default: return "Unknown";
    }
}

const char* to_cstr(AntiLagModeAMD v) {
    switch (v) {
    case AntiLagModeAMD::eDriverControl: return "DriverControl";
    case AntiLagModeAMD::eOn: return "On";
    case AntiLagModeAMD::eOff: return "Off";
    default: return "Unknown";
    }
}

const char* to_cstr(AntiLagStageAMD v) {
    switch (v) {
    case AntiLagStageAMD::eInput: return "Input";
    case AntiLagStageAMD::ePresent: return "Present";
    default: return "Unknown";
    }
}

const char* to_cstr(ShaderCodeTypeEXT v) {
    switch (v) {
    case ShaderCodeTypeEXT::eBinary: return "Binary";
    case ShaderCodeTypeEXT::eSpirv: return "Spirv";
    default: return "Unknown";
    }
}

const char* to_cstr(DepthClampModeEXT v) {
    switch (v) {
    case DepthClampModeEXT::eViewportRange: return "ViewportRange";
    case DepthClampModeEXT::eUserDefinedRange: return "UserDefinedRange";
    default: return "Unknown";
    }
}

const char* to_cstr(RayTracingInvocationReorderModeNV v) {
    switch (v) {
    case RayTracingInvocationReorderModeNV::eNone: return "None";
    case RayTracingInvocationReorderModeNV::eReorder: return "Reorder";
    default: return "Unknown";
    }
}

const char* to_cstr(CooperativeVectorMatrixLayoutNV v) {
    switch (v) {
    case CooperativeVectorMatrixLayoutNV::eRowMajor: return "RowMajor";
    case CooperativeVectorMatrixLayoutNV::eColumnMajor: return "ColumnMajor";
    case CooperativeVectorMatrixLayoutNV::eInferencingOptimal: return "InferencingOptimal";
    case CooperativeVectorMatrixLayoutNV::eTrainingOptimal: return "TrainingOptimal";
    default: return "Unknown";
    }
}

const char* to_cstr(LayerSettingTypeEXT v) {
    switch (v) {
    case LayerSettingTypeEXT::eBool32: return "Bool32";
    case LayerSettingTypeEXT::eInt32: return "Int32";
    case LayerSettingTypeEXT::eInt64: return "Int64";
    case LayerSettingTypeEXT::eUint32: return "Uint32";
    case LayerSettingTypeEXT::eUint64: return "Uint64";
    case LayerSettingTypeEXT::eFloat32: return "Float32";
    case LayerSettingTypeEXT::eFloat64: return "Float64";
    case LayerSettingTypeEXT::eString: return "String";
    default: return "Unknown";
    }
}

const char* to_cstr(LatencyMarkerNV v) {
    switch (v) {
    case LatencyMarkerNV::eSimulationStart: return "SimulationStart";
    case LatencyMarkerNV::eSimulationEnd: return "SimulationEnd";
    case LatencyMarkerNV::eRendersubmitStart: return "RendersubmitStart";
    case LatencyMarkerNV::eRendersubmitEnd: return "RendersubmitEnd";
    case LatencyMarkerNV::ePresentStart: return "PresentStart";
    case LatencyMarkerNV::ePresentEnd: return "PresentEnd";
    case LatencyMarkerNV::eInputSample: return "InputSample";
    case LatencyMarkerNV::eTriggerFlash: return "TriggerFlash";
    case LatencyMarkerNV::eOutOfBandRendersubmitStart: return "OutOfBandRendersubmitStart";
    case LatencyMarkerNV::eOutOfBandRendersubmitEnd: return "OutOfBandRendersubmitEnd";
    case LatencyMarkerNV::eOutOfBandPresentStart: return "OutOfBandPresentStart";
    case LatencyMarkerNV::eOutOfBandPresentEnd: return "OutOfBandPresentEnd";
    default: return "Unknown";
    }
}

const char* to_cstr(OutOfBandQueueTypeNV v) {
    switch (v) {
    case OutOfBandQueueTypeNV::eRender: return "Render";
    case OutOfBandQueueTypeNV::ePresent: return "Present";
    default: return "Unknown";
    }
}

const char* to_cstr(DataGraphPipelineSessionBindPointARM v) {
    switch (v) {
    case DataGraphPipelineSessionBindPointARM::eTransient: return "Transient";
    default: return "Unknown";
    }
}

const char* to_cstr(DataGraphPipelineSessionBindPointTypeARM v) {
    switch (v) {
    case DataGraphPipelineSessionBindPointTypeARM::eMemory: return "Memory";
    default: return "Unknown";
    }
}

const char* to_cstr(DataGraphPipelinePropertyARM v) {
    switch (v) {
    case DataGraphPipelinePropertyARM::eCreationLog: return "CreationLog";
    case DataGraphPipelinePropertyARM::eIdentifier: return "Identifier";
    default: return "Unknown";
    }
}

const char* to_cstr(PhysicalDeviceDataGraphProcessingEngineTypeARM v) {
    switch (v) {
    case PhysicalDeviceDataGraphProcessingEngineTypeARM::eDefault: return "Default";
    default: return "Unknown";
    }
}

const char* to_cstr(PhysicalDeviceDataGraphOperationTypeARM v) {
    switch (v) {
    case PhysicalDeviceDataGraphOperationTypeARM::eSpirvExtendedInstructionSet: return "SpirvExtendedInstructionSet";
    default: return "Unknown";
    }
}

const char* to_cstr(BlockMatchWindowCompareModeQCOM v) {
    switch (v) {
    case BlockMatchWindowCompareModeQCOM::eMin: return "Min";
    case BlockMatchWindowCompareModeQCOM::eMax: return "Max";
    default: return "Unknown";
    }
}

const char* to_cstr(CubicFilterWeightsQCOM v) {
    switch (v) {
    case CubicFilterWeightsQCOM::eCatmullRom: return "CatmullRom";
    case CubicFilterWeightsQCOM::eZeroTangentCardinal: return "ZeroTangentCardinal";
    case CubicFilterWeightsQCOM::eBSpline: return "BSpline";
    case CubicFilterWeightsQCOM::eMitchellNetravali: return "MitchellNetravali";
    default: return "Unknown";
    }
}

const char* to_cstr(LayeredDriverUnderlyingApiMSFT v) {
    switch (v) {
    case LayeredDriverUnderlyingApiMSFT::eNone: return "None";
    case LayeredDriverUnderlyingApiMSFT::eD3D12: return "D3D12";
    default: return "Unknown";
    }
}

const char* to_cstr(DisplaySurfaceStereoTypeNV v) {
    switch (v) {
    case DisplaySurfaceStereoTypeNV::eNone: return "None";
    case DisplaySurfaceStereoTypeNV::eOnboardDin: return "OnboardDin";
    case DisplaySurfaceStereoTypeNV::eHdmi3D: return "Hdmi3D";
    case DisplaySurfaceStereoTypeNV::eInbandDisplayport: return "InbandDisplayport";
    default: return "Unknown";
    }
}

const char* to_cstr(PhysicalDeviceLayeredApiKHR v) {
    switch (v) {
    case PhysicalDeviceLayeredApiKHR::eVulkan: return "Vulkan";
    case PhysicalDeviceLayeredApiKHR::eD3D12: return "D3D12";
    case PhysicalDeviceLayeredApiKHR::eMetal: return "Metal";
    case PhysicalDeviceLayeredApiKHR::eOpengl: return "Opengl";
    case PhysicalDeviceLayeredApiKHR::eOpengles: return "Opengles";
    default: return "Unknown";
    }
}

const char* to_cstr(ClusterAccelerationStructureTypeNV v) {
    switch (v) {
    case ClusterAccelerationStructureTypeNV::eClustersBottomLevel: return "ClustersBottomLevel";
    case ClusterAccelerationStructureTypeNV::eTriangleCluster: return "TriangleCluster";
    case ClusterAccelerationStructureTypeNV::eTriangleClusterTemplate: return "TriangleClusterTemplate";
    default: return "Unknown";
    }
}

const char* to_cstr(ClusterAccelerationStructureOpTypeNV v) {
    switch (v) {
    case ClusterAccelerationStructureOpTypeNV::eMoveObjects: return "MoveObjects";
    case ClusterAccelerationStructureOpTypeNV::eBuildClustersBottomLevel: return "BuildClustersBottomLevel";
    case ClusterAccelerationStructureOpTypeNV::eBuildTriangleCluster: return "BuildTriangleCluster";
    case ClusterAccelerationStructureOpTypeNV::eBuildTriangleClusterTemplate: return "BuildTriangleClusterTemplate";
    case ClusterAccelerationStructureOpTypeNV::eInstantiateTriangleCluster: return "InstantiateTriangleCluster";
    case ClusterAccelerationStructureOpTypeNV::eGetClusterTemplateIndices: return "GetClusterTemplateIndices";
    default: return "Unknown";
    }
}

const char* to_cstr(ClusterAccelerationStructureOpModeNV v) {
    switch (v) {
    case ClusterAccelerationStructureOpModeNV::eImplicitDestinations: return "ImplicitDestinations";
    case ClusterAccelerationStructureOpModeNV::eExplicitDestinations: return "ExplicitDestinations";
    case ClusterAccelerationStructureOpModeNV::eComputeSizes: return "ComputeSizes";
    default: return "Unknown";
    }
}

const char* to_cstr(PartitionedAccelerationStructureOpTypeNV v) {
    switch (v) {
    case PartitionedAccelerationStructureOpTypeNV::eWriteInstance: return "WriteInstance";
    case PartitionedAccelerationStructureOpTypeNV::eUpdateInstance: return "UpdateInstance";
    case PartitionedAccelerationStructureOpTypeNV::eWritePartitionTranslation: return "WritePartitionTranslation";
    default: return "Unknown";
    }
}

const char* to_cstr(IndirectExecutionSetInfoTypeEXT v) {
    switch (v) {
    case IndirectExecutionSetInfoTypeEXT::ePipelines: return "Pipelines";
    case IndirectExecutionSetInfoTypeEXT::eShaderObjects: return "ShaderObjects";
    default: return "Unknown";
    }
}

const char* to_cstr(DefaultVertexAttributeValueKHR v) {
    switch (v) {
    case DefaultVertexAttributeValueKHR::eZeroZeroZeroZero: return "ZeroZeroZeroZero";
    case DefaultVertexAttributeValueKHR::eZeroZeroZeroOne: return "ZeroZeroZeroOne";
    default: return "Unknown";
    }
}

vector<PhysicalDevice> enumeratePhysicalDevices_throw() {
    vector<PhysicalDevice> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumeratePhysicalDevices(detail::_instance.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkEnumeratePhysicalDevices");
        v.alloc(n);
        r = funcs.vkEnumeratePhysicalDevices(detail::_instance.handle(), &n, reinterpret_cast<PhysicalDevice::HandleType*>(v.data()));
        checkSuccess(r, "vkEnumeratePhysicalDevices");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result enumeratePhysicalDevices_noThrow(vector<PhysicalDevice>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumeratePhysicalDevices(detail::_instance.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkEnumeratePhysicalDevices(detail::_instance.handle(), &n, reinterpret_cast<PhysicalDevice::HandleType*>(v.data()));
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<QueueFamilyProperties> getPhysicalDeviceQueueFamilyProperties_throw(PhysicalDevice physicalDevice) {
    vector<QueueFamilyProperties> v;
    uint32_t n;
    funcs.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice.handle(), &n, nullptr);
    v.alloc(n);
    funcs.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice.handle(), &n, v.data());
    return v;
}

Result getPhysicalDeviceQueueFamilyProperties_noThrow(PhysicalDevice physicalDevice, vector<QueueFamilyProperties>& v) noexcept {
    uint32_t n;
    funcs.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice.handle(), &n, nullptr);
    if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
    funcs.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice.handle(), &n, v.data());
    return Result::eSuccess;
}

vector<ExtensionProperties> enumerateInstanceExtensionProperties_throw(const char* pLayerName) {
    vector<ExtensionProperties> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumerateInstanceExtensionProperties(pLayerName, &n, nullptr);
        checkForSuccessValue(r, "vkEnumerateInstanceExtensionProperties");
        v.alloc(n);
        r = funcs.vkEnumerateInstanceExtensionProperties(pLayerName, &n, v.data());
        checkSuccess(r, "vkEnumerateInstanceExtensionProperties");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result enumerateInstanceExtensionProperties_noThrow(const char* pLayerName, vector<ExtensionProperties>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumerateInstanceExtensionProperties(pLayerName, &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkEnumerateInstanceExtensionProperties(pLayerName, &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<ExtensionProperties> enumerateDeviceExtensionProperties_throw(PhysicalDevice physicalDevice, const char* pLayerName) {
    vector<ExtensionProperties> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumerateDeviceExtensionProperties(physicalDevice.handle(), pLayerName, &n, nullptr);
        checkForSuccessValue(r, "vkEnumerateDeviceExtensionProperties");
        v.alloc(n);
        r = funcs.vkEnumerateDeviceExtensionProperties(physicalDevice.handle(), pLayerName, &n, v.data());
        checkSuccess(r, "vkEnumerateDeviceExtensionProperties");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result enumerateDeviceExtensionProperties_noThrow(PhysicalDevice physicalDevice, const char* pLayerName, vector<ExtensionProperties>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumerateDeviceExtensionProperties(physicalDevice.handle(), pLayerName, &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkEnumerateDeviceExtensionProperties(physicalDevice.handle(), pLayerName, &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<LayerProperties> enumerateInstanceLayerProperties_throw() {
    vector<LayerProperties> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumerateInstanceLayerProperties(&n, nullptr);
        checkForSuccessValue(r, "vkEnumerateInstanceLayerProperties");
        v.alloc(n);
        r = funcs.vkEnumerateInstanceLayerProperties(&n, v.data());
        checkSuccess(r, "vkEnumerateInstanceLayerProperties");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result enumerateInstanceLayerProperties_noThrow(vector<LayerProperties>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumerateInstanceLayerProperties(&n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkEnumerateInstanceLayerProperties(&n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<LayerProperties> enumerateDeviceLayerProperties_throw(PhysicalDevice physicalDevice) {
    vector<LayerProperties> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumerateDeviceLayerProperties(physicalDevice.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkEnumerateDeviceLayerProperties");
        v.alloc(n);
        r = funcs.vkEnumerateDeviceLayerProperties(physicalDevice.handle(), &n, v.data());
        checkSuccess(r, "vkEnumerateDeviceLayerProperties");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result enumerateDeviceLayerProperties_noThrow(PhysicalDevice physicalDevice, vector<LayerProperties>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumerateDeviceLayerProperties(physicalDevice.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkEnumerateDeviceLayerProperties(physicalDevice.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<SparseImageMemoryRequirements> getImageSparseMemoryRequirements_throw(Image image) {
    vector<SparseImageMemoryRequirements> v;
    uint32_t n;
    funcs.vkGetImageSparseMemoryRequirements(detail::_device.handle(), image.handle(), &n, nullptr);
    v.alloc(n);
    funcs.vkGetImageSparseMemoryRequirements(detail::_device.handle(), image.handle(), &n, v.data());
    return v;
}

Result getImageSparseMemoryRequirements_noThrow(Image image, vector<SparseImageMemoryRequirements>& v) noexcept {
    uint32_t n;
    funcs.vkGetImageSparseMemoryRequirements(detail::_device.handle(), image.handle(), &n, nullptr);
    if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
    funcs.vkGetImageSparseMemoryRequirements(detail::_device.handle(), image.handle(), &n, v.data());
    return Result::eSuccess;
}

vector<SparseImageFormatProperties> getPhysicalDeviceSparseImageFormatProperties_throw(PhysicalDevice physicalDevice, Format format, ImageType type, SampleCountFlagBits samples, ImageUsageFlags usage, ImageTiling tiling) {
    vector<SparseImageFormatProperties> v;
    uint32_t n;
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice.handle(), format, type, samples, usage, tiling, &n, nullptr);
    v.alloc(n);
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice.handle(), format, type, samples, usage, tiling, &n, v.data());
    return v;
}

Result getPhysicalDeviceSparseImageFormatProperties_noThrow(PhysicalDevice physicalDevice, Format format, ImageType type, SampleCountFlagBits samples, ImageUsageFlags usage, ImageTiling tiling, vector<SparseImageFormatProperties>& v) noexcept {
    uint32_t n;
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice.handle(), format, type, samples, usage, tiling, &n, nullptr);
    if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice.handle(), format, type, samples, usage, tiling, &n, v.data());
    return Result::eSuccess;
}

vector<PhysicalDeviceGroupProperties> enumeratePhysicalDeviceGroups_throw() {
    vector<PhysicalDeviceGroupProperties> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumeratePhysicalDeviceGroups(detail::_instance.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkEnumeratePhysicalDeviceGroups");
        v.alloc(n);
        r = funcs.vkEnumeratePhysicalDeviceGroups(detail::_instance.handle(), &n, v.data());
        checkSuccess(r, "vkEnumeratePhysicalDeviceGroups");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result enumeratePhysicalDeviceGroups_noThrow(vector<PhysicalDeviceGroupProperties>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumeratePhysicalDeviceGroups(detail::_instance.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkEnumeratePhysicalDeviceGroups(detail::_instance.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<SparseImageMemoryRequirements2> getImageSparseMemoryRequirements2_throw(const ImageSparseMemoryRequirementsInfo2& pInfo) {
    vector<SparseImageMemoryRequirements2> v;
    uint32_t n;
    funcs.vkGetImageSparseMemoryRequirements2(detail::_device.handle(), &pInfo, &n, nullptr);
    v.alloc(n);
    funcs.vkGetImageSparseMemoryRequirements2(detail::_device.handle(), &pInfo, &n, v.data());
    return v;
}

Result getImageSparseMemoryRequirements2_noThrow(const ImageSparseMemoryRequirementsInfo2& pInfo, vector<SparseImageMemoryRequirements2>& v) noexcept {
    uint32_t n;
    funcs.vkGetImageSparseMemoryRequirements2(detail::_device.handle(), &pInfo, &n, nullptr);
    if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
    funcs.vkGetImageSparseMemoryRequirements2(detail::_device.handle(), &pInfo, &n, v.data());
    return Result::eSuccess;
}

vector<QueueFamilyProperties2> getPhysicalDeviceQueueFamilyProperties2_throw(PhysicalDevice physicalDevice) {
    vector<QueueFamilyProperties2> v;
    uint32_t n;
    funcs.vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice.handle(), &n, nullptr);
    v.alloc(n);
    funcs.vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice.handle(), &n, v.data());
    return v;
}

Result getPhysicalDeviceQueueFamilyProperties2_noThrow(PhysicalDevice physicalDevice, vector<QueueFamilyProperties2>& v) noexcept {
    uint32_t n;
    funcs.vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice.handle(), &n, nullptr);
    if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
    funcs.vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice.handle(), &n, v.data());
    return Result::eSuccess;
}

vector<SparseImageFormatProperties2> getPhysicalDeviceSparseImageFormatProperties2_throw(PhysicalDevice physicalDevice, const PhysicalDeviceSparseImageFormatInfo2& pFormatInfo) {
    vector<SparseImageFormatProperties2> v;
    uint32_t n;
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice.handle(), &pFormatInfo, &n, nullptr);
    v.alloc(n);
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice.handle(), &pFormatInfo, &n, v.data());
    return v;
}

Result getPhysicalDeviceSparseImageFormatProperties2_noThrow(PhysicalDevice physicalDevice, const PhysicalDeviceSparseImageFormatInfo2& pFormatInfo, vector<SparseImageFormatProperties2>& v) noexcept {
    uint32_t n;
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice.handle(), &pFormatInfo, &n, nullptr);
    if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
    funcs.vkGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice.handle(), &pFormatInfo, &n, v.data());
    return Result::eSuccess;
}

vector<PhysicalDeviceToolProperties> getPhysicalDeviceToolProperties_throw(PhysicalDevice physicalDevice) {
    vector<PhysicalDeviceToolProperties> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceToolProperties(physicalDevice.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceToolProperties");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceToolProperties(physicalDevice.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceToolProperties");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceToolProperties_noThrow(PhysicalDevice physicalDevice, vector<PhysicalDeviceToolProperties>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceToolProperties(physicalDevice.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceToolProperties(physicalDevice.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<SparseImageMemoryRequirements2> getDeviceImageSparseMemoryRequirements_throw(const DeviceImageMemoryRequirements& pInfo) {
    vector<SparseImageMemoryRequirements2> v;
    uint32_t n;
    funcs.vkGetDeviceImageSparseMemoryRequirements(detail::_device.handle(), &pInfo, &n, nullptr);
    v.alloc(n);
    funcs.vkGetDeviceImageSparseMemoryRequirements(detail::_device.handle(), &pInfo, &n, v.data());
    return v;
}

Result getDeviceImageSparseMemoryRequirements_noThrow(const DeviceImageMemoryRequirements& pInfo, vector<SparseImageMemoryRequirements2>& v) noexcept {
    uint32_t n;
    funcs.vkGetDeviceImageSparseMemoryRequirements(detail::_device.handle(), &pInfo, &n, nullptr);
    if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
    funcs.vkGetDeviceImageSparseMemoryRequirements(detail::_device.handle(), &pInfo, &n, v.data());
    return Result::eSuccess;
}

vk::vector<vk::SurfaceFormatKHR> __cdecl vk::getPhysicalDeviceSurfaceFormatsKHR_throw(vk::Handle<struct vk::ePhysicalDevice_T *> physicalDevice, vk::Handle<struct VkSurfaceKHR_T *> surface) {
    vector<SurfaceFormatKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.handle(), surface.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceSurfaceFormatsKHR");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.handle(), surface.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceSurfaceFormatsKHR_noThrow(PhysicalDevice physicalDevice, SurfaceKHR surface, vector<SurfaceFormatKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.handle(), surface.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.handle(), surface.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<PresentModeKHR> getPhysicalDeviceSurfacePresentModesKHR_throw(PhysicalDevice physicalDevice, SurfaceKHR surface) {
    vector<PresentModeKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.handle(), surface.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceSurfacePresentModesKHR");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.handle(), surface.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceSurfacePresentModesKHR_noThrow(PhysicalDevice physicalDevice, SurfaceKHR surface, vector<PresentModeKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.handle(), surface.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.handle(), surface.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<Image> getSwapchainImagesKHR_throw(SwapchainKHR swapchain) {
    vector<Image> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetSwapchainImagesKHR(detail::_device.handle(), swapchain.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetSwapchainImagesKHR");
        v.alloc(n);
        r = funcs.vkGetSwapchainImagesKHR(detail::_device.handle(), swapchain.handle(), &n, reinterpret_cast<Image::HandleType*>(v.data()));
        checkSuccess(r, "vkGetSwapchainImagesKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getSwapchainImagesKHR_noThrow(SwapchainKHR swapchain, vector<Image>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetSwapchainImagesKHR(detail::_device.handle(), swapchain.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetSwapchainImagesKHR(detail::_device.handle(), swapchain.handle(), &n, reinterpret_cast<Image::HandleType*>(v.data()));
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<Rect2D> getPhysicalDevicePresentRectanglesKHR_throw(PhysicalDevice physicalDevice, SurfaceKHR surface) {
    vector<Rect2D> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDevicePresentRectanglesKHR(physicalDevice.handle(), surface.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDevicePresentRectanglesKHR");
        v.alloc(n);
        r = funcs.vkGetPhysicalDevicePresentRectanglesKHR(physicalDevice.handle(), surface.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDevicePresentRectanglesKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDevicePresentRectanglesKHR_noThrow(PhysicalDevice physicalDevice, SurfaceKHR surface, vector<Rect2D>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDevicePresentRectanglesKHR(physicalDevice.handle(), surface.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDevicePresentRectanglesKHR(physicalDevice.handle(), surface.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<DisplayPropertiesKHR> getPhysicalDeviceDisplayPropertiesKHR_throw(PhysicalDevice physicalDevice) {
    vector<DisplayPropertiesKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceDisplayPropertiesKHR");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceDisplayPropertiesKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceDisplayPropertiesKHR_noThrow(PhysicalDevice physicalDevice, vector<DisplayPropertiesKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<DisplayPlanePropertiesKHR> getPhysicalDeviceDisplayPlanePropertiesKHR_throw(PhysicalDevice physicalDevice) {
    vector<DisplayPlanePropertiesKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceDisplayPlanePropertiesKHR_noThrow(PhysicalDevice physicalDevice, vector<DisplayPlanePropertiesKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<DisplayKHR> getDisplayPlaneSupportedDisplaysKHR_throw(PhysicalDevice physicalDevice, uint32_t planeIndex) {
    vector<DisplayKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetDisplayPlaneSupportedDisplaysKHR(physicalDevice.handle(), planeIndex, &n, nullptr);
        checkForSuccessValue(r, "vkGetDisplayPlaneSupportedDisplaysKHR");
        v.alloc(n);
        r = funcs.vkGetDisplayPlaneSupportedDisplaysKHR(physicalDevice.handle(), planeIndex, &n, reinterpret_cast<DisplayKHR::HandleType*>(v.data()));
        checkSuccess(r, "vkGetDisplayPlaneSupportedDisplaysKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getDisplayPlaneSupportedDisplaysKHR_noThrow(PhysicalDevice physicalDevice, uint32_t planeIndex, vector<DisplayKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetDisplayPlaneSupportedDisplaysKHR(physicalDevice.handle(), planeIndex, &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetDisplayPlaneSupportedDisplaysKHR(physicalDevice.handle(), planeIndex, &n, reinterpret_cast<DisplayKHR::HandleType*>(v.data()));
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<DisplayModePropertiesKHR> getDisplayModePropertiesKHR_throw(PhysicalDevice physicalDevice, DisplayKHR display) {
    vector<DisplayModePropertiesKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetDisplayModePropertiesKHR(physicalDevice.handle(), display.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetDisplayModePropertiesKHR");
        v.alloc(n);
        r = funcs.vkGetDisplayModePropertiesKHR(physicalDevice.handle(), display.handle(), &n, v.data());
        checkSuccess(r, "vkGetDisplayModePropertiesKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getDisplayModePropertiesKHR_noThrow(PhysicalDevice physicalDevice, DisplayKHR display, vector<DisplayModePropertiesKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetDisplayModePropertiesKHR(physicalDevice.handle(), display.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetDisplayModePropertiesKHR(physicalDevice.handle(), display.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<PastPresentationTimingGOOGLE> getPastPresentationTimingGOOGLE_throw(SwapchainKHR swapchain) {
    vector<PastPresentationTimingGOOGLE> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPastPresentationTimingGOOGLE(detail::_device.handle(), swapchain.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPastPresentationTimingGOOGLE");
        v.alloc(n);
        r = funcs.vkGetPastPresentationTimingGOOGLE(detail::_device.handle(), swapchain.handle(), &n, v.data());
        checkSuccess(r, "vkGetPastPresentationTimingGOOGLE");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPastPresentationTimingGOOGLE_noThrow(SwapchainKHR swapchain, vector<PastPresentationTimingGOOGLE>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPastPresentationTimingGOOGLE(detail::_device.handle(), swapchain.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPastPresentationTimingGOOGLE(detail::_device.handle(), swapchain.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<SurfaceFormat2KHR> getPhysicalDeviceSurfaceFormats2KHR_throw(PhysicalDevice physicalDevice, const PhysicalDeviceSurfaceInfo2KHR& pSurfaceInfo) {
    vector<SurfaceFormat2KHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice.handle(), &pSurfaceInfo, &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceSurfaceFormats2KHR");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice.handle(), &pSurfaceInfo, &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceSurfaceFormats2KHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceSurfaceFormats2KHR_noThrow(PhysicalDevice physicalDevice, const PhysicalDeviceSurfaceInfo2KHR& pSurfaceInfo, vector<SurfaceFormat2KHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice.handle(), &pSurfaceInfo, &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice.handle(), &pSurfaceInfo, &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<PerformanceCounterKHR> enumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR_throw(PhysicalDevice physicalDevice, uint32_t queueFamilyIndex, PerformanceCounterDescriptionKHR* pCounterDescriptions) {
    vector<PerformanceCounterKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice.handle(), queueFamilyIndex, &n, nullptr, pCounterDescriptions);
        checkForSuccessValue(r, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
        v.alloc(n);
        r = funcs.vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice.handle(), queueFamilyIndex, &n, v.data(), pCounterDescriptions);
        checkSuccess(r, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result enumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR_noThrow(PhysicalDevice physicalDevice, uint32_t queueFamilyIndex, PerformanceCounterDescriptionKHR* pCounterDescriptions, vector<PerformanceCounterKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice.handle(), queueFamilyIndex, &n, nullptr, pCounterDescriptions);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice.handle(), queueFamilyIndex, &n, v.data(), pCounterDescriptions);
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<DisplayProperties2KHR> getPhysicalDeviceDisplayProperties2KHR_throw(PhysicalDevice physicalDevice) {
    vector<DisplayProperties2KHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceDisplayProperties2KHR(physicalDevice.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceDisplayProperties2KHR");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceDisplayProperties2KHR(physicalDevice.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceDisplayProperties2KHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceDisplayProperties2KHR_noThrow(PhysicalDevice physicalDevice, vector<DisplayProperties2KHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceDisplayProperties2KHR(physicalDevice.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceDisplayProperties2KHR(physicalDevice.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<DisplayPlaneProperties2KHR> getPhysicalDeviceDisplayPlaneProperties2KHR_throw(PhysicalDevice physicalDevice) {
    vector<DisplayPlaneProperties2KHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceDisplayPlaneProperties2KHR_noThrow(PhysicalDevice physicalDevice, vector<DisplayPlaneProperties2KHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<DisplayModeProperties2KHR> getDisplayModeProperties2KHR_throw(PhysicalDevice physicalDevice, DisplayKHR display) {
    vector<DisplayModeProperties2KHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetDisplayModeProperties2KHR(physicalDevice.handle(), display.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetDisplayModeProperties2KHR");
        v.alloc(n);
        r = funcs.vkGetDisplayModeProperties2KHR(physicalDevice.handle(), display.handle(), &n, v.data());
        checkSuccess(r, "vkGetDisplayModeProperties2KHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getDisplayModeProperties2KHR_noThrow(PhysicalDevice physicalDevice, DisplayKHR display, vector<DisplayModeProperties2KHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetDisplayModeProperties2KHR(physicalDevice.handle(), display.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetDisplayModeProperties2KHR(physicalDevice.handle(), display.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<CheckpointDataNV> getQueueCheckpointDataNV_throw(Queue queue) {
    vector<CheckpointDataNV> v;
    uint32_t n;
    funcs.vkGetQueueCheckpointDataNV(queue.handle(), &n, nullptr);
    v.alloc(n);
    funcs.vkGetQueueCheckpointDataNV(queue.handle(), &n, v.data());
    return v;
}

Result getQueueCheckpointDataNV_noThrow(Queue queue, vector<CheckpointDataNV>& v) noexcept {
    uint32_t n;
    funcs.vkGetQueueCheckpointDataNV(queue.handle(), &n, nullptr);
    if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
    funcs.vkGetQueueCheckpointDataNV(queue.handle(), &n, v.data());
    return Result::eSuccess;
}

vector<CheckpointData2NV> getQueueCheckpointData2NV_throw(Queue queue) {
    vector<CheckpointData2NV> v;
    uint32_t n;
    funcs.vkGetQueueCheckpointData2NV(queue.handle(), &n, nullptr);
    v.alloc(n);
    funcs.vkGetQueueCheckpointData2NV(queue.handle(), &n, v.data());
    return v;
}

Result getQueueCheckpointData2NV_noThrow(Queue queue, vector<CheckpointData2NV>& v) noexcept {
    uint32_t n;
    funcs.vkGetQueueCheckpointData2NV(queue.handle(), &n, nullptr);
    if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
    funcs.vkGetQueueCheckpointData2NV(queue.handle(), &n, v.data());
    return Result::eSuccess;
}

vector<PhysicalDeviceFragmentShadingRateKHR> getPhysicalDeviceFragmentShadingRatesKHR_throw(PhysicalDevice physicalDevice) {
    vector<PhysicalDeviceFragmentShadingRateKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceFragmentShadingRatesKHR");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceFragmentShadingRatesKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceFragmentShadingRatesKHR_noThrow(PhysicalDevice physicalDevice, vector<PhysicalDeviceFragmentShadingRateKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<CooperativeMatrixPropertiesNV> getPhysicalDeviceCooperativeMatrixPropertiesNV_throw(PhysicalDevice physicalDevice) {
    vector<CooperativeMatrixPropertiesNV> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceCooperativeMatrixPropertiesNV_noThrow(PhysicalDevice physicalDevice, vector<CooperativeMatrixPropertiesNV>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<FramebufferMixedSamplesCombinationNV> getPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV_throw(PhysicalDevice physicalDevice) {
    vector<FramebufferMixedSamplesCombinationNV> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV_noThrow(PhysicalDevice physicalDevice, vector<FramebufferMixedSamplesCombinationNV>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
vector<PresentModeKHR> getPhysicalDeviceSurfacePresentModes2EXT_throw(PhysicalDevice physicalDevice, const PhysicalDeviceSurfaceInfo2KHR& pSurfaceInfo) {
    vector<PresentModeKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice.handle(), &pSurfaceInfo, &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceSurfacePresentModes2EXT");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice.handle(), &pSurfaceInfo, &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceSurfacePresentModes2EXT");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceSurfacePresentModes2EXT_noThrow(PhysicalDevice physicalDevice, const PhysicalDeviceSurfaceInfo2KHR& pSurfaceInfo, vector<PresentModeKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice.handle(), &pSurfaceInfo, &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice.handle(), &pSurfaceInfo, &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

#endif // VK_USE_PLATFORM_WIN32_KHR
vector<PipelineExecutablePropertiesKHR> getPipelineExecutablePropertiesKHR_throw(const PipelineInfoKHR& pPipelineInfo) {
    vector<PipelineExecutablePropertiesKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPipelineExecutablePropertiesKHR(detail::_device.handle(), &pPipelineInfo, &n, nullptr);
        checkForSuccessValue(r, "vkGetPipelineExecutablePropertiesKHR");
        v.alloc(n);
        r = funcs.vkGetPipelineExecutablePropertiesKHR(detail::_device.handle(), &pPipelineInfo, &n, v.data());
        checkSuccess(r, "vkGetPipelineExecutablePropertiesKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPipelineExecutablePropertiesKHR_noThrow(const PipelineInfoKHR& pPipelineInfo, vector<PipelineExecutablePropertiesKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPipelineExecutablePropertiesKHR(detail::_device.handle(), &pPipelineInfo, &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPipelineExecutablePropertiesKHR(detail::_device.handle(), &pPipelineInfo, &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<PipelineExecutableStatisticKHR> getPipelineExecutableStatisticsKHR_throw(const PipelineExecutableInfoKHR& pExecutableInfo) {
    vector<PipelineExecutableStatisticKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPipelineExecutableStatisticsKHR(detail::_device.handle(), &pExecutableInfo, &n, nullptr);
        checkForSuccessValue(r, "vkGetPipelineExecutableStatisticsKHR");
        v.alloc(n);
        r = funcs.vkGetPipelineExecutableStatisticsKHR(detail::_device.handle(), &pExecutableInfo, &n, v.data());
        checkSuccess(r, "vkGetPipelineExecutableStatisticsKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPipelineExecutableStatisticsKHR_noThrow(const PipelineExecutableInfoKHR& pExecutableInfo, vector<PipelineExecutableStatisticKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPipelineExecutableStatisticsKHR(detail::_device.handle(), &pExecutableInfo, &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPipelineExecutableStatisticsKHR(detail::_device.handle(), &pExecutableInfo, &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<PipelineExecutableInternalRepresentationKHR> getPipelineExecutableInternalRepresentationsKHR_throw(const PipelineExecutableInfoKHR& pExecutableInfo) {
    vector<PipelineExecutableInternalRepresentationKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPipelineExecutableInternalRepresentationsKHR(detail::_device.handle(), &pExecutableInfo, &n, nullptr);
        checkForSuccessValue(r, "vkGetPipelineExecutableInternalRepresentationsKHR");
        v.alloc(n);
        r = funcs.vkGetPipelineExecutableInternalRepresentationsKHR(detail::_device.handle(), &pExecutableInfo, &n, v.data());
        checkSuccess(r, "vkGetPipelineExecutableInternalRepresentationsKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPipelineExecutableInternalRepresentationsKHR_noThrow(const PipelineExecutableInfoKHR& pExecutableInfo, vector<PipelineExecutableInternalRepresentationKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPipelineExecutableInternalRepresentationsKHR(detail::_device.handle(), &pExecutableInfo, &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPipelineExecutableInternalRepresentationsKHR(detail::_device.handle(), &pExecutableInfo, &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<TilePropertiesQCOM> getFramebufferTilePropertiesQCOM_throw(Framebuffer framebuffer) {
    vector<TilePropertiesQCOM> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetFramebufferTilePropertiesQCOM(detail::_device.handle(), framebuffer.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetFramebufferTilePropertiesQCOM");
        v.alloc(n);
        r = funcs.vkGetFramebufferTilePropertiesQCOM(detail::_device.handle(), framebuffer.handle(), &n, v.data());
        checkSuccess(r, "vkGetFramebufferTilePropertiesQCOM");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getFramebufferTilePropertiesQCOM_noThrow(Framebuffer framebuffer, vector<TilePropertiesQCOM>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetFramebufferTilePropertiesQCOM(detail::_device.handle(), framebuffer.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetFramebufferTilePropertiesQCOM(detail::_device.handle(), framebuffer.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<OpticalFlowImageFormatPropertiesNV> getPhysicalDeviceOpticalFlowImageFormatsNV_throw(PhysicalDevice physicalDevice, const OpticalFlowImageFormatInfoNV& pOpticalFlowImageFormatInfo) {
    vector<OpticalFlowImageFormatPropertiesNV> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceOpticalFlowImageFormatsNV(physicalDevice.handle(), &pOpticalFlowImageFormatInfo, &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceOpticalFlowImageFormatsNV");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceOpticalFlowImageFormatsNV(physicalDevice.handle(), &pOpticalFlowImageFormatInfo, &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceOpticalFlowImageFormatsNV");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceOpticalFlowImageFormatsNV_noThrow(PhysicalDevice physicalDevice, const OpticalFlowImageFormatInfoNV& pOpticalFlowImageFormatInfo, vector<OpticalFlowImageFormatPropertiesNV>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceOpticalFlowImageFormatsNV(physicalDevice.handle(), &pOpticalFlowImageFormatInfo, &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceOpticalFlowImageFormatsNV(physicalDevice.handle(), &pOpticalFlowImageFormatInfo, &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<CooperativeVectorPropertiesNV> getPhysicalDeviceCooperativeVectorPropertiesNV_throw(PhysicalDevice physicalDevice) {
    vector<CooperativeVectorPropertiesNV> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceCooperativeVectorPropertiesNV(physicalDevice.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceCooperativeVectorPropertiesNV");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceCooperativeVectorPropertiesNV(physicalDevice.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceCooperativeVectorPropertiesNV");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceCooperativeVectorPropertiesNV_noThrow(PhysicalDevice physicalDevice, vector<CooperativeVectorPropertiesNV>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceCooperativeVectorPropertiesNV(physicalDevice.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceCooperativeVectorPropertiesNV(physicalDevice.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<CooperativeMatrixPropertiesKHR> getPhysicalDeviceCooperativeMatrixPropertiesKHR_throw(PhysicalDevice physicalDevice) {
    vector<CooperativeMatrixPropertiesKHR> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(physicalDevice.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(physicalDevice.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceCooperativeMatrixPropertiesKHR_noThrow(PhysicalDevice physicalDevice, vector<CooperativeMatrixPropertiesKHR>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(physicalDevice.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(physicalDevice.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<DataGraphPipelineSessionBindPointRequirementARM> getDataGraphPipelineSessionBindPointRequirementsARM_throw(const DataGraphPipelineSessionBindPointRequirementsInfoARM& pInfo) {
    vector<DataGraphPipelineSessionBindPointRequirementARM> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetDataGraphPipelineSessionBindPointRequirementsARM(detail::_device.handle(), &pInfo, &n, nullptr);
        checkForSuccessValue(r, "vkGetDataGraphPipelineSessionBindPointRequirementsARM");
        v.alloc(n);
        r = funcs.vkGetDataGraphPipelineSessionBindPointRequirementsARM(detail::_device.handle(), &pInfo, &n, v.data());
        checkSuccess(r, "vkGetDataGraphPipelineSessionBindPointRequirementsARM");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getDataGraphPipelineSessionBindPointRequirementsARM_noThrow(const DataGraphPipelineSessionBindPointRequirementsInfoARM& pInfo, vector<DataGraphPipelineSessionBindPointRequirementARM>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetDataGraphPipelineSessionBindPointRequirementsARM(detail::_device.handle(), &pInfo, &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetDataGraphPipelineSessionBindPointRequirementsARM(detail::_device.handle(), &pInfo, &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<DataGraphPipelinePropertyARM> getDataGraphPipelineAvailablePropertiesARM_throw(const DataGraphPipelineInfoARM& pPipelineInfo) {
    vector<DataGraphPipelinePropertyARM> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetDataGraphPipelineAvailablePropertiesARM(detail::_device.handle(), &pPipelineInfo, &n, nullptr);
        checkForSuccessValue(r, "vkGetDataGraphPipelineAvailablePropertiesARM");
        v.alloc(n);
        r = funcs.vkGetDataGraphPipelineAvailablePropertiesARM(detail::_device.handle(), &pPipelineInfo, &n, v.data());
        checkSuccess(r, "vkGetDataGraphPipelineAvailablePropertiesARM");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getDataGraphPipelineAvailablePropertiesARM_noThrow(const DataGraphPipelineInfoARM& pPipelineInfo, vector<DataGraphPipelinePropertyARM>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetDataGraphPipelineAvailablePropertiesARM(detail::_device.handle(), &pPipelineInfo, &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetDataGraphPipelineAvailablePropertiesARM(detail::_device.handle(), &pPipelineInfo, &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<QueueFamilyDataGraphPropertiesARM> getPhysicalDeviceQueueFamilyDataGraphPropertiesARM_throw(PhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
    vector<QueueFamilyDataGraphPropertiesARM> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(physicalDevice.handle(), queueFamilyIndex, &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(physicalDevice.handle(), queueFamilyIndex, &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceQueueFamilyDataGraphPropertiesARM_noThrow(PhysicalDevice physicalDevice, uint32_t queueFamilyIndex, vector<QueueFamilyDataGraphPropertiesARM>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(physicalDevice.handle(), queueFamilyIndex, &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(physicalDevice.handle(), queueFamilyIndex, &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}

vector<CooperativeMatrixFlexibleDimensionsPropertiesNV> getPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV_throw(PhysicalDevice physicalDevice) {
    vector<CooperativeMatrixFlexibleDimensionsPropertiesNV> v;
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(physicalDevice.handle(), &n, nullptr);
        checkForSuccessValue(r, "vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV");
        v.alloc(n);
        r = funcs.vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(physicalDevice.handle(), &n, v.data());
        checkSuccess(r, "vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV");
    } while (r == Result::eIncomplete);
    if (n != v.size()) v.resize(n);
    return v;
}

Result getPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV_noThrow(PhysicalDevice physicalDevice, vector<CooperativeMatrixFlexibleDimensionsPropertiesNV>& v) noexcept {
    uint32_t n;
    Result r;
    do {
        r = funcs.vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(physicalDevice.handle(), &n, nullptr);
        if (r != Result::eSuccess) return r;
        if (!v.alloc_noThrow(n)) return Result::eErrorOutOfHostMemory;
        r = funcs.vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(physicalDevice.handle(), &n, v.data());
        if (int32_t(r) < 0) return r;
    } while (r == Result::eIncomplete);
    if (n != v.size())
        if (!v.resize_noThrow(n)) return Result::eErrorOutOfHostMemory;
    return Result::eSuccess;
}


} // namespace vk
