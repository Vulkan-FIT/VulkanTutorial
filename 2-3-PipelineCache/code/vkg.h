#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <stdlib.h>
#include <string.h>


namespace vk {

// foward declarations
template<typename Type> class UniqueHandle;
struct ExtensionProperties;
template<typename Type> class vector;
struct InstanceCreateInfo;
struct DeviceCreateInfo;
enum class Result;


// handle class
// author: PCJohn (peciva at fit.vut.cz)
template<typename T>
class Handle {
protected:
	T _handle;
public:
	using HandleType = T;

	Handle() noexcept  {}
	Handle(std::nullptr_t) noexcept  : _handle(nullptr) {}
	Handle(T nativeHandle) noexcept  : _handle(nativeHandle) {}
	Handle(const Handle& h) noexcept  : _handle(h._handle) {}
	Handle(const UniqueHandle<Handle<T>>& u) noexcept  : _handle(u.get().handle()) {}
	Handle& operator=(const Handle rhs) noexcept  { _handle = rhs._handle; return *this; }
	Handle& operator=(const UniqueHandle<T>& rhs) noexcept  { _handle = rhs._handle; return *this; }
	T handle() const noexcept  { return _handle; }
	explicit operator bool() const noexcept  { return _handle != nullptr; }
	bool operator==(const Handle rhs) const noexcept  { return _handle == rhs._handle; }
	bool operator!=(const Handle rhs) const noexcept  { return _handle != rhs._handle; }
};


// handle types
// author: PCJohn (peciva at fit.vut.cz)
using Instance = Handle<struct Instance_T*>;
using PhysicalDevice = Handle<struct PhysicalDevice_T*>;
using Device = Handle<struct Device_T*>;
using Queue = Handle<struct Queue_T*>;
using Buffer = Handle<struct Buffer_T*>;
using BufferView = Handle<struct BufferView_T*>;
using CommandBuffer = Handle<struct CommandBuffer_T*>;
using CommandPool = Handle<struct CommandPool_T*>;
using DescriptorPool = Handle<struct DescriptorPool_T*>;
using DescriptorSet = Handle<struct DescriptorSet_T*>;
using DescriptorSetLayout = Handle<struct DescriptorSetLayout_T*>;
using DescriptorUpdateTemplate = Handle<struct DescriptorUpdateTemplate_T*>;
using DeviceMemory = Handle<struct DeviceMemory_T*>;
using DisplayKHR = Handle<struct DisplayKHR_T*>;
using DisplayModeKHR = Handle<struct DisplayModeKHR_T*>;
using Event = Handle<struct Event_T*>;
using Fence = Handle<struct Fence_T*>;
using Framebuffer = Handle<struct Framebuffer_T*>;
using Image = Handle<struct Image_T*>;
using ImageView = Handle<struct ImageView_T*>;
using Pipeline = Handle<struct Pipeline_T*>;
using PipelineBinaryKHR = Handle<struct PipelineBinaryKHR_T*>;
using PipelineCache = Handle<struct PipelineCache_T*>;
using PipelineLayout = Handle<struct PipelineLayout_T*>;
using QueryPool = Handle<struct QueryPool_T*>;
using RenderPass = Handle<struct RenderPass_T*>;
using Sampler = Handle<struct Sampler_T*>;
using Semaphore = Handle<struct Semaphore_T*>;
using ShaderModule = Handle<struct ShaderModule_T*>;
using SurfaceKHR = Handle<struct SurfaceKHR_T*>;
using SwapchainKHR = Handle<struct SwapchainKHR_T*>;
using AccelerationStructureKHR = Handle<struct AccelerationStructureKHR_T*>;
using IndirectCommandsLayoutEXT = Handle<struct IndirectCommandsLayoutEXT_T*>;
using IndirectExecutionSetEXT = Handle<struct IndirectExecutionSetEXT_T*>;
using ShaderEXT = Handle<struct ShaderEXT_T*>;
using VideoSessionKHR = Handle<struct VideoSessionKHR_T*>;
using VideoSessionParametersKHR = Handle<struct VideoSessionParametersKHR_T*>;


// unique handles
// author: PCJohn (peciva at fit.vut.cz)
template<typename Type>
class UniqueHandle {
protected:
	Type _value;
public:
	UniqueHandle() : _value(nullptr)  {}
	explicit UniqueHandle(Type value) noexcept : _value(value)  {}
	UniqueHandle(const UniqueHandle&) = delete;
	UniqueHandle(UniqueHandle&& other) noexcept : _value(other._value)  { other._value = nullptr; }
	inline ~UniqueHandle()  { destroy(_value); }
	UniqueHandle& operator=(const UniqueHandle&) = delete;
	UniqueHandle& operator=(UniqueHandle&& other) noexcept  { reset(other._value); other._value = nullptr; return *this; }
	Type get() const noexcept  { return _value; }
	void reset(Type value = nullptr) noexcept  { if(value==_value) return; destroy(_value); _value=value; }
	Type release() noexcept  { Type r=_value; _value=nullptr; return r; }
	void swap(UniqueHandle& other)  { Type tmp=_value; _value=other._value; other._value=tmp; }
	explicit operator Type() const noexcept  { return _value; }
	explicit operator bool() const noexcept  { return _value != nullptr; }
};


using UniqueInstance = UniqueHandle<Instance>;
using UniqueDevice = UniqueHandle<Device>;
using UniqueBuffer = UniqueHandle<Buffer>;
using UniqueBufferView = UniqueHandle<BufferView>;
using UniqueCommandPool = UniqueHandle<CommandPool>;
using UniqueDescriptorPool = UniqueHandle<DescriptorPool>;
using UniqueDescriptorSetLayout = UniqueHandle<DescriptorSetLayout>;
using UniqueDescriptorUpdateTemplate = UniqueHandle<DescriptorUpdateTemplate>;
using UniqueDeviceMemory = UniqueHandle<DeviceMemory>;
using UniqueEvent = UniqueHandle<Event>;
using UniqueFence = UniqueHandle<Fence>;
using UniqueFramebuffer = UniqueHandle<Framebuffer>;
using UniqueImage = UniqueHandle<Image>;
using UniqueImageView = UniqueHandle<ImageView>;
using UniquePipeline = UniqueHandle<Pipeline>;
using UniquePipelineCache = UniqueHandle<PipelineCache>;
using UniquePipelineLayout = UniqueHandle<PipelineLayout>;
using UniqueQueryPool = UniqueHandle<QueryPool>;
using UniqueRenderPass = UniqueHandle<RenderPass>;
using UniqueSampler = UniqueHandle<Sampler>;
using UniqueSemaphore = UniqueHandle<Semaphore>;
using UniqueShaderModule = UniqueHandle<ShaderModule>;
using UniqueSurfaceKHR = UniqueHandle<SurfaceKHR>;
using UniqueSwapchainKHR = UniqueHandle<SwapchainKHR>;
#if 0 // todo
using UniqueAccelerationStructureKHR = UniqueHandle<AccelerationStructureKHR>;
using UniquePipelineBinaryKHR = UniqueHandle<PipelineBinaryKHR>;
using UniqueIndirectCommandsLayoutEXT = UniqueHandle<IndirectCommandsLayoutEXT>;
using UniqueIndirectExecutionSetEXT = UniqueHandle<IndirectExecutionSetEXT>;
using UniqueShaderEXT = UniqueHandle<ShaderEXT>;
using UniqueVideoSessionKHR = UniqueHandle<VideoSessionKHR>;
using UniqueVideoSessionParametersKHR = UniqueHandle<VideoSessionParametersKHR>;
#endif



// public Vulkan variables
// author: PCJohn (peciva at fit.vut.cz)
namespace detail {
	extern void* _library;
	extern Instance _instance;
	extern PhysicalDevice _physicalDevice;
	extern Device _device;
	extern uint32_t _instanceVersion;
}

inline void* library()  { return detail::_library; }
inline Instance instance()  { return detail::_instance; }
inline PhysicalDevice physicalDevice()  { return detail::_physicalDevice; }
inline Device device()  { return detail::_device; }
inline uint32_t enumerateInstanceVersion()  { return detail::_instanceVersion; }


// initialization and cleanUp functions
// author: PCJohn (peciva at fit.vut.cz)
void loadLib_throw();
Result loadLib_noThrow() noexcept;
inline void loadLib()  { loadLib_throw(); }

void loadLib_throw(const char* libPath);  // or const std::filesystem::path& libPath if import std
Result loadLib_noThrow(const char* libPath) noexcept;  // or const std::filesystem::path& libPath if import std
inline void loadLib(const char* libPath)  { loadLib_throw(libPath); }  // or const std::filesystem::path& libPath if import std

void initInstance_throw(const InstanceCreateInfo& createInfo);
Result initInstance_noThrow(const InstanceCreateInfo& createInfo) noexcept;
inline void initInstance(const InstanceCreateInfo& createInfo)  { initInstance_throw(createInfo); }
void initInstance(Instance instance) noexcept;

void initDevice_throw(PhysicalDevice pd, const DeviceCreateInfo& createInfo);
Result initDevice_noThrow(PhysicalDevice pd, const DeviceCreateInfo& createInfo) noexcept;
inline void initDevice(PhysicalDevice pd, const DeviceCreateInfo& createInfo)  { initDevice_throw(pd, createInfo); }
void initDevice(PhysicalDevice physicalDevice, Device device) noexcept;

template<typename T> inline T getInstanceProcAddr(const char* name) noexcept;
template<typename T> inline T getDeviceProcAddr(const char* name) noexcept;
bool isExtensionSupported(const vector<ExtensionProperties>& extensionList, const char* extensionName) noexcept;
void destroyDevice() noexcept;
void destroyInstance() noexcept;
void unloadLib() noexcept;
void cleanUp() noexcept;




// version macro replacements
// author: PCJohn (peciva at fit.vut.cz)
constexpr uint32_t makeApiVersion(uint32_t variant, uint32_t major, uint32_t minor, uint32_t patch)  { return (variant << 29) | (major << 22) | (minor << 12) | patch; }
constexpr uint32_t makeApiVersion(uint32_t major, uint32_t minor, uint32_t patch)  { return (major << 22) | (minor << 12) | patch; }
constexpr const uint32_t ApiVersion10 = makeApiVersion(0, 1, 0, 0);
constexpr const uint32_t ApiVersion11 = makeApiVersion(0, 1, 1, 0);
constexpr const uint32_t ApiVersion12 = makeApiVersion(0, 1, 2, 0);
constexpr const uint32_t ApiVersion13 = makeApiVersion(0, 1, 3, 0);
constexpr const uint32_t ApiVersion14 = makeApiVersion(0, 1, 4, 0);
constexpr uint32_t apiVersionVariant(uint32_t version)  { return version >> 29; }
constexpr uint32_t apiVersionMajor(uint32_t version)  { return (version >> 22) & 0x7f; }
constexpr uint32_t apiVersionMinor(uint32_t version)  { return (version >> 12) & 0x3ff; }
constexpr uint32_t apiVersionPatch(uint32_t version)  { return version & 0xfff; }




// helper templates
// author: PCJohn (peciva at fit.vut.cz)
namespace detail
{
	template<bool B1, typename T1, bool B2, typename T2>
	struct selectType {};

	template<typename T1, bool B2, typename T2>
	struct selectType<true, T1, B2, T2> { using type = T1; };

	template<typename T1, typename T2>
	struct selectType<false, T1, true, T2> { using type = T2; };
}




// // Flags class
// author: PCJohn (peciva at fit.vut.cz)
template <typename BitType>
class Flags {
public:

	using ValueType = detail::selectType<sizeof(BitType)==4, uint32_t, sizeof(BitType)==8, uint64_t>::type;

protected:
	ValueType _value;
public:

	// constructors
	constexpr Flags() noexcept  : _value(0) {}
	constexpr Flags(BitType bit) noexcept  : _value(static_cast<ValueType>(bit)) {}
	constexpr Flags(const Flags<BitType>& other) noexcept = default;
	constexpr explicit Flags(ValueType flags) noexcept  : _value(flags) {}

	// relational operators
	constexpr bool operator< (const Flags<BitType>& rhs) const noexcept  { return _value <  rhs._value; }
	constexpr bool operator<=(const Flags<BitType>& rhs) const noexcept  { return _value <= rhs._value; }
	constexpr bool operator> (const Flags<BitType>& rhs) const noexcept  { return _value >  rhs._value; }
	constexpr bool operator>=(const Flags<BitType>& rhs) const noexcept  { return _value >= rhs._value; }
	constexpr bool operator==(const Flags<BitType>& rhs) const noexcept  { return _value == rhs._value; }
	constexpr bool operator!=(const Flags<BitType>& rhs) const noexcept  { return _value != rhs._value; }

	// logical operator
	constexpr bool operator!() const noexcept  { return !_value; }

	// bitwise operators
	constexpr Flags<BitType> operator&(const Flags<BitType>& rhs) const noexcept  { return Flags<BitType>(_value & rhs._value); }
	constexpr Flags<BitType> operator|(const Flags<BitType>& rhs) const noexcept  { return Flags<BitType>(_value | rhs._value); }
	constexpr Flags<BitType> operator^(const Flags<BitType>& rhs) const noexcept  { return Flags<BitType>(_value ^ rhs._value); }

	// assignment operators
	constexpr Flags<BitType>& operator= (const Flags<BitType>& rhs) noexcept = default;
	constexpr Flags<BitType>& operator|=(const Flags<BitType>& rhs) noexcept  { _value |= rhs._value; return *this; }
	constexpr Flags<BitType>& operator&=(const Flags<BitType>& rhs) noexcept  { _value &= rhs._value; return *this; }
	constexpr Flags<BitType>& operator^=(const Flags<BitType>& rhs) noexcept  { _value ^= rhs._value; return *this; }

	// cast operators
	explicit constexpr operator bool() const noexcept  { return !!_value; }
	explicit constexpr operator ValueType() const noexcept  { return _value; }

};


template<typename BitType>
constexpr Flags<BitType> operator&(BitType bit, const Flags<BitType>& flags) noexcept  { return flags.operator&(bit); }

template<typename BitType>
constexpr Flags<BitType> operator|(BitType bit, const Flags<BitType>& flags) noexcept  { return flags.operator|(bit); }

template<typename BitType>
constexpr Flags<BitType> operator^(BitType bit, const Flags<BitType>& flags) noexcept  { return flags.operator^(bit); }

template<typename BitType>
constexpr Flags<BitType> operator&(BitType lhs, BitType rhs) noexcept  { return Flags<BitType>(lhs) & rhs; }

template<typename BitType>
constexpr Flags<BitType> operator|(BitType lhs, BitType rhs) noexcept  { return Flags<BitType>(lhs) | rhs; }

template<typename BitType>
constexpr Flags<BitType> operator^(BitType lhs, BitType rhs) noexcept  { return Flags<BitType>(lhs) ^ rhs; }


// enums
// taken from Vulkan headers
enum class Result
{
	eSuccess                                     = 0,
	eNotReady                                    = 1,
	eTimeout                                     = 2,
	eEventSet                                    = 3,
	eEventReset                                  = 4,
	eIncomplete                                  = 5,
	eErrorOutOfHostMemory                        = -1,
	eErrorOutOfDeviceMemory                      = -2,
	eErrorInitializationFailed                   = -3,
	eErrorDeviceLost                             = -4,
	eErrorMemoryMapFailed                        = -5,
	eErrorLayerNotPresent                        = -6,
	eErrorExtensionNotPresent                    = -7,
	eErrorFeatureNotPresent                      = -8,
	eErrorIncompatibleDriver                     = -9,
	eErrorTooManyObjects                         = -10,
	eErrorFormatNotSupported                     = -11,
	eErrorFragmentedPool                         = -12,
	eErrorUnknown                                = -13,
	eErrorOutOfPoolMemory                        = -1000069000,
	eErrorOutOfPoolMemoryKHR                     = eErrorOutOfPoolMemory,
	eErrorInvalidExternalHandle                  = -1000072003,
	eErrorInvalidExternalHandleKHR               = eErrorInvalidExternalHandle,
	eErrorFragmentation                          = -1000161000,
	eErrorFragmentationEXT                       = eErrorFragmentation,
	eErrorInvalidOpaqueCaptureAddress            = -1000257000,
	eErrorInvalidDeviceAddressEXT                = eErrorInvalidOpaqueCaptureAddress,
	eErrorInvalidOpaqueCaptureAddressKHR         = eErrorInvalidOpaqueCaptureAddress,
	ePipelineCompileRequired                     = 1000297000,
	eErrorPipelineCompileRequiredEXT             = ePipelineCompileRequired,
	ePipelineCompileRequiredEXT                  = ePipelineCompileRequired,
	eErrorSurfaceLostKHR                         = -1000000000,
	eErrorNativeWindowInUseKHR                   = -1000000001,
	eSuboptimalKHR                               = 1000001003,
	eErrorOutOfDateKHR                           = -1000001004,
	eErrorIncompatibleDisplayKHR                 = -1000003001,
	eErrorValidationFailedEXT                    = -1000011001,
	eErrorInvalidShaderNV                        = -1000012000,
	eErrorImageUsageNotSupportedKHR              = -1000023000,
	eErrorVideoPictureLayoutNotSupportedKHR      = -1000023001,
	eErrorVideoProfileOperationNotSupportedKHR   = -1000023002,
	eErrorVideoProfileFormatNotSupportedKHR      = -1000023003,
	eErrorVideoProfileCodecNotSupportedKHR       = -1000023004,
	eErrorVideoStdVersionNotSupportedKHR         = -1000023005,
	eErrorInvalidDrmFormatModifierPlaneLayoutEXT = -1000158000,
	eErrorNotPermittedKHR                        = -1000174001,
	eErrorNotPermittedEXT                        = eErrorNotPermittedKHR,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	eErrorFullScreenExclusiveModeLostEXT = -1000255000,
#endif
	eThreadIdleKHR                     = 1000268000,
	eThreadDoneKHR                     = 1000268001,
	eOperationDeferredKHR              = 1000268002,
	eOperationNotDeferredKHR           = 1000268003,
	eErrorInvalidVideoStdParametersKHR = -1000299000,
	eErrorCompressionExhaustedEXT      = -1000338000,
	eIncompatibleShaderBinaryEXT       = 1000482000,
	eErrorIncompatibleShaderBinaryEXT  = eIncompatibleShaderBinaryEXT,
};

enum class StructureType {
	eApplicationInfo = 0,
	eInstanceCreateInfo = 1,
	eDeviceQueueCreateInfo = 2,
	eDeviceCreateInfo = 3,
	eSubmitInfo = 4,
	eMemoryAllocateInfo = 5,
	eMappedMemoryRange = 6,
	eBindSparseInfo = 7,
	eFenceCreateInfo = 8,
	eSemaphoreCreateInfo = 9,
	eEventCreateInfo = 10,
	eQueryPoolCreateInfo = 11,
	eBufferCreateInfo = 12,
	eBufferViewCreateInfo = 13,
	eImageCreateInfo = 14,
	eImageViewCreateInfo = 15,
	eShaderModuleCreateInfo = 16,
	ePipelineCacheCreateInfo = 17,
	ePipelineShaderStageCreateInfo = 18,
	ePipelineVertexInputStateCreateInfo = 19,
	ePipelineInputAssemblyStateCreateInfo = 20,
	ePipelineTessellationStateCreateInfo = 21,
	ePipelineViewportStateCreateInfo = 22,
	ePipelineRasterizationStateCreateInfo = 23,
	ePipelineMultisampleStateCreateInfo = 24,
	ePipelineDepthStencilStateCreateInfo = 25,
	ePipelineColorBlendStateCreateInfo = 26,
	ePipelineDynamicStateCreateInfo = 27,
	eGraphicsPipelineCreateInfo = 28,
	eComputePipelineCreateInfo = 29,
	ePipelineLayoutCreateInfo = 30,
	eSamplerCreateInfo = 31,
	eDescriptorSetLayoutCreateInfo = 32,
	eDescriptorPoolCreateInfo = 33,
	eDescriptorSetAllocateInfo = 34,
	eWriteDescriptorSet = 35,
	eCopyDescriptorSet = 36,
	eFramebufferCreateInfo = 37,
	eRenderPassCreateInfo = 38,
	eCommandPoolCreateInfo = 39,
	eCommandBufferAllocateInfo = 40,
	eCommandBufferInheritanceInfo = 41,
	eCommandBufferBeginInfo = 42,
	eRenderPassBeginInfo = 43,
	eBufferMemoryBarrier = 44,
	eImageMemoryBarrier = 45,
	eMemoryBarrier = 46,
	eLoaderInstanceCreateInfo = 47,
	eLoaderDeviceCreateInfo = 48,
	ePhysicalDeviceSubgroupProperties = 1000094000,
	eBindBufferMemoryInfo = 1000157000,
	eBindImageMemoryInfo = 1000157001,
	ePhysicalDevice16BitStorageFeatures = 1000083000,
	eMemoryDedicatedRequirements = 1000127000,
	eMemoryDedicatedAllocateInfo = 1000127001,
	eMemoryAllocateFlagsInfo = 1000060000,
	eDeviceGroupRenderPassBeginInfo = 1000060003,
	eDeviceGroupCommandBufferBeginInfo = 1000060004,
	eDeviceGroupSubmitInfo = 1000060005,
	eDeviceGroupBindSparseInfo = 1000060006,
	eBindBufferMemoryDeviceGroupInfo = 1000060013,
	eBindImageMemoryDeviceGroupInfo = 1000060014,
	ePhysicalDeviceGroupProperties = 1000070000,
	eDeviceGroupDeviceCreateInfo = 1000070001,
	eBufferMemoryRequirementsInfo2 = 1000146000,
	eImageMemoryRequirementsInfo2 = 1000146001,
	eImageSparseMemoryRequirementsInfo2 = 1000146002,
	eMemoryRequirements2 = 1000146003,
	eSparseImageMemoryRequirements2 = 1000146004,
	ePhysicalDeviceFeatures2 = 1000059000,
	ePhysicalDeviceProperties2 = 1000059001,
	eFormatProperties2 = 1000059002,
	eImageFormatProperties2 = 1000059003,
	ePhysicalDeviceImageFormatInfo2 = 1000059004,
	eQueueFamilyProperties2 = 1000059005,
	ePhysicalDeviceMemoryProperties2 = 1000059006,
	eSparseImageFormatProperties2 = 1000059007,
	ePhysicalDeviceSparseImageFormatInfo2 = 1000059008,
	ePhysicalDevicePointClippingProperties = 1000117000,
	eRenderPassInputAttachmentAspectCreateInfo = 1000117001,
	eImageViewUsageCreateInfo = 1000117002,
	ePipelineTessellationDomainOriginStateCreateInfo = 1000117003,
	eRenderPassMultiviewCreateInfo = 1000053000,
	ePhysicalDeviceMultiviewFeatures = 1000053001,
	ePhysicalDeviceMultiviewProperties = 1000053002,
	ePhysicalDeviceVariablePointersFeatures = 1000120000,
	ePhysicalDeviceVariablePointerFeatures = 1000120000,
	eProtectedSubmitInfo = 1000145000,
	ePhysicalDeviceProtectedMemoryFeatures = 1000145001,
	ePhysicalDeviceProtectedMemoryProperties = 1000145002,
	eDeviceQueueInfo2 = 1000145003,
	eSamplerYcbcrConversionCreateInfo = 1000156000,
	eSamplerYcbcrConversionInfo = 1000156001,
	eBindImagePlaneMemoryInfo = 1000156002,
	eImagePlaneMemoryRequirementsInfo = 1000156003,
	ePhysicalDeviceSamplerYcbcrConversionFeatures = 1000156004,
	eSamplerYcbcrConversionImageFormatProperties = 1000156005,
	eDescriptorUpdateTemplateCreateInfo = 1000085000,
	ePhysicalDeviceExternalImageFormatInfo = 1000071000,
	eExternalImageFormatProperties = 1000071001,
	ePhysicalDeviceExternalBufferInfo = 1000071002,
	eExternalBufferProperties = 1000071003,
	ePhysicalDeviceIdProperties = 1000071004,
	eExternalMemoryBufferCreateInfo = 1000072000,
	eExternalMemoryImageCreateInfo = 1000072001,
	eExportMemoryAllocateInfo = 1000072002,
	ePhysicalDeviceExternalFenceInfo = 1000112000,
	eExternalFenceProperties = 1000112001,
	eExportFenceCreateInfo = 1000113000,
	eExportSemaphoreCreateInfo = 1000077000,
	ePhysicalDeviceExternalSemaphoreInfo = 1000076000,
	eExternalSemaphoreProperties = 1000076001,
	ePhysicalDeviceMaintenance3Properties = 1000168000,
	eDescriptorSetLayoutSupport = 1000168001,
	ePhysicalDeviceShaderDrawParametersFeatures = 1000063000,
	ePhysicalDeviceShaderDrawParameterFeatures = 1000063000,
	ePhysicalDeviceVulkan11Features = 49,
	ePhysicalDeviceVulkan11Properties = 50,
	ePhysicalDeviceVulkan12Features = 51,
	ePhysicalDeviceVulkan12Properties = 52,
	eImageFormatListCreateInfo = 1000147000,
	eAttachmentDescription2 = 1000109000,
	eAttachmentReference2 = 1000109001,
	eSubpassDescription2 = 1000109002,
	eSubpassDependency2 = 1000109003,
	eRenderPassCreateInfo2 = 1000109004,
	eSubpassBeginInfo = 1000109005,
	eSubpassEndInfo = 1000109006,
	ePhysicalDevice8BitStorageFeatures = 1000177000,
	ePhysicalDeviceDriverProperties = 1000196000,
	ePhysicalDeviceShaderAtomicInt64Features = 1000180000,
	ePhysicalDeviceShaderFloat16Int8Features = 1000082000,
	ePhysicalDeviceFloatControlsProperties = 1000197000,
	eDescriptorSetLayoutBindingFlagsCreateInfo = 1000161000,
	ePhysicalDeviceDescriptorIndexingFeatures = 1000161001,
	ePhysicalDeviceDescriptorIndexingProperties = 1000161002,
	eDescriptorSetVariableDescriptorCountAllocateInfo = 1000161003,
	eDescriptorSetVariableDescriptorCountLayoutSupport = 1000161004,
	ePhysicalDeviceDepthStencilResolveProperties = 1000199000,
	eSubpassDescriptionDepthStencilResolve = 1000199001,
	ePhysicalDeviceScalarBlockLayoutFeatures = 1000221000,
	eImageStencilUsageCreateInfo = 1000246000,
	ePhysicalDeviceSamplerFilterMinmaxProperties = 1000130000,
	eSamplerReductionModeCreateInfo = 1000130001,
	ePhysicalDeviceVulkanMemoryModelFeatures = 1000211000,
	ePhysicalDeviceImagelessFramebufferFeatures = 1000108000,
	eFramebufferAttachmentsCreateInfo = 1000108001,
	eFramebufferAttachmentImageInfo = 1000108002,
	eRenderPassAttachmentBeginInfo = 1000108003,
	ePhysicalDeviceUniformBufferStandardLayoutFeatures = 1000253000,
	ePhysicalDeviceShaderSubgroupExtendedTypesFeatures = 1000175000,
	ePhysicalDeviceSeparateDepthStencilLayoutsFeatures = 1000241000,
	eAttachmentReferenceStencilLayout = 1000241001,
	eAttachmentDescriptionStencilLayout = 1000241002,
	ePhysicalDeviceHostQueryResetFeatures = 1000261000,
	ePhysicalDeviceTimelineSemaphoreFeatures = 1000207000,
	ePhysicalDeviceTimelineSemaphoreProperties = 1000207001,
	eSemaphoreTypeCreateInfo = 1000207002,
	eTimelineSemaphoreSubmitInfo = 1000207003,
	eSemaphoreWaitInfo = 1000207004,
	eSemaphoreSignalInfo = 1000207005,
	ePhysicalDeviceBufferDeviceAddressFeatures = 1000257000,
	eBufferDeviceAddressInfo = 1000244001,
	eBufferOpaqueCaptureAddressCreateInfo = 1000257002,
	eMemoryOpaqueCaptureAddressAllocateInfo = 1000257003,
	eDeviceMemoryOpaqueCaptureAddressInfo = 1000257004,
	ePhysicalDeviceVulkan13Features = 53,
	ePhysicalDeviceVulkan13Properties = 54,
	ePipelineCreationFeedbackCreateInfo = 1000192000,
	ePhysicalDeviceShaderTerminateInvocationFeatures = 1000215000,
	ePhysicalDeviceToolProperties = 1000245000,
	ePhysicalDeviceShaderDemoteToHelperInvocationFeatures = 1000276000,
	ePhysicalDevicePrivateDataFeatures = 1000295000,
	eDevicePrivateDataCreateInfo = 1000295001,
	ePrivateDataSlotCreateInfo = 1000295002,
	ePhysicalDevicePipelineCreationCacheControlFeatures = 1000297000,
	eMemoryBarrieR2 = 1000314000,
	eBufferMemoryBarrieR2 = 1000314001,
	eImageMemoryBarrieR2 = 1000314002,
	eDependencyInfo = 1000314003,
	eSubmitInfo2 = 1000314004,
	eSemaphoreSubmitInfo = 1000314005,
	eCommandBufferSubmitInfo = 1000314006,
	ePhysicalDeviceSynchronization2Features = 1000314007,
	ePhysicalDeviceZeroInitializeWorkgroupMemoryFeatures = 1000325000,
	ePhysicalDeviceImageRobustnessFeatures = 1000335000,
	eCopyBufferInfo2 = 1000337000,
	eCopyImageInfo2 = 1000337001,
	eCopyBufferToImageInfo2 = 1000337002,
	eCopyImageToBufferInfo2 = 1000337003,
	eBlitImageInfo2 = 1000337004,
	eResolveImageInfo2 = 1000337005,
	eBufferCopy2 = 1000337006,
	eImageCopy2 = 1000337007,
	eImageBlit2 = 1000337008,
	eBufferImageCopy2 = 1000337009,
	eImageResolve2 = 1000337010,
	ePhysicalDeviceSubgroupSizeControlProperties = 1000225000,
	ePipelineShaderStageRequiredSubgroupSizeCreateInfo = 1000225001,
	ePhysicalDeviceSubgroupSizeControlFeatures = 1000225002,
	ePhysicalDeviceInlineUniformBlockFeatures = 1000138000,
	ePhysicalDeviceInlineUniformBlockProperties = 1000138001,
	eWriteDescriptorSetInlineUniformBlock = 1000138002,
	eDescriptorPoolInlineUniformBlockCreateInfo = 1000138003,
	ePhysicalDeviceTextureCompressionAstcHdrFeatures = 1000066000,
	eRenderingInfo = 1000044000,
	eRenderingAttachmentInfo = 1000044001,
	ePipelineRenderingCreateInfo = 1000044002,
	ePhysicalDeviceDynamicRenderingFeatures = 1000044003,
	eCommandBufferInheritanceRenderingInfo = 1000044004,
	ePhysicalDeviceShaderIntegerDotProductFeatures = 1000280000,
	ePhysicalDeviceShaderIntegerDotProductProperties = 1000280001,
	ePhysicalDeviceTexelBufferAlignmentProperties = 1000281001,
	eFormatProperties3 = 1000360000,
	ePhysicalDeviceMaintenance4Features = 1000413000,
	ePhysicalDeviceMaintenance4Properties = 1000413001,
	eDeviceBufferMemoryRequirements = 1000413002,
	eDeviceImageMemoryRequirements = 1000413003,
	ePhysicalDeviceVulkan14Features = 55,
	ePhysicalDeviceVulkan14Properties = 56,
	eDeviceQueueGlobalPriorityCreateInfo = 1000174000,
	ePhysicalDeviceGlobalPriorityQueryFeatures = 1000388000,
	eQueueFamilyGlobalPriorityProperties = 1000388001,
	ePhysicalDeviceShaderSubgroupRotateFeatures = 1000416000,
	ePhysicalDeviceShaderFloatControls2Features = 1000528000,
	ePhysicalDeviceShaderExpectAssumeFeatures = 1000544000,
	ePhysicalDeviceLineRasterizationFeatures = 1000259000,
	ePipelineRasterizationLineStateCreateInfo = 1000259001,
	ePhysicalDeviceLineRasterizationProperties = 1000259002,
	ePhysicalDeviceVertexAttributeDivisorProperties = 1000525000,
	ePipelineVertexInputDivisorStateCreateInfo = 1000190001,
	ePhysicalDeviceVertexAttributeDivisorFeatures = 1000190002,
	ePhysicalDeviceIndexTypeUint8Features = 1000265000,
	eMemoryMapInfo = 1000271000,
	eMemoryUnmapInfo = 1000271001,
	ePhysicalDeviceMaintenance5Features = 1000470000,
	ePhysicalDeviceMaintenance5Properties = 1000470001,
	eRenderingAreaInfo = 1000470003,
	eDeviceImageSubresourceInfo = 1000470004,
	eSubresourceLayout2 = 1000338002,
	eImageSubresource2 = 1000338003,
	ePipelineCreateFlags2CreateInfo = 1000470005,
	eBufferUsageFlags2CreateInfo = 1000470006,
	ePhysicalDevicePushDescriptorProperties = 1000080000,
	ePhysicalDeviceDynamicRenderingLocalReadFeatures = 1000232000,
	eRenderingAttachmentLocationInfo = 1000232001,
	eRenderingInputAttachmentIndexInfo = 1000232002,
	ePhysicalDeviceMaintenance6Features = 1000545000,
	ePhysicalDeviceMaintenance6Properties = 1000545001,
	eBindMemoryStatus = 1000545002,
	eBindDescriptorSetsInfo = 1000545003,
	ePushConstantsInfo = 1000545004,
	ePushDescriptorSetInfo = 1000545005,
	ePushDescriptorSetWithTemplateInfo = 1000545006,
	ePhysicalDevicePipelineProtectedAccessFeatures = 1000466000,
	ePipelineRobustnessCreateInfo = 1000068000,
	ePhysicalDevicePipelineRobustnessFeatures = 1000068001,
	ePhysicalDevicePipelineRobustnessProperties = 1000068002,
	ePhysicalDeviceHostImageCopyFeatures = 1000270000,
	ePhysicalDeviceHostImageCopyProperties = 1000270001,
	eMemoryToImageCopy = 1000270002,
	eImageToMemoryCopy = 1000270003,
	eCopyImageToMemoryInfo = 1000270004,
	eCopyMemoryToImageInfo = 1000270005,
	eHostImageLayoutTransitionInfo = 1000270006,
	eCopyImageToImageInfo = 1000270007,
	eSubresourceHostMemcpySize = 1000270008,
	eHostImageCopyDevicePerformanceQuery = 1000270009,
	eSwapchainCreateInfoKHR = 1000001000,
	ePresentInfoKHR = 1000001001,
	eDeviceGroupPresentCapabilitiesKHR = 1000060007,
	eImageSwapchainCreateInfoKHR = 1000060008,
	eBindImageMemorySwapchainInfoKHR = 1000060009,
	eAcquireNextImageInfoKHR = 1000060010,
	eDeviceGroupPresentInfoKHR = 1000060011,
	eDeviceGroupSwapchainCreateInfoKHR = 1000060012,
	eDisplayModeCreateInfoKHR = 1000002000,
	eDisplaySurfaceCreateInfoKHR = 1000002001,
	eDisplayPresentInfoKHR = 1000003000,
	eDebugReportCallbackCreateInfoEXT = 1000011000,
	eDebugReportCreateInfoEXT = 1000011000,
	ePipelineRasterizationStateRasterizationOrderAMD = 1000018000,
	eDebugMarkerObjectNameInfoEXT = 1000022000,
	eDebugMarkerObjectTagInfoEXT = 1000022001,
	eDebugMarkerMarkerInfoEXT = 1000022002,
	eVideoProfileInfoKHR = 1000023000,
	eVideoCapabilitiesKHR = 1000023001,
	eVideoPictureResourceInfoKHR = 1000023002,
	eVideoSessionMemoryRequirementsKHR = 1000023003,
	eBindVideoSessionMemoryInfoKHR = 1000023004,
	eVideoSessionCreateInfoKHR = 1000023005,
	eVideoSessionParametersCreateInfoKHR = 1000023006,
	eVideoSessionParametersUpdateInfoKHR = 1000023007,
	eVideoBeginCodingInfoKHR = 1000023008,
	eVideoEndCodingInfoKHR = 1000023009,
	eVideoCodingControlInfoKHR = 1000023010,
	eVideoReferenceSlotInfoKHR = 1000023011,
	eQueueFamilyVideoPropertiesKHR = 1000023012,
	eVideoProfileListInfoKHR = 1000023013,
	ePhysicalDeviceVideoFormatInfoKHR = 1000023014,
	eVideoFormatPropertiesKHR = 1000023015,
	eQueueFamilyQueryResultStatusPropertiesKHR = 1000023016,
	eVideoDecodeInfoKHR = 1000024000,
	eVideoDecodeCapabilitiesKHR = 1000024001,
	eVideoDecodeUsageInfoKHR = 1000024002,
	eDedicatedAllocationImageCreateInfoNV = 1000026000,
	eDedicatedAllocationBufferCreateInfoNV = 1000026001,
	eDedicatedAllocationMemoryAllocateInfoNV = 1000026002,
	ePhysicalDeviceTransformFeedbackFeaturesEXT = 1000028000,
	ePhysicalDeviceTransformFeedbackPropertiesEXT = 1000028001,
	ePipelineRasterizationStateStreamCreateInfoEXT = 1000028002,
	eCuModuleCreateInfoNVX = 1000029000,
	eCuFunctionCreateInfoNVX = 1000029001,
	eCuLaunchInfoNVX = 1000029002,
	eCuModuleTexturingModeCreateInfoNVX = 1000029004,
	eImageViewHandleInfoNVX = 1000030000,
	eImageViewAddressPropertiesNVX = 1000030001,
	eVideoEncodeH264CapabilitiesKHR = 1000038000,
	eVideoEncodeH264SessionParametersCreateInfoKHR = 1000038001,
	eVideoEncodeH264SessionParametersAddInfoKHR = 1000038002,
	eVideoEncodeH264PictureInfoKHR = 1000038003,
	eVideoEncodeH264DpbSlotInfoKHR = 1000038004,
	eVideoEncodeH264NaluSliceInfoKHR = 1000038005,
	eVideoEncodeH264GopRemainingFrameInfoKHR = 1000038006,
	eVideoEncodeH264ProfileInfoKHR = 1000038007,
	eVideoEncodeH264RateControlInfoKHR = 1000038008,
	eVideoEncodeH264RateControlLayerInfoKHR = 1000038009,
	eVideoEncodeH264SessionCreateInfoKHR = 1000038010,
	eVideoEncodeH264QualityLevelPropertiesKHR = 1000038011,
	eVideoEncodeH264SessionParametersGetInfoKHR = 1000038012,
	eVideoEncodeH264SessionParametersFeedbackInfoKHR = 1000038013,
	eVideoEncodeH265CapabilitiesKHR = 1000039000,
	eVideoEncodeH265SessionParametersCreateInfoKHR = 1000039001,
	eVideoEncodeH265SessionParametersAddInfoKHR = 1000039002,
	eVideoEncodeH265PictureInfoKHR = 1000039003,
	eVideoEncodeH265DpbSlotInfoKHR = 1000039004,
	eVideoEncodeH265NaluSliceSegmentInfoKHR = 1000039005,
	eVideoEncodeH265GopRemainingFrameInfoKHR = 1000039006,
	eVideoEncodeH265ProfileInfoKHR = 1000039007,
	eVideoEncodeH265RateControlInfoKHR = 1000039009,
	eVideoEncodeH265RateControlLayerInfoKHR = 1000039010,
	eVideoEncodeH265SessionCreateInfoKHR = 1000039011,
	eVideoEncodeH265QualityLevelPropertiesKHR = 1000039012,
	eVideoEncodeH265SessionParametersGetInfoKHR = 1000039013,
	eVideoEncodeH265SessionParametersFeedbackInfoKHR = 1000039014,
	eVideoDecodeH264CapabilitiesKHR = 1000040000,
	eVideoDecodeH264PictureInfoKHR = 1000040001,
	eVideoDecodeH264ProfileInfoKHR = 1000040003,
	eVideoDecodeH264SessionParametersCreateInfoKHR = 1000040004,
	eVideoDecodeH264SessionParametersAddInfoKHR = 1000040005,
	eVideoDecodeH264DpbSlotInfoKHR = 1000040006,
	eTextureLodGatherFormatPropertiesAMD = 1000041000,
	eRenderingInfoKHR = 1000044000,
	eRenderingAttachmentInfoKHR = 1000044001,
	ePipelineRenderingCreateInfoKHR = 1000044002,
	ePhysicalDeviceDynamicRenderingFeaturesKHR = 1000044003,
	eCommandBufferInheritanceRenderingInfoKHR = 1000044004,
	ePhysicalDeviceCornerSampledImageFeaturesNV = 1000050000,
	eRenderPassMultiviewCreateInfoKHR = 1000053000,
	ePhysicalDeviceMultiviewFeaturesKHR = 1000053001,
	ePhysicalDeviceMultiviewPropertiesKHR = 1000053002,
	eExternalMemoryImageCreateInfoNV = 1000056000,
	eExportMemoryAllocateInfoNV = 1000056001,
	ePhysicalDeviceFeatures2KHR = 1000059000,
	ePhysicalDeviceProperties2KHR = 1000059001,
	eFormatProperties2KHR = 1000059002,
	eImageFormatProperties2KHR = 1000059003,
	ePhysicalDeviceImageFormatInfo2KHR = 1000059004,
	eQueueFamilyProperties2KHR = 1000059005,
	ePhysicalDeviceMemoryProperties2KHR = 1000059006,
	eSparseImageFormatProperties2KHR = 1000059007,
	ePhysicalDeviceSparseImageFormatInfo2KHR = 1000059008,
	eMemoryAllocateFlagsInfoKHR = 1000060000,
	eDeviceGroupRenderPassBeginInfoKHR = 1000060003,
	eDeviceGroupCommandBufferBeginInfoKHR = 1000060004,
	eDeviceGroupSubmitInfoKHR = 1000060005,
	eDeviceGroupBindSparseInfoKHR = 1000060006,
	eBindBufferMemoryDeviceGroupInfoKHR = 1000060013,
	eBindImageMemoryDeviceGroupInfoKHR = 1000060014,
	eValidationFlagsEXT = 1000061000,
	ePhysicalDeviceTextureCompressionAstcHdrFeaturesEXT = 1000066000,
	eImageViewAstcDecodeModeEXT = 1000067000,
	ePhysicalDeviceAstcDecodeFeaturesEXT = 1000067001,
	ePipelineRobustnessCreateInfoEXT = 1000068000,
	ePhysicalDevicePipelineRobustnessFeaturesEXT = 1000068001,
	ePhysicalDevicePipelineRobustnessPropertiesEXT = 1000068002,
	ePhysicalDeviceGroupPropertiesKHR = 1000070000,
	eDeviceGroupDeviceCreateInfoKHR = 1000070001,
	ePhysicalDeviceExternalImageFormatInfoKHR = 1000071000,
	eExternalImageFormatPropertiesKHR = 1000071001,
	ePhysicalDeviceExternalBufferInfoKHR = 1000071002,
	eExternalBufferPropertiesKHR = 1000071003,
	ePhysicalDeviceIdPropertiesKHR = 1000071004,
	eExternalMemoryBufferCreateInfoKHR = 1000072000,
	eExternalMemoryImageCreateInfoKHR = 1000072001,
	eExportMemoryAllocateInfoKHR = 1000072002,
	eImportMemoryFdInfoKHR = 1000074000,
	eMemoryFdPropertiesKHR = 1000074001,
	eMemoryGetFdInfoKHR = 1000074002,
	ePhysicalDeviceExternalSemaphoreInfoKHR = 1000076000,
	eExternalSemaphorePropertiesKHR = 1000076001,
	eExportSemaphoreCreateInfoKHR = 1000077000,
	eImportSemaphoreFdInfoKHR = 1000079000,
	eSemaphoreGetFdInfoKHR = 1000079001,
	ePhysicalDevicePushDescriptorPropertiesKHR = 1000080000,
	eCommandBufferInheritanceConditionalRenderingInfoEXT = 1000081000,
	ePhysicalDeviceConditionalRenderingFeaturesEXT = 1000081001,
	eConditionalRenderingBeginInfoEXT = 1000081002,
	ePhysicalDeviceShaderFloat16Int8FeaturesKHR = 1000082000,
	ePhysicalDeviceFloat16Int8FeaturesKHR = 1000082000,
	ePhysicalDevice16BitStorageFeaturesKHR = 1000083000,
	ePresentRegionsKHR = 1000084000,
	eDescriptorUpdateTemplateCreateInfoKHR = 1000085000,
	ePipelineViewportWScalingStateCreateInfoNV = 1000087000,
	eSurfaceCapabilities2EXT = 1000090000,
	eDisplayPowerInfoEXT = 1000091000,
	eDeviceEventInfoEXT = 1000091001,
	eDisplayEventInfoEXT = 1000091002,
	eSwapchainCounterCreateInfoEXT = 1000091003,
	ePresentTimesInfoGOOGLE = 1000092000,
	ePhysicalDeviceMultiviewPerViewAttributesPropertiesNVX = 1000097000,
	eMultiviewPerViewAttributesInfoNVX = 1000044009,
	ePipelineViewportSwizzleStateCreateInfoNV = 1000098000,
	ePhysicalDeviceDiscardRectanglePropertiesEXT = 1000099000,
	ePipelineDiscardRectangleStateCreateInfoEXT = 1000099001,
	ePhysicalDeviceConservativeRasterizationPropertiesEXT = 1000101000,
	ePipelineRasterizationConservativeStateCreateInfoEXT = 1000101001,
	ePhysicalDeviceDepthClipEnableFeaturesEXT = 1000102000,
	ePipelineRasterizationDepthClipStateCreateInfoEXT = 1000102001,
	eHdrMetadataEXT = 1000105000,
	ePhysicalDeviceImagelessFramebufferFeaturesKHR = 1000108000,
	eFramebufferAttachmentsCreateInfoKHR = 1000108001,
	eFramebufferAttachmentImageInfoKHR = 1000108002,
	eRenderPassAttachmentBeginInfoKHR = 1000108003,
	eAttachmentDescription2KHR = 1000109000,
	eAttachmentReference2KHR = 1000109001,
	eSubpassDescription2KHR = 1000109002,
	eSubpassDependency2KHR = 1000109003,
	eRenderPassCreateInfo2KHR = 1000109004,
	eSubpassBeginInfoKHR = 1000109005,
	eSubpassEndInfoKHR = 1000109006,
	ePhysicalDeviceRelaxedLineRasterizationFeaturesIMG = 1000110000,
	eSharedPresentSurfaceCapabilitiesKHR = 1000111000,
	ePhysicalDeviceExternalFenceInfoKHR = 1000112000,
	eExternalFencePropertiesKHR = 1000112001,
	eExportFenceCreateInfoKHR = 1000113000,
	eImportFenceFdInfoKHR = 1000115000,
	eFenceGetFdInfoKHR = 1000115001,
	ePhysicalDevicePerformanceQueryFeaturesKHR = 1000116000,
	ePhysicalDevicePerformanceQueryPropertiesKHR = 1000116001,
	eQueryPoolPerformanceCreateInfoKHR = 1000116002,
	ePerformanceQuerySubmitInfoKHR = 1000116003,
	eAcquireProfilingLockInfoKHR = 1000116004,
	ePerformanceCounterKHR = 1000116005,
	ePerformanceCounterDescriptionKHR = 1000116006,
	ePhysicalDevicePointClippingPropertiesKHR = 1000117000,
	eRenderPassInputAttachmentAspectCreateInfoKHR = 1000117001,
	eImageViewUsageCreateInfoKHR = 1000117002,
	ePipelineTessellationDomainOriginStateCreateInfoKHR = 1000117003,
	ePhysicalDeviceSurfaceInfo2KHR = 1000119000,
	eSurfaceCapabilities2KHR = 1000119001,
	eSurfaceFormat2KHR = 1000119002,
	ePhysicalDeviceVariablePointersFeaturesKHR = 1000120000,
	ePhysicalDeviceVariablePointerFeaturesKHR = 1000120000,
	eDisplayProperties2KHR = 1000121000,
	eDisplayPlaneProperties2KHR = 1000121001,
	eDisplayModeProperties2KHR = 1000121002,
	eDisplayPlaneInfo2KHR = 1000121003,
	eDisplayPlaneCapabilities2KHR = 1000121004,
	eMemoryDedicatedRequirementsKHR = 1000127000,
	eMemoryDedicatedAllocateInfoKHR = 1000127001,
	eDebugUtilsObjectNameInfoEXT = 1000128000,
	eDebugUtilsObjectTagInfoEXT = 1000128001,
	eDebugUtilsLabelEXT = 1000128002,
	eDebugUtilsMessengerCallbackDataEXT = 1000128003,
	eDebugUtilsMessengerCreateInfoEXT = 1000128004,
	ePhysicalDeviceSamplerFilterMinmaxPropertiesEXT = 1000130000,
	eSamplerReductionModeCreateInfoEXT = 1000130001,
	eAttachmentSampleCountInfoAMD = 1000044008,
	ePhysicalDeviceInlineUniformBlockFeaturesEXT = 1000138000,
	ePhysicalDeviceInlineUniformBlockPropertiesEXT = 1000138001,
	eWriteDescriptorSetInlineUniformBlockEXT = 1000138002,
	eDescriptorPoolInlineUniformBlockCreateInfoEXT = 1000138003,
	eSampleLocationsInfoEXT = 1000143000,
	eRenderPassSampleLocationsBeginInfoEXT = 1000143001,
	ePipelineSampleLocationsStateCreateInfoEXT = 1000143002,
	ePhysicalDeviceSampleLocationsPropertiesEXT = 1000143003,
	eMultisamplePropertiesEXT = 1000143004,
	eBufferMemoryRequirementsInfo2KHR = 1000146000,
	eImageMemoryRequirementsInfo2KHR = 1000146001,
	eImageSparseMemoryRequirementsInfo2KHR = 1000146002,
	eMemoryRequirements2KHR = 1000146003,
	eSparseImageMemoryRequirements2KHR = 1000146004,
	eImageFormatListCreateInfoKHR = 1000147000,
	ePhysicalDeviceBlendOperationAdvancedFeaturesEXT = 1000148000,
	ePhysicalDeviceBlendOperationAdvancedPropertiesEXT = 1000148001,
	ePipelineColorBlendAdvancedStateCreateInfoEXT = 1000148002,
	ePipelineCoverageToColorStateCreateInfoNV = 1000149000,
	eWriteDescriptorSetAccelerationStructureKHR = 1000150007,
	eAccelerationStructureBuildGeometryInfoKHR = 1000150000,
	eAccelerationStructureDeviceAddressInfoKHR = 1000150002,
	eAccelerationStructureGeometryAabbsDataKHR = 1000150003,
	eAccelerationStructureGeometryInstancesDataKHR = 1000150004,
	eAccelerationStructureGeometryTrianglesDataKHR = 1000150005,
	eAccelerationStructureGeometryKHR = 1000150006,
	eAccelerationStructureVersionInfoKHR = 1000150009,
	eCopyAccelerationStructureInfoKHR = 1000150010,
	eCopyAccelerationStructureToMemoryInfoKHR = 1000150011,
	eCopyMemoryToAccelerationStructureInfoKHR = 1000150012,
	ePhysicalDeviceAccelerationStructureFeaturesKHR = 1000150013,
	ePhysicalDeviceAccelerationStructurePropertiesKHR = 1000150014,
	eAccelerationStructureCreateInfoKHR = 1000150017,
	eAccelerationStructureBuildSizesInfoKHR = 1000150020,
	ePhysicalDeviceRayTracingPipelineFeaturesKHR = 1000347000,
	ePhysicalDeviceRayTracingPipelinePropertiesKHR = 1000347001,
	eRayTracingPipelineCreateInfoKHR = 1000150015,
	eRayTracingShaderGroupCreateInfoKHR = 1000150016,
	eRayTracingPipelineInterfaceCreateInfoKHR = 1000150018,
	ePhysicalDeviceRayQueryFeaturesKHR = 1000348013,
	ePipelineCoverageModulationStateCreateInfoNV = 1000152000,
	eAttachmentSampleCountInfoNV = 1000044008,
	ePhysicalDeviceShaderSmBuiltinsFeaturesNV = 1000154000,
	ePhysicalDeviceShaderSmBuiltinsPropertiesNV = 1000154001,
	eSamplerYcbcrConversionCreateInfoKHR = 1000156000,
	eSamplerYcbcrConversionInfoKHR = 1000156001,
	eBindImagePlaneMemoryInfoKHR = 1000156002,
	eImagePlaneMemoryRequirementsInfoKHR = 1000156003,
	ePhysicalDeviceSamplerYcbcrConversionFeaturesKHR = 1000156004,
	eSamplerYcbcrConversionImageFormatPropertiesKHR = 1000156005,
	eBindBufferMemoryInfoKHR = 1000157000,
	eBindImageMemoryInfoKHR = 1000157001,
	eDrmFormatModifierPropertiesListEXT = 1000158000,
	ePhysicalDeviceImageDrmFormatModifierInfoEXT = 1000158002,
	eImageDrmFormatModifierListCreateInfoEXT = 1000158003,
	eImageDrmFormatModifierExplicitCreateInfoEXT = 1000158004,
	eImageDrmFormatModifierPropertiesEXT = 1000158005,
	eDrmFormatModifierPropertiesList2EXT = 1000158006,
	eValidationCacheCreateInfoEXT = 1000160000,
	eShaderModuleValidationCacheCreateInfoEXT = 1000160001,
	eDescriptorSetLayoutBindingFlagsCreateInfoEXT = 1000161000,
	ePhysicalDeviceDescriptorIndexingFeaturesEXT = 1000161001,
	ePhysicalDeviceDescriptorIndexingPropertiesEXT = 1000161002,
	eDescriptorSetVariableDescriptorCountAllocateInfoEXT = 1000161003,
	eDescriptorSetVariableDescriptorCountLayoutSupportEXT = 1000161004,
	ePipelineViewportShadingRateImageStateCreateInfoNV = 1000164000,
	ePhysicalDeviceShadingRateImageFeaturesNV = 1000164001,
	ePhysicalDeviceShadingRateImagePropertiesNV = 1000164002,
	ePipelineViewportCoarseSampleOrderStateCreateInfoNV = 1000164005,
	eRayTracingPipelineCreateInfoNV = 1000165000,
	eAccelerationStructureCreateInfoNV = 1000165001,
	eGeometryNV = 1000165003,
	eGeometryTrianglesNV = 1000165004,
	eGeometryAabbNV = 1000165005,
	eBindAccelerationStructureMemoryInfoNV = 1000165006,
	eWriteDescriptorSetAccelerationStructureNV = 1000165007,
	eAccelerationStructureMemoryRequirementsInfoNV = 1000165008,
	ePhysicalDeviceRayTracingPropertiesNV = 1000165009,
	eRayTracingShaderGroupCreateInfoNV = 1000165011,
	eAccelerationStructureInfoNV = 1000165012,
	ePhysicalDeviceRepresentativeFragmentTestFeaturesNV = 1000166000,
	ePipelineRepresentativeFragmentTestStateCreateInfoNV = 1000166001,
	ePhysicalDeviceMaintenance3PropertiesKHR = 1000168000,
	eDescriptorSetLayoutSupportKHR = 1000168001,
	ePhysicalDeviceImageViewImageFormatInfoEXT = 1000170000,
	eFilterCubicImageViewImageFormatPropertiesEXT = 1000170001,
	eDeviceQueueGlobalPriorityCreateInfoEXT = 1000174000,
	ePhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR = 1000175000,
	ePhysicalDevice8BitStorageFeaturesKHR = 1000177000,
	eImportMemoryHostPointerInfoEXT = 1000178000,
	eMemoryHostPointerPropertiesEXT = 1000178001,
	ePhysicalDeviceExternalMemoryHostPropertiesEXT = 1000178002,
	ePhysicalDeviceShaderAtomicInt64FeaturesKHR = 1000180000,
	ePhysicalDeviceShaderClockFeaturesKHR = 1000181000,
	ePipelineCompilerControlCreateInfoAMD = 1000183000,
	eCalibratedTimestampInfoEXT = 1000184000,
	ePhysicalDeviceShaderCorePropertiesAMD = 1000185000,
	eVideoDecodeH265CapabilitiesKHR = 1000187000,
	eVideoDecodeH265SessionParametersCreateInfoKHR = 1000187001,
	eVideoDecodeH265SessionParametersAddInfoKHR = 1000187002,
	eVideoDecodeH265ProfileInfoKHR = 1000187003,
	eVideoDecodeH265PictureInfoKHR = 1000187004,
	eVideoDecodeH265DpbSlotInfoKHR = 1000187005,
	eDeviceQueueGlobalPriorityCreateInfoKHR = 1000174000,
	ePhysicalDeviceGlobalPriorityQueryFeaturesKHR = 1000388000,
	eQueueFamilyGlobalPriorityPropertiesKHR = 1000388001,
	eDeviceMemoryOverallocationCreateInfoAMD = 1000189000,
	ePhysicalDeviceVertexAttributeDivisorPropertiesEXT = 1000190000,
	ePipelineVertexInputDivisorStateCreateInfoEXT = 1000190001,
	ePhysicalDeviceVertexAttributeDivisorFeaturesEXT = 1000190002,
	ePipelineCreationFeedbackCreateInfoEXT = 1000192000,
	ePhysicalDeviceDriverPropertiesKHR = 1000196000,
	ePhysicalDeviceFloatControlsPropertiesKHR = 1000197000,
	ePhysicalDeviceDepthStencilResolvePropertiesKHR = 1000199000,
	eSubpassDescriptionDepthStencilResolveKHR = 1000199001,
	ePhysicalDeviceComputeShaderDerivativesFeaturesNV = 1000201000,
	ePhysicalDeviceMeshShaderFeaturesNV = 1000202000,
	ePhysicalDeviceMeshShaderPropertiesNV = 1000202001,
	ePhysicalDeviceFragmentShaderBarycentricFeaturesNV = 1000203000,
	ePhysicalDeviceShaderImageFootprintFeaturesNV = 1000204000,
	ePipelineViewportExclusiveScissorStateCreateInfoNV = 1000205000,
	ePhysicalDeviceExclusiveScissorFeaturesNV = 1000205002,
	eCheckpointDataNV = 1000206000,
	eQueueFamilyCheckpointPropertiesNV = 1000206001,
	eQueueFamilyCheckpointProperties2NV = 1000314008,
	eCheckpointDatA2NV = 1000314009,
	ePhysicalDeviceTimelineSemaphoreFeaturesKHR = 1000207000,
	ePhysicalDeviceTimelineSemaphorePropertiesKHR = 1000207001,
	eSemaphoreTypeCreateInfoKHR = 1000207002,
	eTimelineSemaphoreSubmitInfoKHR = 1000207003,
	eSemaphoreWaitInfoKHR = 1000207004,
	eSemaphoreSignalInfoKHR = 1000207005,
	ePhysicalDeviceShaderIntegerFunctions2FeaturesINTEL = 1000209000,
	eQueryPoolPerformanceQueryCreateInfoINTEL = 1000210000,
	eQueryPoolCreateInfoINTEL = 1000210000,
	eInitializePerformanceApiInfoINTEL = 1000210001,
	ePerformanceMarkerInfoINTEL = 1000210002,
	ePerformanceStreamMarkerInfoINTEL = 1000210003,
	ePerformanceOverrideInfoINTEL = 1000210004,
	ePerformanceConfigurationAcquireInfoINTEL = 1000210005,
	ePhysicalDeviceVulkanMemoryModelFeaturesKHR = 1000211000,
	ePhysicalDevicePciBusInfoPropertiesEXT = 1000212000,
	eDisplayNativeHdrSurfaceCapabilitiesAMD = 1000213000,
	eSwapchainDisplayNativeHdrCreateInfoAMD = 1000213001,
	ePhysicalDeviceShaderTerminateInvocationFeaturesKHR = 1000215000,
	ePhysicalDeviceFragmentDensityMapFeaturesEXT = 1000218000,
	ePhysicalDeviceFragmentDensityMapPropertiesEXT = 1000218001,
	eRenderPassFragmentDensityMapCreateInfoEXT = 1000218002,
	eRenderingFragmentDensityMapAttachmentInfoEXT = 1000044007,
	ePhysicalDeviceScalarBlockLayoutFeaturesEXT = 1000221000,
	ePhysicalDeviceSubgroupSizeControlPropertiesEXT = 1000225000,
	ePipelineShaderStageRequiredSubgroupSizeCreateInfoEXT = 1000225001,
	ePhysicalDeviceSubgroupSizeControlFeaturesEXT = 1000225002,
	eFragmentShadingRateAttachmentInfoKHR = 1000226000,
	ePipelineFragmentShadingRateStateCreateInfoKHR = 1000226001,
	ePhysicalDeviceFragmentShadingRatePropertiesKHR = 1000226002,
	ePhysicalDeviceFragmentShadingRateFeaturesKHR = 1000226003,
	ePhysicalDeviceFragmentShadingRateKHR = 1000226004,
	eRenderingFragmentShadingRateAttachmentInfoKHR = 1000044006,
	ePhysicalDeviceShaderCoreProperties2AMD = 1000227000,
	ePhysicalDeviceCoherentMemoryFeaturesAMD = 1000229000,
	ePhysicalDeviceDynamicRenderingLocalReadFeaturesKHR = 1000232000,
	eRenderingAttachmentLocationInfoKHR = 1000232001,
	eRenderingInputAttachmentIndexInfoKHR = 1000232002,
	ePhysicalDeviceShaderImageAtomicInt64FeaturesEXT = 1000234000,
	ePhysicalDeviceShaderQuadControlFeaturesKHR = 1000235000,
	ePhysicalDeviceMemoryBudgetPropertiesEXT = 1000237000,
	ePhysicalDeviceMemoryPriorityFeaturesEXT = 1000238000,
	eMemoryPriorityAllocateInfoEXT = 1000238001,
	eSurfaceProtectedCapabilitiesKHR = 1000239000,
	ePhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV = 1000240000,
	ePhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR = 1000241000,
	eAttachmentReferenceStencilLayoutKHR = 1000241001,
	eAttachmentDescriptionStencilLayoutKHR = 1000241002,
	ePhysicalDeviceBufferDeviceAddressFeaturesEXT = 1000244000,
	ePhysicalDeviceBufferAddressFeaturesEXT = 1000244000,
	eBufferDeviceAddressInfoEXT = 1000244001,
	eBufferDeviceAddressCreateInfoEXT = 1000244002,
	ePhysicalDeviceToolPropertiesEXT = 1000245000,
	eImageStencilUsageCreateInfoEXT = 1000246000,
	eValidationFeaturesEXT = 1000247000,
	ePhysicalDevicePresentWaitFeaturesKHR = 1000248000,
	ePhysicalDeviceCooperativeMatrixFeaturesNV = 1000249000,
	eCooperativeMatrixPropertiesNV = 1000249001,
	ePhysicalDeviceCooperativeMatrixPropertiesNV = 1000249002,
	ePhysicalDeviceCoverageReductionModeFeaturesNV = 1000250000,
	ePipelineCoverageReductionStateCreateInfoNV = 1000250001,
	eFramebufferMixedSamplesCombinationNV = 1000250002,
	ePhysicalDeviceFragmentShaderInterlockFeaturesEXT = 1000251000,
	ePhysicalDeviceYcbcrImageArraysFeaturesEXT = 1000252000,
	ePhysicalDeviceUniformBufferStandardLayoutFeaturesKHR = 1000253000,
	ePhysicalDeviceProvokingVertexFeaturesEXT = 1000254000,
	ePipelineRasterizationProvokingVertexStateCreateInfoEXT = 1000254001,
	ePhysicalDeviceProvokingVertexPropertiesEXT = 1000254002,
	eHeadlessSurfaceCreateInfoEXT = 1000256000,
	ePhysicalDeviceBufferDeviceAddressFeaturesKHR = 1000257000,
	eBufferDeviceAddressInfoKHR = 1000244001,
	eBufferOpaqueCaptureAddressCreateInfoKHR = 1000257002,
	eMemoryOpaqueCaptureAddressAllocateInfoKHR = 1000257003,
	eDeviceMemoryOpaqueCaptureAddressInfoKHR = 1000257004,
	ePhysicalDeviceLineRasterizationFeaturesEXT = 1000259000,
	ePipelineRasterizationLineStateCreateInfoEXT = 1000259001,
	ePhysicalDeviceLineRasterizationPropertiesEXT = 1000259002,
	ePhysicalDeviceShaderAtomicFloatFeaturesEXT = 1000260000,
	ePhysicalDeviceHostQueryResetFeaturesEXT = 1000261000,
	ePhysicalDeviceIndexTypeUint8FeaturesEXT = 1000265000,
	ePhysicalDeviceExtendedDynamicStateFeaturesEXT = 1000267000,
	ePhysicalDevicePipelineExecutablePropertiesFeaturesKHR = 1000269000,
	ePipelineInfoKHR = 1000269001,
	ePipelineExecutablePropertiesKHR = 1000269002,
	ePipelineExecutableInfoKHR = 1000269003,
	ePipelineExecutableStatisticKHR = 1000269004,
	ePipelineExecutableInternalRepresentationKHR = 1000269005,
	ePhysicalDeviceHostImageCopyFeaturesEXT = 1000270000,
	ePhysicalDeviceHostImageCopyPropertiesEXT = 1000270001,
	eMemoryToImageCopyEXT = 1000270002,
	eImageToMemoryCopyEXT = 1000270003,
	eCopyImageToMemoryInfoEXT = 1000270004,
	eCopyMemoryToImageInfoEXT = 1000270005,
	eHostImageLayoutTransitionInfoEXT = 1000270006,
	eCopyImageToImageInfoEXT = 1000270007,
	eSubresourceHostMemcpySizeEXT = 1000270008,
	eHostImageCopyDevicePerformanceQueryEXT = 1000270009,
	eMemoryMapInfoKHR = 1000271000,
	eMemoryUnmapInfoKHR = 1000271001,
	ePhysicalDeviceMapMemoryPlacedFeaturesEXT = 1000272000,
	ePhysicalDeviceMapMemoryPlacedPropertiesEXT = 1000272001,
	eMemoryMapPlacedInfoEXT = 1000272002,
	ePhysicalDeviceShaderAtomicFloat2FeaturesEXT = 1000273000,
	eSurfacePresentModeEXT = 1000274000,
	eSurfacePresentScalingCapabilitiesEXT = 1000274001,
	eSurfacePresentModeCompatibilityEXT = 1000274002,
	ePhysicalDeviceSwapchainMaintenance1FeaturesEXT = 1000275000,
	eSwapchainPresentFenceInfoEXT = 1000275001,
	eSwapchainPresentModesCreateInfoEXT = 1000275002,
	eSwapchainPresentModeInfoEXT = 1000275003,
	eSwapchainPresentScalingCreateInfoEXT = 1000275004,
	eReleaseSwapchainImagesInfoEXT = 1000275005,
	ePhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT = 1000276000,
	ePhysicalDeviceDeviceGeneratedCommandsPropertiesNV = 1000277000,
	eGraphicsShaderGroupCreateInfoNV = 1000277001,
	eGraphicsPipelineShaderGroupsCreateInfoNV = 1000277002,
	eIndirectCommandsLayoutTokenNV = 1000277003,
	eIndirectCommandsLayoutCreateInfoNV = 1000277004,
	eGeneratedCommandsInfoNV = 1000277005,
	eGeneratedCommandsMemoryRequirementsInfoNV = 1000277006,
	ePhysicalDeviceDeviceGeneratedCommandsFeaturesNV = 1000277007,
	ePhysicalDeviceInheritedViewportScissorFeaturesNV = 1000278000,
	eCommandBufferInheritanceViewportScissorInfoNV = 1000278001,
	ePhysicalDeviceShaderIntegerDotProductFeaturesKHR = 1000280000,
	ePhysicalDeviceShaderIntegerDotProductPropertiesKHR = 1000280001,
	ePhysicalDeviceTexelBufferAlignmentFeaturesEXT = 1000281000,
	ePhysicalDeviceTexelBufferAlignmentPropertiesEXT = 1000281001,
	eCommandBufferInheritanceRenderPassTransformInfoQCOM = 1000282000,
	eRenderPassTransformBeginInfoQCOM = 1000282001,
	ePhysicalDeviceDepthBiasControlFeaturesEXT = 1000283000,
	eDepthBiasInfoEXT = 1000283001,
	eDepthBiasRepresentationInfoEXT = 1000283002,
	ePhysicalDeviceDeviceMemoryReportFeaturesEXT = 1000284000,
	eDeviceDeviceMemoryReportCreateInfoEXT = 1000284001,
	eDeviceMemoryReportCallbackDataEXT = 1000284002,
	ePhysicalDeviceRobustness2FeaturesEXT = 1000286000,
	ePhysicalDeviceRobustness2PropertiesEXT = 1000286001,
	eSamplerCustomBorderColorCreateInfoEXT = 1000287000,
	ePhysicalDeviceCustomBorderColorPropertiesEXT = 1000287001,
	ePhysicalDeviceCustomBorderColorFeaturesEXT = 1000287002,
	ePipelineLibraryCreateInfoKHR = 1000290000,
	ePhysicalDevicePresentBarrierFeaturesNV = 1000292000,
	eSurfaceCapabilitiesPresentBarrierNV = 1000292001,
	eSwapchainPresentBarrierCreateInfoNV = 1000292002,
	ePresentIdKHR = 1000294000,
	ePhysicalDevicePresentIdFeaturesKHR = 1000294001,
	ePhysicalDevicePrivateDataFeaturesEXT = 1000295000,
	eDevicePrivateDataCreateInfoEXT = 1000295001,
	ePrivateDataSlotCreateInfoEXT = 1000295002,
	ePhysicalDevicePipelineCreationCacheControlFeaturesEXT = 1000297000,
	eVideoEncodeInfoKHR = 1000299000,
	eVideoEncodeRateControlInfoKHR = 1000299001,
	eVideoEncodeRateControlLayerInfoKHR = 1000299002,
	eVideoEncodeCapabilitiesKHR = 1000299003,
	eVideoEncodeUsageInfoKHR = 1000299004,
	eQueryPoolVideoEncodeFeedbackCreateInfoKHR = 1000299005,
	ePhysicalDeviceVideoEncodeQualityLevelInfoKHR = 1000299006,
	eVideoEncodeQualityLevelPropertiesKHR = 1000299007,
	eVideoEncodeQualityLevelInfoKHR = 1000299008,
	eVideoEncodeSessionParametersGetInfoKHR = 1000299009,
	eVideoEncodeSessionParametersFeedbackInfoKHR = 1000299010,
	ePhysicalDeviceDiagnosticsConfigFeaturesNV = 1000300000,
	eDeviceDiagnosticsConfigCreateInfoNV = 1000300001,
	eCudaModuleCreateInfoNV = 1000307000,
	eCudaFunctionCreateInfoNV = 1000307001,
	eCudaLaunchInfoNV = 1000307002,
	ePhysicalDeviceCudaKernelLaunchFeaturesNV = 1000307003,
	ePhysicalDeviceCudaKernelLaunchPropertiesNV = 1000307004,
	eQueryLowLatencySupportNV = 1000310000,
	eMemoryBarrieR2KHR = 1000314000,
	eBufferMemoryBarrieR2KHR = 1000314001,
	eImageMemoryBarrieR2KHR = 1000314002,
	eDependencyInfoKHR = 1000314003,
	eSubmitInfo2KHR = 1000314004,
	eSemaphoreSubmitInfoKHR = 1000314005,
	eCommandBufferSubmitInfoKHR = 1000314006,
	ePhysicalDeviceSynchronization2FeaturesKHR = 1000314007,
	ePhysicalDeviceDescriptorBufferPropertiesEXT = 1000316000,
	ePhysicalDeviceDescriptorBufferDensityMapPropertiesEXT = 1000316001,
	ePhysicalDeviceDescriptorBufferFeaturesEXT = 1000316002,
	eDescriptorAddressInfoEXT = 1000316003,
	eDescriptorGetInfoEXT = 1000316004,
	eBufferCaptureDescriptorDataInfoEXT = 1000316005,
	eImageCaptureDescriptorDataInfoEXT = 1000316006,
	eImageViewCaptureDescriptorDataInfoEXT = 1000316007,
	eSamplerCaptureDescriptorDataInfoEXT = 1000316008,
	eOpaqueCaptureDescriptorDataCreateInfoEXT = 1000316010,
	eDescriptorBufferBindingInfoEXT = 1000316011,
	eDescriptorBufferBindingPushDescriptorBufferHandleEXT = 1000316012,
	eAccelerationStructureCaptureDescriptorDataInfoEXT = 1000316009,
	ePhysicalDeviceGraphicsPipelineLibraryFeaturesEXT = 1000320000,
	ePhysicalDeviceGraphicsPipelineLibraryPropertiesEXT = 1000320001,
	eGraphicsPipelineLibraryCreateInfoEXT = 1000320002,
	ePhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD = 1000321000,
	ePhysicalDeviceFragmentShaderBarycentricFeaturesKHR = 1000203000,
	ePhysicalDeviceFragmentShaderBarycentricPropertiesKHR = 1000322000,
	ePhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR = 1000323000,
	ePhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR = 1000325000,
	ePhysicalDeviceFragmentShadingRateEnumsPropertiesNV = 1000326000,
	ePhysicalDeviceFragmentShadingRateEnumsFeaturesNV = 1000326001,
	ePipelineFragmentShadingRateEnumStateCreateInfoNV = 1000326002,
	eAccelerationStructureGeometryMotionTrianglesDataNV = 1000327000,
	ePhysicalDeviceRayTracingMotionBlurFeaturesNV = 1000327001,
	eAccelerationStructureMotionInfoNV = 1000327002,
	ePhysicalDeviceMeshShaderFeaturesEXT = 1000328000,
	ePhysicalDeviceMeshShaderPropertiesEXT = 1000328001,
	ePhysicalDeviceYcbcR2Plane444FormatsFeaturesEXT = 1000330000,
	ePhysicalDeviceFragmentDensityMap2FeaturesEXT = 1000332000,
	ePhysicalDeviceFragmentDensityMap2PropertiesEXT = 1000332001,
	eCopyCommandTransformInfoQCOM = 1000333000,
	ePhysicalDeviceImageRobustnessFeaturesEXT = 1000335000,
	ePhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR = 1000336000,
	eCopyBufferInfo2KHR = 1000337000,
	eCopyImageInfo2KHR = 1000337001,
	eCopyBufferToImageInfo2KHR = 1000337002,
	eCopyImageToBufferInfo2KHR = 1000337003,
	eBlitImageInfo2KHR = 1000337004,
	eResolveImageInfo2KHR = 1000337005,
	eBufferCopy2KHR = 1000337006,
	eImageCopy2KHR = 1000337007,
	eImageBlit2KHR = 1000337008,
	eBufferImageCopy2KHR = 1000337009,
	eImageResolve2KHR = 1000337010,
	ePhysicalDeviceImageCompressionControlFeaturesEXT = 1000338000,
	eImageCompressionControlEXT = 1000338001,
	eSubresourceLayout2EXT = 1000338002,
	eImageSubresource2EXT = 1000338003,
	eImageCompressionPropertiesEXT = 1000338004,
	ePhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT = 1000339000,
	ePhysicalDevice4444FormatsFeaturesEXT = 1000340000,
	ePhysicalDeviceFaultFeaturesEXT = 1000341000,
	eDeviceFaultCountsEXT = 1000341001,
	eDeviceFaultInfoEXT = 1000341002,
	ePhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM = 1000342000,
	ePhysicalDeviceRgbA10X6FormatsFeaturesEXT = 1000344000,
	ePhysicalDeviceMutableDescriptorTypeFeaturesVALVE = 1000351000,
	eMutableDescriptorTypeCreateInfoVALVE = 1000351002,
	ePhysicalDeviceVertexInputDynamicStateFeaturesEXT = 1000352000,
	eVertexInputBindingDescription2EXT = 1000352001,
	eVertexInputAttributeDescription2EXT = 1000352002,
	ePhysicalDeviceDrmPropertiesEXT = 1000353000,
	ePhysicalDeviceAddressBindingReportFeaturesEXT = 1000354000,
	eDeviceAddressBindingCallbackDataEXT = 1000354001,
	ePhysicalDeviceDepthClipControlFeaturesEXT = 1000355000,
	ePipelineViewportDepthClipControlCreateInfoEXT = 1000355001,
	ePhysicalDevicePrimitiveTopologyListRestartFeaturesEXT = 1000356000,
	eFormatProperties3KHR = 1000360000,
	ePhysicalDevicePresentModeFifoLatestReadyFeaturesEXT = 1000361000,
	eSubpassShadingPipelineCreateInfoHUAWEI = 1000369000,
	ePhysicalDeviceSubpassShadingFeaturesHUAWEI = 1000369001,
	ePhysicalDeviceSubpassShadingPropertiesHUAWEI = 1000369002,
	ePhysicalDeviceInvocationMaskFeaturesHUAWEI = 1000370000,
	eMemoryGetRemoteAddressInfoNV = 1000371000,
	ePhysicalDeviceExternalMemoryRdmaFeaturesNV = 1000371001,
	ePipelinePropertiesIdentifierEXT = 1000372000,
	ePhysicalDevicePipelinePropertiesFeaturesEXT = 1000372001,
	ePipelineInfoEXT = 1000269001,
	ePhysicalDeviceFrameBoundaryFeaturesEXT = 1000375000,
	eFrameBoundaryEXT = 1000375001,
	ePhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT = 1000376000,
	eSubpassResolvePerformanceQueryEXT = 1000376001,
	eMultisampledRenderToSingleSampledInfoEXT = 1000376002,
	ePhysicalDeviceExtendedDynamicState2FeaturesEXT = 1000377000,
	ePhysicalDeviceColorWriteEnableFeaturesEXT = 1000381000,
	ePipelineColorWriteCreateInfoEXT = 1000381001,
	ePhysicalDevicePrimitivesGeneratedQueryFeaturesEXT = 1000382000,
	ePhysicalDeviceRayTracingMaintenance1FeaturesKHR = 1000386000,
	ePhysicalDeviceGlobalPriorityQueryFeaturesEXT = 1000388000,
	eQueueFamilyGlobalPriorityPropertiesEXT = 1000388001,
	ePhysicalDeviceImageViewMinLodFeaturesEXT = 1000391000,
	eImageViewMinLodCreateInfoEXT = 1000391001,
	ePhysicalDeviceMultiDrawFeaturesEXT = 1000392000,
	ePhysicalDeviceMultiDrawPropertiesEXT = 1000392001,
	ePhysicalDeviceImage2DViewOf3DFeaturesEXT = 1000393000,
	ePhysicalDeviceShaderTileImageFeaturesEXT = 1000395000,
	ePhysicalDeviceShaderTileImagePropertiesEXT = 1000395001,
	eMicromapBuildInfoEXT = 1000396000,
	eMicromapVersionInfoEXT = 1000396001,
	eCopyMicromapInfoEXT = 1000396002,
	eCopyMicromapToMemoryInfoEXT = 1000396003,
	eCopyMemoryToMicromapInfoEXT = 1000396004,
	ePhysicalDeviceOpacityMicromapFeaturesEXT = 1000396005,
	ePhysicalDeviceOpacityMicromapPropertiesEXT = 1000396006,
	eMicromapCreateInfoEXT = 1000396007,
	eMicromapBuildSizesInfoEXT = 1000396008,
	eAccelerationStructureTrianglesOpacityMicromapEXT = 1000396009,
	ePhysicalDeviceClusterCullingShaderFeaturesHUAWEI = 1000404000,
	ePhysicalDeviceClusterCullingShaderPropertiesHUAWEI = 1000404001,
	ePhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI = 1000404002,
	ePhysicalDeviceBorderColorSwizzleFeaturesEXT = 1000411000,
	eSamplerBorderColorComponentMappingCreateInfoEXT = 1000411001,
	ePhysicalDevicePageableDeviceLocalMemoryFeaturesEXT = 1000412000,
	ePhysicalDeviceMaintenance4FeaturesKHR = 1000413000,
	ePhysicalDeviceMaintenance4PropertiesKHR = 1000413001,
	eDeviceBufferMemoryRequirementsKHR = 1000413002,
	eDeviceImageMemoryRequirementsKHR = 1000413003,
	ePhysicalDeviceShaderCorePropertiesARM = 1000415000,
	ePhysicalDeviceShaderSubgroupRotateFeaturesKHR = 1000416000,
	eDeviceQueueShaderCoreControlCreateInfoARM = 1000417000,
	ePhysicalDeviceSchedulingControlsFeaturesARM = 1000417001,
	ePhysicalDeviceSchedulingControlsPropertiesARM = 1000417002,
	ePhysicalDeviceImageSlicedViewOf3DFeaturesEXT = 1000418000,
	eImageViewSlicedCreateInfoEXT = 1000418001,
	ePhysicalDeviceDescriptorSetHostMappingFeaturesVALVE = 1000420000,
	eDescriptorSetBindingReferenceVALVE = 1000420001,
	eDescriptorSetLayoutHostMappingInfoVALVE = 1000420002,
	ePhysicalDeviceDepthClampZeroOneFeaturesEXT = 1000421000,
	ePhysicalDeviceNonSeamlessCubeMapFeaturesEXT = 1000422000,
	ePhysicalDeviceRenderPassStripedFeaturesARM = 1000424000,
	ePhysicalDeviceRenderPassStripedPropertiesARM = 1000424001,
	eRenderPassStripeBeginInfoARM = 1000424002,
	eRenderPassStripeInfoARM = 1000424003,
	eRenderPassStripeSubmitInfoARM = 1000424004,
	ePhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM = 1000425000,
	ePhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM = 1000425001,
	eSubpassFragmentDensityMapOffsetEndInfoQCOM = 1000425002,
	ePhysicalDeviceCopyMemoryIndirectFeaturesNV = 1000426000,
	ePhysicalDeviceCopyMemoryIndirectPropertiesNV = 1000426001,
	ePhysicalDeviceMemoryDecompressionFeaturesNV = 1000427000,
	ePhysicalDeviceMemoryDecompressionPropertiesNV = 1000427001,
	ePhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV = 1000428000,
	eComputePipelineIndirectBufferInfoNV = 1000428001,
	ePipelineIndirectDeviceAddressInfoNV = 1000428002,
	ePhysicalDeviceLinearColorAttachmentFeaturesNV = 1000430000,
	ePhysicalDeviceShaderMaximalReconvergenceFeaturesKHR = 1000434000,
	ePhysicalDeviceImageCompressionControlSwapchainFeaturesEXT = 1000437000,
	ePhysicalDeviceImageProcessingFeaturesQCOM = 1000440000,
	ePhysicalDeviceImageProcessingPropertiesQCOM = 1000440001,
	eImageViewSampleWeightCreateInfoQCOM = 1000440002,
	ePhysicalDeviceNestedCommandBufferFeaturesEXT = 1000451000,
	ePhysicalDeviceNestedCommandBufferPropertiesEXT = 1000451001,
	eExternalMemoryAcquireUnmodifiedEXT = 1000453000,
	ePhysicalDeviceExtendedDynamicState3FeaturesEXT = 1000455000,
	ePhysicalDeviceExtendedDynamicState3PropertiesEXT = 1000455001,
	ePhysicalDeviceSubpassMergeFeedbackFeaturesEXT = 1000458000,
	eRenderPassCreationControlEXT = 1000458001,
	eRenderPassCreationFeedbackCreateInfoEXT = 1000458002,
	eRenderPassSubpassFeedbackCreateInfoEXT = 1000458003,
	eDirectDriverLoadingInfoLUNARG = 1000459000,
	eDirectDriverLoadingListLUNARG = 1000459001,
	ePhysicalDeviceShaderModuleIdentifierFeaturesEXT = 1000462000,
	ePhysicalDeviceShaderModuleIdentifierPropertiesEXT = 1000462001,
	ePipelineShaderStageModuleIdentifierCreateInfoEXT = 1000462002,
	eShaderModuleIdentifierEXT = 1000462003,
	ePhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT = 1000342000,
	ePhysicalDeviceOpticalFlowFeaturesNV = 1000464000,
	ePhysicalDeviceOpticalFlowPropertiesNV = 1000464001,
	eOpticalFlowImageFormatInfoNV = 1000464002,
	eOpticalFlowImageFormatPropertiesNV = 1000464003,
	eOpticalFlowSessionCreateInfoNV = 1000464004,
	eOpticalFlowExecuteInfoNV = 1000464005,
	eOpticalFlowSessionCreatePrivateDataInfoNV = 1000464010,
	ePhysicalDeviceLegacyDitheringFeaturesEXT = 1000465000,
	ePhysicalDevicePipelineProtectedAccessFeaturesEXT = 1000466000,
	ePhysicalDeviceMaintenance5FeaturesKHR = 1000470000,
	ePhysicalDeviceMaintenance5PropertiesKHR = 1000470001,
	eRenderingAreaInfoKHR = 1000470003,
	eDeviceImageSubresourceInfoKHR = 1000470004,
	eSubresourceLayout2KHR = 1000338002,
	eImageSubresource2KHR = 1000338003,
	ePipelineCreateFlags2CreateInfoKHR = 1000470005,
	eBufferUsageFlags2CreateInfoKHR = 1000470006,
	ePhysicalDeviceAntiLagFeaturesAMD = 1000476000,
	eAntiLagDataAMD = 1000476001,
	eAntiLagPresentationInfoAMD = 1000476002,
	ePhysicalDeviceRayTracingPositionFetchFeaturesKHR = 1000481000,
	ePhysicalDeviceShaderObjectFeaturesEXT = 1000482000,
	ePhysicalDeviceShaderObjectPropertiesEXT = 1000482001,
	eShaderCreateInfoEXT = 1000482002,
	eShaderRequiredSubgroupSizeCreateInfoEXT = 1000225001,
	ePhysicalDevicePipelineBinaryFeaturesKHR = 1000483000,
	ePipelineBinaryCreateInfoKHR = 1000483001,
	ePipelineBinaryInfoKHR = 1000483002,
	ePipelineBinaryKeyKHR = 1000483003,
	ePhysicalDevicePipelineBinaryPropertiesKHR = 1000483004,
	eReleaseCapturedPipelineDataInfoKHR = 1000483005,
	ePipelineBinaryDataInfoKHR = 1000483006,
	ePipelineCreateInfoKHR = 1000483007,
	eDevicePipelineBinaryInternalCacheControlKHR = 1000483008,
	ePipelineBinaryHandlesInfoKHR = 1000483009,
	ePhysicalDeviceTilePropertiesFeaturesQCOM = 1000484000,
	eTilePropertiesQCOM = 1000484001,
	ePhysicalDeviceAmigoProfilingFeaturesSEC = 1000485000,
	eAmigoProfilingSubmitInfoSEC = 1000485001,
	ePhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM = 1000488000,
	ePhysicalDeviceRayTracingInvocationReorderFeaturesNV = 1000490000,
	ePhysicalDeviceRayTracingInvocationReorderPropertiesNV = 1000490001,
	ePhysicalDeviceExtendedSparseAddressSpaceFeaturesNV = 1000492000,
	ePhysicalDeviceExtendedSparseAddressSpacePropertiesNV = 1000492001,
	ePhysicalDeviceMutableDescriptorTypeFeaturesEXT = 1000351000,
	eMutableDescriptorTypeCreateInfoEXT = 1000351002,
	ePhysicalDeviceLegacyVertexAttributesFeaturesEXT = 1000495000,
	ePhysicalDeviceLegacyVertexAttributesPropertiesEXT = 1000495001,
	eLayerSettingsCreateInfoEXT = 1000496000,
	ePhysicalDeviceShaderCoreBuiltinsFeaturesARM = 1000497000,
	ePhysicalDeviceShaderCoreBuiltinsPropertiesARM = 1000497001,
	ePhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT = 1000498000,
	ePhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT = 1000499000,
	eLatencySleepModeInfoNV = 1000505000,
	eLatencySleepInfoNV = 1000505001,
	eSetLatencyMarkerInfoNV = 1000505002,
	eGetLatencyMarkerInfoNV = 1000505003,
	eLatencyTimingsFrameReportNV = 1000505004,
	eLatencySubmissionPresentIdNV = 1000505005,
	eOutOfBandQueueTypeInfoNV = 1000505006,
	eSwapchainLatencyCreateInfoNV = 1000505007,
	eLatencySurfaceCapabilitiesNV = 1000505008,
	ePhysicalDeviceCooperativeMatrixFeaturesKHR = 1000506000,
	eCooperativeMatrixPropertiesKHR = 1000506001,
	ePhysicalDeviceCooperativeMatrixPropertiesKHR = 1000506002,
	ePhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM = 1000510000,
	eMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM = 1000510001,
	ePhysicalDeviceComputeShaderDerivativesFeaturesKHR = 1000201000,
	ePhysicalDeviceComputeShaderDerivativesPropertiesKHR = 1000511000,
	eVideoDecodeAv1CapabilitiesKHR = 1000512000,
	eVideoDecodeAv1PictureInfoKHR = 1000512001,
	eVideoDecodeAv1ProfileInfoKHR = 1000512003,
	eVideoDecodeAv1SessionParametersCreateInfoKHR = 1000512004,
	eVideoDecodeAv1DpbSlotInfoKHR = 1000512005,
	eVideoEncodeAv1CapabilitiesKHR = 1000513000,
	eVideoEncodeAv1SessionParametersCreateInfoKHR = 1000513001,
	eVideoEncodeAv1PictureInfoKHR = 1000513002,
	eVideoEncodeAv1DpbSlotInfoKHR = 1000513003,
	ePhysicalDeviceVideoEncodeAv1FeaturesKHR = 1000513004,
	eVideoEncodeAv1ProfileInfoKHR = 1000513005,
	eVideoEncodeAv1RateControlInfoKHR = 1000513006,
	eVideoEncodeAv1RateControlLayerInfoKHR = 1000513007,
	eVideoEncodeAv1QualityLevelPropertiesKHR = 1000513008,
	eVideoEncodeAv1SessionCreateInfoKHR = 1000513009,
	eVideoEncodeAv1GopRemainingFrameInfoKHR = 1000513010,
	ePhysicalDeviceVideoMaintenance1FeaturesKHR = 1000515000,
	eVideoInlineQueryInfoKHR = 1000515001,
	ePhysicalDevicePerStageDescriptorSetFeaturesNV = 1000516000,
	ePhysicalDeviceImageProcessinG2FeaturesQCOM = 1000518000,
	ePhysicalDeviceImageProcessinG2PropertiesQCOM = 1000518001,
	eSamplerBlockMatchWindowCreateInfoQCOM = 1000518002,
	eSamplerCubicWeightsCreateInfoQCOM = 1000519000,
	ePhysicalDeviceCubicWeightsFeaturesQCOM = 1000519001,
	eBlitImageCubicWeightsInfoQCOM = 1000519002,
	ePhysicalDeviceYcbcrDegammaFeaturesQCOM = 1000520000,
	eSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM = 1000520001,
	ePhysicalDeviceCubicClampFeaturesQCOM = 1000521000,
	ePhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT = 1000524000,
	ePhysicalDeviceVertexAttributeDivisorPropertiesKHR = 1000525000,
	ePipelineVertexInputDivisorStateCreateInfoKHR = 1000190001,
	ePhysicalDeviceVertexAttributeDivisorFeaturesKHR = 1000190002,
	ePhysicalDeviceShaderFloatControls2FeaturesKHR = 1000528000,
	ePhysicalDeviceLayeredDriverPropertiesMSFT = 1000530000,
	ePhysicalDeviceIndexTypeUint8FeaturesKHR = 1000265000,
	ePhysicalDeviceLineRasterizationFeaturesKHR = 1000259000,
	ePipelineRasterizationLineStateCreateInfoKHR = 1000259001,
	ePhysicalDeviceLineRasterizationPropertiesKHR = 1000259002,
	eCalibratedTimestampInfoKHR = 1000184000,
	ePhysicalDeviceShaderExpectAssumeFeaturesKHR = 1000544000,
	ePhysicalDeviceMaintenance6FeaturesKHR = 1000545000,
	ePhysicalDeviceMaintenance6PropertiesKHR = 1000545001,
	eBindMemoryStatusKHR = 1000545002,
	eBindDescriptorSetsInfoKHR = 1000545003,
	ePushConstantsInfoKHR = 1000545004,
	ePushDescriptorSetInfoKHR = 1000545005,
	ePushDescriptorSetWithTemplateInfoKHR = 1000545006,
	eSetDescriptorBufferOffsetsInfoEXT = 1000545007,
	eBindDescriptorBufferEmbeddedSamplersInfoEXT = 1000545008,
	ePhysicalDeviceDescriptorPoolOverallocationFeaturesNV = 1000546000,
	eDisplaySurfaceStereoCreateInfoNV = 1000551000,
	eDisplayModeStereoPropertiesNV = 1000551001,
	eVideoEncodeQuantizationMapCapabilitiesKHR = 1000553000,
	eVideoFormatQuantizationMapPropertiesKHR = 1000553001,
	eVideoEncodeQuantizationMapInfoKHR = 1000553002,
	eVideoEncodeQuantizationMapSessionParametersCreateInfoKHR = 1000553005,
	ePhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR = 1000553009,
	eVideoEncodeH264QuantizationMapCapabilitiesKHR = 1000553003,
	eVideoEncodeH265QuantizationMapCapabilitiesKHR = 1000553004,
	eVideoFormatH265QuantizationMapPropertiesKHR = 1000553006,
	eVideoEncodeAv1QuantizationMapCapabilitiesKHR = 1000553007,
	eVideoFormatAv1QuantizationMapPropertiesKHR = 1000553008,
	ePhysicalDeviceRawAccessChainsFeaturesNV = 1000555000,
	ePhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR = 1000558000,
	ePhysicalDeviceCommandBufferInheritanceFeaturesNV = 1000559000,
	ePhysicalDeviceMaintenance7FeaturesKHR = 1000562000,
	ePhysicalDeviceMaintenance7PropertiesKHR = 1000562001,
	ePhysicalDeviceLayeredApiPropertiesListKHR = 1000562002,
	ePhysicalDeviceLayeredApiPropertiesKHR = 1000562003,
	ePhysicalDeviceLayeredApiVulkanPropertiesKHR = 1000562004,
	ePhysicalDeviceShaderAtomicFloat16VectorFeaturesNV = 1000563000,
	ePhysicalDeviceShaderReplicatedCompositesFeaturesEXT = 1000564000,
	ePhysicalDeviceRayTracingValidationFeaturesNV = 1000568000,
	ePhysicalDeviceDeviceGeneratedCommandsFeaturesEXT = 1000572000,
	ePhysicalDeviceDeviceGeneratedCommandsPropertiesEXT = 1000572001,
	eGeneratedCommandsMemoryRequirementsInfoEXT = 1000572002,
	eIndirectExecutionSetCreateInfoEXT = 1000572003,
	eGeneratedCommandsInfoEXT = 1000572004,
	eIndirectCommandsLayoutCreateInfoEXT = 1000572006,
	eIndirectCommandsLayoutTokenEXT = 1000572007,
	eWriteIndirectExecutionSetPipelineEXT = 1000572008,
	eWriteIndirectExecutionSetShaderEXT = 1000572009,
	eIndirectExecutionSetPipelineInfoEXT = 1000572010,
	eIndirectExecutionSetShaderInfoEXT = 1000572011,
	eIndirectExecutionSetShaderLayoutInfoEXT = 1000572012,
	eGeneratedCommandsPipelineInfoEXT = 1000572013,
	eGeneratedCommandsShaderInfoEXT = 1000572014,
	ePhysicalDeviceImageAlignmentControlFeaturesMESA = 1000575000,
	ePhysicalDeviceImageAlignmentControlPropertiesMESA = 1000575001,
	eImageAlignmentControlCreateInfoMESA = 1000575002,
	ePhysicalDeviceDepthClampControlFeaturesEXT = 1000582000,
	ePipelineViewportDepthClampControlCreateInfoEXT = 1000582001,
	ePhysicalDeviceHdrVividFeaturesHUAWEI = 1000590000,
	eHdrVividDynamicMetadataHUAWEI = 1000590001,
	ePhysicalDeviceCooperativeMatrix2FeaturesNV = 1000593000,
	eCooperativeMatrixFlexibleDimensionsPropertiesNV = 1000593001,
	ePhysicalDeviceCooperativeMatrix2PropertiesNV = 1000593002,
	ePhysicalDeviceVertexAttributeRobustnessFeaturesEXT = 1000608000,
#ifdef VK_ENABLE_BETA_EXTENSIONS
	ePhysicalDeviceShaderEnqueueFeaturesAMDX = 1000134000,
	ePhysicalDeviceShaderEnqueuePropertiesAMDX = 1000134001,
	eExecutionGraphPipelineScratchSizeAMDX = 1000134002,
	eExecutionGraphPipelineCreateInfoAMDX = 1000134003,
	ePipelineShaderStageNodeCreateInfoAMDX = 1000134004,
	ePhysicalDevicePortabilitySubsetFeaturesKHR = 1000163000,
	ePhysicalDevicePortabilitySubsetPropertiesKHR = 1000163001,
	ePhysicalDeviceDisplacementMicromapFeaturesNV = 1000397000,
	ePhysicalDeviceDisplacementMicromapPropertiesNV = 1000397001,
	eAccelerationStructureTrianglesDisplacementMicromapNV = 1000397002,
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	eAndroidSurfaceCreateInfoKHR = 1000008000,
	eAndroidHardwareBufferUsageANDROID = 1000129000,
	eAndroidHardwareBufferPropertiesANDROID = 1000129001,
	eAndroidHardwareBufferFormatPropertiesANDROID = 1000129002,
	eImportAndroidHardwareBufferInfoANDROID = 1000129003,
	eMemoryGetAndroidHardwareBufferInfoANDROID = 1000129004,
	eExternalFormatANDROID = 1000129005,
	eAndroidHardwareBufferFormatProperties2ANDROID = 1000129006,
	ePhysicalDeviceExternalFormatResolveFeaturesANDROID = 1000468000,
	ePhysicalDeviceExternalFormatResolvePropertiesANDROID = 1000468001,
	eAndroidHardwareBufferFormatResolvePropertiesANDROID = 1000468002,
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
	eDirectfbSurfaceCreateInfoEXT = 1000346000,
#endif // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_FUCHSIA
	eImagepipeSurfaceCreateInfoFUCHSIA = 1000214000,
	eImportMemoryZirconHandleInfoFUCHSIA = 1000364000,
	eMemoryZirconHandlePropertiesFUCHSIA = 1000364001,
	eMemoryGetZirconHandleInfoFUCHSIA = 1000364002,
	eImportSemaphoreZirconHandleInfoFUCHSIA = 1000365000,
	eSemaphoreGetZirconHandleInfoFUCHSIA = 1000365001,
	eBufferCollectionCreateInfoFUCHSIA = 1000366000,
	eImportMemoryBufferCollectionFUCHSIA = 1000366001,
	eBufferCollectionImageCreateInfoFUCHSIA = 1000366002,
	eBufferCollectionPropertiesFUCHSIA = 1000366003,
	eBufferConstraintsInfoFUCHSIA = 1000366004,
	eBufferCollectionBufferCreateInfoFUCHSIA = 1000366005,
	eImageConstraintsInfoFUCHSIA = 1000366006,
	eImageFormatConstraintsInfoFUCHSIA = 1000366007,
	eSysmemColorSpaceFUCHSIA = 1000366008,
	eBufferCollectionConstraintsInfoFUCHSIA = 1000366009,
#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_GGP
	eStreamDescriptorSurfaceCreateInfoGGP = 1000049000,
	ePresentFrameTokenGGP = 1000191000,
#endif // VK_USE_PLATFORM_GGP
#ifdef VK_USE_PLATFORM_IOS_MVK
	eIosSurfaceCreateInfoMVK = 1000122000,
#endif // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
	eMacosSurfaceCreateInfoMVK = 1000123000,
#endif // VK_USE_PLATFORM_MACOS_MVK
#ifdef VK_USE_PLATFORM_METAL_EXT
	eMetalSurfaceCreateInfoEXT = 1000217000,
	eExportMetalObjectCreateInfoEXT = 1000311000,
	eExportMetalObjectsInfoEXT = 1000311001,
	eExportMetalDeviceInfoEXT = 1000311002,
	eExportMetalCommandQueueInfoEXT = 1000311003,
	eExportMetalBufferInfoEXT = 1000311004,
	eImportMetalBufferInfoEXT = 1000311005,
	eExportMetalTextureInfoEXT = 1000311006,
	eImportMetalTextureInfoEXT = 1000311007,
	eExportMetalIoSurfaceInfoEXT = 1000311008,
	eImportMetalIoSurfaceInfoEXT = 1000311009,
	eExportMetalSharedEventInfoEXT = 1000311010,
	eImportMetalSharedEventInfoEXT = 1000311011,
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_SCREEN_QNX
	eScreenSurfaceCreateInfoQNX = 1000378000,
	eScreenBufferPropertiesQNX = 1000529000,
	eScreenBufferFormatPropertiesQNX = 1000529001,
	eImportScreenBufferInfoQNX = 1000529002,
	eExternalFormatQNX = 1000529003,
	ePhysicalDeviceExternalMemoryScreenBufferFeaturesQNX = 1000529004,
#endif // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_VI_NN
	eViSurfaceCreateInfoNN = 1000062000,
#endif // VK_USE_PLATFORM_VI_NN
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	eWaylandSurfaceCreateInfoKHR = 1000006000,
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
	eWin32SurfaceCreateInfoKHR = 1000009000,
	eImportMemoryWin32HandleInfoNV = 1000057000,
	eExportMemoryWin32HandleInfoNV = 1000057001,
	eWin32KeyedMutexAcquireReleaseInfoNV = 1000058000,
	eImportMemoryWin32HandleInfoKHR = 1000073000,
	eExportMemoryWin32HandleInfoKHR = 1000073001,
	eMemoryWin32HandlePropertiesKHR = 1000073002,
	eMemoryGetWin32HandleInfoKHR = 1000073003,
	eWin32KeyedMutexAcquireReleaseInfoKHR = 1000075000,
	eImportSemaphoreWin32HandleInfoKHR = 1000078000,
	eExportSemaphoreWin32HandleInfoKHR = 1000078001,
	eD3D12FenceSubmitInfoKHR = 1000078002,
	eSemaphoreGetWin32HandleInfoKHR = 1000078003,
	eImportFenceWin32HandleInfoKHR = 1000114000,
	eExportFenceWin32HandleInfoKHR = 1000114001,
	eFenceGetWin32HandleInfoKHR = 1000114002,
	eSurfaceFullScreenExclusiveInfoEXT = 1000255000,
	eSurfaceCapabilitiesFullScreenExclusiveEXT = 1000255002,
	eSurfaceFullScreenExclusiveWin32InfoEXT = 1000255001,
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
	eXcbSurfaceCreateInfoKHR = 1000005000,
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
	eXlibSurfaceCreateInfoKHR = 1000004000,
#endif // VK_USE_PLATFORM_XLIB_KHR
};

enum class AttachmentLoadOp {
	eLoad = 0,
	eClear = 1,
	eDontCare = 2,
	eNone = 1000400000,
	eNoneEXT = 1000400000,
	eNoneKHR = 1000400000,
};

enum class AttachmentStoreOp {
	eStore = 0,
	eDontCare = 1,
	eNone = 1000301000,
	eNoneKHR = 1000301000,
	eNoneQCOM = 1000301000,
	eNoneEXT = 1000301000,
};

enum class BlendFactor {
	eZero = 0,
	eOne = 1,
	eSrcColor = 2,
	eOneMinusSrcColor = 3,
	eDstColor = 4,
	eOneMinusDstColor = 5,
	eSrcAlpha = 6,
	eOneMinusSrcAlpha = 7,
	eDstAlpha = 8,
	eOneMinusDstAlpha = 9,
	eConstantColor = 10,
	eOneMinusConstantColor = 11,
	eConstantAlpha = 12,
	eOneMinusConstantAlpha = 13,
	eSrcAlphaSaturate = 14,
	eSrc1Color = 15,
	eOneMinusSrc1Color = 16,
	eSrc1Alpha = 17,
	eOneMinusSrc1Alpha = 18,
};

enum class BlendOp {
	eAdd = 0,
	eSubtract = 1,
	eReverseSubtract = 2,
	eMin = 3,
	eMax = 4,
	eZeroEXT = 1000148000,
	eSrcEXT = 1000148001,
	eDstEXT = 1000148002,
	eSrcOverEXT = 1000148003,
	eDstOverEXT = 1000148004,
	eSrcInEXT = 1000148005,
	eDstInEXT = 1000148006,
	eSrcOutEXT = 1000148007,
	eDstOutEXT = 1000148008,
	eSrcAtopEXT = 1000148009,
	eDstAtopEXT = 1000148010,
	eXorEXT = 1000148011,
	eMultiplyEXT = 1000148012,
	eScreenEXT = 1000148013,
	eOverlayEXT = 1000148014,
	eDarkenEXT = 1000148015,
	eLightenEXT = 1000148016,
	eColordodgeEXT = 1000148017,
	eColorburnEXT = 1000148018,
	eHardlightEXT = 1000148019,
	eSoftlightEXT = 1000148020,
	eDifferenceEXT = 1000148021,
	eExclusionEXT = 1000148022,
	eInvertEXT = 1000148023,
	eInvertRgbEXT = 1000148024,
	eLineardodgeEXT = 1000148025,
	eLinearburnEXT = 1000148026,
	eVividlightEXT = 1000148027,
	eLinearlightEXT = 1000148028,
	ePinlightEXT = 1000148029,
	eHardmixEXT = 1000148030,
	eHslHueEXT = 1000148031,
	eHslSaturationEXT = 1000148032,
	eHslColorEXT = 1000148033,
	eHslLuminosityEXT = 1000148034,
	ePlusEXT = 1000148035,
	ePlusClampedEXT = 1000148036,
	ePlusClampedAlphaEXT = 1000148037,
	ePlusDarkerEXT = 1000148038,
	eMinusEXT = 1000148039,
	eMinusClampedEXT = 1000148040,
	eContrastEXT = 1000148041,
	eInvertOvgEXT = 1000148042,
	eRedEXT = 1000148043,
	eGreenEXT = 1000148044,
	eBlueEXT = 1000148045,
};

enum class BorderColor {
	eFloatTransparentBlack = 0,
	eIntTransparentBlack = 1,
	eFloatOpaqueBlack = 2,
	eIntOpaqueBlack = 3,
	eFloatOpaqueWhite = 4,
	eIntOpaqueWhite = 5,
	eFloatCustomEXT = 1000287003,
	eIntCustomEXT = 1000287004,
};

enum class PipelineCacheHeaderVersion {
	eOne = 1,
};

enum class ComponentSwizzle {
	eIdentity = 0,
	eZero = 1,
	eOne = 2,
	eR = 3,
	eG = 4,
	eB = 5,
	eA = 6,
};

enum class CommandBufferLevel {
	ePrimary = 0,
	eSecondary = 1,
};

enum class CompareOp {
	eNever = 0,
	eLess = 1,
	eEqual = 2,
	eLessOrEqual = 3,
	eGreater = 4,
	eNotEqual = 5,
	eGreaterOrEqual = 6,
	eAlways = 7,
};

enum class DescriptorType {
	eSampler = 0,
	eCombinedImageSampler = 1,
	eSampledImage = 2,
	eStorageImage = 3,
	eUniformTexelBuffer = 4,
	eStorageTexelBuffer = 5,
	eUniformBuffer = 6,
	eStorageBuffer = 7,
	eUniformBufferDynamic = 8,
	eStorageBufferDynamic = 9,
	eInputAttachment = 10,
	eInlineUniformBlock = 1000138000,
	eInlineUniformBlockEXT = 1000138000,
	eAccelerationStructureKHR = 1000150000,
	eAccelerationStructureNV = 1000165000,
	eMutableVALVE = 1000351000,
	eSampleWeightImageQCOM = 1000440000,
	eBlockMatchImageQCOM = 1000440001,
	eMutableEXT = 1000351000,
};

enum class DynamicState {
	eViewport = 0,
	eScissor = 1,
	eLineWidth = 2,
	eDepthBias = 3,
	eBlendConstants = 4,
	eDepthBounds = 5,
	eStencilCompareMask = 6,
	eStencilWriteMask = 7,
	eStencilReference = 8,
	eCullMode = 1000267000,
	eFrontFace = 1000267001,
	ePrimitiveTopology = 1000267002,
	eViewportWithCount = 1000267003,
	eScissorWithCount = 1000267004,
	eVertexInputBindingStride = 1000267005,
	eDepthTestEnable = 1000267006,
	eDepthWriteEnable = 1000267007,
	eDepthCompareOp = 1000267008,
	eDepthBoundsTestEnable = 1000267009,
	eStencilTestEnable = 1000267010,
	eStencilOp = 1000267011,
	eRasterizerDiscardEnable = 1000377001,
	eDepthBiasEnable = 1000377002,
	ePrimitiveRestartEnable = 1000377004,
	eLineStipple = 1000259000,
	eViewportWScalingNV = 1000087000,
	eDiscardRectangleEXT = 1000099000,
	eDiscardRectangleEnableEXT = 1000099001,
	eDiscardRectangleModeEXT = 1000099002,
	eSampleLocationsEXT = 1000143000,
	eRayTracingPipelineStackSizeKHR = 1000347000,
	eViewportShadingRatePaletteNV = 1000164004,
	eViewportCoarseSampleOrderNV = 1000164006,
	eExclusiveScissorEnableNV = 1000205000,
	eExclusiveScissorNV = 1000205001,
	eFragmentShadingRateKHR = 1000226000,
	eLineStippleEXT = 1000259000,
	eCullModeEXT = 1000267000,
	eFrontFaceEXT = 1000267001,
	ePrimitiveTopologyEXT = 1000267002,
	eViewportWithCountEXT = 1000267003,
	eScissorWithCountEXT = 1000267004,
	eVertexInputBindingStrideEXT = 1000267005,
	eDepthTestEnableEXT = 1000267006,
	eDepthWriteEnableEXT = 1000267007,
	eDepthCompareOpEXT = 1000267008,
	eDepthBoundsTestEnableEXT = 1000267009,
	eStencilTestEnableEXT = 1000267010,
	eStencilOpEXT = 1000267011,
	eVertexInputEXT = 1000352000,
	ePatchControlPointsEXT = 1000377000,
	eRasterizerDiscardEnableEXT = 1000377001,
	eDepthBiasEnableEXT = 1000377002,
	eLogicOpEXT = 1000377003,
	ePrimitiveRestartEnableEXT = 1000377004,
	eColorWriteEnableEXT = 1000381000,
	eDepthClampEnableEXT = 1000455003,
	ePolygonModeEXT = 1000455004,
	eRasterizationSamplesEXT = 1000455005,
	eSampleMaskEXT = 1000455006,
	eAlphaToCoverageEnableEXT = 1000455007,
	eAlphaToOneEnableEXT = 1000455008,
	eLogicOpEnableEXT = 1000455009,
	eColorBlendEnableEXT = 1000455010,
	eColorBlendEquationEXT = 1000455011,
	eColorWriteMaskEXT = 1000455012,
	eTessellationDomainOriginEXT = 1000455002,
	eRasterizationStreamEXT = 1000455013,
	eConservativeRasterizationModeEXT = 1000455014,
	eExtraPrimitiveOverestimationSizeEXT = 1000455015,
	eDepthClipEnableEXT = 1000455016,
	eSampleLocationsEnableEXT = 1000455017,
	eColorBlendAdvancedEXT = 1000455018,
	eProvokingVertexModeEXT = 1000455019,
	eLineRasterizationModeEXT = 1000455020,
	eLineStippleEnableEXT = 1000455021,
	eDepthClipNegativeOneToOneEXT = 1000455022,
	eViewportWScalingEnableNV = 1000455023,
	eViewportSwizzleNV = 1000455024,
	eCoverageToColorEnableNV = 1000455025,
	eCoverageToColorLocationNV = 1000455026,
	eCoverageModulationModeNV = 1000455027,
	eCoverageModulationTableEnableNV = 1000455028,
	eCoverageModulationTableNV = 1000455029,
	eShadingRateImageEnableNV = 1000455030,
	eRepresentativeFragmentTestEnableNV = 1000455031,
	eCoverageReductionModeNV = 1000455032,
	eAttachmentFeedbackLoopEnableEXT = 1000524000,
	eLineStippleKHR = 1000259000,
	eDepthClampRangeEXT = 1000582000,
};

enum class PolygonMode {
	eFill = 0,
	eLine = 1,
	ePoint = 2,
	eFillRectangleNV = 1000153000,
};

enum class DriverId {
	eAmdProprietary = 1,
	eAmdOpenSource = 2,
	eMesaRadv = 3,
	eNvidiaProprietary = 4,
	eIntelProprietaryWindows = 5,
	eIntelOpenSourceMESA = 6,
	eImaginationProprietary = 7,
	eQualcommProprietary = 8,
	eArmProprietary = 9,
	eGoogleSwiftshader = 10,
	eGgpProprietary = 11,
	eBroadcomProprietary = 12,
	eMesaLlvmpipe = 13,
	eMoltenvk = 14,
	eCoreaviProprietary = 15,
	eJuiceProprietary = 16,
	eVerisiliconProprietary = 17,
	eMesaTurnip = 18,
	eMesaV3Dv = 19,
	eMesaPanvk = 20,
	eSamsungProprietary = 21,
	eMesaVenus = 22,
	eMesaDozen = 23,
	eMesaNvk = 24,
	eImaginationOpenSourceMESA = 25,
	eMesaHoneykrisp = 26,
	eVulkanScEmulationOnVulkan = 27,
};

enum class Format {
	eUndefined = 0,
	eR4G4UnormPack8 = 1,
	eR4G4B4A4UnormPack16 = 2,
	eB4G4R4A4UnormPack16 = 3,
	eR5G6B5UnormPack16 = 4,
	eB5G6R5UnormPack16 = 5,
	eR5G5B5A1UnormPack16 = 6,
	eB5G5R5A1UnormPack16 = 7,
	eA1R5G5B5UnormPack16 = 8,
	eR8Unorm = 9,
	eR8Snorm = 10,
	eR8Uscaled = 11,
	eR8Sscaled = 12,
	eR8Uint = 13,
	eR8Sint = 14,
	eR8Srgb = 15,
	eR8G8Unorm = 16,
	eR8G8Snorm = 17,
	eR8G8Uscaled = 18,
	eR8G8Sscaled = 19,
	eR8G8Uint = 20,
	eR8G8Sint = 21,
	eR8G8Srgb = 22,
	eR8G8B8Unorm = 23,
	eR8G8B8Snorm = 24,
	eR8G8B8Uscaled = 25,
	eR8G8B8Sscaled = 26,
	eR8G8B8Uint = 27,
	eR8G8B8Sint = 28,
	eR8G8B8Srgb = 29,
	eB8G8R8Unorm = 30,
	eB8G8R8Snorm = 31,
	eB8G8R8Uscaled = 32,
	eB8G8R8Sscaled = 33,
	eB8G8R8Uint = 34,
	eB8G8R8Sint = 35,
	eB8G8R8Srgb = 36,
	eR8G8B8A8Unorm = 37,
	eR8G8B8A8Snorm = 38,
	eR8G8B8A8Uscaled = 39,
	eR8G8B8A8Sscaled = 40,
	eR8G8B8A8Uint = 41,
	eR8G8B8A8Sint = 42,
	eR8G8B8A8Srgb = 43,
	eB8G8R8A8Unorm = 44,
	eB8G8R8A8Snorm = 45,
	eB8G8R8A8Uscaled = 46,
	eB8G8R8A8Sscaled = 47,
	eB8G8R8A8Uint = 48,
	eB8G8R8A8Sint = 49,
	eB8G8R8A8Srgb = 50,
	eA8B8G8R8UnormPack32 = 51,
	eA8B8G8R8SnormPack32 = 52,
	eA8B8G8R8UscaledPack32 = 53,
	eA8B8G8R8SscaledPack32 = 54,
	eA8B8G8R8UintPack32 = 55,
	eA8B8G8R8SintPack32 = 56,
	eA8B8G8R8SrgbPack32 = 57,
	eA2R10G10B10UnormPack32 = 58,
	eA2R10G10B10SnormPack32 = 59,
	eA2R10G10B10UscaledPack32 = 60,
	eA2R10G10B10SscaledPack32 = 61,
	eA2R10G10B10UintPack32 = 62,
	eA2R10G10B10SintPack32 = 63,
	eA2B10G10R10UnormPack32 = 64,
	eA2B10G10R10SnormPack32 = 65,
	eA2B10G10R10UscaledPack32 = 66,
	eA2B10G10R10SscaledPack32 = 67,
	eA2B10G10R10UintPack32 = 68,
	eA2B10G10R10SintPack32 = 69,
	eR16Unorm = 70,
	eR16Snorm = 71,
	eR16Uscaled = 72,
	eR16Sscaled = 73,
	eR16Uint = 74,
	eR16Sint = 75,
	eR16Sfloat = 76,
	eR16G16Unorm = 77,
	eR16G16Snorm = 78,
	eR16G16Uscaled = 79,
	eR16G16Sscaled = 80,
	eR16G16Uint = 81,
	eR16G16Sint = 82,
	eR16G16Sfloat = 83,
	eR16G16B16Unorm = 84,
	eR16G16B16Snorm = 85,
	eR16G16B16Uscaled = 86,
	eR16G16B16Sscaled = 87,
	eR16G16B16Uint = 88,
	eR16G16B16Sint = 89,
	eR16G16B16Sfloat = 90,
	eR16G16B16A16Unorm = 91,
	eR16G16B16A16Snorm = 92,
	eR16G16B16A16Uscaled = 93,
	eR16G16B16A16Sscaled = 94,
	eR16G16B16A16Uint = 95,
	eR16G16B16A16Sint = 96,
	eR16G16B16A16Sfloat = 97,
	eR32Uint = 98,
	eR32Sint = 99,
	eR32Sfloat = 100,
	eR32G32Uint = 101,
	eR32G32Sint = 102,
	eR32G32Sfloat = 103,
	eR32G32B32Uint = 104,
	eR32G32B32Sint = 105,
	eR32G32B32Sfloat = 106,
	eR32G32B32A32Uint = 107,
	eR32G32B32A32Sint = 108,
	eR32G32B32A32Sfloat = 109,
	eR64Uint = 110,
	eR64Sint = 111,
	eR64Sfloat = 112,
	eR64G64Uint = 113,
	eR64G64Sint = 114,
	eR64G64Sfloat = 115,
	eR64G64B64Uint = 116,
	eR64G64B64Sint = 117,
	eR64G64B64Sfloat = 118,
	eR64G64B64A64Uint = 119,
	eR64G64B64A64Sint = 120,
	eR64G64B64A64Sfloat = 121,
	eB10G11R11UfloatPack32 = 122,
	eE5B9G9R9UfloatPack32 = 123,
	eD16Unorm = 124,
	eX8D24UnormPack32 = 125,
	eD32Sfloat = 126,
	eS8Uint = 127,
	eD16UnormS8Uint = 128,
	eD24UnormS8Uint = 129,
	eD32SfloatS8Uint = 130,
	eBc1RgbUnormBlock = 131,
	eBc1RgbSrgbBlock = 132,
	eBc1RgbaUnormBlock = 133,
	eBc1RgbaSrgbBlock = 134,
	eBc2UnormBlock = 135,
	eBc2SrgbBlock = 136,
	eBc3UnormBlock = 137,
	eBc3SrgbBlock = 138,
	eBc4UnormBlock = 139,
	eBc4SnormBlock = 140,
	eBc5UnormBlock = 141,
	eBc5SnormBlock = 142,
	eBc6hUfloatBlock = 143,
	eBc6hSfloatBlock = 144,
	eBc7UnormBlock = 145,
	eBc7SrgbBlock = 146,
	eEtc2R8G8B8UnormBlock = 147,
	eEtc2R8G8B8SrgbBlock = 148,
	eEtc2R8G8B8A1UnormBlock = 149,
	eEtc2R8G8B8A1SrgbBlock = 150,
	eEtc2R8G8B8A8UnormBlock = 151,
	eEtc2R8G8B8A8SrgbBlock = 152,
	eEacR11UnormBlock = 153,
	eEacR11SnormBlock = 154,
	eEacR11G11UnormBlock = 155,
	eEacR11G11SnormBlock = 156,
	eAstc4x4UnormBlock = 157,
	eAstc4x4SrgbBlock = 158,
	eAstc5x4UnormBlock = 159,
	eAstc5x4SrgbBlock = 160,
	eAstc5x5UnormBlock = 161,
	eAstc5x5SrgbBlock = 162,
	eAstc6x5UnormBlock = 163,
	eAstc6x5SrgbBlock = 164,
	eAstc6x6UnormBlock = 165,
	eAstc6x6SrgbBlock = 166,
	eAstc8x5UnormBlock = 167,
	eAstc8x5SrgbBlock = 168,
	eAstc8x6UnormBlock = 169,
	eAstc8x6SrgbBlock = 170,
	eAstc8x8UnormBlock = 171,
	eAstc8x8SrgbBlock = 172,
	eAstc10x5UnormBlock = 173,
	eAstc10x5SrgbBlock = 174,
	eAstc10x6UnormBlock = 175,
	eAstc10x6SrgbBlock = 176,
	eAstc10x8UnormBlock = 177,
	eAstc10x8SrgbBlock = 178,
	eAstc10x10UnormBlock = 179,
	eAstc10x10SrgbBlock = 180,
	eAstc12x10UnormBlock = 181,
	eAstc12x10SrgbBlock = 182,
	eAstc12x12UnormBlock = 183,
	eAstc12x12SrgbBlock = 184,
	eG8B8G8R8422Unorm = 1000156000,
	eB8G8R8G8422Unorm = 1000156001,
	eG8B8R83plane420Unorm = 1000156002,
	eG8B8R82plane420Unorm = 1000156003,
	eG8B8R83plane422Unorm = 1000156004,
	eG8B8R82plane422Unorm = 1000156005,
	eG8B8R83plane444Unorm = 1000156006,
	eR10x6UnormPack16 = 1000156007,
	eR10x6G10x6Unorm2pack16 = 1000156008,
	eR10x6G10x6B10x6A10x6Unorm4pack16 = 1000156009,
	eG10x6B10x6G10x6R10x6422Unorm4pack16 = 1000156010,
	eB10x6G10x6R10x6G10x6422Unorm4pack16 = 1000156011,
	eG10x6B10x6R10x63plane420Unorm3pack16 = 1000156012,
	eG10x6B10x6R10x62plane420Unorm3pack16 = 1000156013,
	eG10x6B10x6R10x63plane422Unorm3pack16 = 1000156014,
	eG10x6B10x6R10x62plane422Unorm3pack16 = 1000156015,
	eG10x6B10x6R10x63plane444Unorm3pack16 = 1000156016,
	eR12x4UnormPack16 = 1000156017,
	eR12x4G12x4Unorm2pack16 = 1000156018,
	eR12x4G12x4B12x4A12x4Unorm4pack16 = 1000156019,
	eG12x4B12x4G12x4R12x4422Unorm4pack16 = 1000156020,
	eB12x4G12x4R12x4G12x4422Unorm4pack16 = 1000156021,
	eG12x4B12x4R12x43plane420Unorm3pack16 = 1000156022,
	eG12x4B12x4R12x42plane420Unorm3pack16 = 1000156023,
	eG12x4B12x4R12x43plane422Unorm3pack16 = 1000156024,
	eG12x4B12x4R12x42plane422Unorm3pack16 = 1000156025,
	eG12x4B12x4R12x43plane444Unorm3pack16 = 1000156026,
	eG16B16G16R16422Unorm = 1000156027,
	eB16G16R16G16422Unorm = 1000156028,
	eG16B16R163plane420Unorm = 1000156029,
	eG16B16R162plane420Unorm = 1000156030,
	eG16B16R163plane422Unorm = 1000156031,
	eG16B16R162plane422Unorm = 1000156032,
	eG16B16R163plane444Unorm = 1000156033,
	eG8B8R82plane444Unorm = 1000330000,
	eG10x6B10x6R10x62plane444Unorm3pack16 = 1000330001,
	eG12x4B12x4R12x42plane444Unorm3pack16 = 1000330002,
	eG16B16R162plane444Unorm = 1000330003,
	eA4R4G4B4UnormPack16 = 1000340000,
	eA4B4G4R4UnormPack16 = 1000340001,
	eAstc4x4SfloatBlock = 1000066000,
	eAstc5x4SfloatBlock = 1000066001,
	eAstc5x5SfloatBlock = 1000066002,
	eAstc6x5SfloatBlock = 1000066003,
	eAstc6x6SfloatBlock = 1000066004,
	eAstc8x5SfloatBlock = 1000066005,
	eAstc8x6SfloatBlock = 1000066006,
	eAstc8x8SfloatBlock = 1000066007,
	eAstc10x5SfloatBlock = 1000066008,
	eAstc10x6SfloatBlock = 1000066009,
	eAstc10x8SfloatBlock = 1000066010,
	eAstc10x10SfloatBlock = 1000066011,
	eAstc12x10SfloatBlock = 1000066012,
	eAstc12x12SfloatBlock = 1000066013,
	eA1B5G5R5UnormPack16 = 1000470000,
	eA8Unorm = 1000470001,
	ePvrtc12bppUnormBlockIMG = 1000054000,
	ePvrtc14bppUnormBlockIMG = 1000054001,
	ePvrtc22bppUnormBlockIMG = 1000054002,
	ePvrtc24bppUnormBlockIMG = 1000054003,
	ePvrtc12bppSrgbBlockIMG = 1000054004,
	ePvrtc14bppSrgbBlockIMG = 1000054005,
	ePvrtc22bppSrgbBlockIMG = 1000054006,
	ePvrtc24bppSrgbBlockIMG = 1000054007,
	eAstc4x4SfloatBlockEXT = 1000066000,
	eAstc5x4SfloatBlockEXT = 1000066001,
	eAstc5x5SfloatBlockEXT = 1000066002,
	eAstc6x5SfloatBlockEXT = 1000066003,
	eAstc6x6SfloatBlockEXT = 1000066004,
	eAstc8x5SfloatBlockEXT = 1000066005,
	eAstc8x6SfloatBlockEXT = 1000066006,
	eAstc8x8SfloatBlockEXT = 1000066007,
	eAstc10x5SfloatBlockEXT = 1000066008,
	eAstc10x6SfloatBlockEXT = 1000066009,
	eAstc10x8SfloatBlockEXT = 1000066010,
	eAstc10x10SfloatBlockEXT = 1000066011,
	eAstc12x10SfloatBlockEXT = 1000066012,
	eAstc12x12SfloatBlockEXT = 1000066013,
	eG8B8G8R8422UnormKHR = 1000156000,
	eB8G8R8G8422UnormKHR = 1000156001,
	eG8B8R83plane420UnormKHR = 1000156002,
	eG8B8R82plane420UnormKHR = 1000156003,
	eG8B8R83plane422UnormKHR = 1000156004,
	eG8B8R82plane422UnormKHR = 1000156005,
	eG8B8R83plane444UnormKHR = 1000156006,
	eR10x6UnormPack16KHR = 1000156007,
	eR10x6G10x6Unorm2pack16KHR = 1000156008,
	eR10x6G10x6B10x6A10x6Unorm4pack16KHR = 1000156009,
	eG10x6B10x6G10x6R10x6422Unorm4pack16KHR = 1000156010,
	eB10x6G10x6R10x6G10x6422Unorm4pack16KHR = 1000156011,
	eG10x6B10x6R10x63plane420Unorm3pack16KHR = 1000156012,
	eG10x6B10x6R10x62plane420Unorm3pack16KHR = 1000156013,
	eG10x6B10x6R10x63plane422Unorm3pack16KHR = 1000156014,
	eG10x6B10x6R10x62plane422Unorm3pack16KHR = 1000156015,
	eG10x6B10x6R10x63plane444Unorm3pack16KHR = 1000156016,
	eR12x4UnormPack16KHR = 1000156017,
	eR12x4G12x4Unorm2pack16KHR = 1000156018,
	eR12x4G12x4B12x4A12x4Unorm4pack16KHR = 1000156019,
	eG12x4B12x4G12x4R12x4422Unorm4pack16KHR = 1000156020,
	eB12x4G12x4R12x4G12x4422Unorm4pack16KHR = 1000156021,
	eG12x4B12x4R12x43plane420Unorm3pack16KHR = 1000156022,
	eG12x4B12x4R12x42plane420Unorm3pack16KHR = 1000156023,
	eG12x4B12x4R12x43plane422Unorm3pack16KHR = 1000156024,
	eG12x4B12x4R12x42plane422Unorm3pack16KHR = 1000156025,
	eG12x4B12x4R12x43plane444Unorm3pack16KHR = 1000156026,
	eG16B16G16R16422UnormKHR = 1000156027,
	eB16G16R16G16422UnormKHR = 1000156028,
	eG16B16R163plane420UnormKHR = 1000156029,
	eG16B16R162plane420UnormKHR = 1000156030,
	eG16B16R163plane422UnormKHR = 1000156031,
	eG16B16R162plane422UnormKHR = 1000156032,
	eG16B16R163plane444UnormKHR = 1000156033,
	eG8B8R82plane444UnormEXT = 1000330000,
	eG10x6B10x6R10x62plane444Unorm3pack16EXT = 1000330001,
	eG12x4B12x4R12x42plane444Unorm3pack16EXT = 1000330002,
	eG16B16R162plane444UnormEXT = 1000330003,
	eA4R4G4B4UnormPack16EXT = 1000340000,
	eA4B4G4R4UnormPack16EXT = 1000340001,
	eR16G16Sfixed5NV = 1000464000,
	eR16G16S105NV = 1000464000,
	eA1B5G5R5UnormPack16KHR = 1000470000,
	eA8UnormKHR = 1000470001,
};

enum class FrontFace {
	eCounterClockwise = 0,
	eClockwise = 1,
};

enum class ImageLayout {
	eUndefined = 0,
	eGeneral = 1,
	eColorAttachmentOptimal = 2,
	eDepthStencilAttachmentOptimal = 3,
	eDepthStencilReadOnlyOptimal = 4,
	eShaderReadOnlyOptimal = 5,
	eTransferSrcOptimal = 6,
	eTransferDstOptimal = 7,
	ePreinitialized = 8,
	eDepthReadOnlyStencilAttachmentOptimal = 1000117000,
	eDepthAttachmentStencilReadOnlyOptimal = 1000117001,
	eDepthAttachmentOptimal = 1000241000,
	eDepthReadOnlyOptimal = 1000241001,
	eStencilAttachmentOptimal = 1000241002,
	eStencilReadOnlyOptimal = 1000241003,
	eReadOnlyOptimal = 1000314000,
	eAttachmentOptimal = 1000314001,
	eRenderingLocalRead = 1000232000,
	ePresentSrcKHR = 1000001002,
	eVideoDecodeDstKHR = 1000024000,
	eVideoDecodeSrcKHR = 1000024001,
	eVideoDecodeDpbKHR = 1000024002,
	eSharedPresentKHR = 1000111000,
	eDepthReadOnlyStencilAttachmentOptimalKHR = 1000117000,
	eDepthAttachmentStencilReadOnlyOptimalKHR = 1000117001,
	eShadingRateOptimalNV = 1000164003,
	eFragmentDensityMapOptimalEXT = 1000218000,
	eFragmentShadingRateAttachmentOptimalKHR = 1000164003,
	eRenderingLocalReadKHR = 1000232000,
	eDepthAttachmentOptimalKHR = 1000241000,
	eDepthReadOnlyOptimalKHR = 1000241001,
	eStencilAttachmentOptimalKHR = 1000241002,
	eStencilReadOnlyOptimalKHR = 1000241003,
	eVideoEncodeDstKHR = 1000299000,
	eVideoEncodeSrcKHR = 1000299001,
	eVideoEncodeDpbKHR = 1000299002,
	eReadOnlyOptimalKHR = 1000314000,
	eAttachmentOptimalKHR = 1000314001,
	eAttachmentFeedbackLoopOptimalEXT = 1000339000,
	eVideoEncodeQuantizationMapKHR = 1000553000,
};

enum class ImageTiling {
	eOptimal = 0,
	eLinear = 1,
	eDrmFormatModifierEXT = 1000158000,
};

enum class ImageType {
	e1D = 0,
	e2D = 1,
	e3D = 2,
};

enum class ImageViewType {
	e1D = 0,
	e2D = 1,
	e3D = 2,
	eCube = 3,
	e1DArray = 4,
	e2DArray = 5,
	eCubeArray = 6,
};

enum class IndirectCommandsTokenTypeEXT {
	eExecutionSet = 0,
	ePushConstant = 1,
	eSequenceIndex = 2,
	eIndexBuffer = 3,
	eVertexBuffer = 4,
	eDrawIndexed = 5,
	eDraw = 6,
	eDrawIndexedCount = 7,
	eDrawCount = 8,
	eDispatch = 9,
	eDrawMeshTasksNV = 1000202002,
	eDrawMeshTasksCountNV = 1000202003,
	eDrawMeshTasks = 1000328000,
	eDrawMeshTasksCount = 1000328001,
	eTraceRays2 = 1000386004,
};

enum class SharingMode {
	eExclusive = 0,
	eConcurrent = 1,
};

enum class IndexType {
	eUint16 = 0,
	eUint32 = 1,
	eUint8 = 1000265000,
	eNoneKHR = 1000165000,
	eNoneNV = 1000165000,
	eUint8EXT = 1000265000,
	eUint8KHR = 1000265000,
};

enum class LogicOp {
	eClear = 0,
	eAnd = 1,
	eAndReverse = 2,
	eCopy = 3,
	eAndInverted = 4,
	eNo = 5,
	eXor = 6,
	eOr = 7,
	eNor = 8,
	eEquivalent = 9,
	eInvert = 10,
	eOrReverse = 11,
	eCopyInverted = 12,
	eOrInverted = 13,
	eNand = 14,
	eSet = 15,
};

enum class PhysicalDeviceType {
	eOther = 0,
	eIntegratedGpu = 1,
	eDiscreteGpu = 2,
	eVirtualGpu = 3,
	eCpu = 4,
};

enum class PipelineBindPoint {
	eGraphics = 0,
	eCompute = 1,
	eRayTracingKHR = 1000165000,
	eRayTracingNV = 1000165000,
	eSubpassShadingHUAWEI = 1000369003,
#ifdef VK_ENABLE_BETA_EXTENSIONS
	eExecutionGraphAMDX = 1000134000,
#endif // VK_ENABLE_BETA_EXTENSIONS
};

enum class PrimitiveTopology {
	ePointList = 0,
	eLineList = 1,
	eLineStrip = 2,
	eTriangleList = 3,
	eTriangleStrip = 4,
	eTriangleFan = 5,
	eLineListWithAdjacency = 6,
	eLineStripWithAdjacency = 7,
	eTriangleListWithAdjacency = 8,
	eTriangleStripWithAdjacency = 9,
	ePatchList = 10,
};

enum class QueryType {
	eOcclusion = 0,
	ePipelineStatistics = 1,
	eTimestamp = 2,
	eResultStatusOnlyKHR = 1000023000,
	eTransformFeedbackStreamEXT = 1000028004,
	ePerformanceQueryKHR = 1000116000,
	eAccelerationStructureCompactedSizeKHR = 1000150000,
	eAccelerationStructureSerializationSizeKHR = 1000150001,
	eAccelerationStructureCompactedSizeNV = 1000165000,
	ePerformanceQueryINTEL = 1000210000,
	eVideoEncodeFeedbackKHR = 1000299000,
	eMeshPrimitivesGeneratedEXT = 1000328000,
	ePrimitivesGeneratedEXT = 1000382000,
	eAccelerationStructureSerializationBottomLevelPointersKHR = 1000386000,
	eAccelerationStructureSizeKHR = 1000386001,
	eMicromapSerializationSizeEXT = 1000396000,
	eMicromapCompactedSizeEXT = 1000396001,
};

enum class StencilOp {
	eKeep = 0,
	eZero = 1,
	eReplace = 2,
	eIncrementAndClamp = 3,
	eDecrementAndClamp = 4,
	eInvert = 5,
	eIncrementAndWrap = 6,
	eDecrementAndWrap = 7,
};

enum class SubpassContents {
	eInline = 0,
	eSecondaryCommandBuffers = 1,
	eInlineAndSecondaryCommandBuffersEXT = 1000451000,
	eInlineAndSecondaryCommandBuffersKHR = 1000451000,
};

enum class SystemAllocationScope {
	eCommand = 0,
	eObject = 1,
	eCache = 2,
	eDevice = 3,
	eInstance = 4,
};

enum class InternalAllocationType {
	eExecutable = 0,
};

enum class SamplerAddressMode {
	eRepeat = 0,
	eMirroredRepeat = 1,
	eClampToEdge = 2,
	eClampToBorder = 3,
	eMirrorClampToEdge = 4,
	eMirrorClampToEdgeKHR = 4,
};

enum class Filter {
	eNearest = 0,
	eLinear = 1,
	eCubicIMG = 1000015000,
	eCubicEXT = 1000015000,
};

enum class SamplerMipmapMode {
	eNearest = 0,
	eLinear = 1,
};

enum class VertexInputRate {
	eVertex = 0,
	eInstance = 1,
};

enum class ObjectType {
	eUnknown = 0,
	eInstance = 1,
	ePhysicalDevice = 2,
	eDevice = 3,
	eQueue = 4,
	eSemaphore = 5,
	eCommandBuffer = 6,
	eFence = 7,
	eDeviceMemory = 8,
	eBuffer = 9,
	eImage = 10,
	eEvent = 11,
	eQueryPool = 12,
	eBufferView = 13,
	eImageView = 14,
	eShaderModule = 15,
	ePipelineCache = 16,
	ePipelineLayout = 17,
	eRenderPass = 18,
	ePipeline = 19,
	eDescriptorSetLayout = 20,
	eSampler = 21,
	eDescriptorPool = 22,
	eDescriptorSet = 23,
	eFramebuffer = 24,
	eCommandPool = 25,
	eSamplerYcbcrConversion = 1000156000,
	eDescriptorUpdateTemplate = 1000085000,
	ePrivateDataSlot = 1000295000,
	eSurfaceKHR = 1000000000,
	eSwapchainKHR = 1000001000,
	eDisplayKHR = 1000002000,
	eDisplayModeKHR = 1000002001,
	eDebugReportCallbackEXT = 1000011000,
	eVideoSessionKHR = 1000023000,
	eVideoSessionParametersKHR = 1000023001,
	eCuModuleNVX = 1000029000,
	eCuFunctionNVX = 1000029001,
	eDescriptorUpdateTemplateKHR = 1000085000,
	eDebugUtilsMessengerEXT = 1000128000,
	eAccelerationStructureKHR = 1000150000,
	eSamplerYcbcrConversionKHR = 1000156000,
	eValidationCacheEXT = 1000160000,
	eAccelerationStructureNV = 1000165000,
	ePerformanceConfigurationINTEL = 1000210000,
	eDeferredOperationKHR = 1000268000,
	eIndirectCommandsLayoutNV = 1000277000,
	ePrivateDataSlotEXT = 1000295000,
	eCudaModuleNV = 1000307000,
	eCudaFunctionNV = 1000307001,
	eMicromapEXT = 1000396000,
	eOpticalFlowSessionNV = 1000464000,
	eShaderEXT = 1000482000,
	ePipelineBinaryKHR = 1000483000,
	eIndirectCommandsLayoutEXT = 1000572000,
	eIndirectExecutionSetEXT = 1000572001,
#ifdef VK_USE_PLATFORM_FUCHSIA
	eBufferCollectionFUCHSIA = 1000366000,
#endif // VK_USE_PLATFORM_FUCHSIA
};

enum class RayTracingInvocationReorderModeNV {
	eNone = 0,
	eReorder = 1,
};

enum class IndirectCommandsTokenTypeNV {
	eShaderGroup = 0,
	eStateFlags = 1,
	eIndexBuffer = 2,
	eVertexBuffer = 3,
	ePushConstant = 4,
	eDrawIndexed = 5,
	eDraw = 6,
	eDrawTasks = 7,
	eDrawMeshTasks = 1000328000,
	ePipeline = 1000428003,
	eDispatch = 1000428004,
};

enum class DescriptorUpdateTemplateType {
	eDescriptorSet = 0,
	ePushDescriptors = 1,
	ePushDescriptorsKHR = 1,
	eDescriptorSetKHR = 0,
};

enum class PointClippingBehavior {
	eAllClipPlanes = 0,
	eUserClipPlanesOnly = 1,
	eAllClipPlanesKHR = 0,
	eUserClipPlanesOnlyKHR = 1,
};

enum class CoverageModulationModeNV {
	eNone = 0,
	eRgb = 1,
	eAlpha = 2,
	eRgba = 3,
};

enum class QueueGlobalPriority {
	eLow = 128,
	eMedium = 256,
	eHigh = 512,
	eRealtime = 1024,
	eLowEXT = 128,
	eMediumEXT = 256,
	eHighEXT = 512,
	eRealtimeEXT = 1024,
	eLowKHR = 128,
	eMediumKHR = 256,
	eHighKHR = 512,
	eRealtimeKHR = 1024,
};

enum class SemaphoreType {
	eBinary = 0,
	eTimeline = 1,
	eBinaryKHR = 0,
	eTimelineKHR = 1,
};

enum class PipelineRobustnessBufferBehavior {
	eDeviceDefault = 0,
	eDisabled = 1,
	eRobustBufferAccess = 2,
	eRobustBufferAccess2 = 3,
	eDeviceDefaultEXT = 0,
	eDisabledEXT = 1,
	eRobustBufferAccessEXT = 2,
	eRobustBufferAccess2EXT = 3,
};
enum class PipelineRobustnessImageBehavior {
	eDeviceDefault = 0,
	eDisabled = 1,
	eRobustImageAccess = 2,
	eRobustImageAccess2 = 3,
	eDeviceDefaultEXT = 0,
	eDisabledEXT = 1,
	eRobustImageAccessEXT = 2,
	eRobustImageAccess2EXT = 3,
};

enum class TessellationDomainOrigin {
	eUpperLeft = 0,
	eLowerLeft = 1,
	eUpperLeftKHR = 0,
	eLowerLeftKHR = 1,
};

enum class SamplerYcbcrModelConversion {
	eRgbIdentity = 0,
	eYcbcrIdentity = 1,
	eYcbcR709 = 2,
	eYcbcR601 = 3,
	eYcbcR2020 = 4,
	eRgbIdentityKHR = 0,
	eYcbcrIdentityKHR = 1,
	eYcbcR709KHR = 2,
	eYcbcR601KHR = 3,
	eYcbcR2020KHR = 4,
};

enum class SamplerYcbcrRange {
	eItuFull = 0,
	eItuNarrow = 1,
	eItuFullKHR = 0,
	eItuNarrowKHR = 1,
};

enum class ChromaLocation {
	eCositedEven = 0,
	eMidpoint = 1,
	eCositedEvenKHR = 0,
	eMidpointKHR = 1,
};

enum class SamplerReductionMode {
	eWeightedAverage = 0,
	eMin = 1,
	eMax = 2,
	eWeightedAverageEXT = 0,
	eMinEXT = 1,
	eMaxEXT = 2,
	eWeightedAverageRangeclampQCOM = 1000521000,
};

#if defined(VK_USE_PLATFORM_WIN32_KHR)
enum class FullScreenExclusiveEXT {
	eDefault = 0,
	eAllowed = 1,
	eDisallowed = 2,
	eApplicationControlled = 3,
};
#endif // VK_USE_PLATFORM_WIN32_KHR

enum class ShaderFloatControlsIndependence {
	e32BitOnly = 0,
	eAll = 1,
	eNone = 2,
	e32BitOnlyKHR = 0,
	eAllKHR = 1,
	eNoneKHR = 2,
};

enum class VendorId {
	eKhronos = 0x10000,
	eVIV = 0x10001,
	eVSI = 0x10002,
	eKazan = 0x10003,
	eCodeplay = 0x10004,
	eMESA = 0x10005,
	ePocl = 0x10006,
	eMobileye = 0x10007,
};

enum class FramebufferCreateFlagBits : uint32_t {
	eImageless = 0x1,
	eImagelessKHR = 0x1,
};
using FramebufferCreateFlags = Flags<FramebufferCreateFlagBits>;

enum class QueryPoolCreateFlagBits : uint32_t {
};
using QueryPoolCreateFlags = Flags<QueryPoolCreateFlagBits>;

enum class RenderPassCreateFlagBits : uint32_t {
	eTransformQCOM = 0x2,
};
using RenderPassCreateFlags = Flags<RenderPassCreateFlagBits>;

enum class SamplerCreateFlagBits : uint32_t {
	eSubsampledEXT = 0x1,
	eSubsampledCoarseReconstructionEXT = 0x2,
	eDescriptorBufferCaptureReplayEXT = 0x8,
	eNonSeamlessCubeMapEXT = 0x4,
	eImageProcessingQCOM = 0x10,
};
using SamplerCreateFlags = Flags<SamplerCreateFlagBits>;

enum class PipelineLayoutCreateFlagBits : uint32_t {
	eIndependentSetsEXT = 0x2,
};
using PipelineLayoutCreateFlags = Flags<PipelineLayoutCreateFlagBits>;

enum class PipelineCacheCreateFlagBits : uint32_t {
	eExternallySynchronized = 0x1,
	eExternallySynchronizedEXT = 0x1,
};
using PipelineCacheCreateFlags = Flags<PipelineCacheCreateFlagBits>;

enum class PipelineDepthStencilStateCreateFlagBits : uint32_t {
	eRasterizationOrderAttachmentDepthAccessARM = 0x1,
	eRasterizationOrderAttachmentStencilAccessARM = 0x2,
	eRasterizationOrderAttachmentDepthAccessEXT = 0x1,
	eRasterizationOrderAttachmentStencilAccessEXT = 0x2,
};
using PipelineDepthStencilStateCreateFlags = Flags<PipelineDepthStencilStateCreateFlagBits>;

enum class PipelineDynamicStateCreateFlagBits : uint32_t {
};
using PipelineDynamicStateCreateFlags = Flags<PipelineDynamicStateCreateFlagBits>;

enum class PipelineColorBlendStateCreateFlagBits : uint32_t {
	eRasterizationOrderAttachmentAccessARM = 0x1,
	eRasterizationOrderAttachmentAccessEXT = 0x1,
};
using PipelineColorBlendStateCreateFlags = Flags<PipelineColorBlendStateCreateFlagBits>;

enum class PipelineMultisampleStateCreateFlagBits : uint32_t {
};
using PipelineMultisampleStateCreateFlags = Flags<PipelineMultisampleStateCreateFlagBits>;

enum class PipelineRasterizationStateCreateFlagBits : uint32_t {
};
using PipelineRasterizationStateCreateFlags = Flags<PipelineRasterizationStateCreateFlagBits>;

enum class PipelineViewportStateCreateFlagBits : uint32_t {
};
using PipelineViewportStateCreateFlags = Flags<PipelineViewportStateCreateFlagBits>;

enum class PipelineTessellationStateCreateFlagBits : uint32_t {
};
using PipelineTessellationStateCreateFlags = Flags<PipelineTessellationStateCreateFlagBits>;

enum class PipelineInputAssemblyStateCreateFlagBits : uint32_t {
};
using PipelineInputAssemblyStateCreateFlags = Flags<PipelineInputAssemblyStateCreateFlagBits>;

enum class PipelineVertexInputStateCreateFlagBits : uint32_t {
};
using PipelineVertexInputStateCreateFlags = Flags<PipelineVertexInputStateCreateFlagBits>;

enum class PipelineShaderStageCreateFlagBits : uint32_t {
	eAllowVaryingSubgroupSize = 0x1,
	eRequireFullSubgroups = 0x2,
	eAllowVaryingSubgroupSizeEXT = 0x1,
	eRequireFullSubgroupsEXT = 0x2,
};
using PipelineShaderStageCreateFlags = Flags<PipelineShaderStageCreateFlagBits>;

enum class DescriptorSetLayoutCreateFlagBits : uint32_t {
	eUpdateAfterBindPool = 0x2,
	ePushDescriptor = 0x1,
	ePushDescriptorKHR = 0x1,
	eUpdateAfterBindPoolEXT = 0x2,
	eDescriptorBufferEXT = 0x10,
	eEmbeddedImmutableSamplersEXT = 0x20,
	eHostOnlyPoolVALVE = 0x4,
	eIndirectBindableNV = 0x80,
	eHostOnlyPoolEXT = 0x4,
	ePerStageNV = 0x40,
};
using DescriptorSetLayoutCreateFlags = Flags<DescriptorSetLayoutCreateFlagBits>;

enum class BufferViewCreateFlagBits : uint32_t {
};
using BufferViewCreateFlags = Flags<BufferViewCreateFlagBits>;

enum class InstanceCreateFlagBits : uint32_t {
	eEnumeratePortabilityKHR = 0x1,
};
using InstanceCreateFlags = Flags<InstanceCreateFlagBits>;

enum class DeviceCreateFlagBits : uint32_t {
};
using DeviceCreateFlags = Flags<DeviceCreateFlagBits>;

enum class DeviceQueueCreateFlagBits : uint32_t {
	eProtected = 0x1,
};
using DeviceQueueCreateFlags = Flags<DeviceQueueCreateFlagBits>;

enum class QueueFlagBits : uint32_t {
	eGraphics = 0x1,
	eCompute = 0x2,
	eTransfer = 0x4,
	eSparseBinding = 0x8,
	eProtected = 0x10,
	eVideoDecodeKHR = 0x20,
	eVideoEncodeKHR = 0x40,
	eOpticalFlowNV = 0x100,
};
using QueueFlags = Flags<QueueFlagBits>;

enum class MemoryPropertyFlagBits : uint32_t {
	eDeviceLocal = 0x1,
	eHostVisible = 0x2,
	eHostCoherent = 0x4,
	eHostCached = 0x8,
	eLazilyAllocated = 0x10,
	eProtected = 0x20,
	eDeviceCoherentAMD = 0x40,
	eDeviceUncachedAMD = 0x80,
	eRdmaCapableNV = 0x100,
};
using MemoryPropertyFlags = Flags<MemoryPropertyFlagBits>;

enum class MemoryHeapFlagBits : uint32_t {
	eDeviceLocal = 0x1,
	eMultiInstance = 0x2,
	eMultiInstanceKHR = 0x2,
};
using MemoryHeapFlags = Flags<MemoryHeapFlagBits>;

enum class AccessFlagBits : uint32_t {
	eIndirectCommandRead = 0x1,
	eIndexRead = 0x2,
	eVertexAttributeRead = 0x4,
	eUniformRead = 0x8,
	eInputAttachmentRead = 0x10,
	eShaderRead = 0x20,
	eShaderWrite = 0x40,
	eColorAttachmentRead = 0x80,
	eColorAttachmentWrite = 0x100,
	eDepthStencilAttachmentRead = 0x200,
	eDepthStencilAttachmentWrite = 0x400,
	eTransferRead = 0x800,
	eTransferWrite = 0x1000,
	eHostRead = 0x2000,
	eHostWrite = 0x4000,
	eMemoryRead = 0x8000,
	eMemoryWrite = 0x10000,
	eNone = 0,
	eTransformFeedbackWriteEXT = 0x2000000,
	eTransformFeedbackCounterReadEXT = 0x4000000,
	eTransformFeedbackCounterWriteEXT = 0x8000000,
	eConditionalRenderingReadEXT = 0x100000,
	eColorAttachmentReadNoncoherentEXT = 0x80000,
	eAccelerationStructureReadKHR = 0x200000,
	eAccelerationStructureWriteKHR = 0x400000,
	eShadingRateImageReadNV = 0x800000,
	eAccelerationStructureReadNV = 0x200000,
	eAccelerationStructureWriteNV = 0x400000,
	eFragmentDensityMapReadEXT = 0x1000000,
	eFragmentShadingRateAttachmentReadKHR = 0x800000,
	eCommandPreprocessReadNV = 0x20000,
	eCommandPreprocessWriteNV = 0x40000,
	eNoneKHR = 0,
	eCommandPreprocessReadEXT = 0x20000,
	eCommandPreprocessWriteEXT = 0x40000,
};
using AccessFlags = Flags<AccessFlagBits>;

enum class BufferUsageFlagBits : uint32_t {
	eTransferSrc = 0x1,
	eTransferDst = 0x2,
	eUniformTexelBuffer = 0x4,
	eStorageTexelBuffer = 0x8,
	eUniformBuffer = 0x10,
	eStorageBuffer = 0x20,
	eIndexBuffer = 0x40,
	eVertexBuffer = 0x80,
	eIndirectBuffer = 0x100,
	eShaderDeviceAddress = 0x20000,
	eVideoDecodeSrcKHR = 0x2000,
	eVideoDecodeDstKHR = 0x4000,
	eTransformFeedbackBufferEXT = 0x800,
	eTransformFeedbackCounterBufferEXT = 0x1000,
	eConditionalRenderingEXT = 0x200,
	eAccelerationStructureBuildInputReadOnlyKHR = 0x80000,
	eAccelerationStructureStorageKHR = 0x100000,
	eShaderBindingTableKHR = 0x400,
	eRayTracingNV = 0x400,
	eShaderDeviceAddressEXT = 0x20000,
	eShaderDeviceAddressKHR = 0x20000,
	eVideoEncodeDstKHR = 0x8000,
	eVideoEncodeSrcKHR = 0x10000,
	eSamplerDescriptorBufferEXT = 0x200000,
	eResourceDescriptorBufferEXT = 0x400000,
	ePushDescriptorsDescriptorBufferEXT = 0x4000000,
	eMicromapBuildInputReadOnlyEXT = 0x800000,
	eMicromapStorageEXT = 0x1000000,
#ifdef VK_ENABLE_BETA_EXTENSIONS
	eExecutionGraphScratchAMDX = 0x2000000,
#endif // VK_ENABLE_BETA_EXTENSIONS
};
using BufferUsageFlags = Flags<BufferUsageFlagBits>;

enum class BufferCreateFlagBits : uint32_t {
	eSparseBinding = 0x1,
	eSparseResidency = 0x2,
	eSparseAliased = 0x4,
	eProtected = 0x8,
	eDeviceAddressCaptureReplay = 0x10,
	eDeviceAddressCaptureReplayEXT = 0x10,
	eDeviceAddressCaptureReplayKHR = 0x10,
	eDescriptorBufferCaptureReplayEXT = 0x20,
	eVideoProfileIndependentKHR = 0x40,
};
using BufferCreateFlags = Flags<BufferCreateFlagBits>;

enum class ShaderStageFlagBits : uint32_t {
	eVertex = 0x1,
	eTessellationControl = 0x2,
	eTessellationEvaluation = 0x4,
	eGeometry = 0x8,
	eFragment = 0x10,
	eCompute = 0x20,
	eAllGraphics = 0x0000001F,
	eAll = 0x7FFFFFFF,
	eRaygenKHR = 0x100,
	eAnyHitKHR = 0x200,
	eClosestHitKHR = 0x400,
	eMissKHR = 0x800,
	eIntersectionKHR = 0x1000,
	eCallableKHR = 0x2000,
	eRaygenNV = 0x100,
	eAnyHitNV = 0x200,
	eClosestHitNV = 0x400,
	eMissNV = 0x800,
	eIntersectionNV = 0x1000,
	eCallableNV = 0x2000,
	eTaskNV = 0x40,
	eMeshNV = 0x80,
	eTaskEXT = 0x40,
	eMeshEXT = 0x80,
	eSubpassShadingHUAWEI = 0x4000,
	eClusterCullingHUAWEI = 0x80000,
};
using ShaderStageFlags = Flags<ShaderStageFlagBits>;

enum class ImageUsageFlagBits : uint32_t {
	eTransferSrc = 0x1,
	eTransferDst = 0x2,
	eSampled = 0x4,
	eStorage = 0x8,
	eColorAttachment = 0x10,
	eDepthStencilAttachment = 0x20,
	eTransientAttachment = 0x40,
	eInputAttachment = 0x80,
	eHostTransfer = 0x400000,
	eVideoDecodeDstKHR = 0x400,
	eVideoDecodeSrcKHR = 0x800,
	eVideoDecodeDpbKHR = 0x1000,
	eShadingRateImageNV = 0x100,
	eFragmentDensityMapEXT = 0x200,
	eFragmentShadingRateAttachmentKHR = 0x100,
	eHostTransferEXT = 0x400000,
	eVideoEncodeDstKHR = 0x2000,
	eVideoEncodeSrcKHR = 0x4000,
	eVideoEncodeDpbKHR = 0x8000,
	eAttachmentFeedbackLoopEXT = 0x80000,
	eInvocationMaskHUAWEI = 0x40000,
	eSampleWeightQCOM = 0x100000,
	eSampleBlockMatchQCOM = 0x200000,
	eVideoEncodeQuantizationDeltaMapKHR = 0x2000000,
	eVideoEncodeEmphasisMapKHR = 0x4000000,
};
using ImageUsageFlags = Flags<ImageUsageFlagBits>;

enum class ImageCreateFlagBits : uint32_t {
	eSparseBinding = 0x1,
	eSparseResidency = 0x2,
	eSparseAliased = 0x4,
	eMutableFormat = 0x8,
	eCubeCompatible = 0x10,
	eAlias = 0x400,
	eSplitInstanceBindRegions = 0x40,
	e2DArrayCompatible = 0x20,
	eBlockTexelViewCompatible = 0x80,
	eExtendedUsage = 0x100,
	eProtected = 0x800,
	eDisjoint = 0x200,
	eCornerSampledNV = 0x2000,
	eSplitInstanceBindRegionsKHR = 0x40,
	e2DArrayCompatibleKHR = 0x20,
	eBlockTexelViewCompatibleKHR = 0x80,
	eExtendedUsageKHR = 0x100,
	eSampleLocationsCompatibleDepthEXT = 0x1000,
	eDisjointKHR = 0x200,
	eAliasKHR = 0x400,
	eSubsampledEXT = 0x4000,
	eDescriptorBufferCaptureReplayEXT = 0x10000,
	eMultisampledRenderToSingleSampledEXT = 0x40000,
	e2DViewCompatibleEXT = 0x20000,
	eFragmentDensityMapOffsetQCOM = 0x8000,
	eVideoProfileIndependentKHR = 0x100000,
};
using ImageCreateFlags = Flags<ImageCreateFlagBits>;

enum class ImageViewCreateFlagBits : uint32_t {
	eFragmentDensityMapDynamicEXT = 0x1,
	eDescriptorBufferCaptureReplayEXT = 0x4,
	eFragmentDensityMapDeferredEXT = 0x2,
};
using ImageViewCreateFlags = Flags<ImageViewCreateFlagBits>;

enum class PipelineCreateFlagBits : uint32_t {
	eDisableOptimization = 0x1,
	eAllowDerivatives = 0x2,
	eDerivative = 0x4,
	eViewIndexFromDeviceIndex = 0x8,
	eDispatchBase = 0x10,
	eFailOnPipelineCompileRequired = 0x100,
	eEarlyReturnOnFailure = 0x200,
	eNoProtectedAccess = 0x8000000,
	eProtectedAccessOnly = 0x40000000,
	eViewIndexFromDeviceIndexKHR = 0x8,
	eDispatchBaseKHR = 0x10,
	eRayTracingNoNullAnyHitShadersKHR = 0x4000,
	eRayTracingNoNullClosestHitShadersKHR = 0x8000,
	eRayTracingNoNullMissShadersKHR = 0x10000,
	eRayTracingNoNullIntersectionShadersKHR = 0x20000,
	eRayTracingSkipTrianglesKHR = 0x1000,
	eRayTracingSkipAabbsKHR = 0x2000,
	eRayTracingShaderGroupHandleCaptureReplayKHR = 0x80000,
	eDeferCompileNV = 0x20,
	eRenderingFragmentDensityMapAttachmentEXT = 0x400000,
	eRasterizationStateCreateFragmentDensityMapAttachmentEXT = 0x400000,
	eRenderingFragmentShadingRateAttachmentKHR = 0x200000,
	eRasterizationStateCreateFragmentShadingRateAttachmentKHR = 0x200000,
	eCaptureStatisticsKHR = 0x40,
	eCaptureInternalRepresentationsKHR = 0x80,
	eIndirectBindableNV = 0x40000,
	eLibraryKHR = 0x800,
	eFailOnPipelineCompileRequiredEXT = 0x100,
	eEarlyReturnOnFailureEXT = 0x200,
	eDescriptorBufferEXT = 0x20000000,
	eRetainLinkTimeOptimizationInfoEXT = 0x800000,
	eLinkTimeOptimizationEXT = 0x400,
	eRayTracingAllowMotionNV = 0x100000,
	eColorAttachmentFeedbackLoopEXT = 0x2000000,
	eDepthStencilAttachmentFeedbackLoopEXT = 0x4000000,
	eRayTracingOpacityMicromapEXT = 0x1000000,
	eNoProtectedAccessEXT = 0x8000000,
	eProtectedAccessOnlyEXT = 0x40000000,
#ifdef VK_ENABLE_BETA_EXTENSIONS
	eRayTracingDisplacementMicromapNV = 0x10000000,
#endif // VK_ENABLE_BETA_EXTENSIONS
};
using PipelineCreateFlags = Flags<PipelineCreateFlagBits>;

enum class ColorComponentFlagBits : uint32_t {
	eR = 0x1,
	eG = 0x2,
	eB = 0x4,
	eA = 0x8,
};
using ColorComponentFlags = Flags<ColorComponentFlagBits>;

enum class FenceCreateFlagBits : uint32_t {
	eSignaled = 0x1,
};
using FenceCreateFlags = Flags<FenceCreateFlagBits>;

enum class SemaphoreCreateFlagBits : uint32_t {
};
using SemaphoreCreateFlags = Flags<SemaphoreCreateFlagBits>;

enum class FormatFeatureFlagBits : uint32_t {
	eSampledImage = 0x1,
	eStorageImage = 0x2,
	eStorageImageAtomic = 0x4,
	eUniformTexelBuffer = 0x8,
	eStorageTexelBuffer = 0x10,
	eStorageTexelBufferAtomic = 0x20,
	eVertexBuffer = 0x40,
	eColorAttachment = 0x80,
	eColorAttachmentBlend = 0x100,
	eDepthStencilAttachment = 0x200,
	eBlitSrc = 0x400,
	eBlitDst = 0x800,
	eSampledImageFilterLinear = 0x1000,
	eTransferSrc = 0x4000,
	eTransferDst = 0x8000,
	eMidpointChromaSamples = 0x20000,
	eSampledImageYcbcrConversionLinearFilter = 0x40000,
	eSampledImageYcbcrConversionSeparateReconstructionFilter = 0x80000,
	eSampledImageYcbcrConversionChromaReconstructionExplicit = 0x100000,
	eSampledImageYcbcrConversionChromaReconstructionExplicitForceable = 0x200000,
	eDisjoint = 0x400000,
	eCositedChromaSamples = 0x800000,
	eSampledImageFilterMinmax = 0x10000,
	eSampledImageFilterCubicIMG = 0x2000,
	eVideoDecodeOutputKHR = 0x2000000,
	eVideoDecodeDpbKHR = 0x4000000,
	eTransferSrcKHR = 0x4000,
	eTransferDstKHR = 0x8000,
	eSampledImageFilterMinmaxEXT = 0x10000,
	eAccelerationStructureVertexBufferKHR = 0x20000000,
	eMidpointChromaSamplesKHR = 0x20000,
	eSampledImageYcbcrConversionLinearFilterKHR = 0x40000,
	eSampledImageYcbcrConversionSeparateReconstructionFilterKHR = 0x80000,
	eSampledImageYcbcrConversionChromaReconstructionExplicitKHR = 0x100000,
	eSampledImageYcbcrConversionChromaReconstructionExplicitForceableKHR = 0x200000,
	eDisjointKHR = 0x400000,
	eCositedChromaSamplesKHR = 0x800000,
	eSampledImageFilterCubicEXT = 0x2000,
	eFragmentDensityMapEXT = 0x1000000,
	eFragmentShadingRateAttachmentKHR = 0x40000000,
	eVideoEncodeInputKHR = 0x8000000,
	eVideoEncodeDpbKHR = 0x10000000,
};
using FormatFeatureFlags = Flags<FormatFeatureFlagBits>;

enum class QueryControlFlagBits : uint32_t {
	ePrecise = 0x1,
};
using QueryControlFlags = Flags<QueryControlFlagBits>;

enum class QueryResultFlagBits : uint32_t {
	e64 = 0x1,
	eWait = 0x2,
	eWithAvailability = 0x4,
	ePartial = 0x8,
	eWithStatusKHR = 0x10,
};
using QueryResultFlags = Flags<QueryResultFlagBits>;

enum class ShaderModuleCreateFlagBits : uint32_t {
};
using ShaderModuleCreateFlags = Flags<ShaderModuleCreateFlagBits>;

enum class EventCreateFlagBits : uint32_t {
	eDeviceOnly = 0x1,
	eDeviceOnlyKHR = 0x1,
};
using EventCreateFlags = Flags<EventCreateFlagBits>;

enum class CommandPoolCreateFlagBits : uint32_t {
	eTransient = 0x1,
	eResetCommandBuffer = 0x2,
	eProtected = 0x4,
};
using CommandPoolCreateFlags = Flags<CommandPoolCreateFlagBits>;

enum class CommandPoolResetFlagBits : uint32_t {
	eReleaseResources = 0x1,
};
using CommandPoolResetFlags = Flags<CommandPoolResetFlagBits>;

enum class CommandBufferResetFlagBits : uint32_t {
	eReleaseResources = 0x1,
};
using CommandBufferResetFlags = Flags<CommandBufferResetFlagBits>;

enum class CommandBufferUsageFlagBits : uint32_t {
	eOneTimeSubmit = 0x1,
	eRenderPassContinue = 0x2,
	eSimultaneousUse = 0x4,
};
using CommandBufferUsageFlags = Flags<CommandBufferUsageFlagBits>;

enum class QueryPipelineStatisticFlagBits : uint32_t {
	eInputAssemblyVertices = 0x1,
	eInputAssemblyPrimitives = 0x2,
	eVertexShaderInvocations = 0x4,
	eGeometryShaderInvocations = 0x8,
	eGeometryShaderPrimitives = 0x10,
	eClippingInvocations = 0x20,
	eClippingPrimitives = 0x40,
	eFragmentShaderInvocations = 0x80,
	eTessellationControlShaderPatches = 0x100,
	eTessellationEvaluationShaderInvocations = 0x200,
	eComputeShaderInvocations = 0x400,
	eTaskShaderInvocationsEXT = 0x800,
	eMeshShaderInvocationsEXT = 0x1000,
	eClusterCullingShaderInvocationsHUAWEI = 0x2000,
};
using QueryPipelineStatisticFlags = Flags<QueryPipelineStatisticFlagBits>;

enum class MemoryMapFlagBits : uint32_t {
	ePlacedEXT = 0x1,
};
using MemoryMapFlags = Flags<MemoryMapFlagBits>;

enum class MemoryUnmapFlagBits : uint32_t {
	eReserveEXT = 0x1,
};
using MemoryUnmapFlags = Flags<MemoryUnmapFlagBits>;

enum class ImageAspectFlagBits : uint32_t {
	eColor = 0x1,
	eDepth = 0x2,
	eStencil = 0x4,
	eMetadata = 0x8,
	ePlane0 = 0x10,
	ePlane1 = 0x20,
	ePlane2 = 0x40,
	eNone = 0,
	ePlane0KHR = 0x10,
	ePlane1KHR = 0x20,
	ePlane2KHR = 0x40,
	eMemoryPlane0EXT = 0x80,
	eMemoryPlane1EXT = 0x100,
	eMemoryPlane2EXT = 0x200,
	eMemoryPlane3EXT = 0x400,
	eNoneKHR = 0,
};
using ImageAspectFlags = Flags<ImageAspectFlagBits>;

enum class SparseMemoryBindFlagBits : uint32_t {
	eMetadata = 0x1,
};
using SparseMemoryBindFlags = Flags<SparseMemoryBindFlagBits>;

enum class SparseImageFormatFlagBits : uint32_t {
	eSingleMiptail = 0x1,
	eAlignedMipSize = 0x2,
	eNonstandardBlockSize = 0x4,
};
using SparseImageFormatFlags = Flags<SparseImageFormatFlagBits>;

enum class SubpassDescriptionFlagBits : uint32_t {
	ePerViewAttributesNVX = 0x1,
	ePerViewPositionXOnlyNVX = 0x2,
	eFragmentRegionQCOM = 0x4,
	eShaderResolveQCOM = 0x8,
	eRasterizationOrderAttachmentColorAccessARM = 0x10,
	eRasterizationOrderAttachmentDepthAccessARM = 0x20,
	eRasterizationOrderAttachmentStencilAccessARM = 0x40,
	eRasterizationOrderAttachmentColorAccessEXT = 0x10,
	eRasterizationOrderAttachmentDepthAccessEXT = 0x20,
	eRasterizationOrderAttachmentStencilAccessEXT = 0x40,
	eEnableLegacyDitheringEXT = 0x80,
};
using SubpassDescriptionFlags = Flags<SubpassDescriptionFlagBits>;

enum class PipelineStageFlagBits : uint32_t {
	eTopOfPipe = 0x1,
	eDrawIndirect = 0x2,
	eVertexInput = 0x4,
	eVertexShader = 0x8,
	eTessellationControlShader = 0x10,
	eTessellationEvaluationShader = 0x20,
	eGeometryShader = 0x40,
	eFragmentShader = 0x80,
	eEarlyFragmentTests = 0x100,
	eLateFragmentTests = 0x200,
	eColorAttachmentOutput = 0x400,
	eComputeShader = 0x800,
	eTransfer = 0x1000,
	eBottomOfPipe = 0x2000,
	eHost = 0x4000,
	eAllGraphics = 0x8000,
	eAllCommands = 0x10000,
	eNone = 0,
	eTransformFeedbackEXT = 0x1000000,
	eConditionalRenderingEXT = 0x40000,
	eAccelerationStructureBuildKHR = 0x2000000,
	eRayTracingShaderKHR = 0x200000,
	eShadingRateImageNV = 0x400000,
	eRayTracingShaderNV = 0x200000,
	eAccelerationStructureBuildNV = 0x2000000,
	eTaskShaderNV = 0x80000,
	eMeshShaderNV = 0x100000,
	eFragmentDensityProcessEXT = 0x800000,
	eFragmentShadingRateAttachmentKHR = 0x400000,
	eCommandPreprocessNV = 0x20000,
	eNoneKHR = 0,
	eTaskShaderEXT = 0x80000,
	eMeshShaderEXT = 0x100000,
	eCommandPreprocessEXT = 0x20000,
};
using PipelineStageFlags = Flags<PipelineStageFlagBits>;

enum class SampleCountFlagBits : uint32_t {
	e1 = 0x1,
	e2 = 0x2,
	e4 = 0x4,
	e8 = 0x8,
	e16 = 0x10,
	e32 = 0x20,
	e64 = 0x40,
};
using SampleCountFlags = Flags<SampleCountFlagBits>;

enum class AttachmentDescriptionFlagBits : uint32_t {
	eMayAlias = 0x1,
};
using AttachmentDescriptionFlags = Flags<AttachmentDescriptionFlagBits>;

enum class StencilFaceFlagBits : uint32_t {
	eFront = 0x1,
	eBack = 0x2,
	eFrontAndBack = 0x00000003,
};
using StencilFaceFlags = Flags<StencilFaceFlagBits>;

enum class CullModeFlagBits : uint32_t {
	eNone = 0,
	eFront = 0x1,
	eBack = 0x2,
	eFrontAndBack = 0x00000003,
};
using CullModeFlags = Flags<CullModeFlagBits>;

enum class DescriptorPoolCreateFlagBits : uint32_t {
	eFreeDescriptorSet = 0x1,
	eUpdateAfterBind = 0x2,
	eUpdateAfterBindEXT = 0x2,
	eHostOnlyVALVE = 0x4,
	eHostOnlyEXT = 0x4,
	eAllowOverallocationSetsNV = 0x8,
	eAllowOverallocationPoolsNV = 0x10,
};
using DescriptorPoolCreateFlags = Flags<DescriptorPoolCreateFlagBits>;

enum class DescriptorPoolResetFlagBits : uint32_t {

};
using DescriptorPoolResetFlags = Flags<DescriptorPoolResetFlagBits>;

enum class DependencyFlagBits : uint32_t {
	eByRegion = 0x1,
	eDeviceGroup = 0x4,
	eViewLocal = 0x2,
	eViewLocalKHR = 0x2,
	eDeviceGroupKHR = 0x4,
	eFeedbackLoopEXT = 0x8,
};
using DependencyFlags = Flags<DependencyFlagBits>;

enum class SubgroupFeatureFlagBits : uint32_t {
	eBasic = 0x1,
	eVote = 0x2,
	eArithmetic = 0x4,
	eBallot = 0x8,
	eShuffle = 0x10,
	eShuffleRelative = 0x20,
	eClustered = 0x40,
	eQuad = 0x80,
	eRotate = 0x200,
	eRotateClustered = 0x400,
	ePartitionedNV = 0x100,
	eRotateKHR = 0x200,
	eRotateClusteredKHR = 0x400,
};
using SubgroupFeatureFlags = Flags<SubgroupFeatureFlagBits>;

enum class IndirectCommandsLayoutUsageFlagBitsNV : uint32_t {
	eExplicitPreprocess = 0x1,
	eIndexedSequences = 0x2,
	eUnorderedSequences = 0x4,
};
using IndirectCommandsLayoutUsageFlagsNV = Flags<IndirectCommandsLayoutUsageFlagBitsNV>;

enum class IndirectStateFlagBitsNV : uint32_t {
	eFlagFrontface = 0x1,
};
using IndirectStateFlagsNV = Flags<IndirectStateFlagBitsNV>;

enum class GeometryFlagBitsKHR : uint32_t {
	eOpaque = 0x1,
	eNoDuplicateAnyHitInvocation = 0x2,
	eOpaqueNV = 0x1,
	eNoDuplicateAnyHitInvocationNV = 0x2,
};
using GeometryFlagsKHR = Flags<GeometryFlagBitsKHR>;

enum class GeometryInstanceFlagBitsKHR : uint32_t {
	eTriangleFacingCullDisable = 0x1,
	eTriangleFlipFacing = 0x2,
	eForceOpaque = 0x4,
	eForceNoOpaque = 0x8,
	eTriangleFrontCounterclockwise = 0x2,
	eTriangleCullDisableNV = 0x1,
	eTriangleFrontCounterclockwiseNV = 0x2,
	eForceOpaqueNV = 0x4,
	eForceNoOpaqueNV = 0x8,
	eForceOpacityMicromap2StateEXT = 0x10,
	eDisableOpacityMicromapsEXT = 0x20,
};
using GeometryInstanceFlagsKHR = Flags<GeometryInstanceFlagBitsKHR>;

enum class BuildAccelerationStructureFlagBitsKHR : uint32_t {
	eAllowUpdate = 0x1,
	eAllowCompaction = 0x2,
	ePreferFastTrace = 0x4,
	ePreferFastBuild = 0x8,
	eLowMemory = 0x10,
	eAllowUpdateNV = 0x1,
	eAllowCompactionNV = 0x2,
	ePreferFastTraceNV = 0x4,
	ePreferFastBuildNV = 0x8,
	eLowMemoryNV = 0x10,
	eMotionNV = 0x20,
	eAllowOpacityMicromapUpdateEXT = 0x40,
	eAllowDisableOpacityMicromapsEXT = 0x80,
	eAllowOpacityMicromapDataUpdateEXT = 0x100,
	eAllowDataAccess = 0x800,
#ifdef VK_ENABLE_BETA_EXTENSIONS
	eAllowDisplacementMicromapUpdateNV = 0x200,
#endif // VK_ENABLE_BETA_EXTENSIONS
};
using BuildAccelerationStructureFlagsKHR = Flags<BuildAccelerationStructureFlagBitsKHR>;

enum class PrivateDataSlotCreateFlagBits : uint32_t {
};
using PrivateDataSlotCreateFlags = Flags<PrivateDataSlotCreateFlagBits>;

enum class AccelerationStructureCreateFlagBitsKHR : uint32_t {
	eDeviceAddressCaptureReplay = 0x1,
	eDescriptorBufferCaptureReplayEXT = 0x8,
	eMotionNV = 0x4,
};
using AccelerationStructureCreateFlagsKHR = Flags<AccelerationStructureCreateFlagBitsKHR>;

enum class DescriptorUpdateTemplateCreateFlagBits : uint32_t {
};
using DescriptorUpdateTemplateCreateFlags = Flags<DescriptorUpdateTemplateCreateFlagBits>;

enum class PipelineCreationFeedbackFlagBits : uint32_t {
	eValid = 0x1,
	eApplicationPipelineCacheHit = 0x2,
	eBasePipelineAcceleration = 0x4,
	eValidEXT = 0x1,
	eApplicationPipelineCacheHitEXT = 0x2,
	eBasePipelineAccelerationEXT = 0x4,
};
using PipelineCreationFeedbackFlags = Flags<PipelineCreationFeedbackFlagBits>;

enum class PerformanceCounterDescriptionFlagBitsKHR : uint32_t {
	ePerformanceImpacting = 0x1,
	eConcurrentlyImpacted = 0x2,
};
using PerformanceCounterDescriptionFlagsKHR = Flags<PerformanceCounterDescriptionFlagBitsKHR>;

enum class AcquireProfilingLockFlagBitsKHR : uint32_t {
};
using AcquireProfilingLockFlagsKHR = Flags<AcquireProfilingLockFlagBitsKHR>;

enum class SemaphoreWaitFlagBits : uint32_t {
	eAny = 0x1,
	eAnyKHR = 0x1,
};
using SemaphoreWaitFlags = Flags<SemaphoreWaitFlagBits>;

enum class PipelineCompilerControlFlagBitsAMD : uint32_t {
};
using PipelineCompilerControlFlagsAMD = Flags<PipelineCompilerControlFlagBitsAMD>;

enum class ShaderCorePropertiesFlagBitsAMD : uint32_t {
};
using ShaderCorePropertiesFlagsAMD = Flags<ShaderCorePropertiesFlagBitsAMD>;

enum class DeviceDiagnosticsConfigFlagBitsNV : uint32_t {
	eEnableShaderDebugInfo = 0x1,
	eEnableResourceTracking = 0x2,
	eEnableAutomaticCheckpoints = 0x4,
	eEnableShaderErrorReporting = 0x8,
};
using DeviceDiagnosticsConfigFlagsNV = Flags<DeviceDiagnosticsConfigFlagBitsNV>;

enum class AccessFlagBits2 : uint64_t {
	eNone = 0,
	eIndirectCommandRead = 0x1ULL,
	eIndexRead = 0x2ULL,
	eVertexAttributeRead = 0x4ULL,
	eUniformRead = 0x8ULL,
	eInputAttachmentRead = 0x10ULL,
	eShaderRead = 0x20ULL,
	eShaderWrite = 0x40ULL,
	eColorAttachmentRead = 0x80ULL,
	eColorAttachmentWrite = 0x100ULL,
	eDepthStencilAttachmentRead = 0x200ULL,
	eDepthStencilAttachmentWrite = 0x400ULL,
	eTransferRead = 0x800ULL,
	eTransferWrite = 0x1000ULL,
	eHostRead = 0x2000ULL,
	eHostWrite = 0x4000ULL,
	eMemoryRead = 0x8000ULL,
	eMemoryWrite = 0x10000ULL,
	eShaderSampledRead = 0x00000000ULL,
	eShaderStorageRead = 0x00000000ULL,
	eShaderStorageWrite = 0x00000000ULL,
	eVideoDecodeReadKHR = 0x00000000ULL,
	eVideoDecodeWriteKHR = 0x00000000ULL,
	eVideoEncodeReadKHR = 0x00000000ULL,
	eVideoEncodeWriteKHR = 0x00000000ULL,
	eNoneKHR = 0,
	eIndirectCommandReadKHR = 0x1ULL,
	eIndexReadKHR = 0x2ULL,
	eVertexAttributeReadKHR = 0x4ULL,
	eUniformReadKHR = 0x8ULL,
	eInputAttachmentReadKHR = 0x10ULL,
	eShaderReadKHR = 0x20ULL,
	eShaderWriteKHR = 0x40ULL,
	eColorAttachmentReadKHR = 0x80ULL,
	eColorAttachmentWriteKHR = 0x100ULL,
	eDepthStencilAttachmentReadKHR = 0x200ULL,
	eDepthStencilAttachmentWriteKHR = 0x400ULL,
	eTransferReadKHR = 0x800ULL,
	eTransferWriteKHR = 0x1000ULL,
	eHostReadKHR = 0x2000ULL,
	eHostWriteKHR = 0x4000ULL,
	eMemoryReadKHR = 0x8000ULL,
	eMemoryWriteKHR = 0x10000ULL,
	eShaderSampledReadKHR = 0x00000000ULL,
	eShaderStorageReadKHR = 0x00000000ULL,
	eShaderStorageWriteKHR = 0x00000000ULL,
	eTransformFeedbackWriteEXT = 0x2000000ULL,
	eTransformFeedbackCounterReadEXT = 0x4000000ULL,
	eTransformFeedbackCounterWriteEXT = 0x8000000ULL,
	eConditionalRenderingReadEXT = 0x100000ULL,
	eCommandPreprocessReadNV = 0x20000ULL,
	eCommandPreprocessWriteNV = 0x40000ULL,
	eCommandPreprocessReadEXT = 0x20000ULL,
	eCommandPreprocessWriteEXT = 0x40000ULL,
	eFragmentShadingRateAttachmentReadKHR = 0x800000ULL,
	eShadingRateImageReadNV = 0x800000ULL,
	eAccelerationStructureReadKHR = 0x200000ULL,
	eAccelerationStructureWriteKHR = 0x400000ULL,
	eAccelerationStructureReadNV = 0x200000ULL,
	eAccelerationStructureWriteNV = 0x400000ULL,
	eFragmentDensityMapReadEXT = 0x1000000ULL,
	eColorAttachmentReadNoncoherentEXT = 0x80000ULL,
	eDescriptorBufferReadEXT = 0x00000000ULL,
	eInvocationMaskReadHUAWEI = 0x00000000ULL,
	eShaderBindingTableReadKHR = 0x00000000ULL,
	eMicromapReadEXT = 0x00000000ULL,
	eMicromapWriteEXT = 0x00000000ULL,
	eOpticalFlowReadNV = 0x00000000ULL,
	eOpticalFlowWriteNV = 0x00000000ULL,
};
using AccessFlags2 = Flags<AccessFlagBits2>;
using AccessFlags2KHR = AccessFlags2;

enum class PipelineStageFlagBits2 : uint64_t {
	eNone = 0,
	eTopOfPipe = 0x1ULL,
	eDrawIndirect = 0x2ULL,
	eVertexInput = 0x4ULL,
	eVertexShader = 0x8ULL,
	eTessellationControlShader = 0x10ULL,
	eTessellationEvaluationShader = 0x20ULL,
	eGeometryShader = 0x40ULL,
	eFragmentShader = 0x80ULL,
	eEarlyFragmentTests = 0x100ULL,
	eLateFragmentTests = 0x200ULL,
	eColorAttachmentOutput = 0x400ULL,
	eComputeShader = 0x800ULL,
	eAllTransfer = 0x1000ULL,
	eTransfer = 0x1000ULL,
	eBottomOfPipe = 0x2000ULL,
	eHost = 0x4000ULL,
	eAllGraphics = 0x8000ULL,
	eAllCommands = 0x10000ULL,
	eCopy = 0x00000000ULL,
	eResolve = 0x00000000ULL,
	eBlit = 0x00000000ULL,
	eClear = 0x00000000ULL,
	eIndexInput = 0x00000000ULL,
	eVertexAttributeInput = 0x00000000ULL,
	ePreRasterizationShaders = 0x00000000ULL,
	eVideoDecodeKHR = 0x4000000ULL,
	eVideoEncodeKHR = 0x8000000ULL,
	eNoneKHR = 0,
	eTopOfPipeKHR = 0x1ULL,
	eDrawIndirectKHR = 0x2ULL,
	eVertexInputKHR = 0x4ULL,
	eVertexShaderKHR = 0x8ULL,
	eTessellationControlShaderKHR = 0x10ULL,
	eTessellationEvaluationShaderKHR = 0x20ULL,
	eGeometryShaderKHR = 0x40ULL,
	eFragmentShaderKHR = 0x80ULL,
	eEarlyFragmentTestsKHR = 0x100ULL,
	eLateFragmentTestsKHR = 0x200ULL,
	eColorAttachmentOutputKHR = 0x400ULL,
	eComputeShaderKHR = 0x800ULL,
	eAllTransferKHR = 0x1000ULL,
	eTransferKHR = 0x1000ULL,
	eBottomOfPipeKHR = 0x2000ULL,
	eHostKHR = 0x4000ULL,
	eAllGraphicsKHR = 0x8000ULL,
	eAllCommandsKHR = 0x10000ULL,
	eCopyKHR = 0x00000000ULL,
	eResolveKHR = 0x00000000ULL,
	eBlitKHR = 0x00000000ULL,
	eClearKHR = 0x00000000ULL,
	eIndexInputKHR = 0x00000000ULL,
	eVertexAttributeInputKHR = 0x00000000ULL,
	ePreRasterizationShadersKHR = 0x00000000ULL,
	eTransformFeedbackEXT = 0x1000000ULL,
	eConditionalRenderingEXT = 0x40000ULL,
	eCommandPreprocessNV = 0x20000ULL,
	eCommandPreprocessEXT = 0x20000ULL,
	eFragmentShadingRateAttachmentKHR = 0x400000ULL,
	eShadingRateImageNV = 0x400000ULL,
	eAccelerationStructureBuildKHR = 0x2000000ULL,
	eRayTracingShaderKHR = 0x200000ULL,
	eRayTracingShaderNV = 0x200000ULL,
	eAccelerationStructureBuildNV = 0x2000000ULL,
	eFragmentDensityProcessEXT = 0x800000ULL,
	eTaskShaderNV = 0x80000ULL,
	eMeshShaderNV = 0x100000ULL,
	eTaskShaderEXT = 0x80000ULL,
	eMeshShaderEXT = 0x100000ULL,
	eSubpassShaderHUAWEI = 0x00000000ULL,
	eSubpassShadingHUAWEI = 0x00000000ULL,
	eInvocationMaskHUAWEI = 0x00000000ULL,
	eAccelerationStructureCopyKHR = 0x10000000ULL,
	eMicromapBuildEXT = 0x40000000ULL,
	eClusterCullingShaderHUAWEI = 0x00000000ULL,
	eOpticalFlowNV = 0x20000000ULL,
};
using PipelineStageFlags2 = Flags<PipelineStageFlagBits2>;
using PipelineStageFlags2KHR = PipelineStageFlags2;

enum class AccelerationStructureMotionInfoFlagBitsNV : uint32_t {
};
using AccelerationStructureMotionInfoFlagsNV = Flags<AccelerationStructureMotionInfoFlagBitsNV>;

enum class AccelerationStructureMotionInstanceFlagBitsNV : uint32_t {
};
using AccelerationStructureMotionInstanceFlagsNV = Flags<AccelerationStructureMotionInstanceFlagBitsNV>;

enum class FormatFeatureFlagBits2 : uint64_t {
	eSampledImage = 0x1ULL,
	eStorageImage = 0x2ULL,
	eStorageImageAtomic = 0x4ULL,
	eUniformTexelBuffer = 0x8ULL,
	eStorageTexelBuffer = 0x10ULL,
	eStorageTexelBufferAtomic = 0x20ULL,
	eVertexBuffer = 0x40ULL,
	eColorAttachment = 0x80ULL,
	eColorAttachmentBlend = 0x100ULL,
	eDepthStencilAttachment = 0x200ULL,
	eBlitSrc = 0x400ULL,
	eBlitDst = 0x800ULL,
	eSampledImageFilterLinear = 0x1000ULL,
	eTransferSrc = 0x4000ULL,
	eTransferDst = 0x8000ULL,
	eSampledImageFilterMinmax = 0x10000ULL,
	eMidpointChromaSamples = 0x20000ULL,
	eSampledImageYcbcrConversionLinearFilter = 0x40000ULL,
	eSampledImageYcbcrConversionSeparateReconstructionFilter = 0x80000ULL,
	eSampledImageYcbcrConversionChromaReconstructionExplicit = 0x100000ULL,
	eSampledImageYcbcrConversionChromaReconstructionExplicitForceable = 0x200000ULL,
	eDisjoint = 0x400000ULL,
	eCositedChromaSamples = 0x800000ULL,
	eStorageReadWithoutFormat = 0x80000000ULL,
	eStorageWriteWithoutFormat = 0x00000000ULL,
	eSampledImageDepthComparison = 0x00000000ULL,
	eSampledImageFilterCubic = 0x2000ULL,
	eHostImageTransfer = 0x00000000ULL,
	eVideoDecodeOutputKHR = 0x2000000ULL,
	eVideoDecodeDpbKHR = 0x4000000ULL,
	eAccelerationStructureVertexBufferKHR = 0x20000000ULL,
	eFragmentDensityMapEXT = 0x1000000ULL,
	eFragmentShadingRateAttachmentKHR = 0x40000000ULL,
	eHostImageTransferEXT = 0x00000000ULL,
	eVideoEncodeInputKHR = 0x8000000ULL,
	eVideoEncodeDpbKHR = 0x10000000ULL,
	eSampledImageKHR = 0x1ULL,
	eStorageImageKHR = 0x2ULL,
	eStorageImageAtomicKHR = 0x4ULL,
	eUniformTexelBufferKHR = 0x8ULL,
	eStorageTexelBufferKHR = 0x10ULL,
	eStorageTexelBufferAtomicKHR = 0x20ULL,
	eVertexBufferKHR = 0x40ULL,
	eColorAttachmentKHR = 0x80ULL,
	eColorAttachmentBlendKHR = 0x100ULL,
	eDepthStencilAttachmentKHR = 0x200ULL,
	eBlitSrcKHR = 0x400ULL,
	eBlitDstKHR = 0x800ULL,
	eSampledImageFilterLinearKHR = 0x1000ULL,
	eTransferSrcKHR = 0x4000ULL,
	eTransferDstKHR = 0x8000ULL,
	eMidpointChromaSamplesKHR = 0x20000ULL,
	eSampledImageYcbcrConversionLinearFilterKHR = 0x40000ULL,
	eSampledImageYcbcrConversionSeparateReconstructionFilterKHR = 0x80000ULL,
	eSampledImageYcbcrConversionChromaReconstructionExplicitKHR = 0x100000ULL,
	eSampledImageYcbcrConversionChromaReconstructionExplicitForceableKHR = 0x200000ULL,
	eDisjointKHR = 0x400000ULL,
	eCositedChromaSamplesKHR = 0x800000ULL,
	eStorageReadWithoutFormatKHR = 0x80000000ULL,
	eStorageWriteWithoutFormatKHR = 0x00000000ULL,
	eSampledImageDepthComparisonKHR = 0x00000000ULL,
	eSampledImageFilterMinmaxKHR = 0x10000ULL,
	eSampledImageFilterCubicEXT = 0x2000ULL,
	eLinearColorAttachmentNV = 0x00000000ULL,
	eWeightImageQCOM = 0x00000000ULL,
	eWeightSampledImageQCOM = 0x00000000ULL,
	eBlockMatchingQCOM = 0x00000000ULL,
	eBoxFilterSampledQCOM = 0x00000000ULL,
	eOpticalFlowImageNV = 0x00000000ULL,
	eOpticalFlowVectorNV = 0x00000000ULL,
	eOpticalFlowCostNV = 0x00000000ULL,
	eVideoEncodeQuantizationDeltaMapKHR = 0x00000000ULL,
	eVideoEncodeEmphasisMapKHR = 0x00000000ULL,
};
using FormatFeatureFlags2 = Flags<FormatFeatureFlagBits2>;
using FormatFeatureFlags2KHR = FormatFeatureFlags2;

enum class RenderingFlagBits : uint32_t {
	eContentsSecondaryCommandBuffers = 0x1,
	eSuspending = 0x2,
	eResuming = 0x4,
	eContentsSecondaryCommandBuffersKHR = 0x1,
	eSuspendingKHR = 0x2,
	eResumingKHR = 0x4,
	eContentsInlineEXT = 0x10,
	eEnableLegacyDitheringEXT = 0x8,
	eContentsInlineKHR = 0x10,
};
using RenderingFlags = Flags<RenderingFlagBits>;

enum class DirectDriverLoadingFlagBitsLUNARG : uint32_t {
};
using DirectDriverLoadingFlagsLUNARG = Flags<DirectDriverLoadingFlagBitsLUNARG>;

enum class PipelineCreateFlagBits2 : uint64_t {
	eDisableOptimization = 0x1ULL,
	eAllowDerivatives = 0x2ULL,
	eDerivative = 0x4ULL,
	eViewIndexFromDeviceIndex = 0x8ULL,
	eDispatchBase = 0x10ULL,
	eFailOnPipelineCompileRequired = 0x100ULL,
	eEarlyReturnOnFailure = 0x200ULL,
	eNoProtectedAccess = 0x8000000ULL,
	eProtectedAccessOnly = 0x40000000ULL,
	eEnableLegacyDitheringEXT = 0x00000000ULL,
	eDisableOptimizationKHR = 0x1ULL,
	eAllowDerivativesKHR = 0x2ULL,
	eDerivativeKHR = 0x4ULL,
	eViewIndexFromDeviceIndexKHR = 0x8ULL,
	eDispatchBaseKHR = 0x10ULL,
	eDeferCompileNV = 0x20ULL,
	eCaptureStatisticsKHR = 0x40ULL,
	eCaptureInternalRepresentationsKHR = 0x80ULL,
	eFailOnPipelineCompileRequiredKHR = 0x100ULL,
	eEarlyReturnOnFailureKHR = 0x200ULL,
	eLinkTimeOptimizationEXT = 0x400ULL,
	eRetainLinkTimeOptimizationInfoEXT = 0x800000ULL,
	eLibraryKHR = 0x800ULL,
	eRayTracingSkipTrianglesKHR = 0x1000ULL,
	eRayTracingSkipAabbsKHR = 0x2000ULL,
	eRayTracingNoNullAnyHitShadersKHR = 0x4000ULL,
	eRayTracingNoNullClosestHitShadersKHR = 0x8000ULL,
	eRayTracingNoNullMissShadersKHR = 0x10000ULL,
	eRayTracingNoNullIntersectionShadersKHR = 0x20000ULL,
	eRayTracingShaderGroupHandleCaptureReplayKHR = 0x80000ULL,
	eIndirectBindableNV = 0x40000ULL,
	eRayTracingAllowMotionNV = 0x100000ULL,
	eRenderingFragmentShadingRateAttachmentKHR = 0x200000ULL,
	eRenderingFragmentDensityMapAttachmentEXT = 0x400000ULL,
	eRayTracingOpacityMicromapEXT = 0x1000000ULL,
	eColorAttachmentFeedbackLoopEXT = 0x2000000ULL,
	eDepthStencilAttachmentFeedbackLoopEXT = 0x4000000ULL,
	eNoProtectedAccessEXT = 0x8000000ULL,
	eProtectedAccessOnlyEXT = 0x40000000ULL,
	eRayTracingDisplacementMicromapNV = 0x10000000ULL,
	eDescriptorBufferEXT = 0x20000000ULL,
	eCaptureDataKHR = 0x80000000ULL,
	eIndirectBindableEXT = 0x00000000ULL,
#ifdef VK_ENABLE_BETA_EXTENSIONS
	eExecutionGraphAMDX = 0x00000000ULL,
#endif // VK_ENABLE_BETA_EXTENSIONS
};
using PipelineCreateFlags2 = Flags<PipelineCreateFlagBits2>;
using PipelineCreateFlags2KHR = PipelineCreateFlags2;

enum class BufferUsageFlagBits2 : uint64_t {
	eTransferSrc = 0x1ULL,
	eTransferDst = 0x2ULL,
	eUniformTexelBuffer = 0x4ULL,
	eStorageTexelBuffer = 0x8ULL,
	eUniformBuffer = 0x10ULL,
	eStorageBuffer = 0x20ULL,
	eIndexBuffer = 0x40ULL,
	eVertexBuffer = 0x80ULL,
	eIndirectBuffer = 0x100ULL,
	eShaderDeviceAddress = 0x20000ULL,
	eTransferSrcKHR = 0x1ULL,
	eTransferDstKHR = 0x2ULL,
	eUniformTexelBufferKHR = 0x4ULL,
	eStorageTexelBufferKHR = 0x8ULL,
	eUniformBufferKHR = 0x10ULL,
	eStorageBufferKHR = 0x20ULL,
	eIndexBufferKHR = 0x40ULL,
	eVertexBufferKHR = 0x80ULL,
	eIndirectBufferKHR = 0x100ULL,
	eConditionalRenderingEXT = 0x200ULL,
	eShaderBindingTableKHR = 0x400ULL,
	eRayTracingNV = 0x400ULL,
	eTransformFeedbackBufferEXT = 0x800ULL,
	eTransformFeedbackCounterBufferEXT = 0x1000ULL,
	eVideoDecodeSrcKHR = 0x2000ULL,
	eVideoDecodeDstKHR = 0x4000ULL,
	eVideoEncodeDstKHR = 0x8000ULL,
	eVideoEncodeSrcKHR = 0x10000ULL,
	eShaderDeviceAddressKHR = 0x20000ULL,
	eAccelerationStructureBuildInputReadOnlyKHR = 0x80000ULL,
	eAccelerationStructureStorageKHR = 0x100000ULL,
	eSamplerDescriptorBufferEXT = 0x200000ULL,
	eResourceDescriptorBufferEXT = 0x400000ULL,
	ePushDescriptorsDescriptorBufferEXT = 0x4000000ULL,
	eMicromapBuildInputReadOnlyEXT = 0x800000ULL,
	eMicromapStorageEXT = 0x1000000ULL,
	ePreprocessBufferEXT = 0x80000000ULL,
#ifdef VK_ENABLE_BETA_EXTENSIONS
	eExecutionGraphScratchAMDX = 0x2000000ULL,
#endif // VK_ENABLE_BETA_EXTENSIONS
};
using BufferUsageFlags2 = Flags<BufferUsageFlagBits2>;
using BufferUsageFlags2KHR = BufferUsageFlags2;

enum class CompositeAlphaFlagBitsKHR : uint32_t {
	eOpaque = 0x1,
	ePreMultiplied = 0x2,
	ePostMultiplied = 0x4,
	eInherit = 0x8,
};
using CompositeAlphaFlagsKHR = Flags<CompositeAlphaFlagBitsKHR>;

enum class DisplayPlaneAlphaFlagBitsKHR : uint32_t {
	eOpaque = 0x1,
	eGlobal = 0x2,
	ePerPixel = 0x4,
	ePerPixelPremultiplied = 0x8,
};
using DisplayPlaneAlphaFlagsKHR = Flags<DisplayPlaneAlphaFlagBitsKHR>;

enum class SurfaceTransformFlagBitsKHR : uint32_t {
	eIdentity = 0x1,
	eRotate90 = 0x2,
	eRotate180 = 0x4,
	eRotate270 = 0x8,
	eHorizontalMirror = 0x10,
	eHorizontalMirrorRotate90 = 0x20,
	eHorizontalMirrorRotate180 = 0x40,
	eHorizontalMirrorRotate270 = 0x80,
	eInherit = 0x100,
};
using SurfaceTransformFlagsKHR = Flags<SurfaceTransformFlagBitsKHR>;

enum class SwapchainCreateFlagBitsKHR : uint32_t {
	eSplitInstanceBindRegions = 0x1,
	eProtected = 0x2,
	eMutableFormat = 0x4,
	eDeferredMemoryAllocationEXT = 0x8,
};
using SwapchainCreateFlagsKHR = Flags<SwapchainCreateFlagBitsKHR>;

enum class DisplayModeCreateFlagBitsKHR : uint32_t {
};
using DisplayModeCreateFlagsKHR = Flags<DisplayModeCreateFlagBitsKHR>;

enum class DisplaySurfaceCreateFlagBitsKHR : uint32_t {
};
using DisplaySurfaceCreateFlagsKHR = Flags<DisplaySurfaceCreateFlagBitsKHR>;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
enum class AndroidSurfaceCreateFlagBitsKHR : uint32_t {
};
using AndroidSurfaceCreateFlagsKHR = Flags<AndroidSurfaceCreateFlagBitsKHR>;
#endif // VK_USE_PLATFORM_ANDROID_KHR

#if defined(VK_USE_PLATFORM_VI_NN)
enum class ViSurfaceCreateFlagBitsNN : uint32_t {
};
using ViSurfaceCreateFlagsNN = Flags<ViSurfaceCreateFlagBitsNN>;
#endif // VK_USE_PLATFORM_VI_NN

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
enum class WaylandSurfaceCreateFlagBitsKHR : uint32_t {
};
using WaylandSurfaceCreateFlagsKHR = Flags<WaylandSurfaceCreateFlagBitsKHR>;
#endif // VK_USE_PLATFORM_WAYLAND_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
enum class Win32SurfaceCreateFlagBitsKHR : uint32_t {
};
using Win32SurfaceCreateFlagsKHR = Flags<Win32SurfaceCreateFlagBitsKHR>;
#endif // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_XLIB_KHR)
enum class XlibSurfaceCreateFlagBitsKHR : uint32_t {
};
using XlibSurfaceCreateFlagsKHR = Flags<XlibSurfaceCreateFlagBitsKHR>;
#endif // VK_USE_PLATFORM_XLIB_KHR

#if defined(VK_USE_PLATFORM_XCB_KHR)
enum class XcbSurfaceCreateFlagBitsKHR : uint32_t {
};
using XcbSurfaceCreateFlagsKHR = Flags<XcbSurfaceCreateFlagBitsKHR>;
#endif // VK_USE_PLATFORM_XCB_KHR

#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
enum class DirectFBSurfaceCreateFlagBitsEXT : uint32_t {
};
using DirectFBSurfaceCreateFlagsEXT = Flags<DirectFBSurfaceCreateFlagBitsEXT>;
#endif // VK_USE_PLATFORM_DIRECTFB_EXT

#if defined(VK_USE_PLATFORM_IOS_MVK)
enum class IOSSurfaceCreateFlagBitsMVK : uint32_t {
};
using IOSSurfaceCreateFlagsMVK = Flags<IOSSurfaceCreateFlagBitsMVK>;
#endif // VK_USE_PLATFORM_IOS_MVK

#if defined(VK_USE_PLATFORM_MACOS_MVK)
enum class MacOSSurfaceCreateFlagBitsMVK : uint32_t {
};
using MacOSSurfaceCreateFlagsMVK = Flags<MacOSSurfaceCreateFlagBitsMVK>;
#endif // VK_USE_PLATFORM_MACOS_MVK

#if defined(VK_USE_PLATFORM_METAL_EXT)
enum class MetalSurfaceCreateFlagBitsEXT : uint32_t {
};
using MetalSurfaceCreateFlagsEXT = Flags<MetalSurfaceCreateFlagBitsEXT>;
#endif // VK_USE_PLATFORM_METAL_EXT

#if defined(VK_USE_PLATFORM_FUCHSIA)
enum class ImagePipeSurfaceCreateFlagBitsFUCHSIA : uint32_t {
};
using ImagePipeSurfaceCreateFlagsFUCHSIA = Flags<ImagePipeSurfaceCreateFlagBitsFUCHSIA>;
#endif // VK_USE_PLATFORM_FUCHSIA

#if defined(VK_USE_PLATFORM_GGP)
enum class StreamDescriptorSurfaceCreateFlagBitsGGP : uint32_t {
};
using StreamDescriptorSurfaceCreateFlagsGGP = Flags<StreamDescriptorSurfaceCreateFlagBitsGGP>;
#endif // VK_USE_PLATFORM_GGP

enum class HeadlessSurfaceCreateFlagBitsEXT : uint32_t {
};
using HeadlessSurfaceCreateFlagsEXT = Flags<HeadlessSurfaceCreateFlagBitsEXT>;

#if defined(VK_USE_PLATFORM_SCREEN_QNX)
enum class ScreenSurfaceCreateFlagBitsQNX : uint32_t {
};
using ScreenSurfaceCreateFlagsQNX = Flags<ScreenSurfaceCreateFlagBitsQNX>;
#endif // VK_USE_PLATFORM_SCREEN_QNX

enum class PeerMemoryFeatureFlagBits : uint32_t {
	eCopySrc = 0x1,
	eCopyDst = 0x2,
	eGenericSrc = 0x4,
	eGenericDst = 0x8,
	eCopySrcKHR = 0x1,
	eCopyDstKHR = 0x2,
	eGenericSrcKHR = 0x4,
	eGenericDstKHR = 0x8,
};
using PeerMemoryFeatureFlags = Flags<PeerMemoryFeatureFlagBits>;
using PeerMemoryFeatureFlagsKHR = PeerMemoryFeatureFlags;

enum class MemoryAllocateFlagBits : uint32_t {
	eDeviceMask = 0x1,
	eDeviceAddress = 0x2,
	eDeviceAddressCaptureReplay = 0x4,
	eDeviceMaskKHR = 0x1,
	eDeviceAddressKHR = 0x2,
	eDeviceAddressCaptureReplayKHR = 0x4,
};
using MemoryAllocateFlags = Flags<MemoryAllocateFlagBits>;
using MemoryAllocateFlagsKHR = MemoryAllocateFlags;

enum class DeviceGroupPresentModeFlagBitsKHR : uint32_t {
	eLocal = 0x1,
	eRemote = 0x2,
	eSum = 0x4,
	eLocalMultiDevice = 0x8,
};
using DeviceGroupPresentModeFlagsKHR = Flags<DeviceGroupPresentModeFlagBitsKHR>;

enum class DebugReportFlagBitsEXT : uint32_t {
	eInformation = 0x1,
	eWarning = 0x2,
	ePerformanceWarning = 0x4,
	eError = 0x8,
	eDebug = 0x10,
};
using DebugReportFlagsEXT = Flags<DebugReportFlagBitsEXT>;

enum class CommandPoolTrimFlagBits : uint32_t {
};
using CommandPoolTrimFlags = Flags<CommandPoolTrimFlagBits>;
using CommandPoolTrimFlagsKHR = CommandPoolTrimFlags;

enum class ExternalMemoryHandleTypeFlagBits : uint32_t {
	eOpaqueFd = 0x1,
	eOpaqueWin32 = 0x2,
	eOpaqueWin32Kmt = 0x4,
	eD3D11Texture = 0x8,
	eD3D11TextureKmt = 0x10,
	eD3D12Heap = 0x20,
	eD3D12Resource = 0x40,
	eOpaqueFdKHR = 0x1,
	eOpaqueWin32KHR = 0x2,
	eOpaqueWin32KmtKHR = 0x4,
	eD3D11TextureKHR = 0x8,
	eD3D11TextureKmtKHR = 0x10,
	eD3D12HeapKHR = 0x20,
	eD3D12ResourceKHR = 0x40,
	eDmaBufEXT = 0x200,
	eHostAllocationEXT = 0x80,
	eHostMappedForeignMemoryEXT = 0x100,
	eRdmaAddressNV = 0x1000,
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	eAndroidHardwareBufferANDROID = 0x400,
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_FUCHSIA
	eZirconVmoFUCHSIA = 0x800,
#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_SCREEN_QNX
	eScreenBufferQNX = 0x4000,
#endif // VK_USE_PLATFORM_SCREEN_QNX
};
using ExternalMemoryHandleTypeFlags = Flags<ExternalMemoryHandleTypeFlagBits>;
using ExternalMemoryHandleTypeFlagsKHR = ExternalMemoryHandleTypeFlags;

enum class ExternalMemoryFeatureFlagBits : uint32_t {
	eDedicatedOnly = 0x1,
	eExportable = 0x2,
	eImportable = 0x4,
	eDedicatedOnlyKHR = 0x1,
	eExportableKHR = 0x2,
	eImportableKHR = 0x4,
};
using ExternalMemoryFeatureFlags = Flags<ExternalMemoryFeatureFlagBits>;
using ExternalMemoryFeatureFlagsKHR = ExternalMemoryFeatureFlags;

enum class ExternalSemaphoreHandleTypeFlagBits : uint32_t {
	eOpaqueFd = 0x1,
	eOpaqueWin32 = 0x2,
	eOpaqueWin32Kmt = 0x4,
	eD3D12Fence = 0x8,
	eD3D11Fence = 0x8,
	eSyncFd = 0x10,
	eOpaqueFdKHR = 0x1,
	eOpaqueWin32KHR = 0x2,
	eOpaqueWin32KmtKHR = 0x4,
	eD3D12FenceKHR = 0x8,
	eSyncFdKHR = 0x10,
#ifdef VK_USE_PLATFORM_FUCHSIA
	eZirconEventFUCHSIA = 0x80,
#endif // VK_USE_PLATFORM_FUCHSIA
};
using ExternalSemaphoreHandleTypeFlags = Flags<ExternalSemaphoreHandleTypeFlagBits>;
using ExternalSemaphoreHandleTypeFlagsKHR = ExternalSemaphoreHandleTypeFlags;

enum class ExternalSemaphoreFeatureFlagBits : uint32_t {
	eExportable = 0x1,
	eImportable = 0x2,
	eExportableKHR = 0x1,
	eImportableKHR = 0x2,
};
using ExternalSemaphoreFeatureFlags = Flags<ExternalSemaphoreFeatureFlagBits>;
using ExternalSemaphoreFeatureFlagsKHR = ExternalSemaphoreFeatureFlags;

enum class SemaphoreImportFlagBits : uint32_t {
	eTemporary = 0x1,
	eTemporaryKHR = 0x1,
};
using SemaphoreImportFlags = Flags<SemaphoreImportFlagBits>;
using SemaphoreImportFlagsKHR = SemaphoreImportFlags;

enum class ExternalFenceHandleTypeFlagBits : uint32_t {
	eOpaqueFd = 0x1,
	eOpaqueWin32 = 0x2,
	eOpaqueWin32Kmt = 0x4,
	eSyncFd = 0x8,
	eOpaqueFdKHR = 0x1,
	eOpaqueWin32KHR = 0x2,
	eOpaqueWin32KmtKHR = 0x4,
	eSyncFdKHR = 0x8,
};
using ExternalFenceHandleTypeFlags = Flags<ExternalFenceHandleTypeFlagBits>;
using ExternalFenceHandleTypeFlagsKHR = ExternalFenceHandleTypeFlags;

enum class ExternalFenceFeatureFlagBits : uint32_t {
	eExportable = 0x1,
	eImportable = 0x2,
	eExportableKHR = 0x1,
	eImportableKHR = 0x2,
};
using ExternalFenceFeatureFlags = Flags<ExternalFenceFeatureFlagBits>;
using ExternalFenceFeatureFlagsKHR = ExternalFenceFeatureFlags;

enum class FenceImportFlagBits : uint32_t {
	eTemporary = 0x1,
	eTemporaryKHR = 0x1,
};
using FenceImportFlags = Flags<FenceImportFlagBits>;
using FenceImportFlagsKHR = FenceImportFlags;

enum class SurfaceCounterFlagBitsEXT : uint32_t {
	eVblank = 0x1,
};
using SurfaceCounterFlagsEXT = Flags<SurfaceCounterFlagBitsEXT>;

enum class DebugUtilsMessageTypeFlagBitsEXT : uint32_t {
	eGeneral = 0x1,
	eValidation = 0x2,
	ePerformance = 0x4,
	eDeviceAddressBinding = 0x8,
};
using DebugUtilsMessageTypeFlagsEXT = Flags<DebugUtilsMessageTypeFlagBitsEXT>;

enum class DebugUtilsMessengerCreateFlagBitsEXT : uint32_t {
};
using DebugUtilsMessengerCreateFlagsEXT = Flags<DebugUtilsMessengerCreateFlagBitsEXT>;

enum class DebugUtilsMessengerCallbackDataFlagBitsEXT : uint32_t {
};
using DebugUtilsMessengerCallbackDataFlagsEXT = Flags<DebugUtilsMessengerCallbackDataFlagBitsEXT>;

enum class DeviceMemoryReportFlagBitsEXT : uint32_t {
};
using DeviceMemoryReportFlagsEXT = Flags<DeviceMemoryReportFlagBitsEXT>;

enum class PipelineRasterizationConservativeStateCreateFlagBitsEXT : uint32_t {
};
using PipelineRasterizationConservativeStateCreateFlagsEXT = Flags<PipelineRasterizationConservativeStateCreateFlagBitsEXT>;

enum class DescriptorBindingFlagBits : uint32_t {
	eUpdateAfterBind = 0x1,
	eUpdateUnusedWhilePending = 0x2,
	ePartiallyBound = 0x4,
	eVariableDescriptorCount = 0x8,
	eUpdateAfterBindEXT = 0x1,
	eUpdateUnusedWhilePendingEXT = 0x2,
	ePartiallyBoundEXT = 0x4,
	eVariableDescriptorCountEXT = 0x8,
};
using DescriptorBindingFlags = Flags<DescriptorBindingFlagBits>;
using DescriptorBindingFlagsEXT = DescriptorBindingFlags;

enum class ConditionalRenderingFlagBitsEXT : uint32_t {
	eInverted = 0x1,
};
using ConditionalRenderingFlagsEXT = Flags<ConditionalRenderingFlagBitsEXT>;

enum class ResolveModeFlagBits : uint32_t {
	eNone = 0,
	eSampleZero = 0x1,
	eAverage = 0x2,
	eMin = 0x4,
	eMax = 0x8,
	eNoneKHR = 0,
	eSampleZeroKHR = 0x1,
	eAverageKHR = 0x2,
	eMinKHR = 0x4,
	eMaxKHR = 0x8,
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	eExternalFormatDownsampleANDROID = 0x10,
#endif // VK_USE_PLATFORM_ANDROID_KHR
};
using ResolveModeFlags = Flags<ResolveModeFlagBits>;
using ResolveModeFlagsKHR = ResolveModeFlags;

enum class PipelineRasterizationStateStreamCreateFlagBitsEXT : uint32_t {
};
using PipelineRasterizationStateStreamCreateFlagsEXT = Flags<PipelineRasterizationStateStreamCreateFlagBitsEXT>;

enum class PipelineRasterizationDepthClipStateCreateFlagBitsEXT : uint32_t {
};
using PipelineRasterizationDepthClipStateCreateFlagsEXT = Flags<PipelineRasterizationDepthClipStateCreateFlagBitsEXT>;

enum class ToolPurposeFlagBits : uint32_t {
	eValidation = 0x1,
	eProfiling = 0x2,
	eTracing = 0x4,
	eAdditionalFeatures = 0x8,
	eModifyingFeatures = 0x10,
	eValidationEXT = 0x1,
	eProfilingEXT = 0x2,
	eTracingEXT = 0x4,
	eAdditionalFeaturesEXT = 0x8,
	eModifyingFeaturesEXT = 0x10,
	eDebugReportingEXT = 0x20,
	eDebugMarkersEXT = 0x40,
};
using ToolPurposeFlags = Flags<ToolPurposeFlagBits>;
using ToolPurposeFlagsEXT = ToolPurposeFlags;

enum class SubmitFlagBits : uint32_t {
	eProtected = 0x1,
	eProtectedKHR = 0x1,
};
using SubmitFlags = Flags<SubmitFlagBits>;
using SubmitFlagsKHR = SubmitFlags;

#if defined(VK_USE_PLATFORM_FUCHSIA)
enum class ImageFormatConstraintsFlagBitsFUCHSIA : uint32_t {
};
using ImageFormatConstraintsFlagsFUCHSIA = Flags<ImageFormatConstraintsFlagBitsFUCHSIA>;
#endif // VK_USE_PLATFORM_FUCHSIA

enum class HostImageCopyFlagBits : uint32_t {
	eMemcpy = 0x1,
	eMemcpyEXT = 0x1,
};
using HostImageCopyFlags = Flags<HostImageCopyFlagBits>;
using HostImageCopyFlagsEXT = HostImageCopyFlags;

#if defined(VK_USE_PLATFORM_FUCHSIA)
enum class ImageConstraintsInfoFlagBitsFUCHSIA : uint32_t {
	eCpuReadRarely = 0x1,
	eCpuReadOften = 0x2,
	eCpuWriteRarely = 0x4,
	eCpuWriteOften = 0x8,
	eProtectedOptional = 0x10,
};
using ImageConstraintsInfoFlagsFUCHSIA = Flags<ImageConstraintsInfoFlagBitsFUCHSIA>;
#endif // VK_USE_PLATFORM_FUCHSIA

enum class GraphicsPipelineLibraryFlagBitsEXT : uint32_t {
	eVertexInputInterface = 0x1,
	ePreRasterizationShaders = 0x2,
	eFragmentShader = 0x4,
	eFragmentOutputInterface = 0x8,
};
using GraphicsPipelineLibraryFlagsEXT = Flags<GraphicsPipelineLibraryFlagBitsEXT>;

enum class ImageCompressionFlagBitsEXT : uint32_t {
	eDefault = 0,
	eFixedRateDefault = 0x1,
	eFixedRateExplicit = 0x2,
	eDisabled = 0x4,
};
using ImageCompressionFlagsEXT = Flags<ImageCompressionFlagBitsEXT>;

enum class ImageCompressionFixedRateFlagBitsEXT : uint32_t {
	eNone = 0,
	e1bpc = 0x1,
	e2bpc = 0x2,
	e3bpc = 0x4,
	e4bpc = 0x8,
	e5bpc = 0x10,
	e6bpc = 0x20,
	e7bpc = 0x40,
	e8bpc = 0x80,
	e9bpc = 0x100,
	e10bpc = 0x200,
	e11bpc = 0x400,
	e12bpc = 0x800,
	e13bpc = 0x1000,
	e14bpc = 0x2000,
	e15bpc = 0x4000,
	e16bpc = 0x8000,
	e17bpc = 0x10000,
	e18bpc = 0x20000,
	e19bpc = 0x40000,
	e20bpc = 0x80000,
	e21bpc = 0x100000,
	e22bpc = 0x200000,
	e23bpc = 0x400000,
	e24bpc = 0x800000,
};
using ImageCompressionFixedRateFlagsEXT = Flags<ImageCompressionFixedRateFlagBitsEXT>;

#if defined(VK_USE_PLATFORM_METAL_EXT)
enum class ExportMetalObjectTypeFlagBitsEXT : uint32_t {
	eMetalDevice = 0x1,
	eMetalCommandQueue = 0x2,
	eMetalBuffer = 0x4,
	eMetalTexture = 0x8,
	eMetalIosurface = 0x10,
	eMetalSharedEvent = 0x20,
};
using ExportMetalObjectTypeFlagsEXT = Flags<ExportMetalObjectTypeFlagBitsEXT>;
#endif // VK_USE_PLATFORM_METAL_EXT

enum class DeviceAddressBindingFlagBitsEXT : uint32_t {
	eInternalObject = 0x1,
};
using DeviceAddressBindingFlagsEXT = Flags<DeviceAddressBindingFlagBitsEXT>;

enum class VideoCodecOperationFlagBitsKHR : uint32_t {
	eNone = 0,
	eEncodeH264 = 0x10000,
	eEncodeH265 = 0x20000,
	eDecodeH264 = 0x1,
	eDecodeH265 = 0x2,
	eDecodeAV1 = 0x4,
	eEncodeAV1 = 0x40000,
};
using VideoCodecOperationFlagsKHR = Flags<VideoCodecOperationFlagBitsKHR>;

enum class VideoCapabilityFlagBitsKHR : uint32_t {
	eProtectedContent = 0x1,
	eSeparateReferenceImages = 0x2,
};
using VideoCapabilityFlagsKHR = Flags<VideoCapabilityFlagBitsKHR>;

enum class VideoSessionCreateFlagBitsKHR : uint32_t {
	eProtectedContent = 0x1,
	eAllowEncodeParameterOptimizations = 0x2,
	eInlineQueries = 0x4,
	eAllowEncodeQuantizationDeltaMap = 0x8,
	eAllowEncodeEmphasisMap = 0x10,
};
using VideoSessionCreateFlagsKHR = Flags<VideoSessionCreateFlagBitsKHR>;

enum class VideoSessionParametersCreateFlagBitsKHR : uint32_t {
	eQuantizationMapCompatible = 0x1,
};
using VideoSessionParametersCreateFlagsKHR = Flags<VideoSessionParametersCreateFlagBitsKHR>;

enum class VideoBeginCodingFlagBitsKHR : uint32_t {
};
using VideoBeginCodingFlagsKHR = Flags<VideoBeginCodingFlagBitsKHR>;

enum class VideoEndCodingFlagBitsKHR : uint32_t {
};
using VideoEndCodingFlagsKHR = Flags<VideoEndCodingFlagBitsKHR>;

enum class VideoCodingControlFlagBitsKHR : uint32_t {
	eReset = 0x1,
	eEncodeRateControl = 0x2,
	eEncodeQualityLevel = 0x4,
};
using VideoCodingControlFlagsKHR = Flags<VideoCodingControlFlagBitsKHR>;

enum class VideoDecodeUsageFlagBitsKHR : uint32_t {
	eDefault = 0,
	eTranscoding = 0x1,
	eOffline = 0x2,
	eStreaming = 0x4,
};
using VideoDecodeUsageFlagsKHR = Flags<VideoDecodeUsageFlagBitsKHR>;

enum class VideoDecodeCapabilityFlagBitsKHR : uint32_t {
	eDpbAndOutputCoincide = 0x1,
	eDpbAndOutputDistinct = 0x2,
};
using VideoDecodeCapabilityFlagsKHR = Flags<VideoDecodeCapabilityFlagBitsKHR>;

enum class VideoDecodeFlagBitsKHR : uint32_t {
};
using VideoDecodeFlagsKHR = Flags<VideoDecodeFlagBitsKHR>;

enum class VideoDecodeH264PictureLayoutFlagBitsKHR : uint32_t {
	eProgressive = 0,
	eInterlacedInterleavedLines = 0x1,
	eInterlacedSeparatePlanes = 0x2,
};
using VideoDecodeH264PictureLayoutFlagsKHR = Flags<VideoDecodeH264PictureLayoutFlagBitsKHR>;

enum class VideoEncodeFlagBitsKHR : uint32_t {
	eWithQuantizationDeltaMap = 0x1,
	eWithEmphasisMap = 0x2,
};
using VideoEncodeFlagsKHR = Flags<VideoEncodeFlagBitsKHR>;

enum class VideoEncodeUsageFlagBitsKHR : uint32_t {
	eDefault = 0,
	eTranscoding = 0x1,
	eStreaming = 0x2,
	eRecording = 0x4,
	eConferencing = 0x8,
};
using VideoEncodeUsageFlagsKHR = Flags<VideoEncodeUsageFlagBitsKHR>;

enum class VideoEncodeContentFlagBitsKHR : uint32_t {
	eDefault = 0,
	eCamera = 0x1,
	eDesktop = 0x2,
	eRendered = 0x4,
};
using VideoEncodeContentFlagsKHR = Flags<VideoEncodeContentFlagBitsKHR>;

enum class VideoEncodeCapabilityFlagBitsKHR : uint32_t {
	ePrecedingExternallyEncodedBytes = 0x1,
	eInsufficientBitstreamBufferRangeDetection = 0x2,
	eQuantizationDeltaMap = 0x4,
	eEmphasisMap = 0x8,
};
using VideoEncodeCapabilityFlagsKHR = Flags<VideoEncodeCapabilityFlagBitsKHR>;

enum class VideoEncodeFeedbackFlagBitsKHR : uint32_t {
	eBitstreamBufferOffset = 0x1,
	eBitstreamBytesWritten = 0x2,
	eBitstreamHasOverrides = 0x4,
};
using VideoEncodeFeedbackFlagsKHR = Flags<VideoEncodeFeedbackFlagBitsKHR>;

enum class VideoEncodeRateControlFlagBitsKHR : uint32_t {
};
using VideoEncodeRateControlFlagsKHR = Flags<VideoEncodeRateControlFlagBitsKHR>;

enum class VideoEncodeRateControlModeFlagBitsKHR : uint32_t {
	eDefault = 0,
	eDisabled = 0x1,
	eCbr = 0x2,
	eVbr = 0x4,
};
using VideoEncodeRateControlModeFlagsKHR = Flags<VideoEncodeRateControlModeFlagBitsKHR>;

enum class VideoChromaSubsamplingFlagBitsKHR : uint32_t {
	eInvalid = 0,
	eMonochrome = 0x1,
	e420 = 0x2,
	e422 = 0x4,
	e444 = 0x8,
};
using VideoChromaSubsamplingFlagsKHR = Flags<VideoChromaSubsamplingFlagBitsKHR>;

enum class VideoComponentBitDepthFlagBitsKHR : uint32_t {
	eInvalid = 0,
	e8 = 0x1,
	e10 = 0x4,
	e12 = 0x10,
};
using VideoComponentBitDepthFlagsKHR = Flags<VideoComponentBitDepthFlagBitsKHR>;

enum class VideoEncodeH264CapabilityFlagBitsKHR : uint32_t {
	eHrdCompliance = 0x1,
	ePredictionWeightTableGenerated = 0x2,
	eRowUnalignedSlice = 0x4,
	eDifferentSliceType = 0x8,
	eBFrameInL0List = 0x10,
	eBFrameInL1List = 0x20,
	ePerPictureTypeMinMaxQp = 0x40,
	ePerSliceConstantQp = 0x80,
	eGeneratePrefixNalu = 0x100,
	eMbQpDiffWraparound = 0x200,
};
using VideoEncodeH264CapabilityFlagsKHR = Flags<VideoEncodeH264CapabilityFlagBitsKHR>;

enum class VideoEncodeH264StdFlagBitsKHR : uint32_t {
	eSeparateColorPlaneFlagSet = 0x1,
	eQpprimeYZeroTransformBypassFlagSet = 0x2,
	eScalingMatrixPresentFlagSet = 0x4,
	eChromaQpIndexOffset = 0x8,
	eSecondChromaQpIndexOffset = 0x10,
	ePicInitQpMinus26 = 0x20,
	eWeightedPredFlagSet = 0x40,
	eWeightedBipredIdcExplicit = 0x80,
	eWeightedBipredIdcImplicit = 0x100,
	eTransform8x8ModeFlagSet = 0x200,
	eDirectSpatialMvPredFlagUnset = 0x400,
	eEntropyCodingModeFlagUnset = 0x800,
	eEntropyCodingModeFlagSet = 0x1000,
	eDirect8x8InferenceFlagUnset = 0x2000,
	eConstrainedIntraPredFlagSet = 0x4000,
	eDeblockingFilterDisabled = 0x8000,
	eDeblockingFilterEnabled = 0x10000,
	eDeblockingFilterPartial = 0x20000,
	eSliceQpDelta = 0x80000,
	eDifferentSliceQpDelta = 0x100000,
};
using VideoEncodeH264StdFlagsKHR = Flags<VideoEncodeH264StdFlagBitsKHR>;

enum class VideoEncodeH264RateControlFlagBitsKHR : uint32_t {
	eAttemptHrdCompliance = 0x1,
	eRegularGop = 0x2,
	eReferencePatternFlat = 0x4,
	eReferencePatternDyadic = 0x8,
	eTemporalLayerPatternDyadic = 0x10,
};
using VideoEncodeH264RateControlFlagsKHR = Flags<VideoEncodeH264RateControlFlagBitsKHR>;

enum class VideoEncodeH265CapabilityFlagBitsKHR : uint32_t {
	eHrdCompliance = 0x1,
	ePredictionWeightTableGenerated = 0x2,
	eRowUnalignedSliceSegment = 0x4,
	eDifferentSliceSegmentType = 0x8,
	eBFrameInL0List = 0x10,
	eBFrameInL1List = 0x20,
	ePerPictureTypeMinMaxQp = 0x40,
	ePerSliceSegmentConstantQp = 0x80,
	eMultipleTilesPerSliceSegment = 0x100,
	eMultipleSliceSegmentsPerTile = 0x200,
	eCuQpDiffWraparound = 0x400,
};
using VideoEncodeH265CapabilityFlagsKHR = Flags<VideoEncodeH265CapabilityFlagBitsKHR>;

enum class VideoEncodeH265StdFlagBitsKHR : uint32_t {
	eSeparateColorPlaneFlagSet = 0x1,
	eSampleAdaptiveOffsetEnabledFlagSet = 0x2,
	eScalingListDataPresentFlagSet = 0x4,
	ePcmEnabledFlagSet = 0x8,
	eSpsTemporalMvpEnabledFlagSet = 0x10,
	eInitQpMinus26 = 0x20,
	eWeightedPredFlagSet = 0x40,
	eWeightedBipredFlagSet = 0x80,
	eLoG2ParallelMergeLevelMinus2 = 0x100,
	eSignDataHidingEnabledFlagSet = 0x200,
	eTransformSkipEnabledFlagSet = 0x400,
	eTransformSkipEnabledFlagUnset = 0x800,
	ePpsSliceChromaQpOffsetsPresentFlagSet = 0x1000,
	eTransquantBypassEnabledFlagSet = 0x2000,
	eConstrainedIntraPredFlagSet = 0x4000,
	eEntropyCodingSyncEnabledFlagSet = 0x8000,
	eDeblockingFilterOverrideEnabledFlagSet = 0x10000,
	eDependentSliceSegmentsEnabledFlagSet = 0x20000,
	eDependentSliceSegmentFlagSet = 0x40000,
	eSliceQpDelta = 0x80000,
	eDifferentSliceQpDelta = 0x100000,
};
using VideoEncodeH265StdFlagsKHR = Flags<VideoEncodeH265StdFlagBitsKHR>;

enum class VideoEncodeH265RateControlFlagBitsKHR : uint32_t {
	eAttemptHrdCompliance = 0x1,
	eRegularGop = 0x2,
	eReferencePatternFlat = 0x4,
	eReferencePatternDyadic = 0x8,
	eTemporalSubLayerPatternDyadic = 0x10,
};
using VideoEncodeH265RateControlFlagsKHR = Flags<VideoEncodeH265RateControlFlagBitsKHR>;

enum class VideoEncodeH265CtbSizeFlagBitsKHR : uint32_t {
	e16 = 0x1,
	e32 = 0x2,
	e64 = 0x4,
};
using VideoEncodeH265CtbSizeFlagsKHR = Flags<VideoEncodeH265CtbSizeFlagBitsKHR>;

enum class VideoEncodeH265TransformBlockSizeFlagBitsKHR : uint32_t {
	e4 = 0x1,
	e8 = 0x2,
	e16 = 0x4,
	e32 = 0x8,
};
using VideoEncodeH265TransformBlockSizeFlagsKHR = Flags<VideoEncodeH265TransformBlockSizeFlagBitsKHR>;

enum class VideoEncodeAV1CapabilityFlagBitsKHR : uint32_t {
	ePerRateControlGroupMinMaxQIndex = 0x1,
	eGenerateObuExtensionHeader = 0x2,
	ePrimaryReferenceCdfOnly = 0x4,
	eFrameSizeOverride = 0x8,
	eMotionVectorScaling = 0x10,
};
using VideoEncodeAV1CapabilityFlagsKHR = Flags<VideoEncodeAV1CapabilityFlagBitsKHR>;

enum class VideoEncodeAV1StdFlagBitsKHR : uint32_t {
	eUniformTileSpacingFlagSet = 0x1,
	eSkipModePresentUnset = 0x2,
	ePrimaryRefFrame = 0x4,
	eDeltaQ = 0x8,
};
using VideoEncodeAV1StdFlagsKHR = Flags<VideoEncodeAV1StdFlagBitsKHR>;

enum class VideoEncodeAV1RateControlFlagBitsKHR : uint32_t {
	eRegularGop = 0x1,
	eTemporalLayerPatternDyadic = 0x2,
	eReferencePatternFlat = 0x4,
	eReferencePatternDyadic = 0x8,
};
using VideoEncodeAV1RateControlFlagsKHR = Flags<VideoEncodeAV1RateControlFlagBitsKHR>;

enum class VideoEncodeAV1SuperblockSizeFlagBitsKHR : uint32_t {
	e64 = 0x1,
	e128 = 0x2,
};
using VideoEncodeAV1SuperblockSizeFlagsKHR = Flags<VideoEncodeAV1SuperblockSizeFlagBitsKHR>;

enum class TimeDomainKHR {
	eDevice = 0,
	eClockMonotonic = 1,
	eClockMonotonicRaw = 2,
	eQueryPerformanceCounter = 3,
	eDeviceEXT = 0,
	eClockMonotonicEXT = 1,
	eClockMonotonicRawEXT = 2,
	eQueryPerformanceCounterEXT = 3,
};
using TimeDomainEXT = TimeDomainKHR;

enum class BuildAccelerationStructureModeKHR {
	eBuild = 0,
	eUpdate = 1,
};

enum class CopyAccelerationStructureModeKHR {
	eClone = 0,
	eCompact = 1,
	eSerialize = 2,
	eDeserialize = 3,
	eCloneNV = 0,
	eCompactNV = 1,
};
using CopyAccelerationStructureModeNV = CopyAccelerationStructureModeKHR;

enum class AccelerationStructureTypeKHR {
	eTopLevel = 0,
	eBottomLevel = 1,
	eGeneric = 2,
	eTopLevelNV = 0,
	eBottomLevelNV = 1,
};
using AccelerationStructureTypeNV = AccelerationStructureTypeKHR;

enum class GeometryTypeKHR {
	eTriangles = 0,
	eAabbs = 1,
	eInstances = 2,
	eTrianglesNV = 0,
	eAabbsNV = 1,
};
using GeometryTypeNV = GeometryTypeKHR;

enum class RayTracingShaderGroupTypeKHR {
	eGeneral = 0,
	eTrianglesHitGroup = 1,
	eProceduralHitGroup = 2,
	eGeneralNV = 0,
	eTrianglesHitGroupNV = 1,
	eProceduralHitGroupNV = 2,
};
using RayTracingShaderGroupTypeNV = RayTracingShaderGroupTypeKHR;

enum class AccelerationStructureBuildTypeKHR {
	eHost = 0,
	eDevice = 1,
	eHostOrDevice = 2,
};

enum class AccelerationStructureCompatibilityKHR {
	eCompatible = 0,
	eIncompatible = 1,
};

enum class ShaderGroupShaderKHR {
	eGeneral = 0,
	eClosestHit = 1,
	eAnyHit = 2,
	eIntersection = 3,
};

enum class PerformanceCounterScopeKHR {
	eCommandBuffer = 0,
	eRenderPass = 1,
	eCommand = 2,
	eQueryScopeCommandBuffer = 0,
	eQueryScopeRenderPass = 1,
	eQueryScopeCommand = 2,
};

enum class PerformanceCounterUnitKHR {
	eGeneric = 0,
	ePercentage = 1,
	eNanoseconds = 2,
	eBytes = 3,
	eBytesPerSecond = 4,
	eKelvin = 5,
	eWatts = 6,
	eVolts = 7,
	eAmps = 8,
	eHertz = 9,
	eCycles = 10,
};

enum class PerformanceCounterStorageKHR {
	eInt32 = 0,
	eInt64 = 1,
	eUint32 = 2,
	eUint64 = 3,
	eFloat32 = 4,
	eFloat64 = 5,
};

enum class IndirectExecutionSetInfoTypeEXT {
	ePipelines = 0,
	eShaderObjects = 1,
};

enum class DirectDriverLoadingModeLUNARG {
	eExclusive = 0,
	eInclusive = 1,
};

enum class AntiLagModeAMD {
	eDriverControl = 0,
	eOn = 1,
	eOff = 2,
};

enum class AntiLagStageAMD {
	eInput = 0,
	ePresent = 1,
};

enum class ScopeKHR {
	eDevice = 1,
	eWorkgroup = 2,
	eSubgroup = 3,
	eQueueFamily = 5,
	eDeviceNV = 1,
	eWorkgroupNV = 2,
	eSubgroupNV = 3,
	eQueueFamilyNV = 5,
};
using ScopeNV = ScopeKHR;

enum class ComponentTypeKHR {
	eFloat16 = 0,
	eFloat32 = 1,
	eFloat64 = 2,
	eSint8 = 3,
	eSint16 = 4,
	eSint32 = 5,
	eSint64 = 6,
	eUint8 = 7,
	eUint16 = 8,
	eUint32 = 9,
	eUint64 = 10,
	eFloat16NV = 0,
	eFloat32NV = 1,
	eFloat64NV = 2,
	eSint8NV = 3,
	eSint16NV = 4,
	eSint32NV = 5,
	eSint64NV = 6,
	eUint8NV = 7,
	eUint16NV = 8,
	eUint32NV = 9,
	eUint64NV = 10,
};
using ComponentTypeNV = ComponentTypeKHR;

enum class PhysicalDeviceLayeredApiKHR {
	eVulkan = 0,
	eD3D12 = 1,
	eMetal = 2,
	eOpengl = 3,
	eOpengles = 4,
};

enum class ColorSpaceKHR {
	eSrgbNonlinear = 0,
	eDisplayP3NonlinearEXT = 1000104001,
	eExtendedSrgbLinearEXT = 1000104002,
	eDisplayP3LinearEXT = 1000104003,
	eDciP3NonlinearEXT = 1000104004,
	eBt709LinearEXT = 1000104005,
	eBt709NonlinearEXT = 1000104006,
	eBt2020LinearEXT = 1000104007,
	eHdR10St2084EXT = 1000104008,
	eDolbyvisionEXT = 1000104009,
	eHdR10HlgEXT = 1000104010,
	eAdobergbLinearEXT = 1000104011,
	eAdobergbNonlinearEXT = 1000104012,
	ePassThroughEXT = 1000104013,
	eExtendedSrgbNonlinearEXT = 1000104014,
	eDciP3LinearEXT = 1000104003,
	eDisplayNativeAMD = 1000213000,
};

enum class PresentModeKHR {
	eImmediate = 0,
	eMailbox = 1,
	eFifo = 2,
	eFifoRelaxed = 3,
	eSharedDemandRefresh = 1000111000,
	eSharedContinuousRefresh = 1000111001,
	eFifoLatestReadyEXT = 1000361000,
};

enum class DebugReportObjectTypeEXT {
	eUnknown = 0,
	eInstance = 1,
	ePhysicalDevice = 2,
	eDevice = 3,
	eQueue = 4,
	eSemaphore = 5,
	eCommandBuffer = 6,
	eFence = 7,
	eDeviceMemory = 8,
	eBuffer = 9,
	eImage = 10,
	eEvent = 11,
	eQueryPool = 12,
	eBufferView = 13,
	eImageView = 14,
	eShaderModule = 15,
	ePipelineCache = 16,
	ePipelineLayout = 17,
	eRenderPass = 18,
	ePipeline = 19,
	eDescriptorSetLayout = 20,
	eSampler = 21,
	eDescriptorPool = 22,
	eDescriptorSet = 23,
	eFramebuffer = 24,
	eCommandPool = 25,
	eSurfaceKHR = 26,
	eSwapchainKHR = 27,
	eDebugReportCallbackEXT = 28,
	eDebugReport = 28,
	eDisplayKHR = 29,
	eDisplayModeKHR = 30,
	eValidationCacheEXT = 33,
	eValidationCache = 33,
	eSamplerYcbcrConversion = 1000156000,
	eDescriptorUpdateTemplate = 1000085000,
	eCuModuleNVX = 1000029000,
	eCuFunctionNVX = 1000029001,
	eDescriptorUpdateTemplateKHR = 1000085000,
	eAccelerationStructureKHR = 1000150000,
	eSamplerYcbcrConversionKHR = 1000156000,
	eAccelerationStructureNV = 1000165000,
	eCudaModuleNV = 1000307000,
	eCudaFunctionNV = 1000307001,
#ifdef VK_USE_PLATFORM_FUCHSIA
	eBufferCollectionFUCHSIA = 1000366000,
#endif // VK_USE_PLATFORM_FUCHSIA
};

enum class DisplayPowerStateEXT {
	eOff = 0,
	eSuspend = 1,
	eOn = 2,
};

enum class DeviceEventTypeEXT {
	eDisplayHotplug = 0,
};

enum class DisplayEventTypeEXT {
	eFirstPixelOut = 0,
};

enum class FragmentShadingRateCombinerOpKHR {
	eKeep = 0,
	eReplace = 1,
	eMin = 2,
	eMax = 3,
	eMul = 4,
};

enum class LatencyMarkerNV {
	eSimulationStart = 0,
	eSimulationEnd = 1,
	eRendersubmitStart = 2,
	eRendersubmitEnd = 3,
	ePresentStart = 4,
	ePresentEnd = 5,
	eInputSample = 6,
	eTriggerFlash = 7,
	eOutOfBandRendersubmitStart = 8,
	eOutOfBandRendersubmitEnd = 9,
	eOutOfBandPresentStart = 10,
	eOutOfBandPresentEnd = 11,
};

enum class OutOfBandQueueTypeNV {
	eRender = 0,
	ePresent = 1,
};

enum class PipelineExecutableStatisticFormatKHR {
	eBool32 = 0,
	eInt64 = 1,
	eUint64 = 2,
	eFloat64 = 3,
};

enum class QueryResultStatusKHR {
	eError = -1,
	eNotReady = 0,
	eComplete = 1,
	eInsufficientBitstreamBufferRange = -1000299000,
};

enum class VideoEncodeTuningModeKHR {
	eDefault = 0,
	eHighQuality = 1,
	eLowLatency = 2,
	eUltraLowLatency = 3,
	eLossless = 4,
};

enum class VideoEncodeAV1PredictionModeKHR {
	eIntraOnly = 0,
	eSingleReference = 1,
	eUnidirectionalCompound = 2,
	eBidirectionalCompound = 3,
};

enum class VideoEncodeAV1RateControlGroupKHR {
	eIntra = 0,
	ePredictive = 1,
	eBipredictive = 2,
};

enum class DebugUtilsMessageSeverityFlagBitsEXT : uint32_t {
	eVerbose = 0x1,
	eInfo = 0x10,
	eWarning = 0x100,
	eError = 0x1000,
};
using DebugUtilsMessageSeverityFlagsEXT = Flags<DebugUtilsMessageSeverityFlagBitsEXT>;




// various stuff required by Vulkan structs
// copied from vulkan_core.h
constexpr const unsigned MaxExtensionNameSize = 256;
constexpr const unsigned MaxDescriptionSize = 256;
constexpr const unsigned MaxDeviceGroupSize = 32;
constexpr const unsigned MaxDriverNameSize = 256;
constexpr const unsigned MaxDriverInfoSize = 256;
constexpr const unsigned MaxMemoryHeaps = 16;
constexpr const unsigned MaxMemoryTypes = 32;
constexpr const unsigned MaxPipelineBinaryKeySizeKHR = 32;
constexpr const unsigned MaxPhysicalDeviceNameSize = 256;
constexpr const unsigned LuidSize = 8;
constexpr const unsigned UuidSize = 16;
constexpr const unsigned MaxVideoAv1ReferencesPerFrameKHR = 7;
constexpr const unsigned False = 0;
constexpr const unsigned True = 1;
using Bool32 = uint32_t;
using DeviceAddress = uint64_t;
using DeviceSize = uint64_t;


// taken from vk_platform.h
/* Platform-specific calling convention macros.
 *
 * Platforms should define these so that Vulkan clients call Vulkan commands
 * with the same calling conventions that the Vulkan implementation expects.
 *
 * VKAPI_ATTR - Placed before the return type in function declarations.
 *              Useful for C++11 and GCC/Clang-style function attribute syntax.
 * VKAPI_CALL - Placed after the return type in function declarations.
 *              Useful for MSVC-style calling convention syntax.
 * VKAPI_PTR  - Placed between the '(' and '*' in function pointer types.
 *
 * Function declaration:  VKAPI_ATTR void VKAPI_CALL vkCommand(void);
 * Function pointer type: typedef void (VKAPI_PTR *PFN_vkCommand)(void);
 */
#if defined(_WIN32)
	// On Windows, Vulkan commands use the stdcall convention
	#define VKAPI_ATTR
	#define VKAPI_CALL __stdcall
	#define VKAPI_PTR  VKAPI_CALL
#elif defined(__ANDROID__) && defined(__ARM_ARCH) && __ARM_ARCH < 7
	#error "Vulkan is not supported for the 'armeabi' NDK ABI"
#elif defined(__ANDROID__) && defined(__ARM_ARCH) && __ARM_ARCH >= 7 && defined(__ARM_32BIT_STATE)
	// On Android 32-bit ARM targets, Vulkan functions use the "hardfloat"
	// calling convention, i.e. float parameters are passed in registers. This
	// is true even if the rest of the application passes floats on the stack,
	// as it does by default when compiling for the armeabi-v7a NDK ABI.
	#define VKAPI_ATTR __attribute__((pcs("aapcs-vfp")))
	#define VKAPI_CALL
	#define VKAPI_PTR  VKAPI_ATTR
#else
	// On other platforms, use the default calling convention
	#define VKAPI_ATTR
	#define VKAPI_CALL
	#define VKAPI_PTR
#endif


// taken from Vulkan headers
using PFN_vkAllocationFunction = void* (VKAPI_PTR *)(
	void*                                       pUserData,
	size_t                                      size,
	size_t                                      alignment,
	SystemAllocationScope                       allocationScope);

using PFN_vkFreeFunction = void (VKAPI_PTR *)(
	void*                                       pUserData,
	void*                                       pMemory);

using PFN_vkInternalAllocationNotification = void (VKAPI_PTR *)(
	void*                                       pUserData,
	size_t                                      size,
	InternalAllocationType                      allocationType,
	SystemAllocationScope                       allocationScope);

using PFN_vkInternalFreeNotification = void (VKAPI_PTR *)(
	void*                                       pUserData,
	size_t                                      size,
	InternalAllocationType                      allocationType,
	SystemAllocationScope                       allocationScope);

using PFN_vkReallocationFunction = void* (VKAPI_PTR *)(
	void*                                       pUserData,
	void*                                       pOriginal,
	size_t                                      size,
	size_t                                      alignment,
	SystemAllocationScope                       allocationScope);

using PFN_vkDebugReportCallbackEXT = Bool32 (VKAPI_PTR *)(
	DebugReportFlagsEXT                         flags,
	DebugReportObjectTypeEXT                    objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char*                                 pLayerPrefix,
	const char*                                 pMessage,
	void*                                       pUserData);





// taken from Vulkan headers
struct BaseOutStructure {
	StructureType sType = StructureType::eApplicationInfo;
	BaseOutStructure* pNext = nullptr;
};

struct BaseInStructure {
	StructureType sType = StructureType::eApplicationInfo;
	const BaseInStructure* pNext = nullptr;
};

struct Offset2D {
	int32_t x;
	int32_t y;
};

struct Offset3D {
	int32_t x;
	int32_t y;
	int32_t z;
};

struct Extent2D {
	uint32_t width;
	uint32_t height;
};

struct Extent3D {
	uint32_t width;
	uint32_t height;
	uint32_t depth;
};

struct Rect2D {
	Offset2D offset;
	Extent2D extent;
};

struct ComponentMapping {
	ComponentSwizzle r;
	ComponentSwizzle g;
	ComponentSwizzle b;
	ComponentSwizzle a;
};

struct ExtensionProperties {
	char extensionName[MaxExtensionNameSize];
	uint32_t specVersion;
};

struct AllocationCallbacks {
	void* pUserData;
	PFN_vkAllocationFunction pfnAllocation;
	PFN_vkReallocationFunction pfnReallocation;
	PFN_vkFreeFunction pfnFree;
	PFN_vkInternalAllocationNotification pfnInternalAllocation;
	PFN_vkInternalFreeNotification pfnInternalFree;
};

struct QueueFamilyProperties {
	QueueFlags queueFlags;
	uint32_t queueCount;
	uint32_t timestampValidBits;
	Extent3D minImageTransferGranularity;
};

struct MemoryRequirements {
	DeviceSize size;
	DeviceSize alignment;
	uint32_t memoryTypeBits;
};

struct SparseImageFormatProperties {
	ImageAspectFlags aspectMask;
	Extent3D imageGranularity;
	SparseImageFormatFlags flags;
};

struct SparseImageMemoryRequirements {
	SparseImageFormatProperties formatProperties;
	uint32_t imageMipTailFirstLod;
	DeviceSize imageMipTailSize;
	DeviceSize imageMipTailOffset;
	DeviceSize imageMipTailStride;
};

struct MemoryType {
	MemoryPropertyFlags propertyFlags;
	uint32_t heapIndex;
};

struct MemoryHeap {
	DeviceSize size;
	MemoryHeapFlags flags;
};

struct FormatProperties {
	FormatFeatureFlags linearTilingFeatures;
	FormatFeatureFlags optimalTilingFeatures;
	FormatFeatureFlags bufferFeatures;
};

struct ImageFormatProperties {
	Extent3D maxExtent;
	uint32_t maxMipLevels;
	uint32_t maxArrayLayers;
	SampleCountFlags sampleCounts;
	DeviceSize maxResourceSize;
};

struct DescriptorBufferInfo {
	Buffer buffer;
	DeviceSize offset;
	DeviceSize range;
};

struct DescriptorImageInfo {
	Sampler sampler;
	ImageView imageView;
	ImageLayout imageLayout;
};

struct WriteDescriptorSet {
	StructureType sType = StructureType::eWriteDescriptorSet;
	const void*    pNext = {};
	DescriptorSet dstSet = {};
	uint32_t    dstBinding = {};
	uint32_t    dstArrayElement = {};
	uint32_t    descriptorCount = {};
	DescriptorType descriptorType = {};
	const DescriptorImageInfo* pImageInfo = {};
	const DescriptorBufferInfo* pBufferInfo = {};
	const BufferView* pTexelBufferView;
};

struct BufferCreateInfo {
	vk::StructureType sType = StructureType::eBufferCreateInfo;
	const void*    pNext = {};
	vk::BufferCreateFlags flags = {};
	vk::DeviceSize    size = {};
	vk::BufferUsageFlags usage = {};
	vk::SharingMode sharingMode = {};
	uint32_t    queueFamilyIndexCount = {};
	const uint32_t*    pQueueFamilyIndices = {};
};

struct ImageSubresource {
	vk::ImageAspectFlags aspectMask = {};
	uint32_t    mipLevel = {};
	uint32_t    arrayLayer = {};
};

struct ImageSubresourceLayers {
	vk::ImageAspectFlags aspectMask = {};
	uint32_t    mipLevel = {};
	uint32_t    baseArrayLayer = {};
	uint32_t    layerCount = {};
};

struct ImageSubresourceRange {
	vk::ImageAspectFlags aspectMask = {};
	uint32_t    baseMipLevel = {};
	uint32_t    levelCount = {};
	uint32_t    baseArrayLayer = {};
	uint32_t    layerCount = {};
};

struct ImageCreateInfo {
	vk::StructureType sType = StructureType::eImageCreateInfo;
	const void*    pNext = {};
	vk::ImageCreateFlags flags = {};
	vk::ImageType imageType = {};
	vk::Format format = {};
	vk::Extent3D extent = {};
	uint32_t    mipLevels = {};
	uint32_t    arrayLayers = {};
	vk::SampleCountFlagBits samples = {};
	vk::ImageTiling tiling = {};
	vk::ImageUsageFlags usage = {};
	vk::SharingMode sharingMode = {};
	uint32_t    queueFamilyIndexCount = {};
	const uint32_t*    pQueueFamilyIndices = {};
	vk::ImageLayout initialLayout = {};
};

struct SubresourceLayout {
	vk::DeviceSize    offset = {};
	vk::DeviceSize    size = {};
	vk::DeviceSize    rowPitch = {};
	vk::DeviceSize    arrayPitch = {};
	vk::DeviceSize    depthPitch = {};
};

struct DescriptorSetLayoutBinding {
	uint32_t    binding = {};
	vk::DescriptorType descriptorType = {};
	uint32_t    descriptorCount = {};
	vk::ShaderStageFlags stageFlags = {};
	const vk::Sampler* pImmutableSamplers = {};
};

struct DescriptorSetLayoutCreateInfo {
	vk::StructureType sType = StructureType::eDescriptorSetLayoutCreateInfo;
	const void*    pNext = {};
	vk::DescriptorSetLayoutCreateFlags flags = {};
	uint32_t    bindingCount = {};
	const vk::DescriptorSetLayoutBinding* pBindings = {};
};

struct SpecializationMapEntry {
	uint32_t    constantID = {};
	uint32_t    offset = {};
	size_t    size = {};
};

struct SpecializationInfo {
	uint32_t    mapEntryCount = {};
	const vk::SpecializationMapEntry* pMapEntries = {};
	size_t    dataSize = {};
	const void*    pData = {};
};

struct PipelineShaderStageCreateInfo {
	vk::StructureType sType = StructureType::ePipelineShaderStageCreateInfo;
	const void*    pNext = {};
	vk::PipelineShaderStageCreateFlags flags = {};
	vk::ShaderStageFlagBits stage = {};
	vk::ShaderModule module = {};
	const char*    pName = {};
	const vk::SpecializationInfo* pSpecializationInfo = {};
};

struct PipelineDynamicStateCreateInfo {
	vk::StructureType sType = StructureType::ePipelineDynamicStateCreateInfo;
	const void*    pNext = {};
	vk::PipelineDynamicStateCreateFlags flags = {};
	uint32_t    dynamicStateCount = {};
	const vk::DynamicState* pDynamicStates = {};
};

struct PushConstantRange {
	vk::ShaderStageFlags stageFlags = {};
	uint32_t    offset = {};
	uint32_t    size = {};
};

struct PipelineBinaryKeyKHR {
	vk::StructureType sType = StructureType::ePipelineBinaryKeyKHR;
	void*    pNext = {};
	uint32_t    keySize = {};
	uint8_t    key[MaxPipelineBinaryKeySizeKHR] = {};
};

struct PipelineBinaryDataKHR {
	size_t    dataSize = {};
	void*    pData = {};
};

struct PipelineBinaryKeysAndDataKHR {
	uint32_t    binaryCount = {};
	const vk::PipelineBinaryKeyKHR* pPipelineBinaryKeys = {};
	const vk::PipelineBinaryDataKHR* pPipelineBinaryData = {};
};

struct PipelineCreateInfoKHR {
	vk::StructureType sType = StructureType::ePipelineCreateInfoKHR;
	void*    pNext = {};
};

struct PipelineBinaryCreateInfoKHR {
	vk::StructureType sType = StructureType::ePipelineBinaryCreateInfoKHR;
	const void*    pNext = {};
	const vk::PipelineBinaryKeysAndDataKHR* pKeysAndDataInfo = {};
	vk::Pipeline pipeline = {};
	const vk::PipelineCreateInfoKHR* pPipelineCreateInfo = {};
};

struct PipelineBinaryHandlesInfoKHR {
	vk::StructureType sType = StructureType::ePipelineBinaryHandlesInfoKHR;
	const void*    pNext = {};
	uint32_t    pipelineBinaryCount = {};
	vk::PipelineBinaryKHR* pPipelineBinaries = {};
};

struct PipelineBinaryInfoKHR {
	vk::StructureType sType = StructureType::ePipelineBinaryInfoKHR;
	const void*    pNext = {};
	uint32_t    binaryCount = {};
	const vk::PipelineBinaryKHR* pPipelineBinaries = {};
};

struct ReleaseCapturedPipelineDataInfoKHR {
	vk::StructureType sType = StructureType::eReleaseCapturedPipelineDataInfoKHR;
	void*    pNext = {};
	vk::Pipeline pipeline = {};
};

struct PipelineBinaryDataInfoKHR {
	vk::StructureType sType = StructureType::ePipelineBinaryDataInfoKHR;
	void*    pNext = {};
	vk::PipelineBinaryKHR pipelineBinary = {};
};

union ClearColorValue {
	float    float32[4];
	int32_t    int32[4];
	uint32_t    uint32[4];
};

struct ClearDepthStencilValue {
	float    depth = {};
	uint32_t    stencil = {};
};

union ClearValue {
	vk::ClearColorValue    color;
	vk::ClearDepthStencilValue depthStencil;
};

struct RenderPassBeginInfo {
	vk::StructureType sType = StructureType::eRenderPassBeginInfo;
	const void*    pNext = {};
	vk::RenderPass renderPass = {};
	vk::Framebuffer framebuffer = {};
	vk::Rect2D renderArea = {};
	uint32_t    clearValueCount = {};
	const vk::ClearValue* pClearValues = {};
};

struct PhysicalDeviceFeatures {
	vk::Bool32    robustBufferAccess = {};
	vk::Bool32    fullDrawIndexUint32 = {};
	vk::Bool32    imageCubeArray = {};
	vk::Bool32    independentBlend = {};
	vk::Bool32    geometryShader = {};
	vk::Bool32    tessellationShader = {};
	vk::Bool32    sampleRateShading = {};
	vk::Bool32    dualSrcBlend = {};
	vk::Bool32    logicOp = {};
	vk::Bool32    multiDrawIndirect = {};
	vk::Bool32    drawIndirectFirstInstance = {};
	vk::Bool32    depthClamp = {};
	vk::Bool32    depthBiasClamp = {};
	vk::Bool32    fillModeNonSolid = {};
	vk::Bool32    depthBounds = {};
	vk::Bool32    wideLines = {};
	vk::Bool32    largePoints = {};
	vk::Bool32    alphaToOne = {};
	vk::Bool32    multiViewport = {};
	vk::Bool32    samplerAnisotropy = {};
	vk::Bool32    textureCompressionETC2 = {};
	vk::Bool32    textureCompressionASTC_LDR = {};
	vk::Bool32    textureCompressionBC = {};
	vk::Bool32    occlusionQueryPrecise = {};
	vk::Bool32    pipelineStatisticsQuery = {};
	vk::Bool32    vertexPipelineStoresAndAtomics = {};
	vk::Bool32    fragmentStoresAndAtomics = {};
	vk::Bool32    shaderTessellationAndGeometryPointSize = {};
	vk::Bool32    shaderImageGatherExtended = {};
	vk::Bool32    shaderStorageImageExtendedFormats = {};
	vk::Bool32    shaderStorageImageMultisample = {};
	vk::Bool32    shaderStorageImageReadWithoutFormat = {};
	vk::Bool32    shaderStorageImageWriteWithoutFormat = {};
	vk::Bool32    shaderUniformBufferArrayDynamicIndexing = {};
	vk::Bool32    shaderSampledImageArrayDynamicIndexing = {};
	vk::Bool32    shaderStorageBufferArrayDynamicIndexing = {};
	vk::Bool32    shaderStorageImageArrayDynamicIndexing = {};
	vk::Bool32    shaderClipDistance = {};
	vk::Bool32    shaderCullDistance = {};
	vk::Bool32    shaderFloat64 = {};
	vk::Bool32    shaderInt64 = {};
	vk::Bool32    shaderInt16 = {};
	vk::Bool32    shaderResourceResidency = {};
	vk::Bool32    shaderResourceMinLod = {};
	vk::Bool32    sparseBinding = {};
	vk::Bool32    sparseResidencyBuffer = {};
	vk::Bool32    sparseResidencyImage2D = {};
	vk::Bool32    sparseResidencyImage3D = {};
	vk::Bool32    sparseResidency2Samples = {};
	vk::Bool32    sparseResidency4Samples = {};
	vk::Bool32    sparseResidency8Samples = {};
	vk::Bool32    sparseResidency16Samples = {};
	vk::Bool32    sparseResidencyAliased = {};
	vk::Bool32    variableMultisampleRate = {};
	vk::Bool32    inheritedQueries = {};
};

struct PhysicalDeviceSparseProperties {
	vk::Bool32    residencyStandard2DBlockShape = {};
	vk::Bool32    residencyStandard2DMultisampleBlockShape = {};
	vk::Bool32    residencyStandard3DBlockShape = {};
	vk::Bool32    residencyAlignedMipSize = {};
	vk::Bool32    residencyNonResidentStrict = {};
};

struct PhysicalDeviceLimits {
	uint32_t    maxImageDimension1D = {};
	uint32_t    maxImageDimension2D = {};
	uint32_t    maxImageDimension3D = {};
	uint32_t    maxImageDimensionCube = {};
	uint32_t    maxImageArrayLayers = {};
	uint32_t    maxTexelBufferElements = {};
	uint32_t    maxUniformBufferRange = {};
	uint32_t    maxStorageBufferRange = {};
	uint32_t    maxPushConstantsSize = {};
	uint32_t    maxMemoryAllocationCount = {};
	uint32_t    maxSamplerAllocationCount = {};
	vk::DeviceSize    bufferImageGranularity = {};
	vk::DeviceSize    sparseAddressSpaceSize = {};
	uint32_t    maxBoundDescriptorSets = {};
	uint32_t    maxPerStageDescriptorSamplers = {};
	uint32_t    maxPerStageDescriptorUniformBuffers = {};
	uint32_t    maxPerStageDescriptorStorageBuffers = {};
	uint32_t    maxPerStageDescriptorSampledImages = {};
	uint32_t    maxPerStageDescriptorStorageImages = {};
	uint32_t    maxPerStageDescriptorInputAttachments = {};
	uint32_t    maxPerStageResources = {};
	uint32_t    maxDescriptorSetSamplers = {};
	uint32_t    maxDescriptorSetUniformBuffers = {};
	uint32_t    maxDescriptorSetUniformBuffersDynamic = {};
	uint32_t    maxDescriptorSetStorageBuffers = {};
	uint32_t    maxDescriptorSetStorageBuffersDynamic = {};
	uint32_t    maxDescriptorSetSampledImages = {};
	uint32_t    maxDescriptorSetStorageImages = {};
	uint32_t    maxDescriptorSetInputAttachments = {};
	uint32_t    maxVertexInputAttributes = {};
	uint32_t    maxVertexInputBindings = {};
	uint32_t    maxVertexInputAttributeOffset = {};
	uint32_t    maxVertexInputBindingStride = {};
	uint32_t    maxVertexOutputComponents = {};
	uint32_t    maxTessellationGenerationLevel = {};
	uint32_t    maxTessellationPatchSize = {};
	uint32_t    maxTessellationControlPerVertexInputComponents = {};
	uint32_t    maxTessellationControlPerVertexOutputComponents = {};
	uint32_t    maxTessellationControlPerPatchOutputComponents = {};
	uint32_t    maxTessellationControlTotalOutputComponents = {};
	uint32_t    maxTessellationEvaluationInputComponents = {};
	uint32_t    maxTessellationEvaluationOutputComponents = {};
	uint32_t    maxGeometryShaderInvocations = {};
	uint32_t    maxGeometryInputComponents = {};
	uint32_t    maxGeometryOutputComponents = {};
	uint32_t    maxGeometryOutputVertices = {};
	uint32_t    maxGeometryTotalOutputComponents = {};
	uint32_t    maxFragmentInputComponents = {};
	uint32_t    maxFragmentOutputAttachments = {};
	uint32_t    maxFragmentDualSrcAttachments = {};
	uint32_t    maxFragmentCombinedOutputResources = {};
	uint32_t    maxComputeSharedMemorySize = {};
	uint32_t    maxComputeWorkGroupCount[3] = {};
	uint32_t    maxComputeWorkGroupInvocations = {};
	uint32_t    maxComputeWorkGroupSize[3] = {};
	uint32_t    subPixelPrecisionBits = {};
	uint32_t    subTexelPrecisionBits = {};
	uint32_t    mipmapPrecisionBits = {};
	uint32_t    maxDrawIndexedIndexValue = {};
	uint32_t    maxDrawIndirectCount = {};
	float    maxSamplerLodBias = {};
	float    maxSamplerAnisotropy = {};
	uint32_t    maxViewports = {};
	uint32_t    maxViewportDimensions[2] = {};
	float    viewportBoundsRange[2] = {};
	uint32_t    viewportSubPixelBits = {};
	size_t    minMemoryMapAlignment = {};
	vk::DeviceSize    minTexelBufferOffsetAlignment = {};
	vk::DeviceSize    minUniformBufferOffsetAlignment = {};
	vk::DeviceSize    minStorageBufferOffsetAlignment = {};
	int32_t    minTexelOffset = {};
	uint32_t    maxTexelOffset = {};
	int32_t    minTexelGatherOffset = {};
	uint32_t    maxTexelGatherOffset = {};
	float    minInterpolationOffset = {};
	float    maxInterpolationOffset = {};
	uint32_t    subPixelInterpolationOffsetBits = {};
	uint32_t    maxFramebufferWidth = {};
	uint32_t    maxFramebufferHeight = {};
	uint32_t    maxFramebufferLayers = {};
	vk::SampleCountFlags framebufferColorSampleCounts = {};
	vk::SampleCountFlags framebufferDepthSampleCounts = {};
	vk::SampleCountFlags framebufferStencilSampleCounts = {};
	vk::SampleCountFlags framebufferNoAttachmentsSampleCounts = {};
	uint32_t    maxColorAttachments = {};
	vk::SampleCountFlags sampledImageColorSampleCounts = {};
	vk::SampleCountFlags sampledImageIntegerSampleCounts = {};
	vk::SampleCountFlags sampledImageDepthSampleCounts = {};
	vk::SampleCountFlags sampledImageStencilSampleCounts = {};
	vk::SampleCountFlags storageImageSampleCounts = {};
	uint32_t    maxSampleMaskWords = {};
	vk::Bool32    timestampComputeAndGraphics = {};
	float    timestampPeriod = {};
	uint32_t    maxClipDistances = {};
	uint32_t    maxCullDistances = {};
	uint32_t    maxCombinedClipAndCullDistances = {};
	uint32_t    discreteQueuePriorities = {};
	float    pointSizeRange[2] = {};
	float    lineWidthRange[2] = {};
	float    pointSizeGranularity = {};
	float    lineWidthGranularity = {};
	vk::Bool32    strictLines = {};
	vk::Bool32    standardSampleLocations = {};
	vk::DeviceSize    optimalBufferCopyOffsetAlignment = {};
	vk::DeviceSize    optimalBufferCopyRowPitchAlignment = {};
	vk::DeviceSize    nonCoherentAtomSize = {};
};

struct DisplayPropertiesKHR {
	vk::DisplayKHR display = {};
	const char*    displayName = {};
	vk::Extent2D physicalDimensions = {};
	vk::Extent2D physicalResolution = {};
	vk::SurfaceTransformFlagsKHR supportedTransforms = {};
	vk::Bool32    planeReorderPossible = {};
	vk::Bool32    persistentContent = {};
};

struct DisplayPlanePropertiesKHR {
	vk::DisplayKHR currentDisplay = {};
	uint32_t    currentStackIndex = {};
};

struct DisplayModeParametersKHR {
	vk::Extent2D visibleRegion = {};
	uint32_t    refreshRate = {};
};

struct DisplayModePropertiesKHR {
	vk::DisplayModeKHR displayMode = {};
	vk::DisplayModeParametersKHR parameters = {};
};

struct DisplayModeCreateInfoKHR {
	vk::StructureType sType = StructureType::eDisplayModeCreateInfoKHR;
	const void*    pNext = {};
	vk::DisplayModeCreateFlagsKHR flags = {};
	vk::DisplayModeParametersKHR parameters = {};
};

struct DisplayPlaneCapabilitiesKHR {
	vk::DisplayPlaneAlphaFlagsKHR supportedAlpha = {};
	vk::Offset2D minSrcPosition = {};
	vk::Offset2D maxSrcPosition = {};
	vk::Extent2D minSrcExtent = {};
	vk::Extent2D maxSrcExtent = {};
	vk::Offset2D minDstPosition = {};
	vk::Offset2D maxDstPosition = {};
	vk::Extent2D minDstExtent = {};
	vk::Extent2D maxDstExtent = {};
};

struct DisplaySurfaceCreateInfoKHR {
	vk::StructureType sType = StructureType::eDisplaySurfaceCreateInfoKHR;
	const void*    pNext = {};
	vk::DisplaySurfaceCreateFlagsKHR flags = {};
	vk::DisplayModeKHR displayMode = {};
	uint32_t    planeIndex = {};
	uint32_t    planeStackIndex = {};
	vk::SurfaceTransformFlagBitsKHR transform = {};
	float    globalAlpha = {};
	vk::DisplayPlaneAlphaFlagBitsKHR alphaMode = {};
	vk::Extent2D imageExtent = {};
};

struct DisplayPresentInfoKHR {
	vk::StructureType sType = StructureType::eDisplayPresentInfoKHR;
	const void*    pNext = {};
	vk::Rect2D srcRect = {};
	vk::Rect2D dstRect = {};
	vk::Bool32    persistent = {};
};

struct SurfaceCapabilitiesKHR {
	uint32_t    minImageCount = {};
	uint32_t    maxImageCount = {};
	vk::Extent2D currentExtent = {};
	vk::Extent2D minImageExtent = {};
	vk::Extent2D maxImageExtent = {};
	uint32_t    maxImageArrayLayers = {};
	vk::SurfaceTransformFlagsKHR supportedTransforms = {};
	vk::SurfaceTransformFlagBitsKHR currentTransform = {};
	vk::CompositeAlphaFlagsKHR supportedCompositeAlpha = {};
	vk::ImageUsageFlags supportedUsageFlags = {};
};

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
struct AndroidSurfaceCreateInfoKHR {
	vk::StructureType sType = StructureType::eAndroidSurfaceCreateInfoKHR;
	const void*    pNext = {};
	vk::AndroidSurfaceCreateFlagsKHR flags = {};
	struct vk::ANativeWindow*    window = {};
};
#endif // VK_USE_PLATFORM_ANDROID_KHR

#if defined(VK_USE_PLATFORM_VI_NN)
struct ViSurfaceCreateInfoNN {
	vk::StructureType sType = StructureType::eViSurfaceCreateInfoNN;
	const void*    pNext = {};
	vk::ViSurfaceCreateFlagsNN flags = {};
	void*    window = {};
};
#endif // VK_USE_PLATFORM_VI_NN

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
struct WaylandSurfaceCreateInfoKHR {
	vk::StructureType sType = StructureType::eWaylandSurfaceCreateInfoKHR;
	const void*    pNext = {};
	vk::WaylandSurfaceCreateFlagsKHR flags = {};
	struct wl_display*    display = {};
	struct wl_surface*    surface = {};
};
#endif // VK_USE_PLATFORM_WAYLAND_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct Win32SurfaceCreateInfoKHR {
	vk::StructureType sType = StructureType::eWin32SurfaceCreateInfoKHR;
	const void*    pNext = {};
	vk::Win32SurfaceCreateFlagsKHR flags = {};
	HINSTANCE    hinstance = {};
	HWND    hwnd = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_XLIB_KHR)
struct XlibSurfaceCreateInfoKHR {
	vk::StructureType sType = StructureType::eXlibSurfaceCreateInfoKHR;
	const void*    pNext = {};
	vk::XlibSurfaceCreateFlagsKHR flags = {};
	Display*    dpy = {};
	Window    window = {};
};
#endif // VK_USE_PLATFORM_XLIB_KHR

#if defined(VK_USE_PLATFORM_XCB_KHR)
struct XcbSurfaceCreateInfoKHR {
	vk::StructureType sType = StructureType::eXcbSurfaceCreateInfoKHR;
	const void*    pNext = {};
	vk::XcbSurfaceCreateFlagsKHR flags = {};
	xcb_connection_t*    connection = {};
	xcb_window_t    window = {};
};
#endif // VK_USE_PLATFORM_XCB_KHR

struct SurfaceFormatKHR {
	vk::Format format = {};
	vk::ColorSpaceKHR colorSpace = {};
};

struct SwapchainCreateInfoKHR {
	vk::StructureType sType = StructureType::eSwapchainCreateInfoKHR;
	const void*    pNext = {};
	vk::SwapchainCreateFlagsKHR flags = {};
	vk::SurfaceKHR surface = {};
	uint32_t    minImageCount = {};
	vk::Format imageFormat = {};
	vk::ColorSpaceKHR imageColorSpace = {};
	vk::Extent2D imageExtent = {};
	uint32_t    imageArrayLayers = {};
	vk::ImageUsageFlags imageUsage = {};
	vk::SharingMode imageSharingMode = {};
	uint32_t    queueFamilyIndexCount = {};
	const uint32_t*    pQueueFamilyIndices = {};
	vk::SurfaceTransformFlagBitsKHR preTransform = {};
	vk::CompositeAlphaFlagBitsKHR compositeAlpha = {};
	vk::PresentModeKHR presentMode = {};
	vk::Bool32    clipped = {};
	vk::SwapchainKHR oldSwapchain = {};
};

struct PresentInfoKHR {
	vk::StructureType sType = StructureType::ePresentInfoKHR;
	const void*    pNext = {};
	uint32_t    waitSemaphoreCount = {};
	const vk::Semaphore* pWaitSemaphores = {};
	uint32_t    swapchainCount = {};
	const vk::SwapchainKHR* pSwapchains = {};
	const uint32_t*    pImageIndices = {};
	vk::Result* pResults = {};
};

struct DebugReportCallbackCreateInfoEXT {
	vk::StructureType sType = StructureType::eDebugReportCallbackCreateInfoEXT;
	const void*    pNext = {};
	vk::DebugReportFlagsEXT flags = {};
	PFN_vkDebugReportCallbackEXT    pfnCallback = {};
	void*    pUserData = {};
};

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct Win32KeyedMutexAcquireReleaseInfoNV {
	vk::StructureType sType = StructureType::eWin32KeyedMutexAcquireReleaseInfoNV;
	const void*    pNext = {};
	uint32_t    acquireCount = {};
	const vk::DeviceMemory* pAcquireSyncs = {};
	const uint64_t*    pAcquireKeys = {};
	const uint32_t*    pAcquireTimeoutMilliseconds = {};
	uint32_t    releaseCount = {};
	const vk::DeviceMemory* pReleaseSyncs = {};
	const uint64_t*    pReleaseKeys = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

struct PhysicalDeviceFeatures2 {
	vk::StructureType sType = StructureType::ePhysicalDeviceFeatures2;
	void*    pNext = {};
	vk::PhysicalDeviceFeatures features = {};
};
using PhysicalDeviceFeatures2KHR = PhysicalDeviceFeatures2;

struct FormatProperties2 {
	vk::StructureType sType = StructureType::eFormatProperties2;
	void*    pNext = {};
	vk::FormatProperties formatProperties = {};
};
using FormatProperties2KHR = FormatProperties2;

struct ImageFormatProperties2 {
	vk::StructureType sType = StructureType::eImageFormatProperties2;
	void*    pNext = {};
	vk::ImageFormatProperties imageFormatProperties = {};
};
using ImageFormatProperties2KHR = ImageFormatProperties2;

struct PhysicalDeviceImageFormatInfo2 {
	vk::StructureType sType = StructureType::ePhysicalDeviceImageFormatInfo2;
	const void*    pNext = {};
	vk::Format format = {};
	vk::ImageType type = {};
	vk::ImageTiling tiling = {};
	vk::ImageUsageFlags usage = {};
	vk::ImageCreateFlags flags = {};
};
using PhysicalDeviceImageFormatInfo2KHR = PhysicalDeviceImageFormatInfo2;

struct QueueFamilyProperties2 {
	vk::StructureType sType = StructureType::eQueueFamilyProperties2;
	void*    pNext = {};
	vk::QueueFamilyProperties queueFamilyProperties = {};
};
using QueueFamilyProperties2KHR = QueueFamilyProperties2;

struct SparseImageFormatProperties2 {
	vk::StructureType sType = StructureType::eSparseImageFormatProperties2;
	void*    pNext = {};
	vk::SparseImageFormatProperties properties = {};
};
using SparseImageFormatProperties2KHR = SparseImageFormatProperties2;

struct PhysicalDeviceSparseImageFormatInfo2 {
	vk::StructureType sType = StructureType::ePhysicalDeviceSparseImageFormatInfo2;
	const void*    pNext = {};
	vk::Format format = {};
	vk::ImageType type = {};
	vk::SampleCountFlagBits samples = {};
	vk::ImageUsageFlags usage = {};
	vk::ImageTiling tiling = {};
};
using PhysicalDeviceSparseImageFormatInfo2KHR = PhysicalDeviceSparseImageFormatInfo2;

struct RectLayerKHR {
	vk::Offset2D offset = {};
	vk::Extent2D extent = {};
	uint32_t    layer = {};
};

struct PresentRegionKHR {
	uint32_t    rectangleCount = {};
	const vk::RectLayerKHR* pRectangles = {};
};

struct PresentRegionsKHR {
	vk::StructureType sType = StructureType::ePresentRegionsKHR;
	const void*    pNext = {};
	uint32_t    swapchainCount = {};
	const vk::PresentRegionKHR* pRegions = {};
};

struct ExternalMemoryProperties {
	vk::ExternalMemoryFeatureFlags externalMemoryFeatures = {};
	vk::ExternalMemoryHandleTypeFlags exportFromImportedHandleTypes = {};
	vk::ExternalMemoryHandleTypeFlags compatibleHandleTypes = {};
};
using ExternalMemoryPropertiesKHR = ExternalMemoryProperties;

struct PhysicalDeviceExternalBufferInfo {
	vk::StructureType sType = StructureType::ePhysicalDeviceExternalBufferInfo;
	const void*    pNext = {};
	vk::BufferCreateFlags flags = {};
	vk::BufferUsageFlags usage = {};
	vk::ExternalMemoryHandleTypeFlagBits handleType = {};
};
using PhysicalDeviceExternalBufferInfoKHR = PhysicalDeviceExternalBufferInfo;

struct ExternalBufferProperties {
	vk::StructureType sType = StructureType::eExternalBufferProperties;
	void*    pNext = {};
	vk::ExternalMemoryProperties externalMemoryProperties = {};
};
using ExternalBufferPropertiesKHR = ExternalBufferProperties;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct ImportMemoryWin32HandleInfoKHR {
	vk::StructureType sType = StructureType::eImportMemoryWin32HandleInfoKHR;
	const void*    pNext = {};
	vk::ExternalMemoryHandleTypeFlagBits handleType = {};
	HANDLE    handle = {};
	LPCWSTR    name = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct ExportMemoryWin32HandleInfoKHR {
	vk::StructureType sType = StructureType::eExportMemoryWin32HandleInfoKHR;
	const void*    pNext = {};
	const SECURITY_ATTRIBUTES*    pAttributes = {};
	DWORD    dwAccess = {};
	LPCWSTR    name = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct MemoryWin32HandlePropertiesKHR {
	vk::StructureType sType = StructureType::eMemoryWin32HandlePropertiesKHR;
	void*    pNext = {};
	uint32_t    memoryTypeBits = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct MemoryGetWin32HandleInfoKHR {
	vk::StructureType sType = StructureType::eMemoryGetWin32HandleInfoKHR;
	const void*    pNext = {};
	vk::DeviceMemory memory = {};
	vk::ExternalMemoryHandleTypeFlagBits handleType = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

struct ImportMemoryFdInfoKHR {
	vk::StructureType sType = StructureType::eImportMemoryFdInfoKHR;
	const void*    pNext = {};
	vk::ExternalMemoryHandleTypeFlagBits handleType = {};
	int    fd = {};
};

struct MemoryFdPropertiesKHR {
	vk::StructureType sType = StructureType::eMemoryFdPropertiesKHR;
	void*    pNext = {};
	uint32_t    memoryTypeBits = {};
};

struct MemoryGetFdInfoKHR {
	vk::StructureType sType = StructureType::eMemoryGetFdInfoKHR;
	const void*    pNext = {};
	vk::DeviceMemory memory = {};
	vk::ExternalMemoryHandleTypeFlagBits handleType = {};
};

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct Win32KeyedMutexAcquireReleaseInfoKHR {
	vk::StructureType sType = StructureType::eWin32KeyedMutexAcquireReleaseInfoKHR;
	const void*    pNext = {};
	uint32_t    acquireCount = {};
	const vk::DeviceMemory* pAcquireSyncs = {};
	const uint64_t*    pAcquireKeys = {};
	const uint32_t*    pAcquireTimeouts = {};
	uint32_t    releaseCount = {};
	const vk::DeviceMemory* pReleaseSyncs = {};
	const uint64_t*    pReleaseKeys = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

struct PhysicalDeviceExternalSemaphoreInfo {
	vk::StructureType sType = StructureType::ePhysicalDeviceExternalSemaphoreInfo;
	const void*    pNext = {};
	vk::ExternalSemaphoreHandleTypeFlagBits handleType = {};
};
using PhysicalDeviceExternalSemaphoreInfoKHR = PhysicalDeviceExternalSemaphoreInfo;

struct ExternalSemaphoreProperties {
	vk::StructureType sType = StructureType::eExternalSemaphoreProperties;
	void*    pNext = {};
	vk::ExternalSemaphoreHandleTypeFlags exportFromImportedHandleTypes = {};
	vk::ExternalSemaphoreHandleTypeFlags compatibleHandleTypes = {};
	vk::ExternalSemaphoreFeatureFlags externalSemaphoreFeatures = {};
};
using ExternalSemaphorePropertiesKHR = ExternalSemaphoreProperties;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct ImportSemaphoreWin32HandleInfoKHR {
	vk::StructureType sType = StructureType::eImportSemaphoreWin32HandleInfoKHR;
	const void*    pNext = {};
	vk::Semaphore semaphore = {};
	vk::SemaphoreImportFlags flags = {};
	vk::ExternalSemaphoreHandleTypeFlagBits handleType = {};
	HANDLE    handle = {};
	LPCWSTR    name = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct ExportSemaphoreWin32HandleInfoKHR {
	vk::StructureType sType = StructureType::eExportSemaphoreWin32HandleInfoKHR;
	const void*    pNext = {};
	const SECURITY_ATTRIBUTES*    pAttributes = {};
	DWORD    dwAccess = {};
	LPCWSTR    name = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct D3D12FenceSubmitInfoKHR {
	vk::StructureType sType = StructureType::eD3D12FenceSubmitInfoKHR;
	const void*    pNext = {};
	uint32_t    waitSemaphoreValuesCount = {};
	const uint64_t*    pWaitSemaphoreValues = {};
	uint32_t    signalSemaphoreValuesCount = {};
	const uint64_t*    pSignalSemaphoreValues = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct SemaphoreGetWin32HandleInfoKHR {
	vk::StructureType sType = StructureType::eSemaphoreGetWin32HandleInfoKHR;
	const void*    pNext = {};
	vk::Semaphore semaphore = {};
	vk::ExternalSemaphoreHandleTypeFlagBits handleType = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

struct ImportSemaphoreFdInfoKHR {
	vk::StructureType sType = StructureType::eImportSemaphoreFdInfoKHR;
	const void*    pNext = {};
	vk::Semaphore semaphore = {};
	vk::SemaphoreImportFlags flags = {};
	vk::ExternalSemaphoreHandleTypeFlagBits handleType = {};
	int    fd = {};
};

struct SemaphoreGetFdInfoKHR {
	vk::StructureType sType = StructureType::eSemaphoreGetFdInfoKHR;
	const void*    pNext = {};
	vk::Semaphore semaphore = {};
	vk::ExternalSemaphoreHandleTypeFlagBits handleType = {};
};

struct PhysicalDeviceExternalFenceInfo {
	vk::StructureType sType = StructureType::ePhysicalDeviceExternalFenceInfo;
	const void*    pNext = {};
	vk::ExternalFenceHandleTypeFlagBits handleType = {};
};
using PhysicalDeviceExternalFenceInfoKHR = PhysicalDeviceExternalFenceInfo;

struct ExternalFenceProperties {
	vk::StructureType sType = StructureType::eExternalFenceProperties;
	void*    pNext = {};
	vk::ExternalFenceHandleTypeFlags exportFromImportedHandleTypes = {};
	vk::ExternalFenceHandleTypeFlags compatibleHandleTypes = {};
	vk::ExternalFenceFeatureFlags externalFenceFeatures = {};
};
using ExternalFencePropertiesKHR = ExternalFenceProperties;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct ImportFenceWin32HandleInfoKHR {
	vk::StructureType sType = StructureType::eImportFenceWin32HandleInfoKHR;
	const void*    pNext = {};
	vk::Fence fence = {};
	vk::FenceImportFlags flags = {};
	vk::ExternalFenceHandleTypeFlagBits handleType = {};
	HANDLE    handle = {};
	LPCWSTR    name = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct ExportFenceWin32HandleInfoKHR {
	vk::StructureType sType = StructureType::eExportFenceWin32HandleInfoKHR;
	const void*    pNext = {};
	const SECURITY_ATTRIBUTES*    pAttributes = {};
	DWORD    dwAccess = {};
	LPCWSTR    name = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct FenceGetWin32HandleInfoKHR {
	vk::StructureType sType = StructureType::eFenceGetWin32HandleInfoKHR;
	const void*    pNext = {};
	vk::Fence fence = {};
	vk::ExternalFenceHandleTypeFlagBits handleType = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

struct ImportFenceFdInfoKHR {
	vk::StructureType sType = StructureType::eImportFenceFdInfoKHR;
	const void*    pNext = {};
	vk::Fence fence = {};
	vk::FenceImportFlags flags = {};
	vk::ExternalFenceHandleTypeFlagBits handleType = {};
	int    fd = {};
};

struct FenceGetFdInfoKHR {
	vk::StructureType sType = StructureType::eFenceGetFdInfoKHR;
	const void*    pNext = {};
	vk::Fence fence = {};
	vk::ExternalFenceHandleTypeFlagBits handleType = {};
};

struct DisplayPowerInfoEXT {
	vk::StructureType sType = StructureType::eDisplayPowerInfoEXT;
	const void*    pNext = {};
	vk::DisplayPowerStateEXT powerState = {};
};

struct DeviceEventInfoEXT {
	vk::StructureType sType = StructureType::eDeviceEventInfoEXT;
	const void*    pNext = {};
	vk::DeviceEventTypeEXT deviceEvent = {};
};

struct DisplayEventInfoEXT {
	vk::StructureType sType = StructureType::eDisplayEventInfoEXT;
	const void*    pNext = {};
	vk::DisplayEventTypeEXT displayEvent = {};
};

struct SwapchainCounterCreateInfoEXT {
	vk::StructureType sType = StructureType::eSwapchainCounterCreateInfoEXT;
	const void*    pNext = {};
	vk::SurfaceCounterFlagsEXT surfaceCounters = {};
};

struct PhysicalDeviceGroupProperties {
	vk::StructureType sType = StructureType::ePhysicalDeviceGroupProperties;
	void*    pNext = {};
	uint32_t    physicalDeviceCount = {};
	vk::PhysicalDevice    physicalDevices[MaxDeviceGroupSize] = {};
	vk::Bool32    subsetAllocation = {};
};
using PhysicalDeviceGroupPropertiesKHR = PhysicalDeviceGroupProperties;

struct BindBufferMemoryInfo {
	vk::StructureType sType = StructureType::eBindBufferMemoryInfo;
	const void*    pNext = {};
	vk::Buffer buffer = {};
	vk::DeviceMemory memory = {};
	vk::DeviceSize    memoryOffset = {};
};
using BindBufferMemoryInfoKHR = BindBufferMemoryInfo;

struct BindImageMemoryInfo {
	vk::StructureType sType = StructureType::eBindImageMemoryInfo;
	const void*    pNext = {};
	vk::Image image = {};
	vk::DeviceMemory memory = {};
	vk::DeviceSize    memoryOffset = {};
};
using BindImageMemoryInfoKHR = BindImageMemoryInfo;

struct DeviceGroupPresentCapabilitiesKHR {
	vk::StructureType sType = StructureType::eDeviceGroupPresentCapabilitiesKHR;
	void*    pNext = {};
	uint32_t    presentMask[MaxDeviceGroupSize] = {};
	vk::DeviceGroupPresentModeFlagsKHR modes = {};
};

struct ImageSwapchainCreateInfoKHR {
	vk::StructureType sType = StructureType::eImageSwapchainCreateInfoKHR;
	const void*    pNext = {};
	vk::SwapchainKHR swapchain = {};
};

struct BindImageMemorySwapchainInfoKHR {
	vk::StructureType sType = StructureType::eBindImageMemorySwapchainInfoKHR;
	const void*    pNext = {};
	vk::SwapchainKHR swapchain = {};
	uint32_t    imageIndex = {};
};

struct AcquireNextImageInfoKHR {
	vk::StructureType sType = StructureType::eAcquireNextImageInfoKHR;
	const void*    pNext = {};
	vk::SwapchainKHR swapchain = {};
	uint64_t    timeout = {};
	vk::Semaphore semaphore = {};
	vk::Fence fence = {};
	uint32_t    deviceMask = {};
};

struct DeviceGroupPresentInfoKHR {
	vk::StructureType sType = StructureType::eDeviceGroupPresentInfoKHR;
	const void*    pNext = {};
	uint32_t    swapchainCount = {};
	const uint32_t*    pDeviceMasks = {};
	vk::DeviceGroupPresentModeFlagBitsKHR mode = {};
};

struct DeviceGroupSwapchainCreateInfoKHR {
	vk::StructureType sType = StructureType::eDeviceGroupSwapchainCreateInfoKHR;
	const void*    pNext = {};
	vk::DeviceGroupPresentModeFlagsKHR modes = {};
};

struct DescriptorUpdateTemplateEntry {
	uint32_t    dstBinding = {};
	uint32_t    dstArrayElement = {};
	uint32_t    descriptorCount = {};
	vk::DescriptorType descriptorType = {};
	size_t    offset = {};
	size_t    stride = {};
};
using DescriptorUpdateTemplateEntryKHR = DescriptorUpdateTemplateEntry;

struct DescriptorUpdateTemplateCreateInfo {
	vk::StructureType sType = StructureType::eDescriptorUpdateTemplateCreateInfo;
	const void*    pNext = {};
	vk::DescriptorUpdateTemplateCreateFlags flags = {};
	uint32_t    descriptorUpdateEntryCount = {};
	const vk::DescriptorUpdateTemplateEntry* pDescriptorUpdateEntries = {};
	vk::DescriptorUpdateTemplateType templateType = {};
	vk::DescriptorSetLayout descriptorSetLayout = {};
	vk::PipelineBindPoint pipelineBindPoint = {};
	vk::PipelineLayout pipelineLayout = {};
	uint32_t    set = {};
};
using DescriptorUpdateTemplateCreateInfoKHR = DescriptorUpdateTemplateCreateInfo;

struct XYColorEXT {
	float    x = {};
	float    y = {};
};

struct PhysicalDevicePresentIdFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDevicePresentIdFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    presentId = {};
};

struct PresentIdKHR {
	vk::StructureType sType = StructureType::ePresentIdKHR;
	const void*    pNext = {};
	uint32_t    swapchainCount = {};
	const uint64_t*    pPresentIds = {};
};

struct PhysicalDevicePresentWaitFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDevicePresentWaitFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    presentWait = {};
};

struct HdrMetadataEXT {
	vk::StructureType sType = StructureType::eHdrMetadataEXT;
	const void*    pNext = {};
	vk::XYColorEXT displayPrimaryRed = {};
	vk::XYColorEXT displayPrimaryGreen = {};
	vk::XYColorEXT displayPrimaryBlue = {};
	vk::XYColorEXT whitePoint = {};
	float    maxLuminance = {};
	float    minLuminance = {};
	float    maxContentLightLevel = {};
	float    maxFrameAverageLightLevel = {};
};

struct DisplayNativeHdrSurfaceCapabilitiesAMD {
	vk::StructureType sType = StructureType::eDisplayNativeHdrSurfaceCapabilitiesAMD;
	void*    pNext = {};
	vk::Bool32    localDimmingSupport = {};
};

struct SwapchainDisplayNativeHdrCreateInfoAMD {
	vk::StructureType sType = StructureType::eSwapchainDisplayNativeHdrCreateInfoAMD;
	const void*    pNext = {};
	vk::Bool32    localDimmingEnable = {};
};

struct RefreshCycleDurationGOOGLE {
	uint64_t    refreshDuration = {};
};

struct PastPresentationTimingGOOGLE {
	uint32_t    presentID = {};
	uint64_t    desiredPresentTime = {};
	uint64_t    actualPresentTime = {};
	uint64_t    earliestPresentTime = {};
	uint64_t    presentMargin = {};
};

struct PresentTimeGOOGLE {
	uint32_t    presentID = {};
	uint64_t    desiredPresentTime = {};
};

struct PresentTimesInfoGOOGLE {
	vk::StructureType sType = StructureType::ePresentTimesInfoGOOGLE;
	const void*    pNext = {};
	uint32_t    swapchainCount = {};
	const vk::PresentTimeGOOGLE* pTimes = {};
};

#if defined(VK_USE_PLATFORM_IOS_MVK)
struct IOSSurfaceCreateInfoMVK {
	vk::StructureType sType = StructureType::eIosSurfaceCreateInfoMVK;
	const void*    pNext = {};
	vk::IOSSurfaceCreateFlagsMVK flags = {};
	const void*    pView = {};
};
#endif // VK_USE_PLATFORM_IOS_MVK

#if defined(VK_USE_PLATFORM_MACOS_MVK)
struct MacOSSurfaceCreateInfoMVK {
	vk::StructureType sType = StructureType::eMacosSurfaceCreateInfoMVK;
	const void*    pNext = {};
	vk::MacOSSurfaceCreateFlagsMVK flags = {};
	const void*    pView = {};
};
#endif // VK_USE_PLATFORM_MACOS_MVK

struct PhysicalDeviceSurfaceInfo2KHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceSurfaceInfo2KHR;
	const void*    pNext = {};
	vk::SurfaceKHR surface = {};
};

struct SurfaceCapabilities2KHR {
	vk::StructureType sType = StructureType::eSurfaceCapabilities2KHR;
	void*    pNext = {};
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = {};
};

struct SurfaceFormat2KHR {
	vk::StructureType sType = StructureType::eSurfaceFormat2KHR;
	void*    pNext = {};
	vk::SurfaceFormatKHR surfaceFormat = {};
};

struct DisplayProperties2KHR {
	vk::StructureType sType = StructureType::eDisplayProperties2KHR;
	void*    pNext = {};
	vk::DisplayPropertiesKHR displayProperties = {};
};

struct DisplayPlaneProperties2KHR {
	vk::StructureType sType = StructureType::eDisplayPlaneProperties2KHR;
	void*    pNext = {};
	vk::DisplayPlanePropertiesKHR displayPlaneProperties = {};
};

struct DisplayModeProperties2KHR {
	vk::StructureType sType = StructureType::eDisplayModeProperties2KHR;
	void*    pNext = {};
	vk::DisplayModePropertiesKHR displayModeProperties = {};
};

struct DisplayPlaneInfo2KHR {
	vk::StructureType sType = StructureType::eDisplayPlaneInfo2KHR;
	const void*    pNext = {};
	vk::DisplayModeKHR mode = {};
	uint32_t    planeIndex = {};
};

struct DisplayPlaneCapabilities2KHR {
	vk::StructureType sType = StructureType::eDisplayPlaneCapabilities2KHR;
	void*    pNext = {};
	vk::DisplayPlaneCapabilitiesKHR capabilities = {};
};

struct SharedPresentSurfaceCapabilitiesKHR {
	vk::StructureType sType = StructureType::eSharedPresentSurfaceCapabilitiesKHR;
	void*    pNext = {};
	vk::ImageUsageFlags sharedPresentSupportedUsageFlags = {};
};

struct BufferMemoryRequirementsInfo2 {
	vk::StructureType sType = StructureType::eBufferMemoryRequirementsInfo2;
	const void*    pNext = {};
	vk::Buffer buffer = {};
};
using BufferMemoryRequirementsInfo2KHR = BufferMemoryRequirementsInfo2;

struct DeviceBufferMemoryRequirements {
	vk::StructureType sType = StructureType::eDeviceBufferMemoryRequirements;
	const void*    pNext = {};
	const vk::BufferCreateInfo* pCreateInfo = {};
};
using DeviceBufferMemoryRequirementsKHR = DeviceBufferMemoryRequirements;

struct ImageMemoryRequirementsInfo2 {
	vk::StructureType sType = StructureType::eImageMemoryRequirementsInfo2;
	const void*    pNext = {};
	vk::Image image = {};
};
using ImageMemoryRequirementsInfo2KHR = ImageMemoryRequirementsInfo2;

struct ImageSparseMemoryRequirementsInfo2 {
	vk::StructureType sType = StructureType::eImageSparseMemoryRequirementsInfo2;
	const void*    pNext = {};
	vk::Image image = {};
};
using ImageSparseMemoryRequirementsInfo2KHR = ImageSparseMemoryRequirementsInfo2;

struct DeviceImageMemoryRequirements {
	vk::StructureType sType = StructureType::eDeviceImageMemoryRequirements;
	const void*    pNext = {};
	const vk::ImageCreateInfo* pCreateInfo = {};
	vk::ImageAspectFlagBits planeAspect = {};
};
using DeviceImageMemoryRequirementsKHR = DeviceImageMemoryRequirements;

struct MemoryRequirements2 {
	vk::StructureType sType = StructureType::eMemoryRequirements2;
	void*    pNext = {};
	vk::MemoryRequirements memoryRequirements = {};
};
using MemoryRequirements2KHR = MemoryRequirements2;

struct SparseImageMemoryRequirements2 {
	vk::StructureType sType = StructureType::eSparseImageMemoryRequirements2;
	void*    pNext = {};
	vk::SparseImageMemoryRequirements memoryRequirements = {};
};
using SparseImageMemoryRequirements2KHR = SparseImageMemoryRequirements2;

struct SamplerYcbcrConversionCreateInfo {
	vk::StructureType sType = StructureType::eSamplerYcbcrConversionCreateInfo;
	const void*    pNext = {};
	vk::Format format = {};
	vk::SamplerYcbcrModelConversion ycbcrModel = {};
	vk::SamplerYcbcrRange ycbcrRange = {};
	vk::ComponentMapping components = {};
	vk::ChromaLocation xChromaOffset = {};
	vk::ChromaLocation yChromaOffset = {};
	vk::Filter chromaFilter = {};
	vk::Bool32    forceExplicitReconstruction = {};
};
using SamplerYcbcrConversionCreateInfoKHR = SamplerYcbcrConversionCreateInfo;

struct PhysicalDeviceMaintenance7FeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceMaintenance7FeaturesKHR;
	void*    pNext = {};
	vk::Bool32    maintenance7 = {};
};

struct PhysicalDeviceMaintenance7PropertiesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceMaintenance7PropertiesKHR;
	void*    pNext = {};
	vk::Bool32    robustFragmentShadingRateAttachmentAccess = {};
	vk::Bool32    separateDepthStencilAttachmentAccess = {};
	uint32_t    maxDescriptorSetTotalUniformBuffersDynamic = {};
	uint32_t    maxDescriptorSetTotalStorageBuffersDynamic = {};
	uint32_t    maxDescriptorSetTotalBuffersDynamic = {};
	uint32_t    maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic = {};
	uint32_t    maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic = {};
	uint32_t    maxDescriptorSetUpdateAfterBindTotalBuffersDynamic = {};
};

struct PhysicalDeviceLayeredApiPropertiesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceLayeredApiPropertiesKHR;
	void*    pNext = {};
	uint32_t    vendorID = {};
	uint32_t    deviceID = {};
	vk::PhysicalDeviceLayeredApiKHR layeredAPI = {};
	char    deviceName[MaxPhysicalDeviceNameSize] = {};
};

struct PhysicalDeviceLayeredApiPropertiesListKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceLayeredApiPropertiesListKHR;
	void*    pNext = {};
	uint32_t    layeredApiCount = {};
	vk::PhysicalDeviceLayeredApiPropertiesKHR* pLayeredApis = {};
};

struct RenderingAreaInfo {
	vk::StructureType sType = StructureType::eRenderingAreaInfo;
	const void*    pNext = {};
	uint32_t    viewMask = {};
	uint32_t    colorAttachmentCount = {};
	const vk::Format* pColorAttachmentFormats = {};
	vk::Format depthAttachmentFormat = {};
	vk::Format stencilAttachmentFormat = {};
};
using RenderingAreaInfoKHR = RenderingAreaInfo;

struct DescriptorSetLayoutSupport {
	vk::StructureType sType = StructureType::eDescriptorSetLayoutSupport;
	void*    pNext = {};
	vk::Bool32    supported = {};
};
using DescriptorSetLayoutSupportKHR = DescriptorSetLayoutSupport;

struct ShaderResourceUsageAMD {
	uint32_t    numUsedVgprs = {};
	uint32_t    numUsedSgprs = {};
	uint32_t    ldsSizePerLocalWorkGroup = {};
	size_t    ldsUsageSizeInBytes = {};
	size_t    scratchMemUsageInBytes = {};
};

struct ShaderStatisticsInfoAMD {
	vk::ShaderStageFlags shaderStageMask = {};
	vk::ShaderResourceUsageAMD resourceUsage = {};
	uint32_t    numPhysicalVgprs = {};
	uint32_t    numPhysicalSgprs = {};
	uint32_t    numAvailableVgprs = {};
	uint32_t    numAvailableSgprs = {};
	uint32_t    computeWorkGroupSize[3] = {};
};

struct DebugUtilsObjectNameInfoEXT {
	vk::StructureType sType = StructureType::eDebugUtilsObjectNameInfoEXT;
	const void*    pNext = {};
	vk::ObjectType objectType = {};
	uint64_t    objectHandle = {};
	const char*    pObjectName = {};
};

struct DebugUtilsObjectTagInfoEXT {
	vk::StructureType sType = StructureType::eDebugUtilsObjectTagInfoEXT;
	const void*    pNext = {};
	vk::ObjectType objectType = {};
	uint64_t    objectHandle = {};
	uint64_t    tagName = {};
	size_t    tagSize = {};
	const void*    pTag = {};
};

struct DebugUtilsLabelEXT {
	vk::StructureType sType = StructureType::eDebugUtilsLabelEXT;
	const void*    pNext = {};
	const char*    pLabelName = {};
	float    color[4] = {};
};

struct DebugUtilsMessengerCallbackDataEXT {
	vk::StructureType sType = StructureType::eDebugUtilsMessengerCallbackDataEXT;
	const void*    pNext = {};
	vk::DebugUtilsMessengerCallbackDataFlagsEXT flags = {};
	const char*    pMessageIdName = {};
	int32_t    messageIdNumber = {};
	const char*    pMessage = {};
	uint32_t    queueLabelCount = {};
	const vk::DebugUtilsLabelEXT* pQueueLabels = {};
	uint32_t    cmdBufLabelCount = {};
	const vk::DebugUtilsLabelEXT* pCmdBufLabels = {};
	uint32_t    objectCount = {};
	const vk::DebugUtilsObjectNameInfoEXT* pObjects = {};
};

using PFN_vkDebugUtilsMessengerCallbackEXT = Bool32 (VKAPI_PTR *)(
	DebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	DebugUtilsMessageTypeFlagsEXT                  messageTypes,
	const DebugUtilsMessengerCallbackDataEXT*      pCallbackData,
	void*                                          pUserData);

struct DebugUtilsMessengerCreateInfoEXT {
	vk::StructureType sType = StructureType::eDebugUtilsMessengerCreateInfoEXT;
	const void*    pNext = {};
	vk::DebugUtilsMessengerCreateFlagsEXT flags = {};
	vk::DebugUtilsMessageSeverityFlagsEXT messageSeverity = {};
	vk::DebugUtilsMessageTypeFlagsEXT messageType = {};
	PFN_vkDebugUtilsMessengerCallbackEXT    pfnUserCallback = {};
	void*    pUserData = {};
};

struct CalibratedTimestampInfoKHR {
	vk::StructureType sType = StructureType::eCalibratedTimestampInfoKHR;
	const void*    pNext = {};
	vk::TimeDomainKHR timeDomain = {};
};
using CalibratedTimestampInfoEXT = CalibratedTimestampInfoKHR;

struct PhysicalDeviceShaderCorePropertiesAMD {
	vk::StructureType sType = StructureType::ePhysicalDeviceShaderCorePropertiesAMD;
	void*    pNext = {};
	uint32_t    shaderEngineCount = {};
	uint32_t    shaderArraysPerEngineCount = {};
	uint32_t    computeUnitsPerShaderArray = {};
	uint32_t    simdPerComputeUnit = {};
	uint32_t    wavefrontsPerSimd = {};
	uint32_t    wavefrontSize = {};
	uint32_t    sgprsPerSimd = {};
	uint32_t    minSgprAllocation = {};
	uint32_t    maxSgprAllocation = {};
	uint32_t    sgprAllocationGranularity = {};
	uint32_t    vgprsPerSimd = {};
	uint32_t    minVgprAllocation = {};
	uint32_t    maxVgprAllocation = {};
	uint32_t    vgprAllocationGranularity = {};
};

struct PhysicalDeviceShaderCoreProperties2AMD {
	vk::StructureType sType = StructureType::ePhysicalDeviceShaderCoreProperties2AMD;
	void*    pNext = {};
	vk::ShaderCorePropertiesFlagsAMD shaderCoreFeatures = {};
	uint32_t    activeComputeUnitCount = {};
};

struct AttachmentDescription2 {
	vk::StructureType sType = StructureType::eAttachmentDescription2;
	const void*    pNext = {};
	vk::AttachmentDescriptionFlags flags = {};
	vk::Format format = {};
	vk::SampleCountFlagBits samples = {};
	vk::AttachmentLoadOp loadOp = {};
	vk::AttachmentStoreOp storeOp = {};
	vk::AttachmentLoadOp stencilLoadOp = {};
	vk::AttachmentStoreOp stencilStoreOp = {};
	vk::ImageLayout initialLayout = {};
	vk::ImageLayout finalLayout = {};
};
using AttachmentDescription2KHR = AttachmentDescription2;

struct AttachmentReference2 {
	vk::StructureType sType = StructureType::eAttachmentReference2;
	const void*    pNext = {};
	uint32_t    attachment = {};
	vk::ImageLayout layout = {};
	vk::ImageAspectFlags aspectMask = {};
};
using AttachmentReference2KHR = AttachmentReference2;

struct SubpassDescription2 {
	vk::StructureType sType = StructureType::eSubpassDescription2;
	const void*    pNext = {};
	vk::SubpassDescriptionFlags flags = {};
	vk::PipelineBindPoint pipelineBindPoint = {};
	uint32_t    viewMask = {};
	uint32_t    inputAttachmentCount = {};
	const vk::AttachmentReference2* pInputAttachments = {};
	uint32_t    colorAttachmentCount = {};
	const vk::AttachmentReference2* pColorAttachments = {};
	const vk::AttachmentReference2* pResolveAttachments = {};
	const vk::AttachmentReference2* pDepthStencilAttachment = {};
	uint32_t    preserveAttachmentCount = {};
	const uint32_t*    pPreserveAttachments = {};
};
using SubpassDescription2KHR = SubpassDescription2;

struct SubpassDependency2 {
	vk::StructureType sType = StructureType::eSubpassDependency2;
	const void*    pNext = {};
	uint32_t    srcSubpass = {};
	uint32_t    dstSubpass = {};
	vk::PipelineStageFlags srcStageMask = {};
	vk::PipelineStageFlags dstStageMask = {};
	vk::AccessFlags srcAccessMask = {};
	vk::AccessFlags dstAccessMask = {};
	vk::DependencyFlags dependencyFlags = {};
	int32_t    viewOffset = {};
};
using SubpassDependency2KHR = SubpassDependency2;

struct RenderPassCreateInfo2 {
	vk::StructureType sType = StructureType::eRenderPassCreateInfo2;
	const void*    pNext = {};
	vk::RenderPassCreateFlags flags = {};
	uint32_t    attachmentCount = {};
	const vk::AttachmentDescription2* pAttachments = {};
	uint32_t    subpassCount = {};
	const vk::SubpassDescription2* pSubpasses = {};
	uint32_t    dependencyCount = {};
	const vk::SubpassDependency2* pDependencies = {};
	uint32_t    correlatedViewMaskCount = {};
	const uint32_t*    pCorrelatedViewMasks = {};
};
using RenderPassCreateInfo2KHR = RenderPassCreateInfo2;

struct SubpassBeginInfo {
	vk::StructureType sType = StructureType::eSubpassBeginInfo;
	const void*    pNext = {};
	vk::SubpassContents contents = {};
};
using SubpassBeginInfoKHR = SubpassBeginInfo;

struct SubpassEndInfo {
	vk::StructureType sType = StructureType::eSubpassEndInfo;
	const void*    pNext = {};
};
using SubpassEndInfoKHR = SubpassEndInfo;

struct SemaphoreWaitInfo {
	vk::StructureType sType = StructureType::eSemaphoreWaitInfo;
	const void*    pNext = {};
	vk::SemaphoreWaitFlags flags = {};
	uint32_t    semaphoreCount = {};
	const vk::Semaphore* pSemaphores = {};
	const uint64_t*    pValues = {};
};
using SemaphoreWaitInfoKHR = SemaphoreWaitInfo;

struct SemaphoreSignalInfo {
	vk::StructureType sType = StructureType::eSemaphoreSignalInfo;
	const void*    pNext = {};
	vk::Semaphore semaphore = {};
	uint64_t    value = {};
};
using SemaphoreSignalInfoKHR = SemaphoreSignalInfo;

struct PhysicalDevicePCIBusInfoPropertiesEXT {
	vk::StructureType sType = StructureType::ePhysicalDevicePciBusInfoPropertiesEXT;
	void*    pNext = {};
	uint32_t    pciDomain = {};
	uint32_t    pciBus = {};
	uint32_t    pciDevice = {};
	uint32_t    pciFunction = {};
};

struct PhysicalDeviceComputeShaderDerivativesFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceComputeShaderDerivativesFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    computeDerivativeGroupQuads = {};
	vk::Bool32    computeDerivativeGroupLinear = {};
};

struct PhysicalDeviceComputeShaderDerivativesPropertiesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceComputeShaderDerivativesPropertiesKHR;
	void*    pNext = {};
	vk::Bool32    meshAndTaskShaderDerivatives = {};
};

struct PhysicalDeviceMeshShaderFeaturesEXT {
	vk::StructureType sType = StructureType::ePhysicalDeviceMeshShaderFeaturesEXT;
	void*    pNext = {};
	vk::Bool32    taskShader = {};
	vk::Bool32    meshShader = {};
	vk::Bool32    multiviewMeshShader = {};
	vk::Bool32    primitiveFragmentShadingRateMeshShader = {};
	vk::Bool32    meshShaderQueries = {};
};

struct PhysicalDeviceMeshShaderPropertiesEXT {
	vk::StructureType sType = StructureType::ePhysicalDeviceMeshShaderPropertiesEXT;
	void*    pNext = {};
	uint32_t    maxTaskWorkGroupTotalCount = {};
	uint32_t    maxTaskWorkGroupCount[3] = {};
	uint32_t    maxTaskWorkGroupInvocations = {};
	uint32_t    maxTaskWorkGroupSize[3] = {};
	uint32_t    maxTaskPayloadSize = {};
	uint32_t    maxTaskSharedMemorySize = {};
	uint32_t    maxTaskPayloadAndSharedMemorySize = {};
	uint32_t    maxMeshWorkGroupTotalCount = {};
	uint32_t    maxMeshWorkGroupCount[3] = {};
	uint32_t    maxMeshWorkGroupInvocations = {};
	uint32_t    maxMeshWorkGroupSize[3] = {};
	uint32_t    maxMeshSharedMemorySize = {};
	uint32_t    maxMeshPayloadAndSharedMemorySize = {};
	uint32_t    maxMeshOutputMemorySize = {};
	uint32_t    maxMeshPayloadAndOutputMemorySize = {};
	uint32_t    maxMeshOutputComponents = {};
	uint32_t    maxMeshOutputVertices = {};
	uint32_t    maxMeshOutputPrimitives = {};
	uint32_t    maxMeshOutputLayers = {};
	uint32_t    maxMeshMultiviewViewCount = {};
	uint32_t    meshOutputPerVertexGranularity = {};
	uint32_t    meshOutputPerPrimitiveGranularity = {};
	uint32_t    maxPreferredTaskWorkGroupInvocations = {};
	uint32_t    maxPreferredMeshWorkGroupInvocations = {};
	vk::Bool32    prefersLocalInvocationVertexOutput = {};
	vk::Bool32    prefersLocalInvocationPrimitiveOutput = {};
	vk::Bool32    prefersCompactVertexOutput = {};
	vk::Bool32    prefersCompactPrimitiveOutput = {};
};

struct DrawMeshTasksIndirectCommandEXT {
	uint32_t    groupCountX = {};
	uint32_t    groupCountY = {};
	uint32_t    groupCountZ = {};
};

struct RayTracingShaderGroupCreateInfoKHR {
	vk::StructureType sType = StructureType::eRayTracingShaderGroupCreateInfoKHR;
	const void*    pNext = {};
	vk::RayTracingShaderGroupTypeKHR type = {};
	uint32_t    generalShader = {};
	uint32_t    closestHitShader = {};
	uint32_t    anyHitShader = {};
	uint32_t    intersectionShader = {};
	const void*    pShaderGroupCaptureReplayHandle = {};
};

struct PipelineLibraryCreateInfoKHR {
	vk::StructureType sType = StructureType::ePipelineLibraryCreateInfoKHR;
	const void*    pNext = {};
	uint32_t    libraryCount = {};
	const vk::Pipeline* pLibraries = {};
};

struct RayTracingPipelineInterfaceCreateInfoKHR {
	vk::StructureType sType = StructureType::eRayTracingPipelineInterfaceCreateInfoKHR;
	const void*    pNext = {};
	uint32_t    maxPipelineRayPayloadSize = {};
	uint32_t    maxPipelineRayHitAttributeSize = {};
};

struct RayTracingPipelineCreateInfoKHR {
	vk::StructureType sType = StructureType::eRayTracingPipelineCreateInfoKHR;
	const void*    pNext = {};
	vk::PipelineCreateFlags flags = {};
	uint32_t    stageCount = {};
	const vk::PipelineShaderStageCreateInfo* pStages = {};
	uint32_t    groupCount = {};
	const vk::RayTracingShaderGroupCreateInfoKHR* pGroups = {};
	uint32_t    maxPipelineRayRecursionDepth = {};
	const vk::PipelineLibraryCreateInfoKHR* pLibraryInfo = {};
	const vk::RayTracingPipelineInterfaceCreateInfoKHR* pLibraryInterface = {};
	const vk::PipelineDynamicStateCreateInfo* pDynamicState = {};
	vk::PipelineLayout layout = {};
	vk::Pipeline basePipelineHandle = {};
	int32_t    basePipelineIndex = {};
};

struct WriteDescriptorSetAccelerationStructureKHR {
	vk::StructureType sType = StructureType::eWriteDescriptorSetAccelerationStructureKHR;
	const void*    pNext = {};
	uint32_t    accelerationStructureCount = {};
	const vk::AccelerationStructureKHR* pAccelerationStructures = {};
};

struct PhysicalDeviceAccelerationStructureFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceAccelerationStructureFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    accelerationStructure = {};
	vk::Bool32    accelerationStructureCaptureReplay = {};
	vk::Bool32    accelerationStructureIndirectBuild = {};
	vk::Bool32    accelerationStructureHostCommands = {};
	vk::Bool32    descriptorBindingAccelerationStructureUpdateAfterBind = {};
};

struct PhysicalDeviceRayTracingPipelineFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceRayTracingPipelineFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    rayTracingPipeline = {};
	vk::Bool32    rayTracingPipelineShaderGroupHandleCaptureReplay = {};
	vk::Bool32    rayTracingPipelineShaderGroupHandleCaptureReplayMixed = {};
	vk::Bool32    rayTracingPipelineTraceRaysIndirect = {};
	vk::Bool32    rayTraversalPrimitiveCulling = {};
};

struct PhysicalDeviceRayQueryFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceRayQueryFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    rayQuery = {};
};

struct PhysicalDeviceAccelerationStructurePropertiesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceAccelerationStructurePropertiesKHR;
	void*    pNext = {};
	uint64_t    maxGeometryCount = {};
	uint64_t    maxInstanceCount = {};
	uint64_t    maxPrimitiveCount = {};
	uint32_t    maxPerStageDescriptorAccelerationStructures = {};
	uint32_t    maxPerStageDescriptorUpdateAfterBindAccelerationStructures = {};
	uint32_t    maxDescriptorSetAccelerationStructures = {};
	uint32_t    maxDescriptorSetUpdateAfterBindAccelerationStructures = {};
	uint32_t    minAccelerationStructureScratchOffsetAlignment = {};
};

struct PhysicalDeviceRayTracingPipelinePropertiesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceRayTracingPipelinePropertiesKHR;
	void*    pNext = {};
	uint32_t    shaderGroupHandleSize = {};
	uint32_t    maxRayRecursionDepth = {};
	uint32_t    maxShaderGroupStride = {};
	uint32_t    shaderGroupBaseAlignment = {};
	uint32_t    shaderGroupHandleCaptureReplaySize = {};
	uint32_t    maxRayDispatchInvocationCount = {};
	uint32_t    shaderGroupHandleAlignment = {};
	uint32_t    maxRayHitAttributeSize = {};
};

struct StridedDeviceAddressRegionKHR {
	vk::DeviceAddress    deviceAddress = {};
	vk::DeviceSize    stride = {};
	vk::DeviceSize    size = {};
};

struct TraceRaysIndirectCommandKHR {
	uint32_t    width = {};
	uint32_t    height = {};
	uint32_t    depth = {};
};

struct TraceRaysIndirectCommand2KHR {
	vk::DeviceAddress    raygenShaderRecordAddress = {};
	vk::DeviceSize    raygenShaderRecordSize = {};
	vk::DeviceAddress    missShaderBindingTableAddress = {};
	vk::DeviceSize    missShaderBindingTableSize = {};
	vk::DeviceSize    missShaderBindingTableStride = {};
	vk::DeviceAddress    hitShaderBindingTableAddress = {};
	vk::DeviceSize    hitShaderBindingTableSize = {};
	vk::DeviceSize    hitShaderBindingTableStride = {};
	vk::DeviceAddress    callableShaderBindingTableAddress = {};
	vk::DeviceSize    callableShaderBindingTableSize = {};
	vk::DeviceSize    callableShaderBindingTableStride = {};
	uint32_t    width = {};
	uint32_t    height = {};
	uint32_t    depth = {};
};

struct PhysicalDeviceRayTracingMaintenance1FeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceRayTracingMaintenance1FeaturesKHR;
	void*    pNext = {};
	vk::Bool32    rayTracingMaintenance1 = {};
	vk::Bool32    rayTracingPipelineTraceRaysIndirect2 = {};
};

struct SurfaceProtectedCapabilitiesKHR {
	vk::StructureType sType = StructureType::eSurfaceProtectedCapabilitiesKHR;
	const void*    pNext = {};
	vk::Bool32    supportsProtected = {};
};

struct PhysicalDeviceMemoryBudgetPropertiesEXT {
	vk::StructureType sType = StructureType::ePhysicalDeviceMemoryBudgetPropertiesEXT;
	void*    pNext = {};
	vk::DeviceSize    heapBudget[MaxMemoryHeaps] = {};
	vk::DeviceSize    heapUsage[MaxMemoryHeaps] = {};
};

struct BufferDeviceAddressInfo {
	vk::StructureType sType = StructureType::eBufferDeviceAddressInfo;
	const void*    pNext = {};
	vk::Buffer buffer = {};
};
using BufferDeviceAddressInfoKHR = BufferDeviceAddressInfo;
using BufferDeviceAddressInfoEXT = BufferDeviceAddressInfo;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct SurfaceFullScreenExclusiveInfoEXT {
	vk::StructureType sType = StructureType::eSurfaceFullScreenExclusiveInfoEXT;
	void*    pNext = {};
	vk::FullScreenExclusiveEXT fullScreenExclusive = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct SurfaceFullScreenExclusiveWin32InfoEXT {
	vk::StructureType sType = StructureType::eSurfaceFullScreenExclusiveWin32InfoEXT;
	const void*    pNext = {};
	HMONITOR    hmonitor = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
struct SurfaceCapabilitiesFullScreenExclusiveEXT {
	vk::StructureType sType = StructureType::eSurfaceCapabilitiesFullScreenExclusiveEXT;
	void*    pNext = {};
	vk::Bool32    fullScreenExclusiveSupported = {};
};
#endif // VK_USE_PLATFORM_WIN32_KHR

struct PhysicalDevicePerformanceQueryFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDevicePerformanceQueryFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    performanceCounterQueryPools = {};
	vk::Bool32    performanceCounterMultipleQueryPools = {};
};

struct PhysicalDevicePerformanceQueryPropertiesKHR {
	vk::StructureType sType = StructureType::ePhysicalDevicePerformanceQueryPropertiesKHR;
	void*    pNext = {};
	vk::Bool32    allowCommandBufferQueryCopies = {};
};

struct PerformanceCounterKHR {
	vk::StructureType sType = StructureType::ePerformanceCounterKHR;
	void*    pNext = {};
	vk::PerformanceCounterUnitKHR unit = {};
	vk::PerformanceCounterScopeKHR scope = {};
	vk::PerformanceCounterStorageKHR storage = {};
	uint8_t    uuid[UuidSize] = {};
};

struct PerformanceCounterDescriptionKHR {
	vk::StructureType sType = StructureType::ePerformanceCounterDescriptionKHR;
	void*    pNext = {};
	vk::PerformanceCounterDescriptionFlagsKHR flags = {};
	char    name[MaxDescriptionSize] = {};
	char    category[MaxDescriptionSize] = {};
	char    description[MaxDescriptionSize] = {};
};

struct QueryPoolPerformanceCreateInfoKHR {
	vk::StructureType sType = StructureType::eQueryPoolPerformanceCreateInfoKHR;
	const void*    pNext = {};
	uint32_t    queueFamilyIndex = {};
	uint32_t    counterIndexCount = {};
	const uint32_t*    pCounterIndices = {};
};

union PerformanceCounterResultKHR {
	int32_t    int32;
	int64_t    int64;
	uint32_t    uint32;
	uint64_t    uint64;
	float    float32;
	double    float64;
};

struct AcquireProfilingLockInfoKHR {
	vk::StructureType sType = StructureType::eAcquireProfilingLockInfoKHR;
	const void*    pNext = {};
	vk::AcquireProfilingLockFlagsKHR flags = {};
	uint64_t    timeout = {};
};

struct PerformanceQuerySubmitInfoKHR {
	vk::StructureType sType = StructureType::ePerformanceQuerySubmitInfoKHR;
	const void*    pNext = {};
	uint32_t    counterPassIndex = {};
};

struct PhysicalDeviceShaderClockFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceShaderClockFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    shaderSubgroupClock = {};
	vk::Bool32    shaderDeviceClock = {};
};

struct PhysicalDeviceShaderSMBuiltinsPropertiesNV {
	vk::StructureType sType = StructureType::ePhysicalDeviceShaderSmBuiltinsPropertiesNV;
	void*    pNext = {};
	uint32_t    shaderSMCount = {};
	uint32_t    shaderWarpsPerSM = {};
};

struct PhysicalDeviceShaderSMBuiltinsFeaturesNV {
	vk::StructureType sType = StructureType::ePhysicalDeviceShaderSmBuiltinsFeaturesNV;
	void*    pNext = {};
	vk::Bool32    shaderSMBuiltins = {};
};

struct PhysicalDevicePipelineExecutablePropertiesFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDevicePipelineExecutablePropertiesFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    pipelineExecutableInfo = {};
};

struct PipelineInfoKHR {
	vk::StructureType sType = StructureType::ePipelineInfoKHR;
	const void*    pNext = {};
	vk::Pipeline pipeline = {};
};
using PipelineInfoEXT = PipelineInfoKHR;

struct PipelineExecutablePropertiesKHR {
	vk::StructureType sType = StructureType::ePipelineExecutablePropertiesKHR;
	void*    pNext = {};
	vk::ShaderStageFlags stages = {};
	char    name[MaxDescriptionSize] = {};
	char    description[MaxDescriptionSize] = {};
	uint32_t    subgroupSize = {};
};

struct PipelineExecutableInfoKHR {
	vk::StructureType sType = StructureType::ePipelineExecutableInfoKHR;
	const void*    pNext = {};
	vk::Pipeline pipeline = {};
	uint32_t    executableIndex = {};
};

union PipelineExecutableStatisticValueKHR {
	vk::Bool32    b32;
	int64_t    i64;
	uint64_t    u64;
	double    f64;
};

struct PipelineExecutableStatisticKHR {
	vk::StructureType sType = StructureType::ePipelineExecutableStatisticKHR;
	void*    pNext = {};
	char    name[MaxDescriptionSize] = {};
	char    description[MaxDescriptionSize] = {};
	vk::PipelineExecutableStatisticFormatKHR format = {};
	vk::PipelineExecutableStatisticValueKHR value = {};
};

struct PipelineExecutableInternalRepresentationKHR {
	vk::StructureType sType = StructureType::ePipelineExecutableInternalRepresentationKHR;
	void*    pNext = {};
	char    name[MaxDescriptionSize] = {};
	char    description[MaxDescriptionSize] = {};
	vk::Bool32    isText = {};
	size_t    dataSize = {};
	void*    pData = {};
};

struct DeviceMemoryOpaqueCaptureAddressInfo {
	vk::StructureType sType = StructureType::eDeviceMemoryOpaqueCaptureAddressInfo;
	const void*    pNext = {};
	vk::DeviceMemory memory = {};
};
using DeviceMemoryOpaqueCaptureAddressInfoKHR = DeviceMemoryOpaqueCaptureAddressInfo;

struct PipelineCompilerControlCreateInfoAMD {
	vk::StructureType sType = StructureType::ePipelineCompilerControlCreateInfoAMD;
	const void*    pNext = {};
	vk::PipelineCompilerControlFlagsAMD compilerControlFlags = {};
};

struct PhysicalDeviceToolProperties {
	vk::StructureType sType = StructureType::ePhysicalDeviceToolProperties;
	void*    pNext = {};
	char    name[MaxExtensionNameSize] = {};
	char    version[MaxExtensionNameSize] = {};
	vk::ToolPurposeFlags purposes = {};
	char    description[MaxDescriptionSize] = {};
	char    layer[MaxExtensionNameSize] = {};
};
using PhysicalDeviceToolPropertiesEXT = PhysicalDeviceToolProperties;

union DeviceOrHostAddressKHR {
	vk::DeviceAddress    deviceAddress;
	void*    hostAddress;
};

union DeviceOrHostAddressConstKHR {
	vk::DeviceAddress    deviceAddress;
	const void*    hostAddress;
};

struct AccelerationStructureGeometryTrianglesDataKHR {
	vk::StructureType sType = StructureType::eAccelerationStructureGeometryTrianglesDataKHR;
	const void*    pNext = {};
	vk::Format vertexFormat = {};
	vk::DeviceOrHostAddressConstKHR vertexData = {};
	vk::DeviceSize    vertexStride = {};
	uint32_t    maxVertex = {};
	vk::IndexType indexType = {};
	vk::DeviceOrHostAddressConstKHR indexData = {};
	vk::DeviceOrHostAddressConstKHR transformData = {};
};

struct AccelerationStructureGeometryAabbsDataKHR {
	vk::StructureType sType = StructureType::eAccelerationStructureGeometryAabbsDataKHR;
	const void*    pNext = {};
	vk::DeviceOrHostAddressConstKHR data = {};
	vk::DeviceSize    stride = {};
};

struct AccelerationStructureGeometryInstancesDataKHR {
	vk::StructureType sType = StructureType::eAccelerationStructureGeometryInstancesDataKHR;
	const void*    pNext = {};
	vk::Bool32    arrayOfPointers = {};
	vk::DeviceOrHostAddressConstKHR data = {};
};

union AccelerationStructureGeometryDataKHR {
	vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
	vk::AccelerationStructureGeometryAabbsDataKHR aabbs;
	vk::AccelerationStructureGeometryInstancesDataKHR instances;
};

struct AccelerationStructureGeometryKHR {
	vk::StructureType sType = StructureType::eAccelerationStructureGeometryKHR;
	const void*    pNext = {};
	vk::GeometryTypeKHR geometryType = {};
	vk::AccelerationStructureGeometryDataKHR geometry = {};
	vk::GeometryFlagsKHR flags = {};
};

struct AccelerationStructureBuildGeometryInfoKHR {
	vk::StructureType sType = StructureType::eAccelerationStructureBuildGeometryInfoKHR;
	const void*    pNext = {};
	vk::AccelerationStructureTypeKHR type = {};
	vk::BuildAccelerationStructureFlagsKHR flags = {};
	vk::BuildAccelerationStructureModeKHR mode = {};
	vk::AccelerationStructureKHR srcAccelerationStructure = {};
	vk::AccelerationStructureKHR dstAccelerationStructure = {};
	uint32_t    geometryCount = {};
	const vk::AccelerationStructureGeometryKHR* pGeometries = {};
	const vk::AccelerationStructureGeometryKHR* const* ppGeometries = {};
	vk::DeviceOrHostAddressKHR scratchData = {};
};

struct AccelerationStructureBuildRangeInfoKHR {
	uint32_t    primitiveCount = {};
	uint32_t    primitiveOffset = {};
	uint32_t    firstVertex = {};
	uint32_t    transformOffset = {};
};

struct AccelerationStructureCreateInfoKHR {
	vk::StructureType sType = StructureType::eAccelerationStructureCreateInfoKHR;
	const void*    pNext = {};
	vk::AccelerationStructureCreateFlagsKHR createFlags = {};
	vk::Buffer buffer = {};
	vk::DeviceSize    offset = {};
	vk::DeviceSize    size = {};
	vk::AccelerationStructureTypeKHR type = {};
	vk::DeviceAddress    deviceAddress = {};
};

struct AabbPositionsKHR {
	float    minX = {};
	float    minY = {};
	float    minZ = {};
	float    maxX = {};
	float    maxY = {};
	float    maxZ = {};
};

struct TransformMatrixKHR {
	float    matrix[3][4] = {};
};

struct AccelerationStructureInstanceKHR {
	vk::TransformMatrixKHR transform = {};
	uint32_t    instanceCustomIndex:24 = {};
	uint32_t    mask:8 = {};
	uint32_t    instanceShaderBindingTableRecordOffset:24 = {};
	uint32_t    flags:8 = {};  // uint32_t instead of GeometryInstanceFlagsKHR, because of :8
	uint64_t    accelerationStructureReference = {};
};

struct AccelerationStructureDeviceAddressInfoKHR {
	vk::StructureType sType = StructureType::eAccelerationStructureDeviceAddressInfoKHR;
	const void*    pNext = {};
	vk::AccelerationStructureKHR accelerationStructure = {};
};

struct AccelerationStructureVersionInfoKHR {
	vk::StructureType sType = StructureType::eAccelerationStructureVersionInfoKHR;
	const void*    pNext = {};
	const uint8_t*    pVersionData = {};
};

struct CopyAccelerationStructureInfoKHR {
	vk::StructureType sType = StructureType::eCopyAccelerationStructureInfoKHR;
	const void*    pNext = {};
	vk::AccelerationStructureKHR src = {};
	vk::AccelerationStructureKHR dst = {};
	vk::CopyAccelerationStructureModeKHR mode = {};
};

struct CopyAccelerationStructureToMemoryInfoKHR {
	vk::StructureType sType = StructureType::eCopyAccelerationStructureToMemoryInfoKHR;
	const void*    pNext = {};
	vk::AccelerationStructureKHR src = {};
	vk::DeviceOrHostAddressKHR dst = {};
	vk::CopyAccelerationStructureModeKHR mode = {};
};

struct CopyMemoryToAccelerationStructureInfoKHR {
	vk::StructureType sType = StructureType::eCopyMemoryToAccelerationStructureInfoKHR;
	const void*    pNext = {};
	vk::DeviceOrHostAddressConstKHR src = {};
	vk::AccelerationStructureKHR dst = {};
	vk::CopyAccelerationStructureModeKHR mode = {};
};

struct PhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    shaderSubgroupUniformControlFlow = {};
};

struct PhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    workgroupMemoryExplicitLayout = {};
	vk::Bool32    workgroupMemoryExplicitLayoutScalarBlockLayout = {};
	vk::Bool32    workgroupMemoryExplicitLayout8BitAccess = {};
	vk::Bool32    workgroupMemoryExplicitLayout16BitAccess = {};
};

#if defined(VK_ENABLE_BETA_EXTENSIONS)
struct PhysicalDevicePortabilitySubsetFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDevicePortabilitySubsetFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    constantAlphaColorBlendFactors = {};
	vk::Bool32    events = {};
	vk::Bool32    imageViewFormatReinterpretation = {};
	vk::Bool32    imageViewFormatSwizzle = {};
	vk::Bool32    imageView2DOn3DImage = {};
	vk::Bool32    multisampleArrayImage = {};
	vk::Bool32    mutableComparisonSamplers = {};
	vk::Bool32    pointPolygons = {};
	vk::Bool32    samplerMipLodBias = {};
	vk::Bool32    separateStencilMaskRef = {};
	vk::Bool32    shaderSampleRateInterpolationFunctions = {};
	vk::Bool32    tessellationIsolines = {};
	vk::Bool32    tessellationPointMode = {};
	vk::Bool32    triangleFans = {};
	vk::Bool32    vertexAttributeAccessBeyondStride = {};
};
#endif // VK_ENABLE_BETA_EXTENSIONS

#if defined(VK_ENABLE_BETA_EXTENSIONS)
struct PhysicalDevicePortabilitySubsetPropertiesKHR {
	vk::StructureType sType = StructureType::ePhysicalDevicePortabilitySubsetPropertiesKHR;
	void*    pNext = {};
	uint32_t    minVertexInputBindingStrideAlignment = {};
};
#endif // VK_ENABLE_BETA_EXTENSIONS

struct BufferCopy2 {
	vk::StructureType sType = StructureType::eBufferCopy2;
	const void*    pNext = {};
	vk::DeviceSize    srcOffset = {};
	vk::DeviceSize    dstOffset = {};
	vk::DeviceSize    size = {};
};
using BufferCopy2KHR = BufferCopy2;

struct ImageCopy2 {
	vk::StructureType sType = StructureType::eImageCopy2;
	const void*    pNext = {};
	vk::ImageSubresourceLayers srcSubresource = {};
	vk::Offset3D srcOffset = {};
	vk::ImageSubresourceLayers dstSubresource = {};
	vk::Offset3D dstOffset = {};
	vk::Extent3D extent = {};
};
using ImageCopy2KHR = ImageCopy2;

struct ImageBlit2 {
	vk::StructureType sType = StructureType::eImageBlit2;
	const void*    pNext = {};
	vk::ImageSubresourceLayers srcSubresource = {};
	vk::Offset3D    srcOffsets[2] = {};
	vk::ImageSubresourceLayers dstSubresource = {};
	vk::Offset3D    dstOffsets[2] = {};
};
using ImageBlit2KHR = ImageBlit2;

struct BufferImageCopy2 {
	vk::StructureType sType = StructureType::eBufferImageCopy2;
	const void*    pNext = {};
	vk::DeviceSize    bufferOffset = {};
	uint32_t    bufferRowLength = {};
	uint32_t    bufferImageHeight = {};
	vk::ImageSubresourceLayers imageSubresource = {};
	vk::Offset3D imageOffset = {};
	vk::Extent3D imageExtent = {};
};
using BufferImageCopy2KHR = BufferImageCopy2;

struct ImageResolve2 {
	vk::StructureType sType = StructureType::eImageResolve2;
	const void*    pNext = {};
	vk::ImageSubresourceLayers srcSubresource = {};
	vk::Offset3D srcOffset = {};
	vk::ImageSubresourceLayers dstSubresource = {};
	vk::Offset3D dstOffset = {};
	vk::Extent3D extent = {};
};
using ImageResolve2KHR = ImageResolve2;

struct CopyBufferInfo2 {
	vk::StructureType sType = StructureType::eCopyBufferInfo2;
	const void*    pNext = {};
	vk::Buffer srcBuffer = {};
	vk::Buffer dstBuffer = {};
	uint32_t    regionCount = {};
	const vk::BufferCopy2* pRegions = {};
};
using CopyBufferInfo2KHR = CopyBufferInfo2;

struct CopyImageInfo2 {
	vk::StructureType sType = StructureType::eCopyImageInfo2;
	const void*    pNext = {};
	vk::Image srcImage = {};
	vk::ImageLayout srcImageLayout = {};
	vk::Image dstImage = {};
	vk::ImageLayout dstImageLayout = {};
	uint32_t    regionCount = {};
	const vk::ImageCopy2* pRegions = {};
};
using CopyImageInfo2KHR = CopyImageInfo2;

struct BlitImageInfo2 {
	vk::StructureType sType = StructureType::eBlitImageInfo2;
	const void*    pNext = {};
	vk::Image srcImage = {};
	vk::ImageLayout srcImageLayout = {};
	vk::Image dstImage = {};
	vk::ImageLayout dstImageLayout = {};
	uint32_t    regionCount = {};
	const vk::ImageBlit2* pRegions = {};
	vk::Filter filter = {};
};
using BlitImageInfo2KHR = BlitImageInfo2;

struct CopyBufferToImageInfo2 {
	vk::StructureType sType = StructureType::eCopyBufferToImageInfo2;
	const void*    pNext = {};
	vk::Buffer srcBuffer = {};
	vk::Image dstImage = {};
	vk::ImageLayout dstImageLayout = {};
	uint32_t    regionCount = {};
	const vk::BufferImageCopy2* pRegions = {};
};
using CopyBufferToImageInfo2KHR = CopyBufferToImageInfo2;

struct CopyImageToBufferInfo2 {
	vk::StructureType sType = StructureType::eCopyImageToBufferInfo2;
	const void*    pNext = {};
	vk::Image srcImage = {};
	vk::ImageLayout srcImageLayout = {};
	vk::Buffer dstBuffer = {};
	uint32_t    regionCount = {};
	const vk::BufferImageCopy2* pRegions = {};
};
using CopyImageToBufferInfo2KHR = CopyImageToBufferInfo2;

struct ResolveImageInfo2 {
	vk::StructureType sType = StructureType::eResolveImageInfo2;
	const void*    pNext = {};
	vk::Image srcImage = {};
	vk::ImageLayout srcImageLayout = {};
	vk::Image dstImage = {};
	vk::ImageLayout dstImageLayout = {};
	uint32_t    regionCount = {};
	const vk::ImageResolve2* pRegions = {};
};
using ResolveImageInfo2KHR = ResolveImageInfo2;

struct FragmentShadingRateAttachmentInfoKHR {
	vk::StructureType sType = StructureType::eFragmentShadingRateAttachmentInfoKHR;
	const void*    pNext = {};
	const vk::AttachmentReference2* pFragmentShadingRateAttachment = {};
	vk::Extent2D shadingRateAttachmentTexelSize = {};
};

struct PipelineFragmentShadingRateStateCreateInfoKHR {
	vk::StructureType sType = StructureType::ePipelineFragmentShadingRateStateCreateInfoKHR;
	const void*    pNext = {};
	vk::Extent2D fragmentSize = {};
	vk::FragmentShadingRateCombinerOpKHR    combinerOps[2] = {};
};

struct PhysicalDeviceFragmentShadingRateFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceFragmentShadingRateFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    pipelineFragmentShadingRate = {};
	vk::Bool32    primitiveFragmentShadingRate = {};
	vk::Bool32    attachmentFragmentShadingRate = {};
};

struct PhysicalDeviceFragmentShadingRatePropertiesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceFragmentShadingRatePropertiesKHR;
	void*    pNext = {};
	vk::Extent2D minFragmentShadingRateAttachmentTexelSize = {};
	vk::Extent2D maxFragmentShadingRateAttachmentTexelSize = {};
	uint32_t    maxFragmentShadingRateAttachmentTexelSizeAspectRatio = {};
	vk::Bool32    primitiveFragmentShadingRateWithMultipleViewports = {};
	vk::Bool32    layeredShadingRateAttachments = {};
	vk::Bool32    fragmentShadingRateNonTrivialCombinerOps = {};
	vk::Extent2D maxFragmentSize = {};
	uint32_t    maxFragmentSizeAspectRatio = {};
	uint32_t    maxFragmentShadingRateCoverageSamples = {};
	vk::SampleCountFlagBits maxFragmentShadingRateRasterizationSamples = {};
	vk::Bool32    fragmentShadingRateWithShaderDepthStencilWrites = {};
	vk::Bool32    fragmentShadingRateWithSampleMask = {};
	vk::Bool32    fragmentShadingRateWithShaderSampleMask = {};
	vk::Bool32    fragmentShadingRateWithConservativeRasterization = {};
	vk::Bool32    fragmentShadingRateWithFragmentShaderInterlock = {};
	vk::Bool32    fragmentShadingRateWithCustomSampleLocations = {};
	vk::Bool32    fragmentShadingRateStrictMultiplyCombiner = {};
};

struct PhysicalDeviceFragmentShadingRateKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceFragmentShadingRateKHR;
	void*    pNext = {};
	vk::SampleCountFlags sampleCounts = {};
	vk::Extent2D fragmentSize = {};
};

struct AccelerationStructureBuildSizesInfoKHR {
	vk::StructureType sType = StructureType::eAccelerationStructureBuildSizesInfoKHR;
	const void*    pNext = {};
	vk::DeviceSize    accelerationStructureSize = {};
	vk::DeviceSize    updateScratchSize = {};
	vk::DeviceSize    buildScratchSize = {};
};

struct PhysicalDeviceDeviceGeneratedCommandsFeaturesEXT {
	vk::StructureType sType = StructureType::ePhysicalDeviceDeviceGeneratedCommandsFeaturesEXT;
	void*    pNext = {};
	vk::Bool32    deviceGeneratedCommands = {};
	vk::Bool32    dynamicGeneratedPipelineLayout = {};
};

enum class IndirectCommandsInputModeFlagBitsEXT : uint32_t {
	eVulkanIndexBuffer = 0x1,
	eDxgiIndexBuffer = 0x2,
};
using IndirectCommandsInputModeFlagsEXT = Flags<IndirectCommandsInputModeFlagBitsEXT>;

struct PhysicalDeviceDeviceGeneratedCommandsPropertiesEXT {
	vk::StructureType sType = StructureType::ePhysicalDeviceDeviceGeneratedCommandsPropertiesEXT;
	void*    pNext = {};
	uint32_t    maxIndirectPipelineCount = {};
	uint32_t    maxIndirectShaderObjectCount = {};
	uint32_t    maxIndirectSequenceCount = {};
	uint32_t    maxIndirectCommandsTokenCount = {};
	uint32_t    maxIndirectCommandsTokenOffset = {};
	uint32_t    maxIndirectCommandsIndirectStride = {};
	vk::IndirectCommandsInputModeFlagsEXT supportedIndirectCommandsInputModes = {};
	vk::ShaderStageFlags supportedIndirectCommandsShaderStages = {};
	vk::ShaderStageFlags supportedIndirectCommandsShaderStagesPipelineBinding = {};
	vk::ShaderStageFlags supportedIndirectCommandsShaderStagesShaderBinding = {};
	vk::Bool32    deviceGeneratedCommandsTransformFeedback = {};
	vk::Bool32    deviceGeneratedCommandsMultiDrawIndirectCount = {};
};

struct GeneratedCommandsPipelineInfoEXT {
	vk::StructureType sType = StructureType::eGeneratedCommandsPipelineInfoEXT;
	void*    pNext = {};
	vk::Pipeline pipeline = {};
};

struct GeneratedCommandsShaderInfoEXT {
	vk::StructureType sType = StructureType::eGeneratedCommandsShaderInfoEXT;
	void*    pNext = {};
	uint32_t    shaderCount = {};
	const vk::ShaderEXT* pShaders = {};
};

struct GeneratedCommandsMemoryRequirementsInfoEXT {
	vk::StructureType sType = StructureType::eGeneratedCommandsMemoryRequirementsInfoEXT;
	const void*    pNext = {};
	vk::IndirectExecutionSetEXT indirectExecutionSet = {};
	vk::IndirectCommandsLayoutEXT indirectCommandsLayout = {};
	uint32_t    maxSequenceCount = {};
	uint32_t    maxDrawCount = {};
};

struct IndirectExecutionSetPipelineInfoEXT {
	vk::StructureType sType = StructureType::eIndirectExecutionSetPipelineInfoEXT;
	const void*    pNext = {};
	vk::Pipeline initialPipeline = {};
	uint32_t    maxPipelineCount = {};
};

struct IndirectExecutionSetShaderLayoutInfoEXT {
	vk::StructureType sType = StructureType::eIndirectExecutionSetShaderLayoutInfoEXT;
	const void*    pNext = {};
	uint32_t    setLayoutCount = {};
	const vk::DescriptorSetLayout* pSetLayouts = {};
};

struct IndirectExecutionSetShaderInfoEXT {
	vk::StructureType sType = StructureType::eIndirectExecutionSetShaderInfoEXT;
	const void*    pNext = {};
	uint32_t    shaderCount = {};
	const vk::ShaderEXT* pInitialShaders = {};
	const vk::IndirectExecutionSetShaderLayoutInfoEXT* pSetLayoutInfos = {};
	uint32_t    maxShaderCount = {};
	uint32_t    pushConstantRangeCount = {};
	const vk::PushConstantRange* pPushConstantRanges = {};
};

union IndirectExecutionSetInfoEXT {
	const vk::IndirectExecutionSetPipelineInfoEXT* pPipelineInfo;
	const vk::IndirectExecutionSetShaderInfoEXT* pShaderInfo;
};

struct IndirectExecutionSetCreateInfoEXT {
	vk::StructureType sType = StructureType::eIndirectExecutionSetCreateInfoEXT;
	const void*    pNext = {};
	vk::IndirectExecutionSetInfoTypeEXT type = {};
	vk::IndirectExecutionSetInfoEXT info = {};
};

struct GeneratedCommandsInfoEXT {
	vk::StructureType sType = StructureType::eGeneratedCommandsInfoEXT;
	const void*    pNext = {};
	vk::ShaderStageFlags shaderStages = {};
	vk::IndirectExecutionSetEXT indirectExecutionSet = {};
	vk::IndirectCommandsLayoutEXT indirectCommandsLayout = {};
	vk::DeviceAddress    indirectAddress = {};
	vk::DeviceSize    indirectAddressSize = {};
	vk::DeviceAddress    preprocessAddress = {};
	vk::DeviceSize    preprocessSize = {};
	uint32_t    maxSequenceCount = {};
	vk::DeviceAddress    sequenceCountAddress = {};
	uint32_t    maxDrawCount = {};
};

struct WriteIndirectExecutionSetPipelineEXT {
	vk::StructureType sType = StructureType::eWriteIndirectExecutionSetPipelineEXT;
	const void*    pNext = {};
	uint32_t    index = {};
	vk::Pipeline pipeline = {};
};

struct WriteIndirectExecutionSetShaderEXT {
	vk::StructureType sType = StructureType::eWriteIndirectExecutionSetShaderEXT;
	const void*    pNext = {};
	uint32_t    index = {};
	vk::ShaderEXT shader = {};
};

enum class IndirectCommandsLayoutUsageFlagBitsEXT : uint32_t {
	eExplicitPreprocess = 0x1,
	eUnorderedSequences = 0x2,
};
using IndirectCommandsLayoutUsageFlagsEXT = Flags<IndirectCommandsLayoutUsageFlagBitsEXT>;

struct IndirectCommandsPushConstantTokenEXT {
	vk::PushConstantRange updateRange = {};
};

struct IndirectCommandsVertexBufferTokenEXT {
	uint32_t    vertexBindingUnit = {};
};

struct IndirectCommandsIndexBufferTokenEXT {
	vk::IndirectCommandsInputModeFlagBitsEXT mode = {};
};

struct IndirectCommandsExecutionSetTokenEXT {
	vk::IndirectExecutionSetInfoTypeEXT type = {};
	vk::ShaderStageFlags shaderStages = {};
};

union IndirectCommandsTokenDataEXT {
	const vk::IndirectCommandsPushConstantTokenEXT* pPushConstant;
	const vk::IndirectCommandsVertexBufferTokenEXT* pVertexBuffer;
	const vk::IndirectCommandsIndexBufferTokenEXT* pIndexBuffer;
	const vk::IndirectCommandsExecutionSetTokenEXT* pExecutionSet;
};

struct IndirectCommandsLayoutTokenEXT {
	vk::StructureType sType = StructureType::eIndirectCommandsLayoutTokenEXT;
	const void*    pNext = {};
	vk::IndirectCommandsTokenTypeEXT type = {};
	vk::IndirectCommandsTokenDataEXT data = {};
	uint32_t    offset = {};
};

struct IndirectCommandsLayoutCreateInfoEXT {
	vk::StructureType sType = StructureType::eIndirectCommandsLayoutCreateInfoEXT;
	const void*    pNext = {};
	vk::IndirectCommandsLayoutUsageFlagsEXT flags = {};
	vk::ShaderStageFlags shaderStages = {};
	uint32_t    indirectStride = {};
	vk::PipelineLayout pipelineLayout = {};
	uint32_t    tokenCount = {};
	const vk::IndirectCommandsLayoutTokenEXT* pTokens = {};
};

struct DrawIndirectCountIndirectCommandEXT {
	vk::DeviceAddress    bufferAddress = {};
	uint32_t    stride = {};
	uint32_t    commandCount = {};
};

struct BindVertexBufferIndirectCommandEXT {
	vk::DeviceAddress    bufferAddress = {};
	uint32_t    size = {};
	uint32_t    stride = {};
};

struct BindIndexBufferIndirectCommandEXT {
	vk::DeviceAddress    bufferAddress = {};
	uint32_t    size = {};
	vk::IndexType indexType = {};
};

struct PhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    shaderRelaxedExtendedInstruction = {};
};

struct MemoryBarrier2 {
	vk::StructureType sType = StructureType::eMemoryBarrieR2;
	const void*    pNext = {};
	vk::PipelineStageFlags2 srcStageMask = {};
	vk::AccessFlags2 srcAccessMask = {};
	vk::PipelineStageFlags2 dstStageMask = {};
	vk::AccessFlags2 dstAccessMask = {};
};
using MemoryBarrier2KHR = MemoryBarrier2;

struct ImageMemoryBarrier2 {
	vk::StructureType sType = StructureType::eImageMemoryBarrieR2;
	const void*    pNext = {};
	vk::PipelineStageFlags2 srcStageMask = {};
	vk::AccessFlags2 srcAccessMask = {};
	vk::PipelineStageFlags2 dstStageMask = {};
	vk::AccessFlags2 dstAccessMask = {};
	vk::ImageLayout oldLayout = {};
	vk::ImageLayout newLayout = {};
	uint32_t    srcQueueFamilyIndex = {};
	uint32_t    dstQueueFamilyIndex = {};
	vk::Image image = {};
	vk::ImageSubresourceRange subresourceRange = {};
};
using ImageMemoryBarrier2KHR = ImageMemoryBarrier2;

struct BufferMemoryBarrier2 {
	vk::StructureType sType = StructureType::eBufferMemoryBarrieR2;
	const void*    pNext = {};
	vk::PipelineStageFlags2 srcStageMask = {};
	vk::AccessFlags2 srcAccessMask = {};
	vk::PipelineStageFlags2 dstStageMask = {};
	vk::AccessFlags2 dstAccessMask = {};
	uint32_t    srcQueueFamilyIndex = {};
	uint32_t    dstQueueFamilyIndex = {};
	vk::Buffer buffer = {};
	vk::DeviceSize    offset = {};
	vk::DeviceSize    size = {};
};
using BufferMemoryBarrier2KHR = BufferMemoryBarrier2;

struct DependencyInfo {
	vk::StructureType sType = StructureType::eDependencyInfo;
	const void*    pNext = {};
	vk::DependencyFlags dependencyFlags = {};
	uint32_t    memoryBarrierCount = {};
	const vk::MemoryBarrier2* pMemoryBarriers = {};
	uint32_t    bufferMemoryBarrierCount = {};
	const vk::BufferMemoryBarrier2* pBufferMemoryBarriers = {};
	uint32_t    imageMemoryBarrierCount = {};
	const vk::ImageMemoryBarrier2* pImageMemoryBarriers = {};
};
using DependencyInfoKHR = DependencyInfo;

struct SemaphoreSubmitInfo {
	vk::StructureType sType = StructureType::eSemaphoreSubmitInfo;
	const void*    pNext = {};
	vk::Semaphore semaphore = {};
	uint64_t    value = {};
	vk::PipelineStageFlags2 stageMask = {};
	uint32_t    deviceIndex = {};
};
using SemaphoreSubmitInfoKHR = SemaphoreSubmitInfo;

struct CommandBufferSubmitInfo {
	vk::StructureType sType = StructureType::eCommandBufferSubmitInfo;
	const void*    pNext = {};
	vk::CommandBuffer commandBuffer = {};
	uint32_t    deviceMask = {};
};
using CommandBufferSubmitInfoKHR = CommandBufferSubmitInfo;

struct SubmitInfo2 {
	vk::StructureType sType = StructureType::eSubmitInfo2;
	const void*    pNext = {};
	vk::SubmitFlags flags = {};
	uint32_t    waitSemaphoreInfoCount = {};
	const vk::SemaphoreSubmitInfo* pWaitSemaphoreInfos = {};
	uint32_t    commandBufferInfoCount = {};
	const vk::CommandBufferSubmitInfo* pCommandBufferInfos = {};
	uint32_t    signalSemaphoreInfoCount = {};
	const vk::SemaphoreSubmitInfo* pSignalSemaphoreInfos = {};
};
using SubmitInfo2KHR = SubmitInfo2;

struct MemoryToImageCopy {
	vk::StructureType sType = StructureType::eMemoryToImageCopy;
	const void*    pNext = {};
	const void*    pHostPointer = {};
	uint32_t    memoryRowLength = {};
	uint32_t    memoryImageHeight = {};
	vk::ImageSubresourceLayers imageSubresource = {};
	vk::Offset3D imageOffset = {};
	vk::Extent3D imageExtent = {};
};
using MemoryToImageCopyEXT = MemoryToImageCopy;

struct ImageToMemoryCopy {
	vk::StructureType sType = StructureType::eImageToMemoryCopy;
	const void*    pNext = {};
	void*    pHostPointer = {};
	uint32_t    memoryRowLength = {};
	uint32_t    memoryImageHeight = {};
	vk::ImageSubresourceLayers imageSubresource = {};
	vk::Offset3D imageOffset = {};
	vk::Extent3D imageExtent = {};
};
using ImageToMemoryCopyEXT = ImageToMemoryCopy;

struct CopyMemoryToImageInfo {
	vk::StructureType sType = StructureType::eCopyMemoryToImageInfo;
	const void*    pNext = {};
	vk::HostImageCopyFlags flags = {};
	vk::Image dstImage = {};
	vk::ImageLayout dstImageLayout = {};
	uint32_t    regionCount = {};
	const vk::MemoryToImageCopy* pRegions = {};
};
using CopyMemoryToImageInfoEXT = CopyMemoryToImageInfo;

struct CopyImageToMemoryInfo {
	vk::StructureType sType = StructureType::eCopyImageToMemoryInfo;
	const void*    pNext = {};
	vk::HostImageCopyFlags flags = {};
	vk::Image srcImage = {};
	vk::ImageLayout srcImageLayout = {};
	uint32_t    regionCount = {};
	const vk::ImageToMemoryCopy* pRegions = {};
};
using CopyImageToMemoryInfoEXT = CopyImageToMemoryInfo;

struct CopyImageToImageInfo {
	vk::StructureType sType = StructureType::eCopyImageToImageInfo;
	const void*    pNext = {};
	vk::HostImageCopyFlags flags = {};
	vk::Image srcImage = {};
	vk::ImageLayout srcImageLayout = {};
	vk::Image dstImage = {};
	vk::ImageLayout dstImageLayout = {};
	uint32_t    regionCount = {};
	const vk::ImageCopy2* pRegions = {};
};
using CopyImageToImageInfoEXT = CopyImageToImageInfo;

struct HostImageLayoutTransitionInfo {
	vk::StructureType sType = StructureType::eHostImageLayoutTransitionInfo;
	const void*    pNext = {};
	vk::Image image = {};
	vk::ImageLayout oldLayout = {};
	vk::ImageLayout newLayout = {};
	vk::ImageSubresourceRange subresourceRange = {};
};
using HostImageLayoutTransitionInfoEXT = HostImageLayoutTransitionInfo;

struct QueueFamilyVideoPropertiesKHR {
	vk::StructureType sType = StructureType::eQueueFamilyVideoPropertiesKHR;
	void*    pNext = {};
	vk::VideoCodecOperationFlagsKHR videoCodecOperations = {};
};

struct QueueFamilyQueryResultStatusPropertiesKHR {
	vk::StructureType sType = StructureType::eQueueFamilyQueryResultStatusPropertiesKHR;
	void*    pNext = {};
	vk::Bool32    queryResultStatusSupport = {};
};

struct VideoProfileInfoKHR {
	vk::StructureType sType = StructureType::eVideoProfileInfoKHR;
	const void*    pNext = {};
	vk::VideoCodecOperationFlagBitsKHR videoCodecOperation = {};
	vk::VideoChromaSubsamplingFlagsKHR chromaSubsampling = {};
	vk::VideoComponentBitDepthFlagsKHR lumaBitDepth = {};
	vk::VideoComponentBitDepthFlagsKHR chromaBitDepth = {};
};

struct VideoProfileListInfoKHR {
	vk::StructureType sType = StructureType::eVideoProfileListInfoKHR;
	const void*    pNext = {};
	uint32_t    profileCount = {};
	const vk::VideoProfileInfoKHR* pProfiles = {};
};

struct PhysicalDeviceVideoFormatInfoKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceVideoFormatInfoKHR;
	const void*    pNext = {};
	vk::ImageUsageFlags imageUsage = {};
};

struct VideoFormatPropertiesKHR {
	vk::StructureType sType = StructureType::eVideoFormatPropertiesKHR;
	void*    pNext = {};
	vk::Format format = {};
	vk::ComponentMapping componentMapping = {};
	vk::ImageCreateFlags imageCreateFlags = {};
	vk::ImageType imageType = {};
	vk::ImageTiling imageTiling = {};
	vk::ImageUsageFlags imageUsageFlags = {};
};

struct VideoEncodeQuantizationMapCapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoEncodeQuantizationMapCapabilitiesKHR;
	void*    pNext = {};
	vk::Extent2D maxQuantizationMapExtent = {};
};

struct VideoEncodeH264QuantizationMapCapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264QuantizationMapCapabilitiesKHR;
	void*    pNext = {};
	int32_t    minQpDelta = {};
	int32_t    maxQpDelta = {};
};

struct VideoEncodeH265QuantizationMapCapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265QuantizationMapCapabilitiesKHR;
	void*    pNext = {};
	int32_t    minQpDelta = {};
	int32_t    maxQpDelta = {};
};

struct VideoEncodeAV1QuantizationMapCapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoEncodeAv1QuantizationMapCapabilitiesKHR;
	void*    pNext = {};
	int32_t    minQIndexDelta = {};
	int32_t    maxQIndexDelta = {};
};

struct VideoFormatQuantizationMapPropertiesKHR {
	vk::StructureType sType = StructureType::eVideoFormatQuantizationMapPropertiesKHR;
	void*    pNext = {};
	vk::Extent2D quantizationMapTexelSize = {};
};

struct VideoFormatH265QuantizationMapPropertiesKHR {
	vk::StructureType sType = StructureType::eVideoFormatH265QuantizationMapPropertiesKHR;
	void*    pNext = {};
	vk::VideoEncodeH265CtbSizeFlagsKHR compatibleCtbSizes = {};
};

struct VideoFormatAV1QuantizationMapPropertiesKHR {
	vk::StructureType sType = StructureType::eVideoFormatAv1QuantizationMapPropertiesKHR;
	void*    pNext = {};
	vk::VideoEncodeAV1SuperblockSizeFlagsKHR compatibleSuperblockSizes = {};
};

struct VideoCapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoCapabilitiesKHR;
	void*    pNext = {};
	vk::VideoCapabilityFlagsKHR flags = {};
	vk::DeviceSize    minBitstreamBufferOffsetAlignment = {};
	vk::DeviceSize    minBitstreamBufferSizeAlignment = {};
	vk::Extent2D pictureAccessGranularity = {};
	vk::Extent2D minCodedExtent = {};
	vk::Extent2D maxCodedExtent = {};
	uint32_t    maxDpbSlots = {};
	uint32_t    maxActiveReferencePictures = {};
	vk::ExtensionProperties stdHeaderVersion = {};
};

struct VideoSessionMemoryRequirementsKHR {
	vk::StructureType sType = StructureType::eVideoSessionMemoryRequirementsKHR;
	void*    pNext = {};
	uint32_t    memoryBindIndex = {};
	vk::MemoryRequirements memoryRequirements = {};
};

struct BindVideoSessionMemoryInfoKHR {
	vk::StructureType sType = StructureType::eBindVideoSessionMemoryInfoKHR;
	const void*    pNext = {};
	uint32_t    memoryBindIndex = {};
	vk::DeviceMemory memory = {};
	vk::DeviceSize    memoryOffset = {};
	vk::DeviceSize    memorySize = {};
};

struct VideoPictureResourceInfoKHR {
	vk::StructureType sType = StructureType::eVideoPictureResourceInfoKHR;
	const void*    pNext = {};
	vk::Offset2D codedOffset = {};
	vk::Extent2D codedExtent = {};
	uint32_t    baseArrayLayer = {};
	vk::ImageView imageViewBinding = {};
};

struct VideoReferenceSlotInfoKHR {
	vk::StructureType sType = StructureType::eVideoReferenceSlotInfoKHR;
	const void*    pNext = {};
	int32_t    slotIndex = {};
	const vk::VideoPictureResourceInfoKHR* pPictureResource = {};
};

struct VideoDecodeCapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoDecodeCapabilitiesKHR;
	void*    pNext = {};
	vk::VideoDecodeCapabilityFlagsKHR flags = {};
};

struct VideoDecodeUsageInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeUsageInfoKHR;
	const void*    pNext = {};
	vk::VideoDecodeUsageFlagsKHR videoUsageHints = {};
};

struct VideoDecodeInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeInfoKHR;
	const void*    pNext = {};
	vk::VideoDecodeFlagsKHR flags = {};
	vk::Buffer srcBuffer = {};
	vk::DeviceSize    srcBufferOffset = {};
	vk::DeviceSize    srcBufferRange = {};
	vk::VideoPictureResourceInfoKHR dstPictureResource = {};
	const vk::VideoReferenceSlotInfoKHR* pSetupReferenceSlot = {};
	uint32_t    referenceSlotCount = {};
	const vk::VideoReferenceSlotInfoKHR* pReferenceSlots = {};
};

struct PhysicalDeviceVideoMaintenance1FeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceVideoMaintenance1FeaturesKHR;
	void*    pNext = {};
	vk::Bool32    videoMaintenance1 = {};
};

struct VideoInlineQueryInfoKHR {
	vk::StructureType sType = StructureType::eVideoInlineQueryInfoKHR;
	const void*    pNext = {};
	vk::QueryPool queryPool = {};
	uint32_t    firstQuery = {};
	uint32_t    queryCount = {};
};

#define STD_VIDEO_H264_CPB_CNT_LIST_SIZE 32
#define STD_VIDEO_H264_SCALING_LIST_4X4_NUM_LISTS 6
#define STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS 16
#define STD_VIDEO_H264_SCALING_LIST_8X8_NUM_LISTS 6
#define STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS 64
#define STD_VIDEO_H264_MAX_NUM_LIST_REF 32
#define STD_VIDEO_H264_MAX_CHROMA_PLANES 2
#define STD_VIDEO_H264_NO_REFERENCE_PICTURE 0xFF

typedef enum StdVideoH264ChromaFormatIdc {
    STD_VIDEO_H264_CHROMA_FORMAT_IDC_MONOCHROME = 0,
    STD_VIDEO_H264_CHROMA_FORMAT_IDC_420 = 1,
    STD_VIDEO_H264_CHROMA_FORMAT_IDC_422 = 2,
    STD_VIDEO_H264_CHROMA_FORMAT_IDC_444 = 3,
    STD_VIDEO_H264_CHROMA_FORMAT_IDC_INVALID = 0x7FFFFFFF
} StdVideoH264ChromaFormatIdc;

// enum ext: vulkan_video_codec_h264std
typedef enum StdVideoH264ProfileIdc {
    STD_VIDEO_H264_PROFILE_IDC_BASELINE = 66,
    STD_VIDEO_H264_PROFILE_IDC_MAIN = 77,
    STD_VIDEO_H264_PROFILE_IDC_HIGH = 100,
    STD_VIDEO_H264_PROFILE_IDC_HIGH_444_PREDICTIVE = 244,
    STD_VIDEO_H264_PROFILE_IDC_INVALID = 0x7FFFFFFF
} StdVideoH264ProfileIdc;

// enum ext: vulkan_video_codec_h264std
typedef enum StdVideoH264LevelIdc {
    STD_VIDEO_H264_LEVEL_IDC_1_0 = 0,
    STD_VIDEO_H264_LEVEL_IDC_1_1 = 1,
    STD_VIDEO_H264_LEVEL_IDC_1_2 = 2,
    STD_VIDEO_H264_LEVEL_IDC_1_3 = 3,
    STD_VIDEO_H264_LEVEL_IDC_2_0 = 4,
    STD_VIDEO_H264_LEVEL_IDC_2_1 = 5,
    STD_VIDEO_H264_LEVEL_IDC_2_2 = 6,
    STD_VIDEO_H264_LEVEL_IDC_3_0 = 7,
    STD_VIDEO_H264_LEVEL_IDC_3_1 = 8,
    STD_VIDEO_H264_LEVEL_IDC_3_2 = 9,
    STD_VIDEO_H264_LEVEL_IDC_4_0 = 10,
    STD_VIDEO_H264_LEVEL_IDC_4_1 = 11,
    STD_VIDEO_H264_LEVEL_IDC_4_2 = 12,
    STD_VIDEO_H264_LEVEL_IDC_5_0 = 13,
    STD_VIDEO_H264_LEVEL_IDC_5_1 = 14,
    STD_VIDEO_H264_LEVEL_IDC_5_2 = 15,
    STD_VIDEO_H264_LEVEL_IDC_6_0 = 16,
    STD_VIDEO_H264_LEVEL_IDC_6_1 = 17,
    STD_VIDEO_H264_LEVEL_IDC_6_2 = 18,
    STD_VIDEO_H264_LEVEL_IDC_INVALID = 0x7FFFFFFF
} StdVideoH264LevelIdc;

// enum ext: vulkan_video_codec_h264std
typedef enum StdVideoH264PocType {
    STD_VIDEO_H264_POC_TYPE_0 = 0,
    STD_VIDEO_H264_POC_TYPE_1 = 1,
    STD_VIDEO_H264_POC_TYPE_2 = 2,
    STD_VIDEO_H264_POC_TYPE_INVALID = 0x7FFFFFFF
} StdVideoH264PocType;

// enum ext: vulkan_video_codec_h264std
typedef enum StdVideoH264AspectRatioIdc {
    STD_VIDEO_H264_ASPECT_RATIO_IDC_UNSPECIFIED = 0,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_SQUARE = 1,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_12_11 = 2,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_10_11 = 3,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_16_11 = 4,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_40_33 = 5,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_24_11 = 6,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_20_11 = 7,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_32_11 = 8,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_80_33 = 9,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_18_11 = 10,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_15_11 = 11,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_64_33 = 12,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_160_99 = 13,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_4_3 = 14,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_3_2 = 15,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_2_1 = 16,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_EXTENDED_SAR = 255,
    STD_VIDEO_H264_ASPECT_RATIO_IDC_INVALID = 0x7FFFFFFF
} StdVideoH264AspectRatioIdc;

// enum ext: vulkan_video_codec_h264std
typedef enum StdVideoH264WeightedBipredIdc {
    STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_DEFAULT = 0,
    STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_EXPLICIT = 1,
    STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_IMPLICIT = 2,
    STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_INVALID = 0x7FFFFFFF
} StdVideoH264WeightedBipredIdc;

// enum ext: vulkan_video_codec_h264std
typedef enum StdVideoH264ModificationOfPicNumsIdc {
    STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_SHORT_TERM_SUBTRACT = 0,
    STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_SHORT_TERM_ADD = 1,
    STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_LONG_TERM = 2,
    STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_END = 3,
    STD_VIDEO_H264_MODIFICATION_OF_PIC_NUMS_IDC_INVALID = 0x7FFFFFFF
} StdVideoH264ModificationOfPicNumsIdc;

// enum ext: vulkan_video_codec_h264std
typedef enum StdVideoH264MemMgmtControlOp {
    STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_END = 0,
    STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_UNMARK_SHORT_TERM = 1,
    STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_UNMARK_LONG_TERM = 2,
    STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_MARK_LONG_TERM = 3,
    STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_SET_MAX_LONG_TERM_INDEX = 4,
    STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_UNMARK_ALL = 5,
    STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_MARK_CURRENT_AS_LONG_TERM = 6,
    STD_VIDEO_H264_MEM_MGMT_CONTROL_OP_INVALID = 0x7FFFFFFF
} StdVideoH264MemMgmtControlOp;

// enum ext: vulkan_video_codec_h264std
typedef enum StdVideoH264CabacInitIdc {
    STD_VIDEO_H264_CABAC_INIT_IDC_0 = 0,
    STD_VIDEO_H264_CABAC_INIT_IDC_1 = 1,
    STD_VIDEO_H264_CABAC_INIT_IDC_2 = 2,
    STD_VIDEO_H264_CABAC_INIT_IDC_INVALID = 0x7FFFFFFF
} StdVideoH264CabacInitIdc;

// enum ext: vulkan_video_codec_h264std
typedef enum StdVideoH264DisableDeblockingFilterIdc {
    STD_VIDEO_H264_DISABLE_DEBLOCKING_FILTER_IDC_DISABLED = 0,
    STD_VIDEO_H264_DISABLE_DEBLOCKING_FILTER_IDC_ENABLED = 1,
    STD_VIDEO_H264_DISABLE_DEBLOCKING_FILTER_IDC_PARTIAL = 2,
    STD_VIDEO_H264_DISABLE_DEBLOCKING_FILTER_IDC_INVALID = 0x7FFFFFFF
} StdVideoH264DisableDeblockingFilterIdc;

// enum ext: vulkan_video_codec_h264std
typedef enum StdVideoH264SliceType {
    STD_VIDEO_H264_SLICE_TYPE_P = 0,
    STD_VIDEO_H264_SLICE_TYPE_B = 1,
    STD_VIDEO_H264_SLICE_TYPE_I = 2,
    STD_VIDEO_H264_SLICE_TYPE_INVALID = 0x7FFFFFFF
} StdVideoH264SliceType;

// enum ext: vulkan_video_codec_h264std
typedef enum StdVideoH264PictureType {
    STD_VIDEO_H264_PICTURE_TYPE_P = 0,
    STD_VIDEO_H264_PICTURE_TYPE_B = 1,
    STD_VIDEO_H264_PICTURE_TYPE_I = 2,
    STD_VIDEO_H264_PICTURE_TYPE_IDR = 5,
    STD_VIDEO_H264_PICTURE_TYPE_INVALID = 0x7FFFFFFF
} StdVideoH264PictureType;

// enum ext: vulkan_video_codec_h264std
typedef enum StdVideoH264NonVclNaluType {
    STD_VIDEO_H264_NON_VCL_NALU_TYPE_SPS = 0,
    STD_VIDEO_H264_NON_VCL_NALU_TYPE_PPS = 1,
    STD_VIDEO_H264_NON_VCL_NALU_TYPE_AUD = 2,
    STD_VIDEO_H264_NON_VCL_NALU_TYPE_PREFIX = 3,
    STD_VIDEO_H264_NON_VCL_NALU_TYPE_END_OF_SEQUENCE = 4,
    STD_VIDEO_H264_NON_VCL_NALU_TYPE_END_OF_STREAM = 5,
    STD_VIDEO_H264_NON_VCL_NALU_TYPE_PRECODED = 6,
    STD_VIDEO_H264_NON_VCL_NALU_TYPE_INVALID = 0x7FFFFFFF
} StdVideoH264NonVclNaluType;

// struct ext: vulkan_video_codec_h264std
typedef struct StdVideoH264SpsVuiFlags {
    uint32_t    aspect_ratio_info_present_flag : 1;
    uint32_t    overscan_info_present_flag : 1;
    uint32_t    overscan_appropriate_flag : 1;
    uint32_t    video_signal_type_present_flag : 1;
    uint32_t    video_full_range_flag : 1;
    uint32_t    color_description_present_flag : 1;
    uint32_t    chroma_loc_info_present_flag : 1;
    uint32_t    timing_info_present_flag : 1;
    uint32_t    fixed_frame_rate_flag : 1;
    uint32_t    bitstream_restriction_flag : 1;
    uint32_t    nal_hrd_parameters_present_flag : 1;
    uint32_t    vcl_hrd_parameters_present_flag : 1;
} StdVideoH264SpsVuiFlags;

// struct ext: vulkan_video_codec_h264std
typedef struct StdVideoH264HrdParameters {
    uint8_t    cpb_cnt_minus1;
    uint8_t    bit_rate_scale;
    uint8_t    cpb_size_scale;
    uint8_t    reserved1;
    uint32_t    bit_rate_value_minus1[STD_VIDEO_H264_CPB_CNT_LIST_SIZE];
    uint32_t    cpb_size_value_minus1[STD_VIDEO_H264_CPB_CNT_LIST_SIZE];
    uint8_t    cbr_flag[STD_VIDEO_H264_CPB_CNT_LIST_SIZE];
    uint32_t    initial_cpb_removal_delay_length_minus1;
    uint32_t    cpb_removal_delay_length_minus1;
    uint32_t    dpb_output_delay_length_minus1;
    uint32_t    time_offset_length;
} StdVideoH264HrdParameters;

// struct ext: vulkan_video_codec_h264std
typedef struct StdVideoH264SequenceParameterSetVui {
    StdVideoH264SpsVuiFlags    flags;
    StdVideoH264AspectRatioIdc    aspect_ratio_idc;
    uint16_t    sar_width;
    uint16_t    sar_height;
    uint8_t    video_format;
    uint8_t    colour_primaries;
    uint8_t    transfer_characteristics;
    uint8_t    matrix_coefficients;
    uint32_t    num_units_in_tick;
    uint32_t    time_scale;
    uint8_t    max_num_reorder_frames;
    uint8_t    max_dec_frame_buffering;
    uint8_t    chroma_sample_loc_type_top_field;
    uint8_t    chroma_sample_loc_type_bottom_field;
    uint32_t    reserved1;
    const StdVideoH264HrdParameters*    pHrdParameters;
} StdVideoH264SequenceParameterSetVui;

// struct ext: vulkan_video_codec_h264std
typedef struct StdVideoH264SpsFlags {
    uint32_t    constraint_set0_flag : 1;
    uint32_t    constraint_set1_flag : 1;
    uint32_t    constraint_set2_flag : 1;
    uint32_t    constraint_set3_flag : 1;
    uint32_t    constraint_set4_flag : 1;
    uint32_t    constraint_set5_flag : 1;
    uint32_t    direct_8x8_inference_flag : 1;
    uint32_t    mb_adaptive_frame_field_flag : 1;
    uint32_t    frame_mbs_only_flag : 1;
    uint32_t    delta_pic_order_always_zero_flag : 1;
    uint32_t    separate_colour_plane_flag : 1;
    uint32_t    gaps_in_frame_num_value_allowed_flag : 1;
    uint32_t    qpprime_y_zero_transform_bypass_flag : 1;
    uint32_t    frame_cropping_flag : 1;
    uint32_t    seq_scaling_matrix_present_flag : 1;
    uint32_t    vui_parameters_present_flag : 1;
} StdVideoH264SpsFlags;

// struct ext: vulkan_video_codec_h264std
typedef struct StdVideoH264ScalingLists {
    uint16_t    scaling_list_present_mask;
    uint16_t    use_default_scaling_matrix_mask;
    uint8_t    ScalingList4x4[STD_VIDEO_H264_SCALING_LIST_4X4_NUM_LISTS];
    uint8_t    ScalingList8x8[STD_VIDEO_H264_SCALING_LIST_8X8_NUM_LISTS];
} StdVideoH264ScalingLists;

// struct ext: vulkan_video_codec_h264std
typedef struct StdVideoH264SequenceParameterSet {
    StdVideoH264SpsFlags    flags;
    StdVideoH264ProfileIdc    profile_idc;
    StdVideoH264LevelIdc    level_idc;
    StdVideoH264ChromaFormatIdc    chroma_format_idc;
    uint8_t    seq_parameter_set_id;
    uint8_t    bit_depth_luma_minus8;
    uint8_t    bit_depth_chroma_minus8;
    uint8_t    log2_max_frame_num_minus4;
    StdVideoH264PocType    pic_order_cnt_type;
    int32_t    offset_for_non_ref_pic;
    int32_t    offset_for_top_to_bottom_field;
    uint8_t    log2_max_pic_order_cnt_lsb_minus4;
    uint8_t    num_ref_frames_in_pic_order_cnt_cycle;
    uint8_t    max_num_ref_frames;
    uint8_t    reserved1;
    uint32_t    pic_width_in_mbs_minus1;
    uint32_t    pic_height_in_map_units_minus1;
    uint32_t    frame_crop_left_offset;
    uint32_t    frame_crop_right_offset;
    uint32_t    frame_crop_top_offset;
    uint32_t    frame_crop_bottom_offset;
    uint32_t    reserved2;
    const int32_t*    pOffsetForRefFrame;
    const StdVideoH264ScalingLists*    pScalingLists;
    const StdVideoH264SequenceParameterSetVui*    pSequenceParameterSetVui;
} StdVideoH264SequenceParameterSet;

// struct ext: vulkan_video_codec_h264std
typedef struct StdVideoH264PpsFlags {
    uint32_t    transform_8x8_mode_flag : 1;
    uint32_t    redundant_pic_cnt_present_flag : 1;
    uint32_t    constrained_intra_pred_flag : 1;
    uint32_t    deblocking_filter_control_present_flag : 1;
    uint32_t    weighted_pred_flag : 1;
    uint32_t    bottom_field_pic_order_in_frame_present_flag : 1;
    uint32_t    entropy_coding_mode_flag : 1;
    uint32_t    pic_scaling_matrix_present_flag : 1;
} StdVideoH264PpsFlags;

// struct ext: vulkan_video_codec_h264std
typedef struct StdVideoH264PictureParameterSet {
    StdVideoH264PpsFlags    flags;
    uint8_t    seq_parameter_set_id;
    uint8_t    pic_parameter_set_id;
    uint8_t    num_ref_idx_l0_default_active_minus1;
    uint8_t    num_ref_idx_l1_default_active_minus1;
    StdVideoH264WeightedBipredIdc    weighted_bipred_idc;
    int8_t    pic_init_qp_minus26;
    int8_t    pic_init_qs_minus26;
    int8_t    chroma_qp_index_offset;
    int8_t    second_chroma_qp_index_offset;
    const StdVideoH264ScalingLists*    pScalingLists;
} StdVideoH264PictureParameterSet;
struct VideoDecodeH264ProfileInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeH264ProfileInfoKHR;
	const void*    pNext = {};
	StdVideoH264ProfileIdc    stdProfileIdc = {};
	vk::VideoDecodeH264PictureLayoutFlagBitsKHR pictureLayout = {};
};

struct VideoDecodeH264CapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoDecodeH264CapabilitiesKHR;
	void*    pNext = {};
	StdVideoH264LevelIdc    maxLevelIdc = {};
	vk::Offset2D fieldOffsetGranularity = {};
};

struct VideoDecodeH264SessionParametersAddInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeH264SessionParametersAddInfoKHR;
	const void*    pNext = {};
	uint32_t    stdSPSCount = {};
	const StdVideoH264SequenceParameterSet*    pStdSPSs = {};
	uint32_t    stdPPSCount = {};
	const StdVideoH264PictureParameterSet*    pStdPPSs = {};
};

struct VideoDecodeH264SessionParametersCreateInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeH264SessionParametersCreateInfoKHR;
	const void*    pNext = {};
	uint32_t    maxStdSPSCount = {};
	uint32_t    maxStdPPSCount = {};
	const vk::VideoDecodeH264SessionParametersAddInfoKHR* pParametersAddInfo = {};
};

#define VK_STD_VULKAN_VIDEO_CODEC_H264_DECODE_SPEC_VERSION VK_STD_VULKAN_VIDEO_CODEC_H264_DECODE_API_VERSION_1_0_0
#define VK_STD_VULKAN_VIDEO_CODEC_H264_DECODE_EXTENSION_NAME "VK_STD_vulkan_video_codec_h264_decode"
#define STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_LIST_SIZE 2


// vulkan_video_codec_h264std_decode is a preprocessor guard. Do not pass it to API calls.
#define vulkan_video_codec_h264std_decode 1
// enum ext: vulkan_video_codec_h264std_decode
typedef enum StdVideoDecodeH264FieldOrderCount {
    STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_TOP = 0,
    STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_BOTTOM = 1,
    STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_INVALID = 0x7FFFFFFF
} StdVideoDecodeH264FieldOrderCount;

// struct ext: vulkan_video_codec_h264std_decode
typedef struct StdVideoDecodeH264PictureInfoFlags {
    uint32_t    field_pic_flag : 1;
    uint32_t    is_intra : 1;
    uint32_t    IdrPicFlag : 1;
    uint32_t    bottom_field_flag : 1;
    uint32_t    is_reference : 1;
    uint32_t    complementary_field_pair : 1;
} StdVideoDecodeH264PictureInfoFlags;

// struct ext: vulkan_video_codec_h264std_decode
typedef struct StdVideoDecodeH264PictureInfo {
    StdVideoDecodeH264PictureInfoFlags    flags;
    uint8_t    seq_parameter_set_id;
    uint8_t    pic_parameter_set_id;
    uint8_t    reserved1;
    uint8_t    reserved2;
    uint16_t    frame_num;
    uint16_t    idr_pic_id;
    int32_t    PicOrderCnt[STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_LIST_SIZE];
} StdVideoDecodeH264PictureInfo;

// struct ext: vulkan_video_codec_h264std_decode
typedef struct StdVideoDecodeH264ReferenceInfoFlags {
    uint32_t    top_field_flag : 1;
    uint32_t    bottom_field_flag : 1;
    uint32_t    used_for_long_term_reference : 1;
    uint32_t    is_non_existing : 1;
} StdVideoDecodeH264ReferenceInfoFlags;

// struct ext: vulkan_video_codec_h264std_decode
typedef struct StdVideoDecodeH264ReferenceInfo {
    StdVideoDecodeH264ReferenceInfoFlags    flags;
    uint16_t    FrameNum;
    uint16_t    reserved;
    int32_t    PicOrderCnt[STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_LIST_SIZE];
} StdVideoDecodeH264ReferenceInfo;

struct VideoDecodeH264PictureInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeH264PictureInfoKHR;
	const void*    pNext = {};
	const StdVideoDecodeH264PictureInfo*    pStdPictureInfo = {};
	uint32_t    sliceCount = {};
	const uint32_t*    pSliceOffsets = {};
};

struct VideoDecodeH264DpbSlotInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeH264DpbSlotInfoKHR;
	const void*    pNext = {};
	const StdVideoDecodeH264ReferenceInfo*    pStdReferenceInfo = {};
};

#define STD_VIDEO_H265_CPB_CNT_LIST_SIZE 32
#define STD_VIDEO_H265_SUBLAYERS_LIST_SIZE 7
#define STD_VIDEO_H265_SCALING_LIST_4X4_NUM_LISTS 6
#define STD_VIDEO_H265_SCALING_LIST_4X4_NUM_ELEMENTS 16
#define STD_VIDEO_H265_SCALING_LIST_8X8_NUM_LISTS 6
#define STD_VIDEO_H265_SCALING_LIST_8X8_NUM_ELEMENTS 64
#define STD_VIDEO_H265_SCALING_LIST_16X16_NUM_LISTS 6
#define STD_VIDEO_H265_SCALING_LIST_16X16_NUM_ELEMENTS 64
#define STD_VIDEO_H265_SCALING_LIST_32X32_NUM_LISTS 2
#define STD_VIDEO_H265_SCALING_LIST_32X32_NUM_ELEMENTS 64
#define STD_VIDEO_H265_CHROMA_QP_OFFSET_LIST_SIZE 6
#define STD_VIDEO_H265_CHROMA_QP_OFFSET_TILE_COLS_LIST_SIZE 19
#define STD_VIDEO_H265_CHROMA_QP_OFFSET_TILE_ROWS_LIST_SIZE 21
#define STD_VIDEO_H265_PREDICTOR_PALETTE_COMPONENTS_LIST_SIZE 3
#define STD_VIDEO_H265_PREDICTOR_PALETTE_COMP_ENTRIES_LIST_SIZE 128
#define STD_VIDEO_H265_MAX_NUM_LIST_REF 15
#define STD_VIDEO_H265_MAX_CHROMA_PLANES 2
#define STD_VIDEO_H265_MAX_SHORT_TERM_REF_PIC_SETS 64
#define STD_VIDEO_H265_MAX_DPB_SIZE 16
#define STD_VIDEO_H265_MAX_LONG_TERM_REF_PICS_SPS 32
#define STD_VIDEO_H265_MAX_LONG_TERM_PICS 16
#define STD_VIDEO_H265_MAX_DELTA_POC 48
#define STD_VIDEO_H265_NO_REFERENCE_PICTURE 0xFF


// vulkan_video_codec_h265std is a preprocessor guard. Do not pass it to API calls.
#define vulkan_video_codec_h265std 1
// enum ext: vulkan_video_codec_h265std
typedef enum StdVideoH265ChromaFormatIdc {
    STD_VIDEO_H265_CHROMA_FORMAT_IDC_MONOCHROME = 0,
    STD_VIDEO_H265_CHROMA_FORMAT_IDC_420 = 1,
    STD_VIDEO_H265_CHROMA_FORMAT_IDC_422 = 2,
    STD_VIDEO_H265_CHROMA_FORMAT_IDC_444 = 3,
    STD_VIDEO_H265_CHROMA_FORMAT_IDC_INVALID = 0x7FFFFFFF
} StdVideoH265ChromaFormatIdc;

// enum ext: vulkan_video_codec_h265std
typedef enum StdVideoH265ProfileIdc {
    STD_VIDEO_H265_PROFILE_IDC_MAIN = 1,
    STD_VIDEO_H265_PROFILE_IDC_MAIN_10 = 2,
    STD_VIDEO_H265_PROFILE_IDC_MAIN_STILL_PICTURE = 3,
    STD_VIDEO_H265_PROFILE_IDC_FORMAT_RANGE_EXTENSIONS = 4,
    STD_VIDEO_H265_PROFILE_IDC_SCC_EXTENSIONS = 9,
    STD_VIDEO_H265_PROFILE_IDC_INVALID = 0x7FFFFFFF
} StdVideoH265ProfileIdc;

// enum ext: vulkan_video_codec_h265std
typedef enum StdVideoH265LevelIdc {
    STD_VIDEO_H265_LEVEL_IDC_1_0 = 0,
    STD_VIDEO_H265_LEVEL_IDC_2_0 = 1,
    STD_VIDEO_H265_LEVEL_IDC_2_1 = 2,
    STD_VIDEO_H265_LEVEL_IDC_3_0 = 3,
    STD_VIDEO_H265_LEVEL_IDC_3_1 = 4,
    STD_VIDEO_H265_LEVEL_IDC_4_0 = 5,
    STD_VIDEO_H265_LEVEL_IDC_4_1 = 6,
    STD_VIDEO_H265_LEVEL_IDC_5_0 = 7,
    STD_VIDEO_H265_LEVEL_IDC_5_1 = 8,
    STD_VIDEO_H265_LEVEL_IDC_5_2 = 9,
    STD_VIDEO_H265_LEVEL_IDC_6_0 = 10,
    STD_VIDEO_H265_LEVEL_IDC_6_1 = 11,
    STD_VIDEO_H265_LEVEL_IDC_6_2 = 12,
    STD_VIDEO_H265_LEVEL_IDC_INVALID = 0x7FFFFFFF
} StdVideoH265LevelIdc;

// enum ext: vulkan_video_codec_h265std
typedef enum StdVideoH265SliceType {
    STD_VIDEO_H265_SLICE_TYPE_B = 0,
    STD_VIDEO_H265_SLICE_TYPE_P = 1,
    STD_VIDEO_H265_SLICE_TYPE_I = 2,
    STD_VIDEO_H265_SLICE_TYPE_INVALID = 0x7FFFFFFF
} StdVideoH265SliceType;

// enum ext: vulkan_video_codec_h265std
typedef enum StdVideoH265PictureType {
    STD_VIDEO_H265_PICTURE_TYPE_P = 0,
    STD_VIDEO_H265_PICTURE_TYPE_B = 1,
    STD_VIDEO_H265_PICTURE_TYPE_I = 2,
    STD_VIDEO_H265_PICTURE_TYPE_IDR = 3,
    STD_VIDEO_H265_PICTURE_TYPE_INVALID = 0x7FFFFFFF
} StdVideoH265PictureType;

// enum ext: vulkan_video_codec_h265std
typedef enum StdVideoH265AspectRatioIdc {
    STD_VIDEO_H265_ASPECT_RATIO_IDC_UNSPECIFIED = 0,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_SQUARE = 1,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_12_11 = 2,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_10_11 = 3,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_16_11 = 4,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_40_33 = 5,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_24_11 = 6,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_20_11 = 7,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_32_11 = 8,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_80_33 = 9,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_18_11 = 10,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_15_11 = 11,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_64_33 = 12,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_160_99 = 13,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_4_3 = 14,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_3_2 = 15,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_2_1 = 16,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_EXTENDED_SAR = 255,
    STD_VIDEO_H265_ASPECT_RATIO_IDC_INVALID = 0x7FFFFFFF
} StdVideoH265AspectRatioIdc;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265DecPicBufMgr {
    uint32_t    max_latency_increase_plus1[STD_VIDEO_H265_SUBLAYERS_LIST_SIZE];
    uint8_t    max_dec_pic_buffering_minus1[STD_VIDEO_H265_SUBLAYERS_LIST_SIZE];
    uint8_t    max_num_reorder_pics[STD_VIDEO_H265_SUBLAYERS_LIST_SIZE];
} StdVideoH265DecPicBufMgr;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265SubLayerHrdParameters {
    uint32_t    bit_rate_value_minus1[STD_VIDEO_H265_CPB_CNT_LIST_SIZE];
    uint32_t    cpb_size_value_minus1[STD_VIDEO_H265_CPB_CNT_LIST_SIZE];
    uint32_t    cpb_size_du_value_minus1[STD_VIDEO_H265_CPB_CNT_LIST_SIZE];
    uint32_t    bit_rate_du_value_minus1[STD_VIDEO_H265_CPB_CNT_LIST_SIZE];
    uint32_t    cbr_flag;
} StdVideoH265SubLayerHrdParameters;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265HrdFlags {
    uint32_t    nal_hrd_parameters_present_flag : 1;
    uint32_t    vcl_hrd_parameters_present_flag : 1;
    uint32_t    sub_pic_hrd_params_present_flag : 1;
    uint32_t    sub_pic_cpb_params_in_pic_timing_sei_flag : 1;
    uint32_t    fixed_pic_rate_general_flag : 8;
    uint32_t    fixed_pic_rate_within_cvs_flag : 8;
    uint32_t    low_delay_hrd_flag : 8;
} StdVideoH265HrdFlags;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265HrdParameters {
    StdVideoH265HrdFlags    flags;
    uint8_t    tick_divisor_minus2;
    uint8_t    du_cpb_removal_delay_increment_length_minus1;
    uint8_t    dpb_output_delay_du_length_minus1;
    uint8_t    bit_rate_scale;
    uint8_t    cpb_size_scale;
    uint8_t    cpb_size_du_scale;
    uint8_t    initial_cpb_removal_delay_length_minus1;
    uint8_t    au_cpb_removal_delay_length_minus1;
    uint8_t    dpb_output_delay_length_minus1;
    uint8_t    cpb_cnt_minus1[STD_VIDEO_H265_SUBLAYERS_LIST_SIZE];
    uint16_t    elemental_duration_in_tc_minus1[STD_VIDEO_H265_SUBLAYERS_LIST_SIZE];
    uint16_t    reserved[3];
    const StdVideoH265SubLayerHrdParameters*    pSubLayerHrdParametersNal;
    const StdVideoH265SubLayerHrdParameters*    pSubLayerHrdParametersVcl;
} StdVideoH265HrdParameters;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265VpsFlags {
    uint32_t    vps_temporal_id_nesting_flag : 1;
    uint32_t    vps_sub_layer_ordering_info_present_flag : 1;
    uint32_t    vps_timing_info_present_flag : 1;
    uint32_t    vps_poc_proportional_to_timing_flag : 1;
} StdVideoH265VpsFlags;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265ProfileTierLevelFlags {
    uint32_t    general_tier_flag : 1;
    uint32_t    general_progressive_source_flag : 1;
    uint32_t    general_interlaced_source_flag : 1;
    uint32_t    general_non_packed_constraint_flag : 1;
    uint32_t    general_frame_only_constraint_flag : 1;
} StdVideoH265ProfileTierLevelFlags;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265ProfileTierLevel {
    StdVideoH265ProfileTierLevelFlags    flags;
    StdVideoH265ProfileIdc    general_profile_idc;
    StdVideoH265LevelIdc    general_level_idc;
} StdVideoH265ProfileTierLevel;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265VideoParameterSet {
    StdVideoH265VpsFlags    flags;
    uint8_t    vps_video_parameter_set_id;
    uint8_t    vps_max_sub_layers_minus1;
    uint8_t    reserved1;
    uint8_t    reserved2;
    uint32_t    vps_num_units_in_tick;
    uint32_t    vps_time_scale;
    uint32_t    vps_num_ticks_poc_diff_one_minus1;
    uint32_t    reserved3;
    const StdVideoH265DecPicBufMgr*    pDecPicBufMgr;
    const StdVideoH265HrdParameters*    pHrdParameters;
    const StdVideoH265ProfileTierLevel*    pProfileTierLevel;
} StdVideoH265VideoParameterSet;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265ScalingLists {
    uint8_t    ScalingList4x4[STD_VIDEO_H265_SCALING_LIST_4X4_NUM_LISTS];
    uint8_t    ScalingList8x8[STD_VIDEO_H265_SCALING_LIST_8X8_NUM_LISTS];
    uint8_t    ScalingList16x16[STD_VIDEO_H265_SCALING_LIST_16X16_NUM_LISTS];
    uint8_t    ScalingList32x32[STD_VIDEO_H265_SCALING_LIST_32X32_NUM_LISTS];
    uint8_t    ScalingListDCCoef16x16[STD_VIDEO_H265_SCALING_LIST_16X16_NUM_LISTS];
    uint8_t    ScalingListDCCoef32x32[STD_VIDEO_H265_SCALING_LIST_32X32_NUM_LISTS];
} StdVideoH265ScalingLists;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265SpsVuiFlags {
    uint32_t    aspect_ratio_info_present_flag : 1;
    uint32_t    overscan_info_present_flag : 1;
    uint32_t    overscan_appropriate_flag : 1;
    uint32_t    video_signal_type_present_flag : 1;
    uint32_t    video_full_range_flag : 1;
    uint32_t    colour_description_present_flag : 1;
    uint32_t    chroma_loc_info_present_flag : 1;
    uint32_t    neutral_chroma_indication_flag : 1;
    uint32_t    field_seq_flag : 1;
    uint32_t    frame_field_info_present_flag : 1;
    uint32_t    default_display_window_flag : 1;
    uint32_t    vui_timing_info_present_flag : 1;
    uint32_t    vui_poc_proportional_to_timing_flag : 1;
    uint32_t    vui_hrd_parameters_present_flag : 1;
    uint32_t    bitstream_restriction_flag : 1;
    uint32_t    tiles_fixed_structure_flag : 1;
    uint32_t    motion_vectors_over_pic_boundaries_flag : 1;
    uint32_t    restricted_ref_pic_lists_flag : 1;
} StdVideoH265SpsVuiFlags;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265SequenceParameterSetVui {
    StdVideoH265SpsVuiFlags    flags;
    StdVideoH265AspectRatioIdc    aspect_ratio_idc;
    uint16_t    sar_width;
    uint16_t    sar_height;
    uint8_t    video_format;
    uint8_t    colour_primaries;
    uint8_t    transfer_characteristics;
    uint8_t    matrix_coeffs;
    uint8_t    chroma_sample_loc_type_top_field;
    uint8_t    chroma_sample_loc_type_bottom_field;
    uint8_t    reserved1;
    uint8_t    reserved2;
    uint16_t    def_disp_win_left_offset;
    uint16_t    def_disp_win_right_offset;
    uint16_t    def_disp_win_top_offset;
    uint16_t    def_disp_win_bottom_offset;
    uint32_t    vui_num_units_in_tick;
    uint32_t    vui_time_scale;
    uint32_t    vui_num_ticks_poc_diff_one_minus1;
    uint16_t    min_spatial_segmentation_idc;
    uint16_t    reserved3;
    uint8_t    max_bytes_per_pic_denom;
    uint8_t    max_bits_per_min_cu_denom;
    uint8_t    log2_max_mv_length_horizontal;
    uint8_t    log2_max_mv_length_vertical;
    const StdVideoH265HrdParameters*    pHrdParameters;
} StdVideoH265SequenceParameterSetVui;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265PredictorPaletteEntries {
    uint16_t    PredictorPaletteEntries[STD_VIDEO_H265_PREDICTOR_PALETTE_COMPONENTS_LIST_SIZE];
} StdVideoH265PredictorPaletteEntries;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265SpsFlags {
    uint32_t    sps_temporal_id_nesting_flag : 1;
    uint32_t    separate_colour_plane_flag : 1;
    uint32_t    conformance_window_flag : 1;
    uint32_t    sps_sub_layer_ordering_info_present_flag : 1;
    uint32_t    scaling_list_enabled_flag : 1;
    uint32_t    sps_scaling_list_data_present_flag : 1;
    uint32_t    amp_enabled_flag : 1;
    uint32_t    sample_adaptive_offset_enabled_flag : 1;
    uint32_t    pcm_enabled_flag : 1;
    uint32_t    pcm_loop_filter_disabled_flag : 1;
    uint32_t    long_term_ref_pics_present_flag : 1;
    uint32_t    sps_temporal_mvp_enabled_flag : 1;
    uint32_t    strong_intra_smoothing_enabled_flag : 1;
    uint32_t    vui_parameters_present_flag : 1;
    uint32_t    sps_extension_present_flag : 1;
    uint32_t    sps_range_extension_flag : 1;
    uint32_t    transform_skip_rotation_enabled_flag : 1;
    uint32_t    transform_skip_context_enabled_flag : 1;
    uint32_t    implicit_rdpcm_enabled_flag : 1;
    uint32_t    explicit_rdpcm_enabled_flag : 1;
    uint32_t    extended_precision_processing_flag : 1;
    uint32_t    intra_smoothing_disabled_flag : 1;
    uint32_t    high_precision_offsets_enabled_flag : 1;
    uint32_t    persistent_rice_adaptation_enabled_flag : 1;
    uint32_t    cabac_bypass_alignment_enabled_flag : 1;
    uint32_t    sps_scc_extension_flag : 1;
    uint32_t    sps_curr_pic_ref_enabled_flag : 1;
    uint32_t    palette_mode_enabled_flag : 1;
    uint32_t    sps_palette_predictor_initializers_present_flag : 1;
    uint32_t    intra_boundary_filtering_disabled_flag : 1;
} StdVideoH265SpsFlags;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265ShortTermRefPicSetFlags {
    uint32_t    inter_ref_pic_set_prediction_flag : 1;
    uint32_t    delta_rps_sign : 1;
} StdVideoH265ShortTermRefPicSetFlags;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265ShortTermRefPicSet {
    StdVideoH265ShortTermRefPicSetFlags    flags;
    uint32_t    delta_idx_minus1;
    uint16_t    use_delta_flag;
    uint16_t    abs_delta_rps_minus1;
    uint16_t    used_by_curr_pic_flag;
    uint16_t    used_by_curr_pic_s0_flag;
    uint16_t    used_by_curr_pic_s1_flag;
    uint16_t    reserved1;
    uint8_t    reserved2;
    uint8_t    reserved3;
    uint8_t    num_negative_pics;
    uint8_t    num_positive_pics;
    uint16_t    delta_poc_s0_minus1[STD_VIDEO_H265_MAX_DPB_SIZE];
    uint16_t    delta_poc_s1_minus1[STD_VIDEO_H265_MAX_DPB_SIZE];
} StdVideoH265ShortTermRefPicSet;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265LongTermRefPicsSps {
    uint32_t    used_by_curr_pic_lt_sps_flag;
    uint32_t    lt_ref_pic_poc_lsb_sps[STD_VIDEO_H265_MAX_LONG_TERM_REF_PICS_SPS];
} StdVideoH265LongTermRefPicsSps;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265SequenceParameterSet {
    StdVideoH265SpsFlags    flags;
    StdVideoH265ChromaFormatIdc    chroma_format_idc;
    uint32_t    pic_width_in_luma_samples;
    uint32_t    pic_height_in_luma_samples;
    uint8_t    sps_video_parameter_set_id;
    uint8_t    sps_max_sub_layers_minus1;
    uint8_t    sps_seq_parameter_set_id;
    uint8_t    bit_depth_luma_minus8;
    uint8_t    bit_depth_chroma_minus8;
    uint8_t    log2_max_pic_order_cnt_lsb_minus4;
    uint8_t    log2_min_luma_coding_block_size_minus3;
    uint8_t    log2_diff_max_min_luma_coding_block_size;
    uint8_t    log2_min_luma_transform_block_size_minus2;
    uint8_t    log2_diff_max_min_luma_transform_block_size;
    uint8_t    max_transform_hierarchy_depth_inter;
    uint8_t    max_transform_hierarchy_depth_intra;
    uint8_t    num_short_term_ref_pic_sets;
    uint8_t    num_long_term_ref_pics_sps;
    uint8_t    pcm_sample_bit_depth_luma_minus1;
    uint8_t    pcm_sample_bit_depth_chroma_minus1;
    uint8_t    log2_min_pcm_luma_coding_block_size_minus3;
    uint8_t    log2_diff_max_min_pcm_luma_coding_block_size;
    uint8_t    reserved1;
    uint8_t    reserved2;
    uint8_t    palette_max_size;
    uint8_t    delta_palette_max_predictor_size;
    uint8_t    motion_vector_resolution_control_idc;
    uint8_t    sps_num_palette_predictor_initializers_minus1;
    uint32_t    conf_win_left_offset;
    uint32_t    conf_win_right_offset;
    uint32_t    conf_win_top_offset;
    uint32_t    conf_win_bottom_offset;
    const StdVideoH265ProfileTierLevel*    pProfileTierLevel;
    const StdVideoH265DecPicBufMgr*    pDecPicBufMgr;
    const StdVideoH265ScalingLists*    pScalingLists;
    const StdVideoH265ShortTermRefPicSet*    pShortTermRefPicSet;
    const StdVideoH265LongTermRefPicsSps*    pLongTermRefPicsSps;
    const StdVideoH265SequenceParameterSetVui*    pSequenceParameterSetVui;
    const StdVideoH265PredictorPaletteEntries*    pPredictorPaletteEntries;
} StdVideoH265SequenceParameterSet;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265PpsFlags {
    uint32_t    dependent_slice_segments_enabled_flag : 1;
    uint32_t    output_flag_present_flag : 1;
    uint32_t    sign_data_hiding_enabled_flag : 1;
    uint32_t    cabac_init_present_flag : 1;
    uint32_t    constrained_intra_pred_flag : 1;
    uint32_t    transform_skip_enabled_flag : 1;
    uint32_t    cu_qp_delta_enabled_flag : 1;
    uint32_t    pps_slice_chroma_qp_offsets_present_flag : 1;
    uint32_t    weighted_pred_flag : 1;
    uint32_t    weighted_bipred_flag : 1;
    uint32_t    transquant_bypass_enabled_flag : 1;
    uint32_t    tiles_enabled_flag : 1;
    uint32_t    entropy_coding_sync_enabled_flag : 1;
    uint32_t    uniform_spacing_flag : 1;
    uint32_t    loop_filter_across_tiles_enabled_flag : 1;
    uint32_t    pps_loop_filter_across_slices_enabled_flag : 1;
    uint32_t    deblocking_filter_control_present_flag : 1;
    uint32_t    deblocking_filter_override_enabled_flag : 1;
    uint32_t    pps_deblocking_filter_disabled_flag : 1;
    uint32_t    pps_scaling_list_data_present_flag : 1;
    uint32_t    lists_modification_present_flag : 1;
    uint32_t    slice_segment_header_extension_present_flag : 1;
    uint32_t    pps_extension_present_flag : 1;
    uint32_t    cross_component_prediction_enabled_flag : 1;
    uint32_t    chroma_qp_offset_list_enabled_flag : 1;
    uint32_t    pps_curr_pic_ref_enabled_flag : 1;
    uint32_t    residual_adaptive_colour_transform_enabled_flag : 1;
    uint32_t    pps_slice_act_qp_offsets_present_flag : 1;
    uint32_t    pps_palette_predictor_initializers_present_flag : 1;
    uint32_t    monochrome_palette_flag : 1;
    uint32_t    pps_range_extension_flag : 1;
} StdVideoH265PpsFlags;

// struct ext: vulkan_video_codec_h265std
typedef struct StdVideoH265PictureParameterSet {
    StdVideoH265PpsFlags    flags;
    uint8_t    pps_pic_parameter_set_id;
    uint8_t    pps_seq_parameter_set_id;
    uint8_t    sps_video_parameter_set_id;
    uint8_t    num_extra_slice_header_bits;
    uint8_t    num_ref_idx_l0_default_active_minus1;
    uint8_t    num_ref_idx_l1_default_active_minus1;
    int8_t    init_qp_minus26;
    uint8_t    diff_cu_qp_delta_depth;
    int8_t    pps_cb_qp_offset;
    int8_t    pps_cr_qp_offset;
    int8_t    pps_beta_offset_div2;
    int8_t    pps_tc_offset_div2;
    uint8_t    log2_parallel_merge_level_minus2;
    uint8_t    log2_max_transform_skip_block_size_minus2;
    uint8_t    diff_cu_chroma_qp_offset_depth;
    uint8_t    chroma_qp_offset_list_len_minus1;
    int8_t    cb_qp_offset_list[STD_VIDEO_H265_CHROMA_QP_OFFSET_LIST_SIZE];
    int8_t    cr_qp_offset_list[STD_VIDEO_H265_CHROMA_QP_OFFSET_LIST_SIZE];
    uint8_t    log2_sao_offset_scale_luma;
    uint8_t    log2_sao_offset_scale_chroma;
    int8_t    pps_act_y_qp_offset_plus5;
    int8_t    pps_act_cb_qp_offset_plus5;
    int8_t    pps_act_cr_qp_offset_plus3;
    uint8_t    pps_num_palette_predictor_initializers;
    uint8_t    luma_bit_depth_entry_minus8;
    uint8_t    chroma_bit_depth_entry_minus8;
    uint8_t    num_tile_columns_minus1;
    uint8_t    num_tile_rows_minus1;
    uint8_t    reserved1;
    uint8_t    reserved2;
    uint16_t    column_width_minus1[STD_VIDEO_H265_CHROMA_QP_OFFSET_TILE_COLS_LIST_SIZE];
    uint16_t    row_height_minus1[STD_VIDEO_H265_CHROMA_QP_OFFSET_TILE_ROWS_LIST_SIZE];
    uint32_t    reserved3;
    const StdVideoH265ScalingLists*    pScalingLists;
    const StdVideoH265PredictorPaletteEntries*    pPredictorPaletteEntries;
} StdVideoH265PictureParameterSet;

struct VideoDecodeH265ProfileInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeH265ProfileInfoKHR;
	const void*    pNext = {};
	StdVideoH265ProfileIdc    stdProfileIdc = {};
};

struct VideoDecodeH265CapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoDecodeH265CapabilitiesKHR;
	void*    pNext = {};
	StdVideoH265LevelIdc    maxLevelIdc = {};
};

struct VideoDecodeH265SessionParametersAddInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeH265SessionParametersAddInfoKHR;
	const void*    pNext = {};
	uint32_t    stdVPSCount = {};
	const StdVideoH265VideoParameterSet*    pStdVPSs = {};
	uint32_t    stdSPSCount = {};
	const StdVideoH265SequenceParameterSet*    pStdSPSs = {};
	uint32_t    stdPPSCount = {};
	const StdVideoH265PictureParameterSet*    pStdPPSs = {};
};

struct VideoDecodeH265SessionParametersCreateInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeH265SessionParametersCreateInfoKHR;
	const void*    pNext = {};
	uint32_t    maxStdVPSCount = {};
	uint32_t    maxStdSPSCount = {};
	uint32_t    maxStdPPSCount = {};
	const vk::VideoDecodeH265SessionParametersAddInfoKHR* pParametersAddInfo = {};
};

#define VK_STD_VULKAN_VIDEO_CODEC_H265_DECODE_SPEC_VERSION VK_STD_VULKAN_VIDEO_CODEC_H265_DECODE_API_VERSION_1_0_0
#define VK_STD_VULKAN_VIDEO_CODEC_H265_DECODE_EXTENSION_NAME "VK_STD_vulkan_video_codec_h265_decode"
#define STD_VIDEO_DECODE_H265_REF_PIC_SET_LIST_SIZE 8


// vulkan_video_codec_h265std_decode is a preprocessor guard. Do not pass it to API calls.
#define vulkan_video_codec_h265std_decode 1
// struct ext: vulkan_video_codec_h265std_decode
typedef struct StdVideoDecodeH265PictureInfoFlags {
    uint32_t    IrapPicFlag : 1;
    uint32_t    IdrPicFlag  : 1;
    uint32_t    IsReference : 1;
    uint32_t    short_term_ref_pic_set_sps_flag : 1;
} StdVideoDecodeH265PictureInfoFlags;

// struct ext: vulkan_video_codec_h265std_decode
typedef struct StdVideoDecodeH265PictureInfo {
    StdVideoDecodeH265PictureInfoFlags    flags;
    uint8_t    sps_video_parameter_set_id;
    uint8_t    pps_seq_parameter_set_id;
    uint8_t    pps_pic_parameter_set_id;
    uint8_t    NumDeltaPocsOfRefRpsIdx;
    int32_t    PicOrderCntVal;
    uint16_t    NumBitsForSTRefPicSetInSlice;
    uint16_t    reserved;
    uint8_t    RefPicSetStCurrBefore[STD_VIDEO_DECODE_H265_REF_PIC_SET_LIST_SIZE];
    uint8_t    RefPicSetStCurrAfter[STD_VIDEO_DECODE_H265_REF_PIC_SET_LIST_SIZE];
    uint8_t    RefPicSetLtCurr[STD_VIDEO_DECODE_H265_REF_PIC_SET_LIST_SIZE];
} StdVideoDecodeH265PictureInfo;

// struct ext: vulkan_video_codec_h265std_decode
typedef struct StdVideoDecodeH265ReferenceInfoFlags {
    uint32_t    used_for_long_term_reference : 1;
    uint32_t    unused_for_reference : 1;
} StdVideoDecodeH265ReferenceInfoFlags;

// struct ext: vulkan_video_codec_h265std_decode
typedef struct StdVideoDecodeH265ReferenceInfo {
    StdVideoDecodeH265ReferenceInfoFlags    flags;
    int32_t    PicOrderCntVal;
} StdVideoDecodeH265ReferenceInfo;

struct VideoDecodeH265PictureInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeH265PictureInfoKHR;
	const void*    pNext = {};
	const StdVideoDecodeH265PictureInfo*    pStdPictureInfo = {};
	uint32_t    sliceSegmentCount = {};
	const uint32_t*    pSliceSegmentOffsets = {};
};

struct VideoDecodeH265DpbSlotInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeH265DpbSlotInfoKHR;
	const void*    pNext = {};
	const StdVideoDecodeH265ReferenceInfo*    pStdReferenceInfo = {};
};

#define STD_VIDEO_AV1_NUM_REF_FRAMES 8
#define STD_VIDEO_AV1_REFS_PER_FRAME 7
#define STD_VIDEO_AV1_TOTAL_REFS_PER_FRAME 8
#define STD_VIDEO_AV1_MAX_TILE_COLS 64
#define STD_VIDEO_AV1_MAX_TILE_ROWS 64
#define STD_VIDEO_AV1_MAX_SEGMENTS 8
#define STD_VIDEO_AV1_SEG_LVL_MAX 8
#define STD_VIDEO_AV1_PRIMARY_REF_NONE 7
#define STD_VIDEO_AV1_SELECT_INTEGER_MV 2
#define STD_VIDEO_AV1_SELECT_SCREEN_CONTENT_TOOLS 2
#define STD_VIDEO_AV1_SKIP_MODE_FRAMES 2
#define STD_VIDEO_AV1_MAX_LOOP_FILTER_STRENGTHS 4
#define STD_VIDEO_AV1_LOOP_FILTER_ADJUSTMENTS 2
#define STD_VIDEO_AV1_MAX_CDEF_FILTER_STRENGTHS 8
#define STD_VIDEO_AV1_MAX_NUM_PLANES 3
#define STD_VIDEO_AV1_GLOBAL_MOTION_PARAMS 6
#define STD_VIDEO_AV1_MAX_NUM_Y_POINTS 14
#define STD_VIDEO_AV1_MAX_NUM_CB_POINTS 10
#define STD_VIDEO_AV1_MAX_NUM_CR_POINTS 10
#define STD_VIDEO_AV1_MAX_NUM_POS_LUMA 24
#define STD_VIDEO_AV1_MAX_NUM_POS_CHROMA 25


// vulkan_video_codec_av1std is a preprocessor guard. Do not pass it to API calls.
#define vulkan_video_codec_av1std 1
// enum ext: vulkan_video_codec_av1std
typedef enum StdVideoAV1Profile {
    STD_VIDEO_AV1_PROFILE_MAIN = 0,
    STD_VIDEO_AV1_PROFILE_HIGH = 1,
    STD_VIDEO_AV1_PROFILE_PROFESSIONAL = 2,
    STD_VIDEO_AV1_PROFILE_INVALID = 0x7FFFFFFF
} StdVideoAV1Profile;

// enum ext: vulkan_video_codec_av1std
typedef enum StdVideoAV1Level {
    STD_VIDEO_AV1_LEVEL_2_0 = 0,
    STD_VIDEO_AV1_LEVEL_2_1 = 1,
    STD_VIDEO_AV1_LEVEL_2_2 = 2,
    STD_VIDEO_AV1_LEVEL_2_3 = 3,
    STD_VIDEO_AV1_LEVEL_3_0 = 4,
    STD_VIDEO_AV1_LEVEL_3_1 = 5,
    STD_VIDEO_AV1_LEVEL_3_2 = 6,
    STD_VIDEO_AV1_LEVEL_3_3 = 7,
    STD_VIDEO_AV1_LEVEL_4_0 = 8,
    STD_VIDEO_AV1_LEVEL_4_1 = 9,
    STD_VIDEO_AV1_LEVEL_4_2 = 10,
    STD_VIDEO_AV1_LEVEL_4_3 = 11,
    STD_VIDEO_AV1_LEVEL_5_0 = 12,
    STD_VIDEO_AV1_LEVEL_5_1 = 13,
    STD_VIDEO_AV1_LEVEL_5_2 = 14,
    STD_VIDEO_AV1_LEVEL_5_3 = 15,
    STD_VIDEO_AV1_LEVEL_6_0 = 16,
    STD_VIDEO_AV1_LEVEL_6_1 = 17,
    STD_VIDEO_AV1_LEVEL_6_2 = 18,
    STD_VIDEO_AV1_LEVEL_6_3 = 19,
    STD_VIDEO_AV1_LEVEL_7_0 = 20,
    STD_VIDEO_AV1_LEVEL_7_1 = 21,
    STD_VIDEO_AV1_LEVEL_7_2 = 22,
    STD_VIDEO_AV1_LEVEL_7_3 = 23,
    STD_VIDEO_AV1_LEVEL_INVALID = 0x7FFFFFFF
} StdVideoAV1Level;

// enum ext: vulkan_video_codec_av1std
typedef enum StdVideoAV1FrameType {
    STD_VIDEO_AV1_FRAME_TYPE_KEY = 0,
    STD_VIDEO_AV1_FRAME_TYPE_INTER = 1,
    STD_VIDEO_AV1_FRAME_TYPE_INTRA_ONLY = 2,
    STD_VIDEO_AV1_FRAME_TYPE_SWITCH = 3,
    STD_VIDEO_AV1_FRAME_TYPE_INVALID = 0x7FFFFFFF
} StdVideoAV1FrameType;

// enum ext: vulkan_video_codec_av1std
typedef enum StdVideoAV1ReferenceName {
    STD_VIDEO_AV1_REFERENCE_NAME_INTRA_FRAME = 0,
    STD_VIDEO_AV1_REFERENCE_NAME_LAST_FRAME = 1,
    STD_VIDEO_AV1_REFERENCE_NAME_LAST2_FRAME = 2,
    STD_VIDEO_AV1_REFERENCE_NAME_LAST3_FRAME = 3,
    STD_VIDEO_AV1_REFERENCE_NAME_GOLDEN_FRAME = 4,
    STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME = 5,
    STD_VIDEO_AV1_REFERENCE_NAME_ALTREF2_FRAME = 6,
    STD_VIDEO_AV1_REFERENCE_NAME_ALTREF_FRAME = 7,
    STD_VIDEO_AV1_REFERENCE_NAME_INVALID = 0x7FFFFFFF
} StdVideoAV1ReferenceName;

// enum ext: vulkan_video_codec_av1std
typedef enum StdVideoAV1InterpolationFilter {
    STD_VIDEO_AV1_INTERPOLATION_FILTER_EIGHTTAP = 0,
    STD_VIDEO_AV1_INTERPOLATION_FILTER_EIGHTTAP_SMOOTH = 1,
    STD_VIDEO_AV1_INTERPOLATION_FILTER_EIGHTTAP_SHARP = 2,
    STD_VIDEO_AV1_INTERPOLATION_FILTER_BILINEAR = 3,
    STD_VIDEO_AV1_INTERPOLATION_FILTER_SWITCHABLE = 4,
    STD_VIDEO_AV1_INTERPOLATION_FILTER_INVALID = 0x7FFFFFFF
} StdVideoAV1InterpolationFilter;

// enum ext: vulkan_video_codec_av1std
typedef enum StdVideoAV1TxMode {
    STD_VIDEO_AV1_TX_MODE_ONLY_4X4 = 0,
    STD_VIDEO_AV1_TX_MODE_LARGEST = 1,
    STD_VIDEO_AV1_TX_MODE_SELECT = 2,
    STD_VIDEO_AV1_TX_MODE_INVALID = 0x7FFFFFFF
} StdVideoAV1TxMode;

// enum ext: vulkan_video_codec_av1std
typedef enum StdVideoAV1FrameRestorationType {
    STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_NONE = 0,
    STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_WIENER = 1,
    STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_SGRPROJ = 2,
    STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_SWITCHABLE = 3,
    STD_VIDEO_AV1_FRAME_RESTORATION_TYPE_INVALID = 0x7FFFFFFF
} StdVideoAV1FrameRestorationType;

// enum ext: vulkan_video_codec_av1std
typedef enum StdVideoAV1ColorPrimaries {
    STD_VIDEO_AV1_COLOR_PRIMARIES_BT_709 = 1,
    STD_VIDEO_AV1_COLOR_PRIMARIES_UNSPECIFIED = 2,
    STD_VIDEO_AV1_COLOR_PRIMARIES_BT_UNSPECIFIED = 2,
    STD_VIDEO_AV1_COLOR_PRIMARIES_BT_470_M = 4,
    STD_VIDEO_AV1_COLOR_PRIMARIES_BT_470_B_G = 5,
    STD_VIDEO_AV1_COLOR_PRIMARIES_BT_601 = 6,
    STD_VIDEO_AV1_COLOR_PRIMARIES_SMPTE_240 = 7,
    STD_VIDEO_AV1_COLOR_PRIMARIES_GENERIC_FILM = 8,
    STD_VIDEO_AV1_COLOR_PRIMARIES_BT_2020 = 9,
    STD_VIDEO_AV1_COLOR_PRIMARIES_XYZ = 10,
    STD_VIDEO_AV1_COLOR_PRIMARIES_SMPTE_431 = 11,
    STD_VIDEO_AV1_COLOR_PRIMARIES_SMPTE_432 = 12,
    STD_VIDEO_AV1_COLOR_PRIMARIES_EBU_3213 = 22,
    STD_VIDEO_AV1_COLOR_PRIMARIES_INVALID = 0x7FFFFFFF
} StdVideoAV1ColorPrimaries;

// enum ext: vulkan_video_codec_av1std
typedef enum StdVideoAV1TransferCharacteristics {
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_RESERVED_0 = 0,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_709 = 1,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_UNSPECIFIED = 2,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_RESERVED_3 = 3,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_470_M = 4,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_470_B_G = 5,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_601 = 6,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_SMPTE_240 = 7,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_LINEAR = 8,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_LOG_100 = 9,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_LOG_100_SQRT10 = 10,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_IEC_61966 = 11,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_1361 = 12,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_SRGB = 13,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_2020_10_BIT = 14,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_BT_2020_12_BIT = 15,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_SMPTE_2084 = 16,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_SMPTE_428 = 17,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_HLG = 18,
    STD_VIDEO_AV1_TRANSFER_CHARACTERISTICS_INVALID = 0x7FFFFFFF
} StdVideoAV1TransferCharacteristics;

// enum ext: vulkan_video_codec_av1std
typedef enum StdVideoAV1MatrixCoefficients {
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_IDENTITY = 0,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_709 = 1,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_UNSPECIFIED = 2,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_RESERVED_3 = 3,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_FCC = 4,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_470_B_G = 5,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_601 = 6,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_SMPTE_240 = 7,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_SMPTE_YCGCO = 8,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_2020_NCL = 9,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_BT_2020_CL = 10,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_SMPTE_2085 = 11,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_CHROMAT_NCL = 12,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_CHROMAT_CL = 13,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_ICTCP = 14,
    STD_VIDEO_AV1_MATRIX_COEFFICIENTS_INVALID = 0x7FFFFFFF
} StdVideoAV1MatrixCoefficients;

// enum ext: vulkan_video_codec_av1std
typedef enum StdVideoAV1ChromaSamplePosition {
    STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_UNKNOWN = 0,
    STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_VERTICAL = 1,
    STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_COLOCATED = 2,
    STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_RESERVED = 3,
    STD_VIDEO_AV1_CHROMA_SAMPLE_POSITION_INVALID = 0x7FFFFFFF
} StdVideoAV1ChromaSamplePosition;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1ColorConfigFlags {
    uint32_t    mono_chrome : 1;
    uint32_t    color_range : 1;
    uint32_t    separate_uv_delta_q : 1;
    uint32_t    color_description_present_flag : 1;
    uint32_t    reserved : 28;
} StdVideoAV1ColorConfigFlags;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1ColorConfig {
    StdVideoAV1ColorConfigFlags    flags;
    uint8_t    BitDepth;
    uint8_t    subsampling_x;
    uint8_t    subsampling_y;
    uint8_t    reserved1;
    StdVideoAV1ColorPrimaries    color_primaries;
    StdVideoAV1TransferCharacteristics    transfer_characteristics;
    StdVideoAV1MatrixCoefficients    matrix_coefficients;
    StdVideoAV1ChromaSamplePosition    chroma_sample_position;
} StdVideoAV1ColorConfig;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1TimingInfoFlags {
    uint32_t    equal_picture_interval : 1;
    uint32_t    reserved : 31;
} StdVideoAV1TimingInfoFlags;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1TimingInfo {
    StdVideoAV1TimingInfoFlags    flags;
    uint32_t    num_units_in_display_tick;
    uint32_t    time_scale;
    uint32_t    num_ticks_per_picture_minus_1;
} StdVideoAV1TimingInfo;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1LoopFilterFlags {
    uint32_t    loop_filter_delta_enabled : 1;
    uint32_t    loop_filter_delta_update : 1;
    uint32_t    reserved : 30;
} StdVideoAV1LoopFilterFlags;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1LoopFilter {
    StdVideoAV1LoopFilterFlags    flags;
    uint8_t    loop_filter_level[STD_VIDEO_AV1_MAX_LOOP_FILTER_STRENGTHS];
    uint8_t    loop_filter_sharpness;
    uint8_t    update_ref_delta;
    int8_t    loop_filter_ref_deltas[STD_VIDEO_AV1_TOTAL_REFS_PER_FRAME];
    uint8_t    update_mode_delta;
    int8_t    loop_filter_mode_deltas[STD_VIDEO_AV1_LOOP_FILTER_ADJUSTMENTS];
} StdVideoAV1LoopFilter;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1QuantizationFlags {
    uint32_t    using_qmatrix : 1;
    uint32_t    diff_uv_delta : 1;
    uint32_t    reserved : 30;
} StdVideoAV1QuantizationFlags;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1Quantization {
    StdVideoAV1QuantizationFlags    flags;
    uint8_t    base_q_idx;
    int8_t    DeltaQYDc;
    int8_t    DeltaQUDc;
    int8_t    DeltaQUAc;
    int8_t    DeltaQVDc;
    int8_t    DeltaQVAc;
    uint8_t    qm_y;
    uint8_t    qm_u;
    uint8_t    qm_v;
} StdVideoAV1Quantization;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1Segmentation {
    uint8_t    FeatureEnabled[STD_VIDEO_AV1_MAX_SEGMENTS];
    int16_t    FeatureData[STD_VIDEO_AV1_MAX_SEGMENTS];
} StdVideoAV1Segmentation;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1TileInfoFlags {
    uint32_t    uniform_tile_spacing_flag : 1;
    uint32_t    reserved : 31;
} StdVideoAV1TileInfoFlags;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1TileInfo {
    StdVideoAV1TileInfoFlags    flags;
    uint8_t    TileCols;
    uint8_t    TileRows;
    uint16_t    context_update_tile_id;
    uint8_t    tile_size_bytes_minus_1;
    uint8_t    reserved1[7];
    const uint16_t*    pMiColStarts;
    const uint16_t*    pMiRowStarts;
    const uint16_t*    pWidthInSbsMinus1;
    const uint16_t*    pHeightInSbsMinus1;
} StdVideoAV1TileInfo;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1CDEF {
    uint8_t    cdef_damping_minus_3;
    uint8_t    cdef_bits;
    uint8_t    cdef_y_pri_strength[STD_VIDEO_AV1_MAX_CDEF_FILTER_STRENGTHS];
    uint8_t    cdef_y_sec_strength[STD_VIDEO_AV1_MAX_CDEF_FILTER_STRENGTHS];
    uint8_t    cdef_uv_pri_strength[STD_VIDEO_AV1_MAX_CDEF_FILTER_STRENGTHS];
    uint8_t    cdef_uv_sec_strength[STD_VIDEO_AV1_MAX_CDEF_FILTER_STRENGTHS];
} StdVideoAV1CDEF;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1LoopRestoration {
    StdVideoAV1FrameRestorationType    FrameRestorationType[STD_VIDEO_AV1_MAX_NUM_PLANES];
    uint16_t    LoopRestorationSize[STD_VIDEO_AV1_MAX_NUM_PLANES];
} StdVideoAV1LoopRestoration;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1GlobalMotion {
    uint8_t    GmType[STD_VIDEO_AV1_NUM_REF_FRAMES];
    int32_t    gm_params[STD_VIDEO_AV1_NUM_REF_FRAMES];
} StdVideoAV1GlobalMotion;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1FilmGrainFlags {
    uint32_t    chroma_scaling_from_luma : 1;
    uint32_t    overlap_flag : 1;
    uint32_t    clip_to_restricted_range : 1;
    uint32_t    update_grain : 1;
    uint32_t    reserved : 28;
} StdVideoAV1FilmGrainFlags;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1FilmGrain {
    StdVideoAV1FilmGrainFlags    flags;
    uint8_t    grain_scaling_minus_8;
    uint8_t    ar_coeff_lag;
    uint8_t    ar_coeff_shift_minus_6;
    uint8_t    grain_scale_shift;
    uint16_t    grain_seed;
    uint8_t    film_grain_params_ref_idx;
    uint8_t    num_y_points;
    uint8_t    point_y_value[STD_VIDEO_AV1_MAX_NUM_Y_POINTS];
    uint8_t    point_y_scaling[STD_VIDEO_AV1_MAX_NUM_Y_POINTS];
    uint8_t    num_cb_points;
    uint8_t    point_cb_value[STD_VIDEO_AV1_MAX_NUM_CB_POINTS];
    uint8_t    point_cb_scaling[STD_VIDEO_AV1_MAX_NUM_CB_POINTS];
    uint8_t    num_cr_points;
    uint8_t    point_cr_value[STD_VIDEO_AV1_MAX_NUM_CR_POINTS];
    uint8_t    point_cr_scaling[STD_VIDEO_AV1_MAX_NUM_CR_POINTS];
    int8_t    ar_coeffs_y_plus_128[STD_VIDEO_AV1_MAX_NUM_POS_LUMA];
    int8_t    ar_coeffs_cb_plus_128[STD_VIDEO_AV1_MAX_NUM_POS_CHROMA];
    int8_t    ar_coeffs_cr_plus_128[STD_VIDEO_AV1_MAX_NUM_POS_CHROMA];
    uint8_t    cb_mult;
    uint8_t    cb_luma_mult;
    uint16_t    cb_offset;
    uint8_t    cr_mult;
    uint8_t    cr_luma_mult;
    uint16_t    cr_offset;
} StdVideoAV1FilmGrain;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1SequenceHeaderFlags {
    uint32_t    still_picture : 1;
    uint32_t    reduced_still_picture_header : 1;
    uint32_t    use_128x128_superblock : 1;
    uint32_t    enable_filter_intra : 1;
    uint32_t    enable_intra_edge_filter : 1;
    uint32_t    enable_interintra_compound : 1;
    uint32_t    enable_masked_compound : 1;
    uint32_t    enable_warped_motion : 1;
    uint32_t    enable_dual_filter : 1;
    uint32_t    enable_order_hint : 1;
    uint32_t    enable_jnt_comp : 1;
    uint32_t    enable_ref_frame_mvs : 1;
    uint32_t    frame_id_numbers_present_flag : 1;
    uint32_t    enable_superres : 1;
    uint32_t    enable_cdef : 1;
    uint32_t    enable_restoration : 1;
    uint32_t    film_grain_params_present : 1;
    uint32_t    timing_info_present_flag : 1;
    uint32_t    initial_display_delay_present_flag : 1;
    uint32_t    reserved : 13;
} StdVideoAV1SequenceHeaderFlags;

// struct ext: vulkan_video_codec_av1std
typedef struct StdVideoAV1SequenceHeader {
    StdVideoAV1SequenceHeaderFlags    flags;
    StdVideoAV1Profile    seq_profile;
    uint8_t    frame_width_bits_minus_1;
    uint8_t    frame_height_bits_minus_1;
    uint16_t    max_frame_width_minus_1;
    uint16_t    max_frame_height_minus_1;
    uint8_t    delta_frame_id_length_minus_2;
    uint8_t    additional_frame_id_length_minus_1;
    uint8_t    order_hint_bits_minus_1;
    uint8_t    seq_force_integer_mv;
    uint8_t    seq_force_screen_content_tools;
    uint8_t    reserved1[5];
    const StdVideoAV1ColorConfig*    pColorConfig;
    const StdVideoAV1TimingInfo*    pTimingInfo;
} StdVideoAV1SequenceHeader;

struct VideoDecodeAV1ProfileInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeAv1ProfileInfoKHR;
	const void*    pNext = {};
	StdVideoAV1Profile    stdProfile = {};
	vk::Bool32    filmGrainSupport = {};
};

struct VideoDecodeAV1CapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoDecodeAv1CapabilitiesKHR;
	void*    pNext = {};
	StdVideoAV1Level    maxLevel = {};
};

struct VideoDecodeAV1SessionParametersCreateInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeAv1SessionParametersCreateInfoKHR;
	const void*    pNext = {};
	const StdVideoAV1SequenceHeader*    pStdSequenceHeader = {};
};

typedef struct StdVideoDecodeAV1PictureInfoFlags {
    uint32_t    error_resilient_mode : 1;
    uint32_t    disable_cdf_update : 1;
    uint32_t    use_superres : 1;
    uint32_t    render_and_frame_size_different : 1;
    uint32_t    allow_screen_content_tools : 1;
    uint32_t    is_filter_switchable : 1;
    uint32_t    force_integer_mv : 1;
    uint32_t    frame_size_override_flag : 1;
    uint32_t    buffer_removal_time_present_flag : 1;
    uint32_t    allow_intrabc : 1;
    uint32_t    frame_refs_short_signaling : 1;
    uint32_t    allow_high_precision_mv : 1;
    uint32_t    is_motion_mode_switchable : 1;
    uint32_t    use_ref_frame_mvs : 1;
    uint32_t    disable_frame_end_update_cdf : 1;
    uint32_t    allow_warped_motion : 1;
    uint32_t    reduced_tx_set : 1;
    uint32_t    reference_select : 1;
    uint32_t    skip_mode_present : 1;
    uint32_t    delta_q_present : 1;
    uint32_t    delta_lf_present : 1;
    uint32_t    delta_lf_multi : 1;
    uint32_t    segmentation_enabled : 1;
    uint32_t    segmentation_update_map : 1;
    uint32_t    segmentation_temporal_update : 1;
    uint32_t    segmentation_update_data : 1;
    uint32_t    UsesLr : 1;
    uint32_t    usesChromaLr : 1;
    uint32_t    apply_grain : 1;
    uint32_t    reserved : 3;
} StdVideoDecodeAV1PictureInfoFlags;

// struct ext: vulkan_video_codec_av1std_decode
typedef struct StdVideoDecodeAV1PictureInfo {
    StdVideoDecodeAV1PictureInfoFlags    flags;
    StdVideoAV1FrameType    frame_type;
    uint32_t    current_frame_id;
    uint8_t    OrderHint;
    uint8_t    primary_ref_frame;
    uint8_t    refresh_frame_flags;
    uint8_t    reserved1;
    StdVideoAV1InterpolationFilter    interpolation_filter;
    StdVideoAV1TxMode    TxMode;
    uint8_t    delta_q_res;
    uint8_t    delta_lf_res;
    uint8_t    SkipModeFrame[STD_VIDEO_AV1_SKIP_MODE_FRAMES];
    uint8_t    coded_denom;
    uint8_t    reserved2[3];
    uint8_t    OrderHints[STD_VIDEO_AV1_NUM_REF_FRAMES];
    uint32_t    expectedFrameId[STD_VIDEO_AV1_NUM_REF_FRAMES];
    const StdVideoAV1TileInfo*    pTileInfo;
    const StdVideoAV1Quantization*    pQuantization;
    const StdVideoAV1Segmentation*    pSegmentation;
    const StdVideoAV1LoopFilter*    pLoopFilter;
    const StdVideoAV1CDEF*    pCDEF;
    const StdVideoAV1LoopRestoration*    pLoopRestoration;
    const StdVideoAV1GlobalMotion*    pGlobalMotion;
    const StdVideoAV1FilmGrain*    pFilmGrain;
} StdVideoDecodeAV1PictureInfo;

// struct ext: vulkan_video_codec_av1std_decode
typedef struct StdVideoDecodeAV1ReferenceInfoFlags {
    uint32_t    disable_frame_end_update_cdf : 1;
    uint32_t    segmentation_enabled : 1;
    uint32_t    reserved : 30;
} StdVideoDecodeAV1ReferenceInfoFlags;

// struct ext: vulkan_video_codec_av1std_decode
typedef struct StdVideoDecodeAV1ReferenceInfo {
    StdVideoDecodeAV1ReferenceInfoFlags    flags;
    uint8_t    frame_type;
    uint8_t    RefFrameSignBias;
    uint8_t    OrderHint;
    uint8_t    SavedOrderHints[STD_VIDEO_AV1_NUM_REF_FRAMES];
} StdVideoDecodeAV1ReferenceInfo;

struct VideoDecodeAV1PictureInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeAv1PictureInfoKHR;
	const void*    pNext = {};
	const StdVideoDecodeAV1PictureInfo*    pStdPictureInfo = {};
	int32_t    referenceNameSlotIndices[MaxVideoAv1ReferencesPerFrameKHR] = {};
	uint32_t    frameHeaderOffset = {};
	uint32_t    tileCount = {};
	const uint32_t*    pTileOffsets = {};
	const uint32_t*    pTileSizes = {};
};

struct VideoDecodeAV1DpbSlotInfoKHR {
	vk::StructureType sType = StructureType::eVideoDecodeAv1DpbSlotInfoKHR;
	const void*    pNext = {};
	const StdVideoDecodeAV1ReferenceInfo*    pStdReferenceInfo = {};
};

struct VideoSessionCreateInfoKHR {
	vk::StructureType sType = StructureType::eVideoSessionCreateInfoKHR;
	const void*    pNext = {};
	uint32_t    queueFamilyIndex = {};
	vk::VideoSessionCreateFlagsKHR flags = {};
	const vk::VideoProfileInfoKHR* pVideoProfile = {};
	vk::Format pictureFormat = {};
	vk::Extent2D maxCodedExtent = {};
	vk::Format referencePictureFormat = {};
	uint32_t    maxDpbSlots = {};
	uint32_t    maxActiveReferencePictures = {};
	const vk::ExtensionProperties* pStdHeaderVersion = {};
};

struct VideoSessionParametersCreateInfoKHR {
	vk::StructureType sType = StructureType::eVideoSessionParametersCreateInfoKHR;
	const void*    pNext = {};
	vk::VideoSessionParametersCreateFlagsKHR flags = {};
	vk::VideoSessionParametersKHR videoSessionParametersTemplate = {};
	vk::VideoSessionKHR videoSession = {};
};

struct VideoSessionParametersUpdateInfoKHR {
	vk::StructureType sType = StructureType::eVideoSessionParametersUpdateInfoKHR;
	const void*    pNext = {};
	uint32_t    updateSequenceCount = {};
};

struct VideoEncodeSessionParametersGetInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeSessionParametersGetInfoKHR;
	const void*    pNext = {};
	vk::VideoSessionParametersKHR videoSessionParameters = {};
};

struct VideoEncodeSessionParametersFeedbackInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeSessionParametersFeedbackInfoKHR;
	void*    pNext = {};
	vk::Bool32    hasOverrides = {};
};

struct VideoBeginCodingInfoKHR {
	vk::StructureType sType = StructureType::eVideoBeginCodingInfoKHR;
	const void*    pNext = {};
	vk::VideoBeginCodingFlagsKHR flags = {};
	vk::VideoSessionKHR videoSession = {};
	vk::VideoSessionParametersKHR videoSessionParameters = {};
	uint32_t    referenceSlotCount = {};
	const vk::VideoReferenceSlotInfoKHR* pReferenceSlots = {};
};

struct VideoEndCodingInfoKHR {
	vk::StructureType sType = StructureType::eVideoEndCodingInfoKHR;
	const void*    pNext = {};
	vk::VideoEndCodingFlagsKHR flags = {};
};

struct VideoCodingControlInfoKHR {
	vk::StructureType sType = StructureType::eVideoCodingControlInfoKHR;
	const void*    pNext = {};
	vk::VideoCodingControlFlagsKHR flags = {};
};

struct VideoEncodeUsageInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeUsageInfoKHR;
	const void*    pNext = {};
	vk::VideoEncodeUsageFlagsKHR videoUsageHints = {};
	vk::VideoEncodeContentFlagsKHR videoContentHints = {};
	vk::VideoEncodeTuningModeKHR tuningMode = {};
};

struct VideoEncodeInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeInfoKHR;
	const void*    pNext = {};
	vk::VideoEncodeFlagsKHR flags = {};
	vk::Buffer dstBuffer = {};
	vk::DeviceSize    dstBufferOffset = {};
	vk::DeviceSize    dstBufferRange = {};
	vk::VideoPictureResourceInfoKHR srcPictureResource = {};
	const vk::VideoReferenceSlotInfoKHR* pSetupReferenceSlot = {};
	uint32_t    referenceSlotCount = {};
	const vk::VideoReferenceSlotInfoKHR* pReferenceSlots = {};
	uint32_t    precedingExternallyEncodedBytes = {};
};

struct VideoEncodeQuantizationMapInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeQuantizationMapInfoKHR;
	const void*    pNext = {};
	vk::ImageView quantizationMap = {};
	vk::Extent2D quantizationMapExtent = {};
};

struct VideoEncodeQuantizationMapSessionParametersCreateInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeQuantizationMapSessionParametersCreateInfoKHR;
	const void*    pNext = {};
	vk::Extent2D quantizationMapTexelSize = {};
};

struct PhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    videoEncodeQuantizationMap = {};
};

struct QueryPoolVideoEncodeFeedbackCreateInfoKHR {
	vk::StructureType sType = StructureType::eQueryPoolVideoEncodeFeedbackCreateInfoKHR;
	const void*    pNext = {};
	vk::VideoEncodeFeedbackFlagsKHR encodeFeedbackFlags = {};
};

struct VideoEncodeQualityLevelInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeQualityLevelInfoKHR;
	const void*    pNext = {};
	uint32_t    qualityLevel = {};
};

struct PhysicalDeviceVideoEncodeQualityLevelInfoKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceVideoEncodeQualityLevelInfoKHR;
	const void*    pNext = {};
	const vk::VideoProfileInfoKHR* pVideoProfile = {};
	uint32_t    qualityLevel = {};
};

struct VideoEncodeQualityLevelPropertiesKHR {
	vk::StructureType sType = StructureType::eVideoEncodeQualityLevelPropertiesKHR;
	void*    pNext = {};
	vk::VideoEncodeRateControlModeFlagBitsKHR preferredRateControlMode = {};
	uint32_t    preferredRateControlLayerCount = {};
};

struct VideoEncodeRateControlLayerInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeRateControlLayerInfoKHR;
	const void*    pNext = {};
	uint64_t    averageBitrate = {};
	uint64_t    maxBitrate = {};
	uint32_t    frameRateNumerator = {};
	uint32_t    frameRateDenominator = {};
};

struct VideoEncodeRateControlInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeRateControlInfoKHR;
	const void*    pNext = {};
	vk::VideoEncodeRateControlFlagsKHR flags = {};
	vk::VideoEncodeRateControlModeFlagBitsKHR rateControlMode = {};
	uint32_t    layerCount = {};
	const vk::VideoEncodeRateControlLayerInfoKHR* pLayers = {};
	uint32_t    virtualBufferSizeInMs = {};
	uint32_t    initialVirtualBufferSizeInMs = {};
};

struct VideoEncodeCapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoEncodeCapabilitiesKHR;
	void*    pNext = {};
	vk::VideoEncodeCapabilityFlagsKHR flags = {};
	vk::VideoEncodeRateControlModeFlagsKHR rateControlModes = {};
	uint32_t    maxRateControlLayers = {};
	uint64_t    maxBitrate = {};
	uint32_t    maxQualityLevels = {};
	vk::Extent2D encodeInputPictureGranularity = {};
	vk::VideoEncodeFeedbackFlagsKHR supportedEncodeFeedbackFlags = {};
};

struct VideoEncodeH264CapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264CapabilitiesKHR;
	void*    pNext = {};
	vk::VideoEncodeH264CapabilityFlagsKHR flags = {};
	StdVideoH264LevelIdc    maxLevelIdc = {};
	uint32_t    maxSliceCount = {};
	uint32_t    maxPPictureL0ReferenceCount = {};
	uint32_t    maxBPictureL0ReferenceCount = {};
	uint32_t    maxL1ReferenceCount = {};
	uint32_t    maxTemporalLayerCount = {};
	vk::Bool32    expectDyadicTemporalLayerPattern = {};
	int32_t    minQp = {};
	int32_t    maxQp = {};
	vk::Bool32    prefersGopRemainingFrames = {};
	vk::Bool32    requiresGopRemainingFrames = {};
	vk::VideoEncodeH264StdFlagsKHR stdSyntaxFlags = {};
};

struct VideoEncodeH264SessionCreateInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264SessionCreateInfoKHR;
	const void*    pNext = {};
	vk::Bool32    useMaxLevelIdc = {};
	StdVideoH264LevelIdc    maxLevelIdc = {};
};

struct VideoEncodeH264SessionParametersAddInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264SessionParametersAddInfoKHR;
	const void*    pNext = {};
	uint32_t    stdSPSCount = {};
	const StdVideoH264SequenceParameterSet*    pStdSPSs = {};
	uint32_t    stdPPSCount = {};
	const StdVideoH264PictureParameterSet*    pStdPPSs = {};
};

struct VideoEncodeH264SessionParametersCreateInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264SessionParametersCreateInfoKHR;
	const void*    pNext = {};
	uint32_t    maxStdSPSCount = {};
	uint32_t    maxStdPPSCount = {};
	const vk::VideoEncodeH264SessionParametersAddInfoKHR* pParametersAddInfo = {};
};

struct VideoEncodeH264SessionParametersGetInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264SessionParametersGetInfoKHR;
	const void*    pNext = {};
	vk::Bool32    writeStdSPS = {};
	vk::Bool32    writeStdPPS = {};
	uint32_t    stdSPSId = {};
	uint32_t    stdPPSId = {};
};

struct VideoEncodeH264SessionParametersFeedbackInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264SessionParametersFeedbackInfoKHR;
	void*    pNext = {};
	vk::Bool32    hasStdSPSOverrides = {};
	vk::Bool32    hasStdPPSOverrides = {};
};

// struct ext: vulkan_video_codec_h264std_encode
typedef struct StdVideoEncodeH264WeightTableFlags {
    uint32_t    luma_weight_l0_flag;
    uint32_t    chroma_weight_l0_flag;
    uint32_t    luma_weight_l1_flag;
    uint32_t    chroma_weight_l1_flag;
} StdVideoEncodeH264WeightTableFlags;

// struct ext: vulkan_video_codec_h264std_encode
typedef struct StdVideoEncodeH264WeightTable {
    StdVideoEncodeH264WeightTableFlags    flags;
    uint8_t    luma_log2_weight_denom;
    uint8_t    chroma_log2_weight_denom;
    int8_t    luma_weight_l0[STD_VIDEO_H264_MAX_NUM_LIST_REF];
    int8_t    luma_offset_l0[STD_VIDEO_H264_MAX_NUM_LIST_REF];
    int8_t    chroma_weight_l0[STD_VIDEO_H264_MAX_NUM_LIST_REF];
    int8_t    chroma_offset_l0[STD_VIDEO_H264_MAX_NUM_LIST_REF];
    int8_t    luma_weight_l1[STD_VIDEO_H264_MAX_NUM_LIST_REF];
    int8_t    luma_offset_l1[STD_VIDEO_H264_MAX_NUM_LIST_REF];
    int8_t    chroma_weight_l1[STD_VIDEO_H264_MAX_NUM_LIST_REF];
    int8_t    chroma_offset_l1[STD_VIDEO_H264_MAX_NUM_LIST_REF];
} StdVideoEncodeH264WeightTable;

// struct ext: vulkan_video_codec_h264std_encode
typedef struct StdVideoEncodeH264SliceHeaderFlags {
    uint32_t    direct_spatial_mv_pred_flag : 1;
    uint32_t    num_ref_idx_active_override_flag : 1;
    uint32_t    reserved : 30;
} StdVideoEncodeH264SliceHeaderFlags;

// struct ext: vulkan_video_codec_h264std_encode
typedef struct StdVideoEncodeH264PictureInfoFlags {
    uint32_t    IdrPicFlag : 1;
    uint32_t    is_reference : 1;
    uint32_t    no_output_of_prior_pics_flag : 1;
    uint32_t    long_term_reference_flag : 1;
    uint32_t    adaptive_ref_pic_marking_mode_flag : 1;
    uint32_t    reserved : 27;
} StdVideoEncodeH264PictureInfoFlags;

// struct ext: vulkan_video_codec_h264std_encode
typedef struct StdVideoEncodeH264ReferenceInfoFlags {
    uint32_t    used_for_long_term_reference : 1;
    uint32_t    reserved : 31;
} StdVideoEncodeH264ReferenceInfoFlags;

// struct ext: vulkan_video_codec_h264std_encode
typedef struct StdVideoEncodeH264ReferenceListsInfoFlags {
    uint32_t    ref_pic_list_modification_flag_l0 : 1;
    uint32_t    ref_pic_list_modification_flag_l1 : 1;
    uint32_t    reserved : 30;
} StdVideoEncodeH264ReferenceListsInfoFlags;

// struct ext: vulkan_video_codec_h264std_encode
typedef struct StdVideoEncodeH264RefListModEntry {
    StdVideoH264ModificationOfPicNumsIdc    modification_of_pic_nums_idc;
    uint16_t    abs_diff_pic_num_minus1;
    uint16_t    long_term_pic_num;
} StdVideoEncodeH264RefListModEntry;

// struct ext: vulkan_video_codec_h264std_encode
typedef struct StdVideoEncodeH264RefPicMarkingEntry {
    StdVideoH264MemMgmtControlOp    memory_management_control_operation;
    uint16_t    difference_of_pic_nums_minus1;
    uint16_t    long_term_pic_num;
    uint16_t    long_term_frame_idx;
    uint16_t    max_long_term_frame_idx_plus1;
} StdVideoEncodeH264RefPicMarkingEntry;

// struct ext: vulkan_video_codec_h264std_encode
typedef struct StdVideoEncodeH264ReferenceListsInfo {
    StdVideoEncodeH264ReferenceListsInfoFlags    flags;
    uint8_t    num_ref_idx_l0_active_minus1;
    uint8_t    num_ref_idx_l1_active_minus1;
    uint8_t    RefPicList0;
    uint8_t    RefPicList1;
    uint8_t    refList0ModOpCount;
    uint8_t    refList1ModOpCount;
    uint8_t    refPicMarkingOpCount;
    uint8_t    reserved1[7];
    const StdVideoEncodeH264RefListModEntry*    pRefList0ModOperations;
    const StdVideoEncodeH264RefListModEntry*    pRefList1ModOperations;
    const StdVideoEncodeH264RefPicMarkingEntry*    pRefPicMarkingOperations;
} StdVideoEncodeH264ReferenceListsInfo;

// struct ext: vulkan_video_codec_h264std_encode
typedef struct StdVideoEncodeH264PictureInfo {
    StdVideoEncodeH264PictureInfoFlags    flags;
    uint8_t    seq_parameter_set_id;
    uint8_t    pic_parameter_set_id;
    uint16_t    idr_pic_id;
    StdVideoH264PictureType    primary_pic_type;
    uint32_t    frame_num;
    int32_t    PicOrderCnt;
    uint8_t    temporal_id;
    uint8_t    reserved1[3];
    const StdVideoEncodeH264ReferenceListsInfo*    pRefLists;
} StdVideoEncodeH264PictureInfo;

// struct ext: vulkan_video_codec_h264std_encode
typedef struct StdVideoEncodeH264ReferenceInfo {
    StdVideoEncodeH264ReferenceInfoFlags    flags;
    StdVideoH264PictureType    primary_pic_type;
    uint32_t    FrameNum;
    int32_t    PicOrderCnt;
    uint16_t    long_term_pic_num;
    uint16_t    long_term_frame_idx;
    uint8_t    temporal_id;
} StdVideoEncodeH264ReferenceInfo;

// struct ext: vulkan_video_codec_h264std_encode
typedef struct StdVideoEncodeH264SliceHeader {
    StdVideoEncodeH264SliceHeaderFlags    flags;
    uint32_t    first_mb_in_slice;
    StdVideoH264SliceType    slice_type;
    int8_t    slice_alpha_c0_offset_div2;
    int8_t    slice_beta_offset_div2;
    int8_t    slice_qp_delta;
    uint8_t    reserved1;
    StdVideoH264CabacInitIdc    cabac_init_idc;
    StdVideoH264DisableDeblockingFilterIdc    disable_deblocking_filter_idc;
    const StdVideoEncodeH264WeightTable*    pWeightTable;
} StdVideoEncodeH264SliceHeader;

struct VideoEncodeH264DpbSlotInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264DpbSlotInfoKHR;
	const void*    pNext = {};
	const StdVideoEncodeH264ReferenceInfo*    pStdReferenceInfo = {};
};

struct VideoEncodeH264NaluSliceInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264NaluSliceInfoKHR;
	const void*    pNext = {};
	int32_t    constantQp = {};
	const StdVideoEncodeH264SliceHeader*    pStdSliceHeader = {};
};

struct VideoEncodeH264PictureInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264PictureInfoKHR;
	const void*    pNext = {};
	uint32_t    naluSliceEntryCount = {};
	const vk::VideoEncodeH264NaluSliceInfoKHR* pNaluSliceEntries = {};
	const StdVideoEncodeH264PictureInfo*    pStdPictureInfo = {};
	vk::Bool32    generatePrefixNalu = {};
};

struct VideoEncodeH264ProfileInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264ProfileInfoKHR;
	const void*    pNext = {};
	StdVideoH264ProfileIdc    stdProfileIdc = {};
};

struct VideoEncodeH264RateControlInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264RateControlInfoKHR;
	const void*    pNext = {};
	vk::VideoEncodeH264RateControlFlagsKHR flags = {};
	uint32_t    gopFrameCount = {};
	uint32_t    idrPeriod = {};
	uint32_t    consecutiveBFrameCount = {};
	uint32_t    temporalLayerCount = {};
};

struct VideoEncodeH264QpKHR {
	int32_t    qpI = {};
	int32_t    qpP = {};
	int32_t    qpB = {};
};

struct VideoEncodeH264FrameSizeKHR {
	uint32_t    frameISize = {};
	uint32_t    framePSize = {};
	uint32_t    frameBSize = {};
};

struct VideoEncodeH264GopRemainingFrameInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264GopRemainingFrameInfoKHR;
	const void*    pNext = {};
	vk::Bool32    useGopRemainingFrames = {};
	uint32_t    gopRemainingI = {};
	uint32_t    gopRemainingP = {};
	uint32_t    gopRemainingB = {};
};

struct VideoEncodeH264RateControlLayerInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264RateControlLayerInfoKHR;
	const void*    pNext = {};
	vk::Bool32    useMinQp = {};
	vk::VideoEncodeH264QpKHR minQp = {};
	vk::Bool32    useMaxQp = {};
	vk::VideoEncodeH264QpKHR maxQp = {};
	vk::Bool32    useMaxFrameSize = {};
	vk::VideoEncodeH264FrameSizeKHR maxFrameSize = {};
};

struct VideoEncodeH265CapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265CapabilitiesKHR;
	void*    pNext = {};
	vk::VideoEncodeH265CapabilityFlagsKHR flags = {};
	StdVideoH265LevelIdc    maxLevelIdc = {};
	uint32_t    maxSliceSegmentCount = {};
	vk::Extent2D maxTiles = {};
	vk::VideoEncodeH265CtbSizeFlagsKHR ctbSizes = {};
	vk::VideoEncodeH265TransformBlockSizeFlagsKHR transformBlockSizes = {};
	uint32_t    maxPPictureL0ReferenceCount = {};
	uint32_t    maxBPictureL0ReferenceCount = {};
	uint32_t    maxL1ReferenceCount = {};
	uint32_t    maxSubLayerCount = {};
	vk::Bool32    expectDyadicTemporalSubLayerPattern = {};
	int32_t    minQp = {};
	int32_t    maxQp = {};
	vk::Bool32    prefersGopRemainingFrames = {};
	vk::Bool32    requiresGopRemainingFrames = {};
	vk::VideoEncodeH265StdFlagsKHR stdSyntaxFlags = {};
};

struct VideoEncodeH265SessionCreateInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265SessionCreateInfoKHR;
	const void*    pNext = {};
	vk::Bool32    useMaxLevelIdc = {};
	StdVideoH265LevelIdc    maxLevelIdc = {};
};

struct VideoEncodeH265SessionParametersAddInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265SessionParametersAddInfoKHR;
	const void*    pNext = {};
	uint32_t    stdVPSCount = {};
	const StdVideoH265VideoParameterSet*    pStdVPSs = {};
	uint32_t    stdSPSCount = {};
	const StdVideoH265SequenceParameterSet*    pStdSPSs = {};
	uint32_t    stdPPSCount = {};
	const StdVideoH265PictureParameterSet*    pStdPPSs = {};
};

struct VideoEncodeH265SessionParametersCreateInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265SessionParametersCreateInfoKHR;
	const void*    pNext = {};
	uint32_t    maxStdVPSCount = {};
	uint32_t    maxStdSPSCount = {};
	uint32_t    maxStdPPSCount = {};
	const vk::VideoEncodeH265SessionParametersAddInfoKHR* pParametersAddInfo = {};
};

struct VideoEncodeH265SessionParametersGetInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265SessionParametersGetInfoKHR;
	const void*    pNext = {};
	vk::Bool32    writeStdVPS = {};
	vk::Bool32    writeStdSPS = {};
	vk::Bool32    writeStdPPS = {};
	uint32_t    stdVPSId = {};
	uint32_t    stdSPSId = {};
	uint32_t    stdPPSId = {};
};

struct VideoEncodeH265SessionParametersFeedbackInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265SessionParametersFeedbackInfoKHR;
	void*    pNext = {};
	vk::Bool32    hasStdVPSOverrides = {};
	vk::Bool32    hasStdSPSOverrides = {};
	vk::Bool32    hasStdPPSOverrides = {};
};

// struct ext: vulkan_video_codec_h265std_encode
typedef struct StdVideoEncodeH265WeightTableFlags {
    uint16_t    luma_weight_l0_flag;
    uint16_t    chroma_weight_l0_flag;
    uint16_t    luma_weight_l1_flag;
    uint16_t    chroma_weight_l1_flag;
} StdVideoEncodeH265WeightTableFlags;

// struct ext: vulkan_video_codec_h265std_encode
typedef struct StdVideoEncodeH265WeightTable {
    StdVideoEncodeH265WeightTableFlags    flags;
    uint8_t    luma_log2_weight_denom;
    int8_t    delta_chroma_log2_weight_denom;
    int8_t    delta_luma_weight_l0[STD_VIDEO_H265_MAX_NUM_LIST_REF];
    int8_t    luma_offset_l0[STD_VIDEO_H265_MAX_NUM_LIST_REF];
    int8_t    delta_chroma_weight_l0[STD_VIDEO_H265_MAX_NUM_LIST_REF];
    int8_t    delta_chroma_offset_l0[STD_VIDEO_H265_MAX_NUM_LIST_REF];
    int8_t    delta_luma_weight_l1[STD_VIDEO_H265_MAX_NUM_LIST_REF];
    int8_t    luma_offset_l1[STD_VIDEO_H265_MAX_NUM_LIST_REF];
    int8_t    delta_chroma_weight_l1[STD_VIDEO_H265_MAX_NUM_LIST_REF];
    int8_t    delta_chroma_offset_l1[STD_VIDEO_H265_MAX_NUM_LIST_REF];
} StdVideoEncodeH265WeightTable;

// struct ext: vulkan_video_codec_h265std_encode
typedef struct StdVideoEncodeH265SliceSegmentHeaderFlags {
    uint32_t    first_slice_segment_in_pic_flag : 1;
    uint32_t    dependent_slice_segment_flag : 1;
    uint32_t    slice_sao_luma_flag : 1;
    uint32_t    slice_sao_chroma_flag : 1;
    uint32_t    num_ref_idx_active_override_flag : 1;
    uint32_t    mvd_l1_zero_flag : 1;
    uint32_t    cabac_init_flag : 1;
    uint32_t    cu_chroma_qp_offset_enabled_flag : 1;
    uint32_t    deblocking_filter_override_flag : 1;
    uint32_t    slice_deblocking_filter_disabled_flag : 1;
    uint32_t    collocated_from_l0_flag : 1;
    uint32_t    slice_loop_filter_across_slices_enabled_flag : 1;
    uint32_t    reserved : 20;
} StdVideoEncodeH265SliceSegmentHeaderFlags;

// struct ext: vulkan_video_codec_h265std_encode
typedef struct StdVideoEncodeH265SliceSegmentHeader {
    StdVideoEncodeH265SliceSegmentHeaderFlags    flags;
    StdVideoH265SliceType    slice_type;
    uint32_t    slice_segment_address;
    uint8_t    collocated_ref_idx;
    uint8_t    MaxNumMergeCand;
    int8_t    slice_cb_qp_offset;
    int8_t    slice_cr_qp_offset;
    int8_t    slice_beta_offset_div2;
    int8_t    slice_tc_offset_div2;
    int8_t    slice_act_y_qp_offset;
    int8_t    slice_act_cb_qp_offset;
    int8_t    slice_act_cr_qp_offset;
    int8_t    slice_qp_delta;
    uint16_t    reserved1;
    const StdVideoEncodeH265WeightTable*    pWeightTable;
} StdVideoEncodeH265SliceSegmentHeader;

// struct ext: vulkan_video_codec_h265std_encode
typedef struct StdVideoEncodeH265ReferenceListsInfoFlags {
    uint32_t    ref_pic_list_modification_flag_l0 : 1;
    uint32_t    ref_pic_list_modification_flag_l1 : 1;
    uint32_t    reserved : 30;
} StdVideoEncodeH265ReferenceListsInfoFlags;

// struct ext: vulkan_video_codec_h265std_encode
typedef struct StdVideoEncodeH265ReferenceListsInfo {
    StdVideoEncodeH265ReferenceListsInfoFlags    flags;
    uint8_t    num_ref_idx_l0_active_minus1;
    uint8_t    num_ref_idx_l1_active_minus1;
    uint8_t    RefPicList0;
    uint8_t    RefPicList1;
    uint8_t    list_entry_l0;
    uint8_t    list_entry_l1;
} StdVideoEncodeH265ReferenceListsInfo;

// struct ext: vulkan_video_codec_h265std_encode
typedef struct StdVideoEncodeH265PictureInfoFlags {
    uint32_t    is_reference : 1;
    uint32_t    IrapPicFlag : 1;
    uint32_t    used_for_long_term_reference : 1;
    uint32_t    discardable_flag : 1;
    uint32_t    cross_layer_bla_flag : 1;
    uint32_t    pic_output_flag : 1;
    uint32_t    no_output_of_prior_pics_flag : 1;
    uint32_t    short_term_ref_pic_set_sps_flag : 1;
    uint32_t    slice_temporal_mvp_enabled_flag : 1;
    uint32_t    reserved : 23;
} StdVideoEncodeH265PictureInfoFlags;

// struct ext: vulkan_video_codec_h265std_encode
typedef struct StdVideoEncodeH265LongTermRefPics {
    uint8_t    num_long_term_sps;
    uint8_t    num_long_term_pics;
    uint8_t    lt_idx_sps[STD_VIDEO_H265_MAX_LONG_TERM_REF_PICS_SPS];
    uint8_t    poc_lsb_lt[STD_VIDEO_H265_MAX_LONG_TERM_PICS];
    uint16_t    used_by_curr_pic_lt_flag;
    uint8_t    delta_poc_msb_present_flag[STD_VIDEO_H265_MAX_DELTA_POC];
    uint8_t    delta_poc_msb_cycle_lt[STD_VIDEO_H265_MAX_DELTA_POC];
} StdVideoEncodeH265LongTermRefPics;

// struct ext: vulkan_video_codec_h265std_encode
typedef struct StdVideoEncodeH265PictureInfo {
    StdVideoEncodeH265PictureInfoFlags    flags;
    StdVideoH265PictureType    pic_type;
    uint8_t    sps_video_parameter_set_id;
    uint8_t    pps_seq_parameter_set_id;
    uint8_t    pps_pic_parameter_set_id;
    uint8_t    short_term_ref_pic_set_idx;
    int32_t    PicOrderCntVal;
    uint8_t    TemporalId;
    uint8_t    reserved1[7];
    const StdVideoEncodeH265ReferenceListsInfo*    pRefLists;
    const StdVideoH265ShortTermRefPicSet*    pShortTermRefPicSet;
    const StdVideoEncodeH265LongTermRefPics*    pLongTermRefPics;
} StdVideoEncodeH265PictureInfo;

// struct ext: vulkan_video_codec_h265std_encode
typedef struct StdVideoEncodeH265ReferenceInfoFlags {
    uint32_t    used_for_long_term_reference : 1;
    uint32_t    unused_for_reference : 1;
    uint32_t    reserved : 30;
} StdVideoEncodeH265ReferenceInfoFlags;

// struct ext: vulkan_video_codec_h265std_encode
typedef struct StdVideoEncodeH265ReferenceInfo {
    StdVideoEncodeH265ReferenceInfoFlags    flags;
    StdVideoH265PictureType    pic_type;
    int32_t    PicOrderCntVal;
    uint8_t    TemporalId;
} StdVideoEncodeH265ReferenceInfo;

struct VideoEncodeH265NaluSliceSegmentInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265NaluSliceSegmentInfoKHR;
	const void*    pNext = {};
	int32_t    constantQp = {};
	const StdVideoEncodeH265SliceSegmentHeader*    pStdSliceSegmentHeader = {};
};

struct VideoEncodeH265PictureInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265PictureInfoKHR;
	const void*    pNext = {};
	uint32_t    naluSliceSegmentEntryCount = {};
	const vk::VideoEncodeH265NaluSliceSegmentInfoKHR* pNaluSliceSegmentEntries = {};
	const StdVideoEncodeH265PictureInfo*    pStdPictureInfo = {};
};

struct VideoEncodeH265RateControlInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265RateControlInfoKHR;
	const void*    pNext = {};
	vk::VideoEncodeH265RateControlFlagsKHR flags = {};
	uint32_t    gopFrameCount = {};
	uint32_t    idrPeriod = {};
	uint32_t    consecutiveBFrameCount = {};
	uint32_t    subLayerCount = {};
};

struct VideoEncodeH265QpKHR {
	int32_t    qpI = {};
	int32_t    qpP = {};
	int32_t    qpB = {};
};

struct VideoEncodeH265FrameSizeKHR {
	uint32_t    frameISize = {};
	uint32_t    framePSize = {};
	uint32_t    frameBSize = {};
};

struct VideoEncodeH265GopRemainingFrameInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265GopRemainingFrameInfoKHR;
	const void*    pNext = {};
	vk::Bool32    useGopRemainingFrames = {};
	uint32_t    gopRemainingI = {};
	uint32_t    gopRemainingP = {};
	uint32_t    gopRemainingB = {};
};

struct VideoEncodeH265RateControlLayerInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265RateControlLayerInfoKHR;
	const void*    pNext = {};
	vk::Bool32    useMinQp = {};
	vk::VideoEncodeH265QpKHR minQp = {};
	vk::Bool32    useMaxQp = {};
	vk::VideoEncodeH265QpKHR maxQp = {};
	vk::Bool32    useMaxFrameSize = {};
	vk::VideoEncodeH265FrameSizeKHR maxFrameSize = {};
};

struct VideoEncodeH265ProfileInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265ProfileInfoKHR;
	const void*    pNext = {};
	StdVideoH265ProfileIdc    stdProfileIdc = {};
};

struct VideoEncodeH265DpbSlotInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265DpbSlotInfoKHR;
	const void*    pNext = {};
	const StdVideoEncodeH265ReferenceInfo*    pStdReferenceInfo = {};
};

struct VideoEncodeAV1CapabilitiesKHR {
	vk::StructureType sType = StructureType::eVideoEncodeAv1CapabilitiesKHR;
	void*    pNext = {};
	vk::VideoEncodeAV1CapabilityFlagsKHR flags = {};
	StdVideoAV1Level    maxLevel = {};
	vk::Extent2D codedPictureAlignment = {};
	vk::Extent2D maxTiles = {};
	vk::Extent2D minTileSize = {};
	vk::Extent2D maxTileSize = {};
	vk::VideoEncodeAV1SuperblockSizeFlagsKHR superblockSizes = {};
	uint32_t    maxSingleReferenceCount = {};
	uint32_t    singleReferenceNameMask = {};
	uint32_t    maxUnidirectionalCompoundReferenceCount = {};
	uint32_t    maxUnidirectionalCompoundGroup1ReferenceCount = {};
	uint32_t    unidirectionalCompoundReferenceNameMask = {};
	uint32_t    maxBidirectionalCompoundReferenceCount = {};
	uint32_t    maxBidirectionalCompoundGroup1ReferenceCount = {};
	uint32_t    maxBidirectionalCompoundGroup2ReferenceCount = {};
	uint32_t    bidirectionalCompoundReferenceNameMask = {};
	uint32_t    maxTemporalLayerCount = {};
	uint32_t    maxSpatialLayerCount = {};
	uint32_t    maxOperatingPoints = {};
	uint32_t    minQIndex = {};
	uint32_t    maxQIndex = {};
	vk::Bool32    prefersGopRemainingFrames = {};
	vk::Bool32    requiresGopRemainingFrames = {};
	vk::VideoEncodeAV1StdFlagsKHR stdSyntaxFlags = {};
};

struct PhysicalDeviceVideoEncodeAV1FeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceVideoEncodeAv1FeaturesKHR;
	void*    pNext = {};
	vk::Bool32    videoEncodeAV1 = {};
};

struct VideoEncodeAV1SessionCreateInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeAv1SessionCreateInfoKHR;
	const void*    pNext = {};
	vk::Bool32    useMaxLevel = {};
	StdVideoAV1Level    maxLevel = {};
};

// struct ext: vulkan_video_codec_av1std_encode
typedef struct StdVideoEncodeAV1DecoderModelInfo {
    uint8_t    buffer_delay_length_minus_1;
    uint8_t    buffer_removal_time_length_minus_1;
    uint8_t    frame_presentation_time_length_minus_1;
    uint8_t    reserved1;
    uint32_t    num_units_in_decoding_tick;
} StdVideoEncodeAV1DecoderModelInfo;

// struct ext: vulkan_video_codec_av1std_encode
typedef struct StdVideoEncodeAV1ExtensionHeader {
    uint8_t    temporal_id;
    uint8_t    spatial_id;
} StdVideoEncodeAV1ExtensionHeader;

// struct ext: vulkan_video_codec_av1std_encode
typedef struct StdVideoEncodeAV1OperatingPointInfoFlags {
    uint32_t    decoder_model_present_for_this_op : 1;
    uint32_t    low_delay_mode_flag : 1;
    uint32_t    initial_display_delay_present_for_this_op : 1;
    uint32_t    reserved : 29;
} StdVideoEncodeAV1OperatingPointInfoFlags;

// struct ext: vulkan_video_codec_av1std_encode
typedef struct StdVideoEncodeAV1OperatingPointInfo {
    StdVideoEncodeAV1OperatingPointInfoFlags    flags;
    uint16_t    operating_point_idc;
    uint8_t    seq_level_idx;
    uint8_t    seq_tier;
    uint32_t    decoder_buffer_delay;
    uint32_t    encoder_buffer_delay;
    uint8_t    initial_display_delay_minus_1;
} StdVideoEncodeAV1OperatingPointInfo;

// struct ext: vulkan_video_codec_av1std_encode
typedef struct StdVideoEncodeAV1PictureInfoFlags {
    uint32_t    error_resilient_mode : 1;
    uint32_t    disable_cdf_update : 1;
    uint32_t    use_superres : 1;
    uint32_t    render_and_frame_size_different : 1;
    uint32_t    allow_screen_content_tools : 1;
    uint32_t    is_filter_switchable : 1;
    uint32_t    force_integer_mv : 1;
    uint32_t    frame_size_override_flag : 1;
    uint32_t    buffer_removal_time_present_flag : 1;
    uint32_t    allow_intrabc : 1;
    uint32_t    frame_refs_short_signaling : 1;
    uint32_t    allow_high_precision_mv : 1;
    uint32_t    is_motion_mode_switchable : 1;
    uint32_t    use_ref_frame_mvs : 1;
    uint32_t    disable_frame_end_update_cdf : 1;
    uint32_t    allow_warped_motion : 1;
    uint32_t    reduced_tx_set : 1;
    uint32_t    skip_mode_present : 1;
    uint32_t    delta_q_present : 1;
    uint32_t    delta_lf_present : 1;
    uint32_t    delta_lf_multi : 1;
    uint32_t    segmentation_enabled : 1;
    uint32_t    segmentation_update_map : 1;
    uint32_t    segmentation_temporal_update : 1;
    uint32_t    segmentation_update_data : 1;
    uint32_t    UsesLr : 1;
    uint32_t    usesChromaLr : 1;
    uint32_t    show_frame : 1;
    uint32_t    showable_frame : 1;
    uint32_t    reserved : 3;
} StdVideoEncodeAV1PictureInfoFlags;

// struct ext: vulkan_video_codec_av1std_encode
typedef struct StdVideoEncodeAV1PictureInfo {
    StdVideoEncodeAV1PictureInfoFlags    flags;
    StdVideoAV1FrameType    frame_type;
    uint32_t    frame_presentation_time;
    uint32_t    current_frame_id;
    uint8_t    order_hint;
    uint8_t    primary_ref_frame;
    uint8_t    refresh_frame_flags;
    uint8_t    coded_denom;
    uint16_t    render_width_minus_1;
    uint16_t    render_height_minus_1;
    StdVideoAV1InterpolationFilter    interpolation_filter;
    StdVideoAV1TxMode    TxMode;
    uint8_t    delta_q_res;
    uint8_t    delta_lf_res;
    uint8_t    ref_order_hint[STD_VIDEO_AV1_NUM_REF_FRAMES];
    int8_t    ref_frame_idx[STD_VIDEO_AV1_REFS_PER_FRAME];
    uint8_t    reserved1[3];
    uint32_t    delta_frame_id_minus_1[STD_VIDEO_AV1_REFS_PER_FRAME];
    const StdVideoAV1TileInfo*    pTileInfo;
    const StdVideoAV1Quantization*    pQuantization;
    const StdVideoAV1Segmentation*    pSegmentation;
    const StdVideoAV1LoopFilter*    pLoopFilter;
    const StdVideoAV1CDEF*    pCDEF;
    const StdVideoAV1LoopRestoration*    pLoopRestoration;
    const StdVideoAV1GlobalMotion*    pGlobalMotion;
    const StdVideoEncodeAV1ExtensionHeader*    pExtensionHeader;
    const uint32_t*    pBufferRemovalTimes;
} StdVideoEncodeAV1PictureInfo;

// struct ext: vulkan_video_codec_av1std_encode
typedef struct StdVideoEncodeAV1ReferenceInfoFlags {
    uint32_t    disable_frame_end_update_cdf : 1;
    uint32_t    segmentation_enabled : 1;
    uint32_t    reserved : 30;
} StdVideoEncodeAV1ReferenceInfoFlags;

// struct ext: vulkan_video_codec_av1std_encode
typedef struct StdVideoEncodeAV1ReferenceInfo {
    StdVideoEncodeAV1ReferenceInfoFlags    flags;
    uint32_t    RefFrameId;
    StdVideoAV1FrameType    frame_type;
    uint8_t    OrderHint;
    uint8_t    reserved1[3];
    const StdVideoEncodeAV1ExtensionHeader*    pExtensionHeader;
} StdVideoEncodeAV1ReferenceInfo;

struct VideoEncodeAV1SessionParametersCreateInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeAv1SessionParametersCreateInfoKHR;
	const void*    pNext = {};
	const StdVideoAV1SequenceHeader*    pStdSequenceHeader = {};
	const StdVideoEncodeAV1DecoderModelInfo*    pStdDecoderModelInfo = {};
	uint32_t    stdOperatingPointCount = {};
	const StdVideoEncodeAV1OperatingPointInfo*    pStdOperatingPoints = {};
};

struct VideoEncodeAV1DpbSlotInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeAv1DpbSlotInfoKHR;
	const void*    pNext = {};
	const StdVideoEncodeAV1ReferenceInfo*    pStdReferenceInfo = {};
};

struct VideoEncodeAV1PictureInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeAv1PictureInfoKHR;
	const void*    pNext = {};
	vk::VideoEncodeAV1PredictionModeKHR predictionMode = {};
	vk::VideoEncodeAV1RateControlGroupKHR rateControlGroup = {};
	uint32_t    constantQIndex = {};
	const StdVideoEncodeAV1PictureInfo*    pStdPictureInfo = {};
	int32_t    referenceNameSlotIndices[MaxVideoAv1ReferencesPerFrameKHR] = {};
	vk::Bool32    primaryReferenceCdfOnly = {};
	vk::Bool32    generateObuExtensionHeader = {};
};

struct VideoEncodeAV1ProfileInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeAv1ProfileInfoKHR;
	const void*    pNext = {};
	StdVideoAV1Profile    stdProfile = {};
};

struct VideoEncodeAV1RateControlInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeAv1RateControlInfoKHR;
	const void*    pNext = {};
	vk::VideoEncodeAV1RateControlFlagsKHR flags = {};
	uint32_t    gopFrameCount = {};
	uint32_t    keyFramePeriod = {};
	uint32_t    consecutiveBipredictiveFrameCount = {};
	uint32_t    temporalLayerCount = {};
};

struct VideoEncodeAV1QIndexKHR {
	uint32_t    intraQIndex = {};
	uint32_t    predictiveQIndex = {};
	uint32_t    bipredictiveQIndex = {};
};

struct VideoEncodeAV1FrameSizeKHR {
	uint32_t    intraFrameSize = {};
	uint32_t    predictiveFrameSize = {};
	uint32_t    bipredictiveFrameSize = {};
};

struct VideoEncodeAV1GopRemainingFrameInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeAv1GopRemainingFrameInfoKHR;
	const void*    pNext = {};
	vk::Bool32    useGopRemainingFrames = {};
	uint32_t    gopRemainingIntra = {};
	uint32_t    gopRemainingPredictive = {};
	uint32_t    gopRemainingBipredictive = {};
};

struct VideoEncodeAV1RateControlLayerInfoKHR {
	vk::StructureType sType = StructureType::eVideoEncodeAv1RateControlLayerInfoKHR;
	const void*    pNext = {};
	vk::Bool32    useMinQIndex = {};
	vk::VideoEncodeAV1QIndexKHR minQIndex = {};
	vk::Bool32    useMaxQIndex = {};
	vk::VideoEncodeAV1QIndexKHR maxQIndex = {};
	vk::Bool32    useMaxFrameSize = {};
	vk::VideoEncodeAV1FrameSizeKHR maxFrameSize = {};
};

struct PhysicalDeviceFragmentShaderBarycentricFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceFragmentShaderBarycentricFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    fragmentShaderBarycentric = {};
};

struct PhysicalDeviceFragmentShaderBarycentricPropertiesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceFragmentShaderBarycentricPropertiesKHR;
	void*    pNext = {};
	vk::Bool32    triStripVertexOrderIndependentOfProvokingVertex = {};
};

struct RenderingAttachmentInfo {
	vk::StructureType sType = StructureType::eRenderingAttachmentInfo;
	const void*    pNext = {};
	vk::ImageView imageView = {};
	vk::ImageLayout imageLayout = {};
	vk::ResolveModeFlagBits resolveMode = {};
	vk::ImageView resolveImageView = {};
	vk::ImageLayout resolveImageLayout = {};
	vk::AttachmentLoadOp loadOp = {};
	vk::AttachmentStoreOp storeOp = {};
	vk::ClearValue clearValue = {};
};
using RenderingAttachmentInfoKHR = RenderingAttachmentInfo;

struct RenderingInfo {
	vk::StructureType sType = StructureType::eRenderingInfo;
	const void*    pNext = {};
	vk::RenderingFlags flags = {};
	vk::Rect2D renderArea = {};
	uint32_t    layerCount = {};
	uint32_t    viewMask = {};
	uint32_t    colorAttachmentCount = {};
	const vk::RenderingAttachmentInfo* pColorAttachments = {};
	const vk::RenderingAttachmentInfo* pDepthAttachment = {};
	const vk::RenderingAttachmentInfo* pStencilAttachment = {};
};
using RenderingInfoKHR = RenderingInfo;

struct RenderingFragmentShadingRateAttachmentInfoKHR {
	vk::StructureType sType = StructureType::eRenderingFragmentShadingRateAttachmentInfoKHR;
	const void*    pNext = {};
	vk::ImageView imageView = {};
	vk::ImageLayout imageLayout = {};
	vk::Extent2D shadingRateAttachmentTexelSize = {};
};

struct PhysicalDevicePipelineBinaryFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDevicePipelineBinaryFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    pipelineBinaries = {};
};

struct DevicePipelineBinaryInternalCacheControlKHR {
	vk::StructureType sType = StructureType::eDevicePipelineBinaryInternalCacheControlKHR;
	const void*    pNext = {};
	vk::Bool32    disableInternalCache = {};
};

struct PhysicalDevicePipelineBinaryPropertiesKHR {
	vk::StructureType sType = StructureType::ePhysicalDevicePipelineBinaryPropertiesKHR;
	void*    pNext = {};
	vk::Bool32    pipelineBinaryInternalCache = {};
	vk::Bool32    pipelineBinaryInternalCacheControl = {};
	vk::Bool32    pipelineBinaryPrefersInternalCache = {};
	vk::Bool32    pipelineBinaryPrecompiledInternalCache = {};
	vk::Bool32    pipelineBinaryCompressedData = {};
};

struct ImageSubresource2 {
	vk::StructureType sType = StructureType::eImageSubresource2;
	void*    pNext = {};
	vk::ImageSubresource imageSubresource = {};
};
using ImageSubresource2KHR = ImageSubresource2;

struct SubresourceLayout2 {
	vk::StructureType sType = StructureType::eSubresourceLayout2;
	void*    pNext = {};
	vk::SubresourceLayout subresourceLayout = {};
};
using SubresourceLayout2KHR = SubresourceLayout2;

struct PhysicalDeviceShaderCoreBuiltinsPropertiesARM {
	vk::StructureType sType = StructureType::ePhysicalDeviceShaderCoreBuiltinsPropertiesARM;
	void*    pNext = {};
	uint64_t    shaderCoreMask = {};
	uint32_t    shaderCoreCount = {};
	uint32_t    shaderWarpsPerCore = {};
};

struct PhysicalDeviceShaderCoreBuiltinsFeaturesARM {
	vk::StructureType sType = StructureType::ePhysicalDeviceShaderCoreBuiltinsFeaturesARM;
	void*    pNext = {};
	vk::Bool32    shaderCoreBuiltins = {};
};

struct SurfacePresentModeEXT {
	vk::StructureType sType = StructureType::eSurfacePresentModeEXT;
	void*    pNext = {};
	vk::PresentModeKHR presentMode = {};
};

enum class PresentScalingFlagBitsEXT : uint32_t {
	eOneToOne = 0x1,
	eAspectRatioStretch = 0x2,
	eStretch = 0x4,
};
using PresentScalingFlagsEXT = Flags<PresentScalingFlagBitsEXT>;

enum class PresentGravityFlagBitsEXT : uint32_t {
	eMin = 0x1,
	eMax = 0x2,
	eCentered = 0x4,
};
using PresentGravityFlagsEXT = Flags<PresentGravityFlagBitsEXT>;

struct SurfacePresentScalingCapabilitiesEXT {
	vk::StructureType sType = StructureType::eSurfacePresentScalingCapabilitiesEXT;
	void*    pNext = {};
	vk::PresentScalingFlagsEXT supportedPresentScaling = {};
	vk::PresentGravityFlagsEXT supportedPresentGravityX = {};
	vk::PresentGravityFlagsEXT supportedPresentGravityY = {};
	vk::Extent2D minScaledImageExtent = {};
	vk::Extent2D maxScaledImageExtent = {};
};

struct SurfacePresentModeCompatibilityEXT {
	vk::StructureType sType = StructureType::eSurfacePresentModeCompatibilityEXT;
	void*    pNext = {};
	uint32_t    presentModeCount = {};
	vk::PresentModeKHR* pPresentModes = {};
};

struct PhysicalDeviceSwapchainMaintenance1FeaturesEXT {
	vk::StructureType sType = StructureType::ePhysicalDeviceSwapchainMaintenance1FeaturesEXT;
	void*    pNext = {};
	vk::Bool32    swapchainMaintenance1 = {};
};

struct SwapchainPresentFenceInfoEXT {
	vk::StructureType sType = StructureType::eSwapchainPresentFenceInfoEXT;
	const void*    pNext = {};
	uint32_t    swapchainCount = {};
	const vk::Fence* pFences = {};
};

struct SwapchainPresentModesCreateInfoEXT {
	vk::StructureType sType = StructureType::eSwapchainPresentModesCreateInfoEXT;
	const void*    pNext = {};
	uint32_t    presentModeCount = {};
	const vk::PresentModeKHR* pPresentModes = {};
};

struct SwapchainPresentModeInfoEXT {
	vk::StructureType sType = StructureType::eSwapchainPresentModeInfoEXT;
	const void*    pNext = {};
	uint32_t    swapchainCount = {};
	const vk::PresentModeKHR* pPresentModes = {};
};

struct SwapchainPresentScalingCreateInfoEXT {
	vk::StructureType sType = StructureType::eSwapchainPresentScalingCreateInfoEXT;
	const void*    pNext = {};
	vk::PresentScalingFlagsEXT scalingBehavior = {};
	vk::PresentGravityFlagsEXT presentGravityX = {};
	vk::PresentGravityFlagsEXT presentGravityY = {};
};

struct ReleaseSwapchainImagesInfoEXT {
	vk::StructureType sType = StructureType::eReleaseSwapchainImagesInfoEXT;
	const void*    pNext = {};
	vk::SwapchainKHR swapchain = {};
	uint32_t    imageIndexCount = {};
	const uint32_t*    pImageIndices = {};
};

using PFN_vkVoidFunction = void (VKAPI_PTR *)();
using PFN_vkGetInstanceProcAddrLUNARG = PFN_vkVoidFunction (VKAPI_PTR *)(Instance::HandleType instanceHandle, const char* pName);

struct DirectDriverLoadingInfoLUNARG {
	vk::StructureType sType = StructureType::eDirectDriverLoadingInfoLUNARG;
	void*    pNext = {};
	vk::DirectDriverLoadingFlagsLUNARG flags = {};
	PFN_vkGetInstanceProcAddrLUNARG    pfnGetInstanceProcAddr = {};
};

struct DirectDriverLoadingListLUNARG {
	vk::StructureType sType = StructureType::eDirectDriverLoadingListLUNARG;
	const void*    pNext = {};
	vk::DirectDriverLoadingModeLUNARG mode = {};
	uint32_t    driverCount = {};
	const vk::DirectDriverLoadingInfoLUNARG* pDrivers = {};
};

struct PhysicalDeviceRayTracingPositionFetchFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceRayTracingPositionFetchFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    rayTracingPositionFetch = {};
};

struct DeviceImageSubresourceInfo {
	vk::StructureType sType = StructureType::eDeviceImageSubresourceInfo;
	const void*    pNext = {};
	const vk::ImageCreateInfo* pCreateInfo = {};
	const vk::ImageSubresource2* pSubresource = {};
};
using DeviceImageSubresourceInfoKHR = DeviceImageSubresourceInfo;

struct PhysicalDeviceShaderCorePropertiesARM {
	vk::StructureType sType = StructureType::ePhysicalDeviceShaderCorePropertiesARM;
	void*    pNext = {};
	uint32_t    pixelRate = {};
	uint32_t    texelRate = {};
	uint32_t    fmaRate = {};
};

struct QueryLowLatencySupportNV {
	vk::StructureType sType = StructureType::eQueryLowLatencySupportNV;
	const void*    pNext = {};
	void*    pQueriedLowLatencyData = {};
};

struct MemoryMapInfo {
	vk::StructureType sType = StructureType::eMemoryMapInfo;
	const void*    pNext = {};
	vk::MemoryMapFlags flags = {};
	vk::DeviceMemory memory = {};
	vk::DeviceSize    offset = {};
	vk::DeviceSize    size = {};
};
using MemoryMapInfoKHR = MemoryMapInfo;

struct MemoryUnmapInfo {
	vk::StructureType sType = StructureType::eMemoryUnmapInfo;
	const void*    pNext = {};
	vk::MemoryUnmapFlags flags = {};
	vk::DeviceMemory memory = {};
};
using MemoryUnmapInfoKHR = MemoryUnmapInfo;

struct PhysicalDeviceCooperativeMatrixFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceCooperativeMatrixFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    cooperativeMatrix = {};
	vk::Bool32    cooperativeMatrixRobustBufferAccess = {};
};

struct CooperativeMatrixPropertiesKHR {
	vk::StructureType sType = StructureType::eCooperativeMatrixPropertiesKHR;
	void*    pNext = {};
	uint32_t    MSize = {};
	uint32_t    NSize = {};
	uint32_t    KSize = {};
	vk::ComponentTypeKHR AType = {};
	vk::ComponentTypeKHR BType = {};
	vk::ComponentTypeKHR CType = {};
	vk::ComponentTypeKHR ResultType = {};
	vk::Bool32    saturatingAccumulation = {};
	vk::ScopeKHR scope = {};
};

struct PhysicalDeviceCooperativeMatrixPropertiesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceCooperativeMatrixPropertiesKHR;
	void*    pNext = {};
	vk::ShaderStageFlags cooperativeMatrixSupportedStages = {};
};

struct PhysicalDeviceAntiLagFeaturesAMD {
	vk::StructureType sType = StructureType::ePhysicalDeviceAntiLagFeaturesAMD;
	void*    pNext = {};
	vk::Bool32    antiLag = {};
};

struct AntiLagPresentationInfoAMD {
	vk::StructureType sType = StructureType::eAntiLagPresentationInfoAMD;
	void*    pNext = {};
	vk::AntiLagStageAMD stage = {};
	uint64_t    frameIndex = {};
};

struct AntiLagDataAMD {
	vk::StructureType sType = StructureType::eAntiLagDataAMD;
	const void*    pNext = {};
	vk::AntiLagModeAMD mode = {};
	uint32_t    maxFPS = {};
	const vk::AntiLagPresentationInfoAMD* pPresentationInfo = {};
};

struct BindDescriptorSetsInfo {
	vk::StructureType sType = StructureType::eBindDescriptorSetsInfo;
	const void*    pNext = {};
	vk::ShaderStageFlags stageFlags = {};
	vk::PipelineLayout layout = {};
	uint32_t    firstSet = {};
	uint32_t    descriptorSetCount = {};
	const vk::DescriptorSet* pDescriptorSets = {};
	uint32_t    dynamicOffsetCount = {};
	const uint32_t*    pDynamicOffsets = {};
};
using BindDescriptorSetsInfoKHR = BindDescriptorSetsInfo;

struct PushConstantsInfo {
	vk::StructureType sType = StructureType::ePushConstantsInfo;
	const void*    pNext = {};
	vk::PipelineLayout layout = {};
	vk::ShaderStageFlags stageFlags = {};
	uint32_t    offset = {};
	uint32_t    size = {};
	const void*    pValues = {};
};
using PushConstantsInfoKHR = PushConstantsInfo;

struct PushDescriptorSetInfo {
	vk::StructureType sType = StructureType::ePushDescriptorSetInfo;
	const void*    pNext = {};
	vk::ShaderStageFlags stageFlags = {};
	vk::PipelineLayout layout = {};
	uint32_t    set = {};
	uint32_t    descriptorWriteCount = {};
	const vk::WriteDescriptorSet* pDescriptorWrites = {};
};
using PushDescriptorSetInfoKHR = PushDescriptorSetInfo;

struct PushDescriptorSetWithTemplateInfo {
	vk::StructureType sType = StructureType::ePushDescriptorSetWithTemplateInfo;
	const void*    pNext = {};
	vk::DescriptorUpdateTemplate descriptorUpdateTemplate = {};
	vk::PipelineLayout layout = {};
	uint32_t    set = {};
	const void*    pData = {};
};
using PushDescriptorSetWithTemplateInfoKHR = PushDescriptorSetWithTemplateInfo;

struct SetDescriptorBufferOffsetsInfoEXT {
	vk::StructureType sType = StructureType::eSetDescriptorBufferOffsetsInfoEXT;
	const void*    pNext = {};
	vk::ShaderStageFlags stageFlags = {};
	vk::PipelineLayout layout = {};
	uint32_t    firstSet = {};
	uint32_t    setCount = {};
	const uint32_t*    pBufferIndices = {};
	const vk::DeviceSize*    pOffsets = {};
};

struct BindDescriptorBufferEmbeddedSamplersInfoEXT {
	vk::StructureType sType = StructureType::eBindDescriptorBufferEmbeddedSamplersInfoEXT;
	const void*    pNext = {};
	vk::ShaderStageFlags stageFlags = {};
	vk::PipelineLayout layout = {};
	uint32_t    set = {};
};

struct LatencySleepModeInfoNV {
	vk::StructureType sType = StructureType::eLatencySleepModeInfoNV;
	const void*    pNext = {};
	vk::Bool32    lowLatencyMode = {};
	vk::Bool32    lowLatencyBoost = {};
	uint32_t    minimumIntervalUs = {};
};

struct LatencySleepInfoNV {
	vk::StructureType sType = StructureType::eLatencySleepInfoNV;
	const void*    pNext = {};
	vk::Semaphore signalSemaphore = {};
	uint64_t    value = {};
};

struct SetLatencyMarkerInfoNV {
	vk::StructureType sType = StructureType::eSetLatencyMarkerInfoNV;
	const void*    pNext = {};
	uint64_t    presentID = {};
	vk::LatencyMarkerNV marker = {};
};

struct LatencyTimingsFrameReportNV {
	vk::StructureType sType = StructureType::eLatencyTimingsFrameReportNV;
	const void*    pNext = {};
	uint64_t    presentID = {};
	uint64_t    inputSampleTimeUs = {};
	uint64_t    simStartTimeUs = {};
	uint64_t    simEndTimeUs = {};
	uint64_t    renderSubmitStartTimeUs = {};
	uint64_t    renderSubmitEndTimeUs = {};
	uint64_t    presentStartTimeUs = {};
	uint64_t    presentEndTimeUs = {};
	uint64_t    driverStartTimeUs = {};
	uint64_t    driverEndTimeUs = {};
	uint64_t    osRenderQueueStartTimeUs = {};
	uint64_t    osRenderQueueEndTimeUs = {};
	uint64_t    gpuRenderStartTimeUs = {};
	uint64_t    gpuRenderEndTimeUs = {};
};

struct GetLatencyMarkerInfoNV {
	vk::StructureType sType = StructureType::eGetLatencyMarkerInfoNV;
	const void*    pNext = {};
	uint32_t    timingCount = {};
	vk::LatencyTimingsFrameReportNV* pTimings = {};
};

struct OutOfBandQueueTypeInfoNV {
	vk::StructureType sType = StructureType::eOutOfBandQueueTypeInfoNV;
	const void*    pNext = {};
	vk::OutOfBandQueueTypeNV queueType = {};
};

struct LatencySubmissionPresentIdNV {
	vk::StructureType sType = StructureType::eLatencySubmissionPresentIdNV;
	const void*    pNext = {};
	uint64_t    presentID = {};
};

struct SwapchainLatencyCreateInfoNV {
	vk::StructureType sType = StructureType::eSwapchainLatencyCreateInfoNV;
	const void*    pNext = {};
	vk::Bool32    latencyModeEnable = {};
};

struct LatencySurfaceCapabilitiesNV {
	vk::StructureType sType = StructureType::eLatencySurfaceCapabilitiesNV;
	const void*    pNext = {};
	uint32_t    presentModeCount = {};
	vk::PresentModeKHR* pPresentModes = {};
};

struct PhysicalDeviceShaderMaximalReconvergenceFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceShaderMaximalReconvergenceFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    shaderMaximalReconvergence = {};
};

struct RenderingAttachmentLocationInfo {
	vk::StructureType sType = StructureType::eRenderingAttachmentLocationInfo;
	const void*    pNext = {};
	uint32_t    colorAttachmentCount = {};
	const uint32_t*    pColorAttachmentLocations = {};
};
using RenderingAttachmentLocationInfoKHR = RenderingAttachmentLocationInfo;

struct RenderingInputAttachmentIndexInfo {
	vk::StructureType sType = StructureType::eRenderingInputAttachmentIndexInfo;
	const void*    pNext = {};
	uint32_t    colorAttachmentCount = {};
	const uint32_t*    pColorAttachmentInputIndices = {};
	const uint32_t*    pDepthInputAttachmentIndex = {};
	const uint32_t*    pStencilInputAttachmentIndex = {};
};
using RenderingInputAttachmentIndexInfoKHR = RenderingInputAttachmentIndexInfo;

struct PhysicalDeviceShaderQuadControlFeaturesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceShaderQuadControlFeaturesKHR;
	void*    pNext = {};
	vk::Bool32    shaderQuadControl = {};
};

struct PhysicalDeviceProperties {
	uint32_t    apiVersion = {};
	uint32_t    driverVersion = {};
	uint32_t    vendorID = {};
	uint32_t    deviceID = {};
	vk::PhysicalDeviceType deviceType = {};
	char    deviceName[MaxPhysicalDeviceNameSize] = {};
	uint8_t    pipelineCacheUUID[UuidSize] = {};
	vk::PhysicalDeviceLimits limits = {};
	vk::PhysicalDeviceSparseProperties sparseProperties = {};
};

struct PhysicalDeviceMemoryProperties {
	uint32_t    memoryTypeCount = {};
	vk::MemoryType    memoryTypes[MaxMemoryTypes] = {};
	uint32_t    memoryHeapCount = {};
	vk::MemoryHeap    memoryHeaps[MaxMemoryHeaps] = {};
};

struct PhysicalDeviceProperties2 {
	vk::StructureType sType = StructureType::ePhysicalDeviceProperties2;
	void*    pNext = {};
	vk::PhysicalDeviceProperties properties = {};
};
using PhysicalDeviceProperties2KHR = PhysicalDeviceProperties2;

struct PhysicalDeviceMemoryProperties2 {
	vk::StructureType sType = StructureType::ePhysicalDeviceMemoryProperties2;
	void*    pNext = {};
	vk::PhysicalDeviceMemoryProperties memoryProperties = {};
};
using PhysicalDeviceMemoryProperties2KHR = PhysicalDeviceMemoryProperties2;

struct PhysicalDeviceLayeredApiVulkanPropertiesKHR {
	vk::StructureType sType = StructureType::ePhysicalDeviceLayeredApiVulkanPropertiesKHR;
	void*    pNext = {};
	vk::PhysicalDeviceProperties2 properties = {};
};

struct VideoEncodeH264QualityLevelPropertiesKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH264QualityLevelPropertiesKHR;
	void*    pNext = {};
	vk::VideoEncodeH264RateControlFlagsKHR preferredRateControlFlags = {};
	uint32_t    preferredGopFrameCount = {};
	uint32_t    preferredIdrPeriod = {};
	uint32_t    preferredConsecutiveBFrameCount = {};
	uint32_t    preferredTemporalLayerCount = {};
	vk::VideoEncodeH264QpKHR preferredConstantQp = {};
	uint32_t    preferredMaxL0ReferenceCount = {};
	uint32_t    preferredMaxL1ReferenceCount = {};
	vk::Bool32    preferredStdEntropyCodingModeFlag = {};
};

struct VideoEncodeH265QualityLevelPropertiesKHR {
	vk::StructureType sType = StructureType::eVideoEncodeH265QualityLevelPropertiesKHR;
	void*    pNext = {};
	vk::VideoEncodeH265RateControlFlagsKHR preferredRateControlFlags = {};
	uint32_t    preferredGopFrameCount = {};
	uint32_t    preferredIdrPeriod = {};
	uint32_t    preferredConsecutiveBFrameCount = {};
	uint32_t    preferredSubLayerCount = {};
	vk::VideoEncodeH265QpKHR preferredConstantQp = {};
	uint32_t    preferredMaxL0ReferenceCount = {};
	uint32_t    preferredMaxL1ReferenceCount = {};
};

struct VideoEncodeAV1QualityLevelPropertiesKHR {
	vk::StructureType sType = StructureType::eVideoEncodeAv1QualityLevelPropertiesKHR;
	void*    pNext = {};
	vk::VideoEncodeAV1RateControlFlagsKHR preferredRateControlFlags = {};
	uint32_t    preferredGopFrameCount = {};
	uint32_t    preferredKeyFramePeriod = {};
	uint32_t    preferredConsecutiveBipredictiveFrameCount = {};
	uint32_t    preferredTemporalLayerCount = {};
	vk::VideoEncodeAV1QIndexKHR preferredConstantQIndex = {};
	uint32_t    preferredMaxSingleReferenceCount = {};
	uint32_t    preferredSingleReferenceNameMask = {};
	uint32_t    preferredMaxUnidirectionalCompoundReferenceCount = {};
	uint32_t    preferredMaxUnidirectionalCompoundGroup1ReferenceCount = {};
	uint32_t    preferredUnidirectionalCompoundReferenceNameMask = {};
	uint32_t    preferredMaxBidirectionalCompoundReferenceCount = {};
	uint32_t    preferredMaxBidirectionalCompoundGroup1ReferenceCount = {};
	uint32_t    preferredMaxBidirectionalCompoundGroup2ReferenceCount = {};
	uint32_t    preferredBidirectionalCompoundReferenceNameMask = {};
};


// structs copied from vulkan_core.h
typedef struct LayerProperties {
	char        layerName[MaxExtensionNameSize];
	uint32_t    specVersion;
	uint32_t    implementationVersion;
	char        description[MaxDescriptionSize];
} LayerProperties;

typedef struct ApplicationInfo {
	StructureType      sType = StructureType::eApplicationInfo;
	const void*        pNext = nullptr;
	const char*        pApplicationName;
	uint32_t           applicationVersion;
	const char*        pEngineName;
	uint32_t           engineVersion;
	uint32_t           apiVersion;
} ApplicationInfo;

typedef struct InstanceCreateInfo {
	StructureType               sType = StructureType::eInstanceCreateInfo;
	const void*                 pNext = nullptr;
	InstanceCreateFlags         flags;
	const ApplicationInfo*      pApplicationInfo;
	uint32_t                    enabledLayerCount;
	const char* const*          ppEnabledLayerNames;
	uint32_t                    enabledExtensionCount;
	const char* const*          ppEnabledExtensionNames;
} InstanceCreateInfo;

typedef struct PhysicalDeviceVulkan11Properties {
	StructureType            sType = StructureType::ePhysicalDeviceVulkan11Properties;
	const void*              pNext = nullptr;
	uint8_t                  deviceUUID[UuidSize];
	uint8_t                  driverUUID[UuidSize];
	uint8_t                  deviceLUID[LuidSize];
	uint32_t                 deviceNodeMask;
	Bool32                   deviceLUIDValid;
	uint32_t                 subgroupSize;
	ShaderStageFlags         subgroupSupportedStages;
	SubgroupFeatureFlags     subgroupSupportedOperations;
	Bool32                   subgroupQuadOperationsInAllStages;
	PointClippingBehavior    pointClippingBehavior;
	uint32_t                 maxMultiviewViewCount;
	uint32_t                 maxMultiviewInstanceIndex;
	Bool32                   protectedNoFault;
	uint32_t                 maxPerSetDescriptors;
	DeviceSize               maxMemoryAllocationSize;
} PhysicalDeviceVulkan11Properties;

typedef struct ConformanceVersion {
	uint8_t    major;
	uint8_t    minor;
	uint8_t    subminor;
	uint8_t    patch;
} ConformanceVersion;

typedef struct PhysicalDeviceVulkan12Properties {
	StructureType                      sType = StructureType::ePhysicalDeviceVulkan12Properties;
	const void*                        pNext = nullptr;
	DriverId                           driverID;
	char                               driverName[MaxDriverNameSize];
	char                               driverInfo[MaxDriverInfoSize];
	ConformanceVersion                 conformanceVersion;
	ShaderFloatControlsIndependence    denormBehaviorIndependence;
	ShaderFloatControlsIndependence    roundingModeIndependence;
	Bool32                             shaderSignedZeroInfNanPreserveFloat16;
	Bool32                             shaderSignedZeroInfNanPreserveFloat32;
	Bool32                             shaderSignedZeroInfNanPreserveFloat64;
	Bool32                             shaderDenormPreserveFloat16;
	Bool32                             shaderDenormPreserveFloat32;
	Bool32                             shaderDenormPreserveFloat64;
	Bool32                             shaderDenormFlushToZeroFloat16;
	Bool32                             shaderDenormFlushToZeroFloat32;
	Bool32                             shaderDenormFlushToZeroFloat64;
	Bool32                             shaderRoundingModeRTEFloat16;
	Bool32                             shaderRoundingModeRTEFloat32;
	Bool32                             shaderRoundingModeRTEFloat64;
	Bool32                             shaderRoundingModeRTZFloat16;
	Bool32                             shaderRoundingModeRTZFloat32;
	Bool32                             shaderRoundingModeRTZFloat64;
	uint32_t                           maxUpdateAfterBindDescriptorsInAllPools;
	Bool32                             shaderUniformBufferArrayNonUniformIndexingNative;
	Bool32                             shaderSampledImageArrayNonUniformIndexingNative;
	Bool32                             shaderStorageBufferArrayNonUniformIndexingNative;
	Bool32                             shaderStorageImageArrayNonUniformIndexingNative;
	Bool32                             shaderInputAttachmentArrayNonUniformIndexingNative;
	Bool32                             robustBufferAccessUpdateAfterBind;
	Bool32                             quadDivergentImplicitLod;
	uint32_t                           maxPerStageDescriptorUpdateAfterBindSamplers;
	uint32_t                           maxPerStageDescriptorUpdateAfterBindUniformBuffers;
	uint32_t                           maxPerStageDescriptorUpdateAfterBindStorageBuffers;
	uint32_t                           maxPerStageDescriptorUpdateAfterBindSampledImages;
	uint32_t                           maxPerStageDescriptorUpdateAfterBindStorageImages;
	uint32_t                           maxPerStageDescriptorUpdateAfterBindInputAttachments;
	uint32_t                           maxPerStageUpdateAfterBindResources;
	uint32_t                           maxDescriptorSetUpdateAfterBindSamplers;
	uint32_t                           maxDescriptorSetUpdateAfterBindUniformBuffers;
	uint32_t                           maxDescriptorSetUpdateAfterBindUniformBuffersDynamic;
	uint32_t                           maxDescriptorSetUpdateAfterBindStorageBuffers;
	uint32_t                           maxDescriptorSetUpdateAfterBindStorageBuffersDynamic;
	uint32_t                           maxDescriptorSetUpdateAfterBindSampledImages;
	uint32_t                           maxDescriptorSetUpdateAfterBindStorageImages;
	uint32_t                           maxDescriptorSetUpdateAfterBindInputAttachments;
	ResolveModeFlags                   supportedDepthResolveModes;
	ResolveModeFlags                   supportedStencilResolveModes;
	Bool32                             independentResolveNone;
	Bool32                             independentResolve;
	Bool32                             filterMinmaxSingleComponentFormats;
	Bool32                             filterMinmaxImageComponentMapping;
	uint64_t                           maxTimelineSemaphoreValueDifference;
	SampleCountFlags                   framebufferIntegerColorSampleCounts;
} PhysicalDeviceVulkan12Properties;

typedef struct PhysicalDeviceVulkan12Features {
	StructureType    sType = StructureType::ePhysicalDeviceVulkan12Features;
	const void*      pNext = nullptr;
	Bool32           samplerMirrorClampToEdge;
	Bool32           drawIndirectCount;
	Bool32           storageBuffer8BitAccess;
	Bool32           uniformAndStorageBuffer8BitAccess;
	Bool32           storagePushConstant8;
	Bool32           shaderBufferInt64Atomics;
	Bool32           shaderSharedInt64Atomics;
	Bool32           shaderFloat16;
	Bool32           shaderInt8;
	Bool32           descriptorIndexing;
	Bool32           shaderInputAttachmentArrayDynamicIndexing;
	Bool32           shaderUniformTexelBufferArrayDynamicIndexing;
	Bool32           shaderStorageTexelBufferArrayDynamicIndexing;
	Bool32           shaderUniformBufferArrayNonUniformIndexing;
	Bool32           shaderSampledImageArrayNonUniformIndexing;
	Bool32           shaderStorageBufferArrayNonUniformIndexing;
	Bool32           shaderStorageImageArrayNonUniformIndexing;
	Bool32           shaderInputAttachmentArrayNonUniformIndexing;
	Bool32           shaderUniformTexelBufferArrayNonUniformIndexing;
	Bool32           shaderStorageTexelBufferArrayNonUniformIndexing;
	Bool32           descriptorBindingUniformBufferUpdateAfterBind;
	Bool32           descriptorBindingSampledImageUpdateAfterBind;
	Bool32           descriptorBindingStorageImageUpdateAfterBind;
	Bool32           descriptorBindingStorageBufferUpdateAfterBind;
	Bool32           descriptorBindingUniformTexelBufferUpdateAfterBind;
	Bool32           descriptorBindingStorageTexelBufferUpdateAfterBind;
	Bool32           descriptorBindingUpdateUnusedWhilePending;
	Bool32           descriptorBindingPartiallyBound;
	Bool32           descriptorBindingVariableDescriptorCount;
	Bool32           runtimeDescriptorArray;
	Bool32           samplerFilterMinmax;
	Bool32           scalarBlockLayout;
	Bool32           imagelessFramebuffer;
	Bool32           uniformBufferStandardLayout;
	Bool32           shaderSubgroupExtendedTypes;
	Bool32           separateDepthStencilLayouts;
	Bool32           hostQueryReset;
	Bool32           timelineSemaphore;
	Bool32           bufferDeviceAddress;
	Bool32           bufferDeviceAddressCaptureReplay;
	Bool32           bufferDeviceAddressMultiDevice;
	Bool32           vulkanMemoryModel;
	Bool32           vulkanMemoryModelDeviceScope;
	Bool32           vulkanMemoryModelAvailabilityVisibilityChains;
	Bool32           shaderOutputViewportIndex;
	Bool32           shaderOutputLayer;
	Bool32           subgroupBroadcastDynamicId;
	constexpr PhysicalDeviceVulkan12Features& setPNext(const void* pNext_) noexcept  { pNext = pNext_; return *this; }
} PhysicalDeviceVulkan12Features;

typedef struct PhysicalDeviceVulkan13Features {
	StructureType    sType = StructureType::ePhysicalDeviceVulkan13Features;
	const void*      pNext = nullptr;
	Bool32           robustImageAccess;
	Bool32           inlineUniformBlock;
	Bool32           descriptorBindingInlineUniformBlockUpdateAfterBind;
	Bool32           pipelineCreationCacheControl;
	Bool32           privateData;
	Bool32           shaderDemoteToHelperInvocation;
	Bool32           shaderTerminateInvocation;
	Bool32           subgroupSizeControl;
	Bool32           computeFullSubgroups;
	Bool32           synchronization2;
	Bool32           textureCompressionASTC_HDR;
	Bool32           shaderZeroInitializeWorkgroupMemory;
	Bool32           dynamicRendering;
	Bool32           shaderIntegerDotProduct;
	Bool32           maintenance4;
	constexpr PhysicalDeviceVulkan13Features& setPNext(const void* pNext_) noexcept  { pNext = pNext_; return *this; }
} PhysicalDeviceVulkan13Features;

typedef struct DeviceQueueCreateInfo {
	StructureType               sType = StructureType::eDeviceQueueCreateInfo;
	const void*                 pNext = nullptr;
	DeviceQueueCreateFlags      flags;
	uint32_t                    queueFamilyIndex;
	uint32_t                    queueCount;
	const float*                pQueuePriorities;
} DeviceQueueCreateInfo;

typedef struct DeviceCreateInfo {
	StructureType                      sType = StructureType::eDeviceCreateInfo;
	const void*                        pNext = nullptr;
	DeviceCreateFlags                  flags;
	uint32_t                           queueCreateInfoCount;
	const DeviceQueueCreateInfo*       pQueueCreateInfos;
	uint32_t                           enabledLayerCount;
	const char* const*                 ppEnabledLayerNames;
	uint32_t                           enabledExtensionCount;
	const char* const*                 ppEnabledExtensionNames;
	const PhysicalDeviceFeatures*      pEnabledFeatures;

	constexpr DeviceCreateInfo& setPNext(const void* pNext_) noexcept  { pNext = pNext_; return *this; }
} DeviceCreateInfo;

typedef struct BufferViewCreateInfo {
	StructureType            sType = StructureType::eBufferViewCreateInfo;
	const void*              pNext = nullptr;
	BufferViewCreateFlags    flags;
	Buffer                   buffer;
	Format                   format;
	DeviceSize               offset;
	DeviceSize               range;
} BufferViewCreateInfo;

typedef struct SubmitInfo {
    StructureType  sType = StructureType::eSubmitInfo;
    const void*    pNext = nullptr;
    uint32_t              waitSemaphoreCount;
    const Semaphore*      pWaitSemaphores;
    const PipelineStageFlags*  pWaitDstStageMask;
    uint32_t              commandBufferCount;
    const CommandBuffer*  pCommandBuffers;
    uint32_t              signalSemaphoreCount;
    const Semaphore*      pSignalSemaphores;
} SubmitInfo;

typedef struct MemoryAllocateInfo {
    StructureType  sType = StructureType::eMemoryAllocateInfo;
    const void*    pNext = nullptr;
    DeviceSize     allocationSize;
    uint32_t       memoryTypeIndex;
} MemoryAllocateInfo;

typedef struct MappedMemoryRange {
    StructureType  sType = StructureType::eMappedMemoryRange;
    const void*    pNext = nullptr;
    DeviceMemory   memory;
    DeviceSize     offset;
    DeviceSize     size;
} MappedMemoryRange;

typedef struct SparseMemoryBind {
    DeviceSize    resourceOffset;
    DeviceSize    size;
    DeviceMemory  memory;
    DeviceSize    memoryOffset;
    SparseMemoryBindFlags  flags;
} SparseMemoryBind;

typedef struct SparseBufferMemoryBindInfo {
    Buffer    buffer;
    uint32_t  bindCount;
    const SparseMemoryBind*  pBinds;
} SparseBufferMemoryBindInfo;

typedef struct SparseImageOpaqueMemoryBindInfo {
    Image     image;
    uint32_t  bindCount;
    const SparseMemoryBind*  pBinds;
} SparseImageOpaqueMemoryBindInfo;

typedef struct SparseImageMemoryBind {
    ImageSubresource  subresource;
    Offset3D          offset;
    Extent3D          extent;
    DeviceMemory      memory;
    DeviceSize        memoryOffset;
    SparseMemoryBindFlags  flags;
} SparseImageMemoryBind;

typedef struct SparseImageMemoryBindInfo {
    Image     image;
    uint32_t  bindCount;
    const SparseImageMemoryBind*  pBinds;
} SparseImageMemoryBindInfo;

typedef struct BindSparseInfo {
    StructureType  sType = StructureType::eBindSparseInfo;
    const void*    pNext = nullptr;
    uint32_t       waitSemaphoreCount;
    const Semaphore*  pWaitSemaphores;
    uint32_t       bufferBindCount;
    const SparseBufferMemoryBindInfo*  pBufferBinds;
    uint32_t       imageOpaqueBindCount;
    const SparseImageOpaqueMemoryBindInfo*  pImageOpaqueBinds;
    uint32_t       imageBindCount;
    const SparseImageMemoryBindInfo*  pImageBinds;
    uint32_t       signalSemaphoreCount;
    const Semaphore*  pSignalSemaphores;
} BindSparseInfo;

typedef struct FenceCreateInfo {
    StructureType     sType = StructureType::eFenceCreateInfo;
    const void*       pNext = nullptr;
    FenceCreateFlags  flags;
} FenceCreateInfo;

typedef struct SemaphoreCreateInfo {
    StructureType  sType = StructureType::eSemaphoreCreateInfo;
    const void*    pNext = nullptr;
    SemaphoreCreateFlags  flags;
} SemaphoreCreateInfo;

typedef struct EventCreateInfo {
    StructureType  sType = StructureType::eEventCreateInfo;
    const void*    pNext = nullptr;
    EventCreateFlags  flags;
} EventCreateInfo;

typedef struct QueryPoolCreateInfo {
    StructureType  sType = StructureType::eQueryPoolCreateInfo;
    const void*    pNext = nullptr;
    QueryPoolCreateFlags  flags;
    QueryType      queryType;
    uint32_t       queryCount;
    QueryPipelineStatisticFlags  pipelineStatistics;
} QueryPoolCreateInfo;

typedef struct ImageViewCreateInfo {
    StructureType  sType = StructureType::eImageViewCreateInfo;
    const void*    pNext = nullptr;
    ImageViewCreateFlags  flags;
    Image          image;
    ImageViewType  viewType;
    Format         format;
    ComponentMapping  components;
    ImageSubresourceRange  subresourceRange;
} ImageViewCreateInfo;

typedef struct ShaderModuleCreateInfo {
    StructureType  sType = StructureType::eShaderModuleCreateInfo;
    const void*    pNext = nullptr;
    ShaderModuleCreateFlags  flags;
    size_t         codeSize;
    const uint32_t*  pCode;
} ShaderModuleCreateInfo;

typedef struct PipelineCacheCreateInfo {
    StructureType  sType = StructureType::ePipelineCacheCreateInfo;
    const void*    pNext = nullptr;
    PipelineCacheCreateFlags  flags;
    size_t         initialDataSize;
    const void*    pInitialData;
} PipelineCacheCreateInfo;

typedef struct VertexInputAttributeDescription {
    uint32_t    location;
    uint32_t    binding;
    Format    format;
    uint32_t    offset;
} VertexInputAttributeDescription;

typedef struct VertexInputBindingDescription {
    uint32_t    binding;
    uint32_t    stride;
    VertexInputRate    inputRate;
} VertexInputBindingDescription;

typedef struct PipelineVertexInputStateCreateInfo {
    StructureType    sType = StructureType::ePipelineVertexInputStateCreateInfo;
    const void*    pNext = nullptr;
    PipelineVertexInputStateCreateFlags    flags;
    uint32_t    vertexBindingDescriptionCount;
    const VertexInputBindingDescription*    pVertexBindingDescriptions;
    uint32_t    vertexAttributeDescriptionCount;
    const VertexInputAttributeDescription*    pVertexAttributeDescriptions;
} PipelineVertexInputStateCreateInfo;

typedef struct PipelineInputAssemblyStateCreateInfo {
    StructureType    sType = StructureType::ePipelineInputAssemblyStateCreateInfo;
    const void*    pNext = nullptr;
    PipelineInputAssemblyStateCreateFlags    flags;
    PrimitiveTopology    topology;
    Bool32    primitiveRestartEnable;
} PipelineInputAssemblyStateCreateInfo;

typedef struct PipelineTessellationStateCreateInfo {
    StructureType    sType = StructureType::ePipelineTessellationStateCreateInfo;
    const void*    pNext = nullptr;
    PipelineTessellationStateCreateFlags    flags;
    uint32_t    patchControlPoints;
} PipelineTessellationStateCreateInfo;

typedef struct PipelineRasterizationStateCreateInfo {
    StructureType    sType = StructureType::ePipelineRasterizationStateCreateInfo;
    const void*    pNext = nullptr;
    PipelineRasterizationStateCreateFlags    flags;
    Bool32    depthClampEnable;
    Bool32    rasterizerDiscardEnable;
    PolygonMode    polygonMode;
    CullModeFlags    cullMode;
    FrontFace    frontFace;
    Bool32    depthBiasEnable;
    float    depthBiasConstantFactor;
    float    depthBiasClamp;
    float    depthBiasSlopeFactor;
    float    lineWidth;
} PipelineRasterizationStateCreateInfo;

typedef struct Viewport {
    float    x;
    float    y;
    float    width;
    float    height;
    float    minDepth;
    float    maxDepth;
} Viewport;

typedef struct PipelineViewportStateCreateInfo {
    StructureType    sType = StructureType::ePipelineViewportStateCreateInfo;
    const void*    pNext = nullptr;
    PipelineViewportStateCreateFlags    flags;
    uint32_t    viewportCount;
    const Viewport*    pViewports;
    uint32_t    scissorCount;
    const Rect2D*    pScissors;
} PipelineViewportStateCreateInfo;

using SampleMask = uint32_t;

typedef struct PipelineMultisampleStateCreateInfo {
    StructureType    sType = StructureType::ePipelineMultisampleStateCreateInfo;
    const void*    pNext = nullptr;
    PipelineMultisampleStateCreateFlags    flags;
    SampleCountFlagBits    rasterizationSamples;
    Bool32    sampleShadingEnable;
    float    minSampleShading;
    const SampleMask*    pSampleMask;
    Bool32    alphaToCoverageEnable;
    Bool32    alphaToOneEnable;
} PipelineMultisampleStateCreateInfo;

typedef struct StencilOpState {
    StencilOp    failOp;
    StencilOp    passOp;
    StencilOp    depthFailOp;
    CompareOp    compareOp;
    uint32_t    compareMask;
    uint32_t    writeMask;
    uint32_t    reference;
} StencilOpState;

typedef struct PipelineDepthStencilStateCreateInfo {
    StructureType    sType = StructureType::ePipelineDepthStencilStateCreateInfo;
    const void*    pNext = nullptr;
    PipelineDepthStencilStateCreateFlags    flags;
    Bool32    depthTestEnable;
    Bool32    depthWriteEnable;
    CompareOp    depthCompareOp;
    Bool32    depthBoundsTestEnable;
    Bool32    stencilTestEnable;
    StencilOpState    front;
    StencilOpState    back;
    float    minDepthBounds;
    float    maxDepthBounds;
} PipelineDepthStencilStateCreateInfo;

typedef struct VkPipelineColorBlendAttachmentState {
    Bool32    blendEnable;
    BlendFactor    srcColorBlendFactor;
    BlendFactor    dstColorBlendFactor;
    BlendOp    colorBlendOp;
    BlendFactor    srcAlphaBlendFactor;
    BlendFactor    dstAlphaBlendFactor;
    BlendOp    alphaBlendOp;
    ColorComponentFlags    colorWriteMask;
} PipelineColorBlendAttachmentState;

typedef struct VkPipelineColorBlendStateCreateInfo {
    StructureType    sType = StructureType::ePipelineColorBlendStateCreateInfo;
    const void*    pNext = nullptr;
    PipelineColorBlendStateCreateFlags    flags;
    Bool32    logicOpEnable;
    LogicOp    logicOp;
    uint32_t    attachmentCount;
    const PipelineColorBlendAttachmentState*    pAttachments;
    float    blendConstants[4];
} PipelineColorBlendStateCreateInfo;

typedef struct GraphicsPipelineCreateInfo {
    StructureType    sType = StructureType::eGraphicsPipelineCreateInfo;
    const void*    pNext = nullptr;
    PipelineCreateFlags    flags;
    uint32_t    stageCount;
    const PipelineShaderStageCreateInfo*    pStages;
    const PipelineVertexInputStateCreateInfo*    pVertexInputState;
    const PipelineInputAssemblyStateCreateInfo*    pInputAssemblyState;
    const PipelineTessellationStateCreateInfo*    pTessellationState;
    const PipelineViewportStateCreateInfo*    pViewportState;
    const PipelineRasterizationStateCreateInfo*    pRasterizationState;
    const PipelineMultisampleStateCreateInfo*    pMultisampleState;
    const PipelineDepthStencilStateCreateInfo*    pDepthStencilState;
    const PipelineColorBlendStateCreateInfo*    pColorBlendState;
    const PipelineDynamicStateCreateInfo*    pDynamicState;
    PipelineLayout    layout;
    RenderPass    renderPass;
    uint32_t    subpass;
    Pipeline    basePipelineHandle;
    int32_t    basePipelineIndex;
} GraphicsPipelineCreateInfo;

typedef struct ComputePipelineCreateInfo {
    StructureType    sType = StructureType::eComputePipelineCreateInfo;
    const void*    pNext = nullptr;
    PipelineCreateFlags    flags;
    PipelineShaderStageCreateInfo    stage;
    PipelineLayout    layout;
    Pipeline    basePipelineHandle;
    int32_t    basePipelineIndex;
} ComputePipelineCreateInfo;

typedef struct PipelineLayoutCreateInfo {
    StructureType    sType = StructureType::ePipelineLayoutCreateInfo;
    const void*    pNext = nullptr;
    PipelineLayoutCreateFlags    flags;
    uint32_t    setLayoutCount;
    const DescriptorSetLayout*    pSetLayouts;
    uint32_t    pushConstantRangeCount;
    const PushConstantRange*    pPushConstantRanges;
} PipelineLayoutCreateInfo;

typedef struct SamplerCreateInfo {
    StructureType         sType = StructureType::eSamplerCreateInfo;
    const void*             pNext = nullptr;
    SamplerCreateFlags    flags;
    Filter                magFilter;
    Filter                minFilter;
    SamplerMipmapMode     mipmapMode;
    SamplerAddressMode    addressModeU;
    SamplerAddressMode    addressModeV;
    SamplerAddressMode    addressModeW;
    float                   mipLodBias;
    Bool32                anisotropyEnable;
    float                   maxAnisotropy;
    Bool32                compareEnable;
    CompareOp             compareOp;
    float                   minLod;
    float                   maxLod;
    BorderColor           borderColor;
    Bool32                unnormalizedCoordinates;
} SamplerCreateInfo;

typedef struct DescriptorPoolSize {
    DescriptorType    type;
    uint32_t            descriptorCount;
} DescriptorPoolSize;

typedef struct DescriptorPoolCreateInfo {
    StructureType                sType = StructureType::eDescriptorPoolCreateInfo;
    const void*                    pNext = nullptr;
    DescriptorPoolCreateFlags    flags;
    uint32_t                       maxSets;
    uint32_t                       poolSizeCount;
    const DescriptorPoolSize*    pPoolSizes;
} DescriptorPoolCreateInfo;

typedef struct DescriptorSetAllocateInfo {
    StructureType                 sType = StructureType::eDescriptorSetAllocateInfo;
    const void*                     pNext = nullptr;
    DescriptorPool                descriptorPool;
    uint32_t                        descriptorSetCount;
    const DescriptorSetLayout*    pSetLayouts;
} DescriptorSetAllocateInfo;

typedef struct CopyDescriptorSet {
    StructureType    sType = StructureType::eCopyDescriptorSet;
    const void*        pNext = nullptr;
    DescriptorSet    srcSet;
    uint32_t           srcBinding;
    uint32_t           srcArrayElement;
    DescriptorSet    dstSet;
    uint32_t           dstBinding;
    uint32_t           dstArrayElement;
    uint32_t           descriptorCount;
} CopyDescriptorSet;

typedef struct FramebufferCreateInfo {
    StructureType             sType = StructureType::eFramebufferCreateInfo;
    const void*                 pNext = nullptr;
    FramebufferCreateFlags    flags;
    RenderPass                renderPass;
    uint32_t                    attachmentCount;
    const ImageView*          pAttachments;
    uint32_t                    width;
    uint32_t                    height;
    uint32_t                    layers;
} FramebufferCreateInfo;

typedef struct AttachmentReference {
    uint32_t         attachment;
    ImageLayout    layout;
} AttachmentReference;

typedef struct SubpassDescription {
    SubpassDescriptionFlags       flags;
    PipelineBindPoint             pipelineBindPoint;
    uint32_t                        inputAttachmentCount;
    const AttachmentReference*    pInputAttachments;
    uint32_t                        colorAttachmentCount;
    const AttachmentReference*    pColorAttachments;
    const AttachmentReference*    pResolveAttachments;
    const AttachmentReference*    pDepthStencilAttachment;
    uint32_t                        preserveAttachmentCount;
    const uint32_t*                 pPreserveAttachments;
} SubpassDescription;

typedef struct SubpassDependency {
    uint32_t                srcSubpass;
    uint32_t                dstSubpass;
    PipelineStageFlags    srcStageMask;
    PipelineStageFlags    dstStageMask;
    AccessFlags           srcAccessMask;
    AccessFlags           dstAccessMask;
    DependencyFlags       dependencyFlags;
} SubpassDependency;

typedef struct AttachmentDescription {
    AttachmentDescriptionFlags    flags;
    Format                        format;
    SampleCountFlagBits           samples;
    AttachmentLoadOp              loadOp;
    AttachmentStoreOp             storeOp;
    AttachmentLoadOp              stencilLoadOp;
    AttachmentStoreOp             stencilStoreOp;
    ImageLayout                   initialLayout;
    ImageLayout                   finalLayout;
} AttachmentDescription;

typedef struct RenderPassCreateInfo {
    StructureType                   sType = StructureType::eRenderPassCreateInfo;
    const void*                       pNext = nullptr;
    RenderPassCreateFlags           flags;
    uint32_t                          attachmentCount;
    const AttachmentDescription*    pAttachments;
    uint32_t                          subpassCount;
    const SubpassDescription*       pSubpasses;
    uint32_t                          dependencyCount;
    const SubpassDependency*        pDependencies;
} RenderPassCreateInfo;

typedef struct CommandPoolCreateInfo {
    StructureType             sType = StructureType::eCommandPoolCreateInfo;
    const void*                 pNext = nullptr;
    CommandPoolCreateFlags    flags;
    uint32_t                    queueFamilyIndex;
} CommandPoolCreateInfo;

typedef struct CommandBufferAllocateInfo {
    StructureType         sType = StructureType::eCommandBufferAllocateInfo;
    const void*             pNext = nullptr;
    CommandPool           commandPool;
    CommandBufferLevel    level;
    uint32_t                commandBufferCount;
} CommandBufferAllocateInfo;

typedef struct CommandBufferInheritanceInfo {
    StructureType                  sType = StructureType::eCommandBufferInheritanceInfo;
    const void*                      pNext = nullptr;
    RenderPass                     renderPass;
    uint32_t                         subpass;
    Framebuffer                    framebuffer;
    Bool32                         occlusionQueryEnable;
    QueryControlFlags              queryFlags;
    QueryPipelineStatisticFlags    pipelineStatistics;
} CommandBufferInheritanceInfo;

typedef struct CommandBufferBeginInfo {
    StructureType                          sType = StructureType::eCommandBufferBeginInfo;
    const void*                              pNext = nullptr;
    CommandBufferUsageFlags                flags;
    const CommandBufferInheritanceInfo*    pInheritanceInfo;
} CommandBufferBeginInfo;

typedef struct BufferCopy {
    DeviceSize    srcOffset;
    DeviceSize    dstOffset;
    DeviceSize    size;
} BufferCopy;

typedef struct BufferImageCopy {
    DeviceSize                bufferOffset;
    uint32_t                    bufferRowLength;
    uint32_t                    bufferImageHeight;
    ImageSubresourceLayers    imageSubresource;
    Offset3D                  imageOffset;
    Extent3D                  imageExtent;
} BufferImageCopy;

typedef struct ClearAttachment {
    ImageAspectFlags    aspectMask;
    uint32_t              colorAttachment;
    ClearValue          clearValue;
} ClearAttachment;

typedef struct ClearRect {
    Rect2D    rect;
    uint32_t    baseArrayLayer;
    uint32_t    layerCount;
} ClearRect;

typedef struct ImageBlit {
    ImageSubresourceLayers    srcSubresource;
    Offset3D                  srcOffsets[2];
    ImageSubresourceLayers    dstSubresource;
    Offset3D                  dstOffsets[2];
} ImageBlit;

typedef struct ImageCopy {
    ImageSubresourceLayers    srcSubresource;
    Offset3D                  srcOffset;
    ImageSubresourceLayers    dstSubresource;
    Offset3D                  dstOffset;
    Extent3D                  extent;
} ImageCopy;

typedef struct ImageResolve {
    ImageSubresourceLayers    srcSubresource;
    Offset3D                  srcOffset;
    ImageSubresourceLayers    dstSubresource;
    Offset3D                  dstOffset;
    Extent3D                  extent;
} ImageResolve;

typedef struct MemoryBarrier {
    StructureType    sType = StructureType::eMemoryBarrier;
    const void*        pNext = nullptr;
    AccessFlags      srcAccessMask;
    AccessFlags      dstAccessMask;
} MemoryBarrier;

typedef struct ImageMemoryBarrier {
    StructureType            sType = StructureType::eImageMemoryBarrier;
    const void*                pNext = nullptr;
    AccessFlags              srcAccessMask;
    AccessFlags              dstAccessMask;
    ImageLayout              oldLayout;
    ImageLayout              newLayout;
    uint32_t                   srcQueueFamilyIndex;
    uint32_t                   dstQueueFamilyIndex;
    Image                    image;
    ImageSubresourceRange    subresourceRange;
} ImageMemoryBarrier;

typedef struct BufferMemoryBarrier {
    StructureType    sType = StructureType::eBufferMemoryBarrier;
    const void*        pNext = nullptr;
    AccessFlags      srcAccessMask;
    AccessFlags      dstAccessMask;
    uint32_t           srcQueueFamilyIndex;
    uint32_t           dstQueueFamilyIndex;
    Buffer           buffer;
    DeviceSize       offset;
    DeviceSize       size;
} BufferMemoryBarrier;





// function pointer types
// taken from Vulkan headers
using PFN_vkVoidFunction = void (VKAPI_PTR *)(void);
using PFN_vkGetInstanceProcAddr = PFN_vkVoidFunction (VKAPI_PTR *)(Instance::HandleType instanceHandle, const char* pName);
using PFN_vkEnumerateInstanceVersion = Result (VKAPI_PTR *)(uint32_t* pApiVersion);
using PFN_vkEnumerateInstanceExtensionProperties = Result (VKAPI_PTR *)(const char* pLayerName, uint32_t* pPropertyCount, ExtensionProperties* pProperties);
using PFN_vkEnumerateInstanceLayerProperties = Result (VKAPI_PTR *)(uint32_t* pPropertyCount, LayerProperties* pProperties);
using PFN_vkCreateInstance = Result (VKAPI_PTR *)(const InstanceCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, Instance::HandleType* pInstanceHandle);
using PFN_vkDestroyInstance = void (VKAPI_PTR *)(Instance::HandleType instanceHandle, const AllocationCallbacks* pAllocator);
using PFN_vkEnumeratePhysicalDevices = Result (VKAPI_PTR *)(Instance::HandleType instanceHandle, uint32_t* pPhysicalDeviceCount, PhysicalDevice::HandleType* pPhysicalDevices);
using PFN_vkEnumerateDeviceExtensionProperties = Result (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, const char* pLayerName, uint32_t* pPropertyCount, ExtensionProperties* pProperties);
using PFN_vkGetPhysicalDeviceProperties = void (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, PhysicalDeviceProperties* pProperties);
using PFN_vkGetPhysicalDeviceProperties2 = void (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, PhysicalDeviceProperties2* pProperties);
using PFN_vkGetPhysicalDeviceFeatures = void (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, PhysicalDeviceFeatures* pFeatures);
using PFN_vkGetPhysicalDeviceFeatures2 = void (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, PhysicalDeviceFeatures2* pFeatures);
using PFN_vkGetPhysicalDeviceFormatProperties = void (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, Format format, FormatProperties* pFormatProperties);
using PFN_vkGetPhysicalDeviceImageFormatProperties = Result (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, Format format, ImageType type, ImageTiling tiling, ImageUsageFlags usage, ImageCreateFlags flags, ImageFormatProperties* pImageFormatProperties);
using PFN_vkGetPhysicalDeviceMemoryProperties = void (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, PhysicalDeviceMemoryProperties* pMemoryProperties);
using PFN_vkGetPhysicalDeviceMemoryProperties2 = void (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, PhysicalDeviceMemoryProperties2* pMemoryProperties);
using PFN_vkGetPhysicalDeviceQueueFamilyProperties = void (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, uint32_t* pQueueFamilyPropertyCount, QueueFamilyProperties* pQueueFamilyProperties);
using PFN_vkGetPhysicalDeviceQueueFamilyProperties2 = void (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, uint32_t* pQueueFamilyPropertyCount, QueueFamilyProperties2* pQueueFamilyProperties);
using PFN_vkCreateDevice = Result (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, const DeviceCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, Device::HandleType* pDeviceHandle);
using PFN_vkDestroyDevice = void (VKAPI_PTR *)(Device::HandleType deviceHandle, const AllocationCallbacks* pAllocator);
using PFN_vkGetDeviceProcAddr = PFN_vkVoidFunction (VKAPI_PTR *)(Device::HandleType deviceHandle, const char* pName);
using PFN_vkGetDeviceQueue = void (VKAPI_PTR *)(Device::HandleType deviceHandle, uint32_t queueFamilyIndex, uint32_t queueIndex, Queue::HandleType* pQueueHandle);
using PFN_vkCreateBuffer = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const BufferCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, Buffer::HandleType* pBufferHandle);
using PFN_vkDestroyBuffer = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Buffer::HandleType bufferHandle, const AllocationCallbacks* pAllocator);
using PFN_vkGetBufferDeviceAddress = vk::DeviceAddress (VKAPI_PTR *)(Device::HandleType deviceHandle, const vk::BufferDeviceAddressInfo* pInfo);
using PFN_vkCreateBufferView = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const BufferViewCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, BufferView::HandleType* pViewHandle);
using PFN_vkDestroyBufferView = void (VKAPI_PTR *)(Device::HandleType deviceHandle, BufferView::HandleType bufferViewHandle, const AllocationCallbacks* pAllocator);
using PFN_vkEnumerateDeviceLayerProperties = Result (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, uint32_t* pPropertyCount, LayerProperties* pProperties);
using PFN_vkQueueSubmit = Result (VKAPI_PTR *)(Queue::HandleType queueHandle, uint32_t submitCount, const SubmitInfo* pSubmits, Fence::HandleType fenceHandle);
using PFN_vkQueueWaitIdle = Result (VKAPI_PTR *)(Queue::HandleType queueHandle);
using PFN_vkDeviceWaitIdle = Result (VKAPI_PTR *)(Device::HandleType deviceHandle);
using PFN_vkAllocateMemory = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const MemoryAllocateInfo* pAllocateInfo, const AllocationCallbacks* pAllocator, DeviceMemory::HandleType* pMemoryHandle);
using PFN_vkFreeMemory = void (VKAPI_PTR *)(Device::HandleType deviceHandle, DeviceMemory::HandleType memoryHandle, const AllocationCallbacks* pAllocator);
using PFN_vkMapMemory = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, DeviceMemory::HandleType memoryHandle, DeviceSize offset, DeviceSize size, MemoryMapFlags flags, void** ppData);
using PFN_vkUnmapMemory = void (VKAPI_PTR *)(Device::HandleType deviceHandle, DeviceMemory::HandleType memoryHandle);
using PFN_vkFlushMappedMemoryRanges = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, uint32_t memoryRangeCount, const MappedMemoryRange* pMemoryRanges);
using PFN_vkInvalidateMappedMemoryRanges = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, uint32_t memoryRangeCount, const MappedMemoryRange* pMemoryRanges);
using PFN_vkGetDeviceMemoryCommitment = void (VKAPI_PTR *)(Device::HandleType deviceHandle, DeviceMemory::HandleType memoryHandle, DeviceSize* pCommittedMemoryInBytes);
using PFN_vkBindBufferMemory = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, Buffer::HandleType bufferHandle, DeviceMemory::HandleType memoryHandle, DeviceSize memoryOffset);
using PFN_vkBindImageMemory = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, Image::HandleType imageHandle, DeviceMemory::HandleType memoryHandle, DeviceSize memoryOffset);
using PFN_vkGetBufferMemoryRequirements = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Buffer::HandleType bufferHandle, MemoryRequirements* pMemoryRequirements);
using PFN_vkGetImageMemoryRequirements = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Image::HandleType imageHandle, MemoryRequirements* pMemoryRequirements);
using PFN_vkGetImageSparseMemoryRequirements = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Image::HandleType imageHandle, uint32_t* pSparseMemoryRequirementCount, SparseImageMemoryRequirements* pSparseMemoryRequirements);
using PFN_vkGetPhysicalDeviceSparseImageFormatProperties = void (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, Format format, ImageType type, SampleCountFlagBits samples, ImageUsageFlags usage, ImageTiling tiling, uint32_t* pPropertyCount, SparseImageFormatProperties* pProperties);
using PFN_vkQueueBindSparse = Result (VKAPI_PTR *)(Queue::HandleType queueHandle, uint32_t bindInfoCount, const BindSparseInfo* pBindInfo, Fence::HandleType fenceHandle);
using PFN_vkCreateFence = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const FenceCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, Fence::HandleType* pFenceHandle);
using PFN_vkDestroyFence = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Fence::HandleType fenceHandle, const AllocationCallbacks* pAllocator);
using PFN_vkResetFences = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, uint32_t fenceCount, const Fence::HandleType* pFenceHandles);
using PFN_vkGetFenceStatus = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, Fence::HandleType fenceHandle);
using PFN_vkWaitForFences = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, uint32_t fenceCount, const Fence::HandleType* pFenceHandles, Bool32 waitAll, uint64_t timeout);
using PFN_vkCreateSemaphore = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const SemaphoreCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, Semaphore::HandleType* pSemaphoreHandle);
using PFN_vkDestroySemaphore = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Semaphore::HandleType semaphoreHandle, const AllocationCallbacks* pAllocator);
using PFN_vkCreateEvent = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const EventCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, Event::HandleType* pEventHandle);
using PFN_vkDestroyEvent = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Event::HandleType eventHandle, const AllocationCallbacks* pAllocator);
using PFN_vkGetEventStatus = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, Event::HandleType eventHandle);
using PFN_vkSetEvent = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, Event::HandleType eventHandle);
using PFN_vkResetEvent = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, Event::HandleType eventHandle);
using PFN_vkCreateQueryPool = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const QueryPoolCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, QueryPool::HandleType* pQueryPoolHandle);
using PFN_vkDestroyQueryPool = void (VKAPI_PTR *)(Device::HandleType deviceHandle, QueryPool::HandleType queryPoolHandle, const AllocationCallbacks* pAllocator);
using PFN_vkGetQueryPoolResults = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, QueryPool::HandleType queryPoolHandle, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, DeviceSize stride, QueryResultFlags flags);
using PFN_vkCreateBuffer = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const BufferCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, Buffer::HandleType* pBufferHandle);
using PFN_vkDestroyBuffer = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Buffer::HandleType bufferHandle, const AllocationCallbacks* pAllocator);
using PFN_vkCreateBufferView = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const BufferViewCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, BufferView::HandleType* pViewHandle);
using PFN_vkDestroyBufferView = void (VKAPI_PTR *)(Device::HandleType deviceHandle, BufferView::HandleType bufferViewHandle, const AllocationCallbacks* pAllocator);
using PFN_vkCreateImage = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const ImageCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, Image::HandleType* pImageHandle);
using PFN_vkDestroyImage = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Image::HandleType imageHandle, const AllocationCallbacks* pAllocator);
using PFN_vkGetImageSubresourceLayout = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Image::HandleType imageHandle, const ImageSubresource* pSubresource, SubresourceLayout* pLayout);
using PFN_vkCreateImageView = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const ImageViewCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, ImageView::HandleType* pViewHandle);
using PFN_vkDestroyImageView = void (VKAPI_PTR *)(Device::HandleType deviceHandle, ImageView::HandleType imageViewHandle, const AllocationCallbacks* pAllocator);
using PFN_vkCreateShaderModule = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const ShaderModuleCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, ShaderModule::HandleType* pShaderModuleHandle);
using PFN_vkDestroyShaderModule = void (VKAPI_PTR *)(Device::HandleType deviceHandle, ShaderModule::HandleType shaderModuleHandle, const AllocationCallbacks* pAllocator);
using PFN_vkCreatePipelineCache = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const PipelineCacheCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, PipelineCache::HandleType* pPipelineCacheHandle);
using PFN_vkDestroyPipelineCache = void (VKAPI_PTR *)(Device::HandleType deviceHandle, PipelineCache::HandleType pipelineCacheHandle, const AllocationCallbacks* pAllocator);
using PFN_vkGetPipelineCacheData = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, PipelineCache::HandleType pipelineCacheHandle, size_t* pDataSize, void* pData);
using PFN_vkMergePipelineCaches = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, PipelineCache::HandleType dstCacheHandle, uint32_t srcCacheCount, const PipelineCache::HandleType* pSrcCacheHandles);
using PFN_vkCreateGraphicsPipelines = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, PipelineCache::HandleType pipelineCacheHandle, uint32_t createInfoCount, const GraphicsPipelineCreateInfo* pCreateInfos, const AllocationCallbacks* pAllocator, Pipeline::HandleType* pPipelineHandles);
using PFN_vkCreateComputePipelines = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, PipelineCache::HandleType pipelineCacheHandle, uint32_t createInfoCount, const ComputePipelineCreateInfo* pCreateInfos, const AllocationCallbacks* pAllocator, Pipeline::HandleType* pPipelineHandles);
using PFN_vkDestroyPipeline = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Pipeline::HandleType pipelineHandle, const AllocationCallbacks* pAllocator);
using PFN_vkCreatePipelineLayout = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const PipelineLayoutCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, PipelineLayout::HandleType* pPipelineLayoutHandle);
using PFN_vkDestroyPipelineLayout = void (VKAPI_PTR *)(Device::HandleType deviceHandle, PipelineLayout::HandleType pipelineLayoutHandle, const AllocationCallbacks* pAllocator);
using PFN_vkCreateSampler = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const SamplerCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, Sampler::HandleType* pSamplerHandle);
using PFN_vkDestroySampler = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Sampler::HandleType samplerHandle, const AllocationCallbacks* pAllocator);
using PFN_vkCreateDescriptorSetLayout = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const DescriptorSetLayoutCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, DescriptorSetLayout::HandleType* pSetLayoutHandle);
using PFN_vkDestroyDescriptorSetLayout = void (VKAPI_PTR *)(Device::HandleType deviceHandle, DescriptorSetLayout::HandleType descriptorSetLayoutHandle, const AllocationCallbacks* pAllocator);
using PFN_vkCreateDescriptorPool = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const DescriptorPoolCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, DescriptorPool::HandleType* pDescriptorPoolHandle);
using PFN_vkDestroyDescriptorPool = void (VKAPI_PTR *)(Device::HandleType deviceHandle, DescriptorPool::HandleType descriptorPoolHandle, const AllocationCallbacks* pAllocator);
using PFN_vkResetDescriptorPool = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, DescriptorPool::HandleType descriptorPoolHandle, DescriptorPoolResetFlags flags);
using PFN_vkAllocateDescriptorSets = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const DescriptorSetAllocateInfo* pAllocateInfo, DescriptorSet::HandleType* pDescriptorSetHandles);
using PFN_vkFreeDescriptorSets = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, DescriptorPool::HandleType descriptorPoolHandle, uint32_t descriptorSetCount, const DescriptorSet::HandleType* pDescriptorSetHandles);
using PFN_vkUpdateDescriptorSets = void (VKAPI_PTR *)(Device::HandleType deviceHandle, uint32_t descriptorWriteCount, const WriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const CopyDescriptorSet* pDescriptorCopies);
using PFN_vkCreateFramebuffer = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const FramebufferCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, Framebuffer::HandleType* pFramebufferHandle);
using PFN_vkDestroyFramebuffer = void (VKAPI_PTR *)(Device::HandleType deviceHandle, Framebuffer::HandleType framebufferHandle, const AllocationCallbacks* pAllocator);
using PFN_vkCreateRenderPass = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const RenderPassCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, RenderPass::HandleType* pRenderPassHandle);
using PFN_vkDestroyRenderPass = void (VKAPI_PTR *)(Device::HandleType deviceHandle, RenderPass::HandleType renderPassHandle, const AllocationCallbacks* pAllocator);
using PFN_vkGetRenderAreaGranularity = void (VKAPI_PTR *)(Device::HandleType deviceHandle, RenderPass::HandleType renderPassHandle, Extent2D* pGranularity);
using PFN_vkCreateCommandPool = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const CommandPoolCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, CommandPool::HandleType* pCommandPoolHandle);
using PFN_vkDestroyCommandPool = void (VKAPI_PTR *)(Device::HandleType deviceHandle, CommandPool::HandleType commandPoolHandle, const AllocationCallbacks* pAllocator);
using PFN_vkResetCommandPool = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, CommandPool::HandleType commandPoolHandle, CommandPoolResetFlags flags);
using PFN_vkAllocateCommandBuffers = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const CommandBufferAllocateInfo* pAllocateInfo, CommandBuffer::HandleType* pCommandBufferHandles);
using PFN_vkFreeCommandBuffers = void (VKAPI_PTR *)(Device::HandleType deviceHandle, CommandPool::HandleType commandPoolHandle, uint32_t commandBufferCount, const CommandBuffer::HandleType* pCommandBufferHandles);
using PFN_vkBeginCommandBuffer = Result (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, const CommandBufferBeginInfo* pBeginInfo);
using PFN_vkEndCommandBuffer = Result (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle);
using PFN_vkResetCommandBuffer = Result (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, CommandBufferResetFlags flags);
using PFN_vkCmdBindPipeline = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, PipelineBindPoint pipelineBindPoint, Pipeline::HandleType pipelineHandle);
using PFN_vkCmdSetViewport = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, uint32_t firstViewport, uint32_t viewportCount, const Viewport* pViewports);
using PFN_vkCmdSetScissor = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, uint32_t firstScissor, uint32_t scissorCount, const Rect2D* pScissors);
using PFN_vkCmdSetLineWidth = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, float lineWidth);
using PFN_vkCmdSetLineStippleEXT = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern);
using PFN_vkCmdSetDepthBias = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
using PFN_vkCmdSetBlendConstants = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, const float blendConstants[4]);
using PFN_vkCmdSetDepthBounds = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, float minDepthBounds, float maxDepthBounds);
using PFN_vkCmdSetStencilCompareMask = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, StencilFaceFlags faceMask, uint32_t compareMask);
using PFN_vkCmdSetStencilWriteMask = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, StencilFaceFlags faceMask, uint32_t writeMask);
using PFN_vkCmdSetStencilReference = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, StencilFaceFlags faceMask, uint32_t reference);
using PFN_vkCmdBindDescriptorSets = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, PipelineBindPoint pipelineBindPoint, PipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const DescriptorSet::HandleType* pDescriptorSetHandles, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);
using PFN_vkCmdBindIndexBuffer = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Buffer::HandleType bufferHandle, DeviceSize offset, IndexType indexType);
using PFN_vkCmdBindVertexBuffers = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, uint32_t firstBinding, uint32_t bindingCount, const Buffer::HandleType* pBufferHandles, const DeviceSize* pOffsets);
using PFN_vkCmdDraw = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
using PFN_vkCmdDrawIndexed = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
using PFN_vkCmdDrawIndirect = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Buffer::HandleType bufferHandle, DeviceSize offset, uint32_t drawCount, uint32_t stride);
using PFN_vkCmdDrawIndexedIndirect = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Buffer::HandleType bufferHandle, DeviceSize offset, uint32_t drawCount, uint32_t stride);
using PFN_vkCmdDispatch = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
using PFN_vkCmdDispatchIndirect = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Buffer::HandleType bufferHandle, DeviceSize offset);
using PFN_vkCmdDispatchBase = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
using PFN_vkCmdCopyBuffer = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Buffer::HandleType srcBufferHandle, Buffer::HandleType dstBufferHandle, uint32_t regionCount, const BufferCopy* pRegions);
using PFN_vkCmdCopyImage = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Image::HandleType srcImageHandle, ImageLayout srcImageLayout, Image::HandleType dstImageHandle, ImageLayout dstImageLayout, uint32_t regionCount, const ImageCopy* pRegions);
using PFN_vkCmdBlitImage = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Image::HandleType srcImageHandle, ImageLayout srcImageLayout, Image::HandleType dstImageHandle, ImageLayout dstImageLayout, uint32_t regionCount, const ImageBlit* pRegions, Filter filter);
using PFN_vkCmdCopyBufferToImage = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Buffer::HandleType srcBufferHandle, Image::HandleType dstImageHandle, ImageLayout dstImageLayout, uint32_t regionCount, const BufferImageCopy* pRegions);
using PFN_vkCmdCopyImageToBuffer = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Image::HandleType srcImageHandle, ImageLayout srcImageLayout, Buffer::HandleType dstBufferHandle, uint32_t regionCount, const BufferImageCopy* pRegions);
using PFN_vkCmdUpdateBuffer = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Buffer::HandleType dstBufferHandle, DeviceSize dstOffset, DeviceSize dataSize, const void* pData);
using PFN_vkCmdFillBuffer = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Buffer::HandleType dstBufferHandle, DeviceSize dstOffset, DeviceSize size, uint32_t data);
using PFN_vkCmdClearColorImage = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Image::HandleType imageHandle, ImageLayout imageLayout, const ClearColorValue* pColor, uint32_t rangeCount, const ImageSubresourceRange* pRanges);
using PFN_vkCmdClearDepthStencilImage = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Image::HandleType imageHandle, ImageLayout imageLayout, const ClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const ImageSubresourceRange* pRanges);
using PFN_vkCmdClearAttachments = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, uint32_t attachmentCount, const ClearAttachment* pAttachments, uint32_t rectCount, const ClearRect* pRects);
using PFN_vkCmdResolveImage = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Image::HandleType srcImageHandle, ImageLayout srcImageLayout, Image::HandleType dstImageHandle, ImageLayout dstImageLayout, uint32_t regionCount, const ImageResolve* pRegions);
using PFN_vkCmdSetEvent = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Event::HandleType eventHandle, PipelineStageFlags stageMask);
using PFN_vkCmdResetEvent = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, Event::HandleType eventHandle, PipelineStageFlags stageMask);
using PFN_vkCmdWaitEvents = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, uint32_t eventCount, const Event::HandleType* pEventHandles, PipelineStageFlags srcStageMask, PipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const MemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const BufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const ImageMemoryBarrier* pImageMemoryBarriers);
using PFN_vkCmdPipelineBarrier = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, PipelineStageFlags srcStageMask, PipelineStageFlags dstStageMask, DependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const MemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const BufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const ImageMemoryBarrier* pImageMemoryBarriers);
using PFN_vkCmdBeginQuery = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, QueryPool::HandleType queryPoolHandle, uint32_t query, QueryControlFlags flags);
using PFN_vkCmdEndQuery = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, QueryPool::HandleType queryPoolHandle, uint32_t query);
using PFN_vkCmdResetQueryPool = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, QueryPool::HandleType queryPoolHandle, uint32_t firstQuery, uint32_t queryCount);
using PFN_vkCmdWriteTimestamp = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, PipelineStageFlagBits pipelineStage, QueryPool::HandleType queryPoolHandle, uint32_t query);
using PFN_vkGetCalibratedTimestampsEXT = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, uint32_t timestampCount, const CalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation);
using PFN_vkCmdCopyQueryPoolResults = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, QueryPool::HandleType queryPoolHandle, uint32_t firstQuery, uint32_t queryCount, Buffer::HandleType dstBufferHandle, DeviceSize dstOffset, DeviceSize stride, QueryResultFlags flags);
using PFN_vkCmdPushConstants = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, PipelineLayout layout, ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues);
using PFN_vkCmdBeginRenderPass = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, const RenderPassBeginInfo* pRenderPassBegin, SubpassContents contents);
using PFN_vkCmdNextSubpass = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, SubpassContents contents);
using PFN_vkCmdEndRenderPass = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle);
using PFN_vkCmdExecuteCommands = void (VKAPI_PTR *)(CommandBuffer::HandleType commandBufferHandle, uint32_t commandBufferCount, const CommandBuffer::HandleType* pCommandBufferHandles);
using PFN_vkCreateDescriptorUpdateTemplate = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const DescriptorUpdateTemplateCreateInfo* pCreateInfo, const AllocationCallbacks* pAllocator, DescriptorUpdateTemplate::HandleType* pDescriptorUpdateTemplateHandle);
using PFN_vkDestroyDescriptorUpdateTemplate =  void (VKAPI_PTR *)(Device::HandleType deviceHandle, DescriptorUpdateTemplate::HandleType descriptorUpdateTemplateHandle, const AllocationCallbacks* pAllocator);
using PFN_vkUpdateDescriptorSetWithTemplate = void (VKAPI_PTR *)(Device::HandleType deviceHandle, DescriptorSet::HandleType descriptorSetHandle, DescriptorUpdateTemplate::HandleType descriptorUpdateTemplateHandle, const void* pData);
using PFN_vkDestroySurfaceKHR = void (VKAPI_PTR *)(Instance::HandleType instanceHandle, SurfaceKHR::HandleType surfaceHandle, const AllocationCallbacks* pAllocator);
using PFN_vkGetPhysicalDeviceSurfaceSupportKHR = Result (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, uint32_t queueFamilyIndex, SurfaceKHR::HandleType surfaceHandle, Bool32* pSupported);
using PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = Result (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, SurfaceKHR::HandleType surfaceHandle, SurfaceCapabilitiesKHR* pSurfaceCapabilities);
using PFN_vkGetPhysicalDeviceSurfaceFormatsKHR = Result (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, SurfaceKHR::HandleType surfaceHandle, uint32_t* pSurfaceFormatCount, SurfaceFormatKHR* pSurfaceFormats);
using PFN_vkGetPhysicalDeviceSurfacePresentModesKHR = Result (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, SurfaceKHR::HandleType surfaceHandle, uint32_t* pPresentModeCount, PresentModeKHR* pPresentModes);
using PFN_vkCreateSwapchainKHR = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const SwapchainCreateInfoKHR* pCreateInfo, const AllocationCallbacks* pAllocator, SwapchainKHR::HandleType* pSwapchainHandle);
using PFN_vkDestroySwapchainKHR = void (VKAPI_PTR *)(Device::HandleType deviceHandle, SwapchainKHR::HandleType swapchainHandle, const AllocationCallbacks* pAllocator);
using PFN_vkGetSwapchainImagesKHR = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, SwapchainKHR::HandleType swapchainHandle, uint32_t* pSwapchainImageCount, Image::HandleType* pSwapchainImageHandles);
using PFN_vkAcquireNextImageKHR = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, SwapchainKHR::HandleType swapchainHandle, uint64_t timeout, Semaphore::HandleType semaphoreHandle, Fence::HandleType fenceHandle, uint32_t* pImageIndex);
using PFN_vkQueuePresentKHR = Result (VKAPI_PTR *)(Queue::HandleType queueHandle, const PresentInfoKHR* pPresentInfo);
using PFN_vkGetDeviceGroupPresentCapabilitiesKHR = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, DeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities);
using PFN_vkGetDeviceGroupSurfacePresentModesKHR = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, SurfaceKHR::HandleType surfaceHandle, DeviceGroupPresentModeFlagsKHR* pModes);
using PFN_vkGetPhysicalDevicePresentRectanglesKHR = Result (VKAPI_PTR *)(PhysicalDevice::HandleType physicalDeviceHandle, SurfaceKHR::HandleType surfaceHandle, uint32_t* pRectCount, Rect2D* pRects);
using PFN_vkAcquireNextImage2KHR = Result (VKAPI_PTR *)(Device::HandleType deviceHandle, const AcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);


struct Funcs {
	PFN_vkGetInstanceProcAddr       vkGetInstanceProcAddr = nullptr;
	PFN_vkEnumerateInstanceExtensionProperties    vkEnumerateInstanceExtensionProperties = nullptr;
	PFN_vkEnumerateInstanceLayerProperties        vkEnumerateInstanceLayerProperties = nullptr;
	PFN_vkCreateInstance            vkCreateInstance = nullptr;
	PFN_vkDestroyInstance           vkDestroyInstance = nullptr;
	PFN_vkEnumeratePhysicalDevices  vkEnumeratePhysicalDevices = nullptr;
	PFN_vkEnumerateDeviceExtensionProperties      vkEnumerateDeviceExtensionProperties = nullptr;
	PFN_vkGetPhysicalDeviceProperties             vkGetPhysicalDeviceProperties = nullptr;
	PFN_vkGetPhysicalDeviceFeatures               vkGetPhysicalDeviceFeatures = nullptr;
	PFN_vkGetPhysicalDeviceFormatProperties       vkGetPhysicalDeviceFormatProperties = nullptr;
	PFN_vkGetPhysicalDeviceImageFormatProperties  vkGetPhysicalDeviceImageFormatProperties = nullptr;
	PFN_vkGetPhysicalDeviceMemoryProperties       vkGetPhysicalDeviceMemoryProperties = nullptr;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties  vkGetPhysicalDeviceQueueFamilyProperties = nullptr;
	PFN_vkGetPhysicalDeviceProperties2            vkGetPhysicalDeviceProperties2 = nullptr;
	PFN_vkGetPhysicalDeviceFeatures2              vkGetPhysicalDeviceFeatures2 = nullptr;
	PFN_vkGetPhysicalDeviceMemoryProperties2      vkGetPhysicalDeviceMemoryProperties2 = nullptr;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2 = nullptr;
	PFN_vkCreateDevice              vkCreateDevice = nullptr;
	PFN_vkDestroyDevice             vkDestroyDevice = nullptr;
	PFN_vkGetDeviceProcAddr         vkGetDeviceProcAddr = nullptr;
	PFN_vkGetDeviceQueue            vkGetDeviceQueue = nullptr;
	PFN_vkCreateBuffer              vkCreateBuffer = nullptr;
	PFN_vkDestroyBuffer             vkDestroyBuffer = nullptr;
	PFN_vkGetBufferDeviceAddress    vkGetBufferDeviceAddress = nullptr;
	PFN_vkCreateBufferView          vkCreateBufferView = nullptr;
	PFN_vkDestroyBufferView         vkDestroyBufferView = nullptr;
	PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties = nullptr;
	PFN_vkQueueSubmit               vkQueueSubmit = nullptr;
	PFN_vkQueueWaitIdle             vkQueueWaitIdle = nullptr;
	PFN_vkDeviceWaitIdle            vkDeviceWaitIdle = nullptr;
	PFN_vkAllocateMemory            vkAllocateMemory = nullptr;
	PFN_vkFreeMemory                vkFreeMemory = nullptr;
	PFN_vkMapMemory                 vkMapMemory = nullptr;
	PFN_vkUnmapMemory               vkUnmapMemory = nullptr;
	PFN_vkFlushMappedMemoryRanges   vkFlushMappedMemoryRanges = nullptr;
	PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges = nullptr;
	PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment = nullptr;
	PFN_vkBindBufferMemory          vkBindBufferMemory = nullptr;
	PFN_vkBindImageMemory           vkBindImageMemory = nullptr;
	PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = nullptr;
	PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements = nullptr;
	PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements = nullptr;
	PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties = nullptr;
	PFN_vkQueueBindSparse           vkQueueBindSparse = nullptr;
	PFN_vkCreateFence               vkCreateFence = nullptr;
	PFN_vkDestroyFence              vkDestroyFence = nullptr;
	PFN_vkResetFences               vkResetFences = nullptr;
	PFN_vkGetFenceStatus            vkGetFenceStatus = nullptr;
	PFN_vkWaitForFences             vkWaitForFences = nullptr;
	PFN_vkCreateSemaphore           vkCreateSemaphore = nullptr;
	PFN_vkDestroySemaphore          vkDestroySemaphore = nullptr;
	PFN_vkCreateEvent               vkCreateEvent = nullptr;
	PFN_vkDestroyEvent              vkDestroyEvent = nullptr;
	PFN_vkGetEventStatus            vkGetEventStatus = nullptr;
	PFN_vkSetEvent                  vkSetEvent = nullptr;
	PFN_vkResetEvent                vkResetEvent = nullptr;
	PFN_vkCreateQueryPool           vkCreateQueryPool = nullptr;
	PFN_vkDestroyQueryPool          vkDestroyQueryPool = nullptr;
	PFN_vkGetQueryPoolResults       vkGetQueryPoolResults = nullptr;
	PFN_vkCreateImage               vkCreateImage = nullptr;
	PFN_vkDestroyImage              vkDestroyImage = nullptr;
	PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout = nullptr;
	PFN_vkCreateImageView           vkCreateImageView = nullptr;
	PFN_vkDestroyImageView          vkDestroyImageView = nullptr;
	PFN_vkCreateShaderModule        vkCreateShaderModule = nullptr;
	PFN_vkDestroyShaderModule       vkDestroyShaderModule = nullptr;
	PFN_vkCreatePipelineCache       vkCreatePipelineCache = nullptr;
	PFN_vkDestroyPipelineCache      vkDestroyPipelineCache = nullptr;
	PFN_vkGetPipelineCacheData      vkGetPipelineCacheData = nullptr;
	PFN_vkMergePipelineCaches       vkMergePipelineCaches = nullptr;
	PFN_vkCreateGraphicsPipelines   vkCreateGraphicsPipelines = nullptr;
	PFN_vkCreateComputePipelines    vkCreateComputePipelines = nullptr;
	PFN_vkDestroyPipeline           vkDestroyPipeline = nullptr;
	PFN_vkCreatePipelineLayout      vkCreatePipelineLayout = nullptr;
	PFN_vkDestroyPipelineLayout     vkDestroyPipelineLayout = nullptr;
	PFN_vkCreateSampler             vkCreateSampler = nullptr;
	PFN_vkDestroySampler            vkDestroySampler = nullptr;
	PFN_vkCreateFramebuffer         vkCreateFramebuffer = nullptr;
	PFN_vkDestroyFramebuffer        vkDestroyFramebuffer = nullptr;
	PFN_vkCreateRenderPass          vkCreateRenderPass = nullptr;
	PFN_vkDestroyRenderPass         vkDestroyRenderPass = nullptr;
	PFN_vkGetRenderAreaGranularity  vkGetRenderAreaGranularity = nullptr;
	PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout = nullptr;
	PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout = nullptr;
	PFN_vkCreateDescriptorPool      vkCreateDescriptorPool = nullptr;
	PFN_vkDestroyDescriptorPool     vkDestroyDescriptorPool = nullptr;
	PFN_vkResetDescriptorPool       vkResetDescriptorPool = nullptr;
	PFN_vkAllocateDescriptorSets    vkAllocateDescriptorSets = nullptr;
	PFN_vkFreeDescriptorSets        vkFreeDescriptorSets = nullptr;
	PFN_vkUpdateDescriptorSets      vkUpdateDescriptorSets = nullptr;
	PFN_vkCreateDescriptorUpdateTemplate vkCreateDescriptorUpdateTemplate = nullptr;
	PFN_vkDestroyDescriptorUpdateTemplate vkDestroyDescriptorUpdateTemplate = nullptr;
	PFN_vkUpdateDescriptorSetWithTemplate vkUpdateDescriptorSetWithTemplate = nullptr;
	PFN_vkCreateCommandPool         vkCreateCommandPool = nullptr;
	PFN_vkDestroyCommandPool        vkDestroyCommandPool = nullptr;
	PFN_vkResetCommandPool          vkResetCommandPool = nullptr;
	PFN_vkAllocateCommandBuffers    vkAllocateCommandBuffers = nullptr;
	PFN_vkFreeCommandBuffers        vkFreeCommandBuffers = nullptr;
	PFN_vkBeginCommandBuffer        vkBeginCommandBuffer = nullptr;
	PFN_vkEndCommandBuffer          vkEndCommandBuffer = nullptr;
	PFN_vkResetCommandBuffer        vkResetCommandBuffer = nullptr;
	PFN_vkCmdPushConstants          vkCmdPushConstants = nullptr;
	PFN_vkCmdBeginRenderPass        vkCmdBeginRenderPass = nullptr;
	PFN_vkCmdNextSubpass            vkCmdNextSubpass = nullptr;
	PFN_vkCmdEndRenderPass          vkCmdEndRenderPass = nullptr;
	PFN_vkCmdExecuteCommands        vkCmdExecuteCommands = nullptr;
	PFN_vkCmdCopyBuffer             vkCmdCopyBuffer = nullptr;
	PFN_vkCmdBindPipeline           vkCmdBindPipeline = nullptr;
	PFN_vkCmdBindDescriptorSets     vkCmdBindDescriptorSets = nullptr;
	PFN_vkCmdBindIndexBuffer        vkCmdBindIndexBuffer = nullptr;
	PFN_vkCmdBindVertexBuffers      vkCmdBindVertexBuffers = nullptr;
	PFN_vkCmdDrawIndexedIndirect    vkCmdDrawIndexedIndirect = nullptr;
	PFN_vkCmdDrawIndexed            vkCmdDrawIndexed = nullptr;
	PFN_vkCmdDraw                   vkCmdDraw = nullptr;
	PFN_vkCmdDrawIndirect           vkCmdDrawIndirect = nullptr;
	PFN_vkCmdDispatch               vkCmdDispatch = nullptr;
	PFN_vkCmdDispatchIndirect       vkCmdDispatchIndirect = nullptr;
	PFN_vkCmdDispatchBase           vkCmdDispatchBase = nullptr;
	PFN_vkCmdPipelineBarrier        vkCmdPipelineBarrier = nullptr;
	PFN_vkCmdSetDepthBias           vkCmdSetDepthBias = nullptr;
	PFN_vkCmdSetLineWidth           vkCmdSetLineWidth = nullptr;
	PFN_vkCmdSetLineStippleEXT      vkCmdSetLineStippleEXT = nullptr;
	PFN_vkCmdSetViewport            vkCmdSetViewport = nullptr;
	PFN_vkCmdSetScissor             vkCmdSetScissor = nullptr;
	PFN_vkCmdSetBlendConstants      vkCmdSetBlendConstants = nullptr;
	PFN_vkCmdSetDepthBounds         vkCmdSetDepthBounds = nullptr;
	PFN_vkCmdSetStencilCompareMask  vkCmdSetStencilCompareMask = nullptr;
	PFN_vkCmdSetStencilWriteMask    vkCmdSetStencilWriteMask = nullptr;
	PFN_vkCmdSetStencilReference    vkCmdSetStencilReference = nullptr;
	PFN_vkCmdResetQueryPool         vkCmdResetQueryPool = nullptr;
	PFN_vkCmdWriteTimestamp         vkCmdWriteTimestamp = nullptr;
	PFN_vkGetCalibratedTimestampsEXT vkGetCalibratedTimestampsEXT = nullptr;
	PFN_vkDestroySurfaceKHR         vkDestroySurfaceKHR = nullptr;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
	PFN_vkCreateSwapchainKHR        vkCreateSwapchainKHR = nullptr;
	PFN_vkDestroySwapchainKHR       vkDestroySwapchainKHR = nullptr;
	PFN_vkGetSwapchainImagesKHR     vkGetSwapchainImagesKHR = nullptr;
	PFN_vkAcquireNextImageKHR       vkAcquireNextImageKHR = nullptr;
	PFN_vkQueuePresentKHR           vkQueuePresentKHR = nullptr;
	PFN_vkGetDeviceGroupPresentCapabilitiesKHR vkGetDeviceGroupPresentCapabilitiesKHR = nullptr;
	PFN_vkGetDeviceGroupSurfacePresentModesKHR vkGetDeviceGroupSurfacePresentModesKHR = nullptr;
	PFN_vkGetPhysicalDevicePresentRectanglesKHR vkGetPhysicalDevicePresentRectanglesKHR = nullptr;
	PFN_vkAcquireNextImage2KHR      vkAcquireNextImage2KHR = nullptr;
	PFN_vkCmdCopyImage              vkCmdCopyImage = nullptr;
	PFN_vkCmdBlitImage              vkCmdBlitImage = nullptr;
	PFN_vkCmdCopyBufferToImage      vkCmdCopyBufferToImage = nullptr;
	PFN_vkCmdCopyImageToBuffer      vkCmdCopyImageToBuffer = nullptr;
	PFN_vkCmdUpdateBuffer           vkCmdUpdateBuffer = nullptr;
	PFN_vkCmdFillBuffer             vkCmdFillBuffer = nullptr;
	PFN_vkCmdClearColorImage        vkCmdClearColorImage = nullptr;
	PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage = nullptr;
	PFN_vkCmdClearAttachments       vkCmdClearAttachments = nullptr;
	PFN_vkCmdResolveImage           vkCmdResolveImage = nullptr;
	PFN_vkCmdSetEvent               vkCmdSetEvent = nullptr;
	PFN_vkCmdResetEvent             vkCmdResetEvent = nullptr;
	PFN_vkCmdWaitEvents             vkCmdWaitEvents = nullptr;
	PFN_vkCmdBeginQuery             vkCmdBeginQuery = nullptr;
	PFN_vkCmdEndQuery               vkCmdEndQuery = nullptr;
	PFN_vkCmdCopyQueryPoolResults   vkCmdCopyQueryPoolResults = nullptr;
};
extern Funcs funcs;




// taken from vkcpp-gen
#if 0
namespace detail {
	template<typename T>
	class Iterator {
	public:
		using value_type = std::remove_cv<T>::type;
		using element_type = std::add_cv<T>::type;
		using pointer    = T*;
		using reference  = T&;
		using difference_type = std::ptrdiff_t;
		//using iterator_category=std::random_access_iterator_tag;
		//using iterator_concept=std::contiguous_iterator_tag;
	private:
		pointer v;
	public:

		Iterator() : v(nullptr) {}
		Iterator(reference r) : v(&r) {}
		Iterator(pointer p) : v(p) {}

		reference                 operator*()             { return *v; }
		// std::add_const<reference> operator*()       const { return *v; }
		pointer                   operator->()  const          { return v; }
		// std::add_const<pointer>   operator->()      const { return v; }
		reference                 operator[](difference_type m) const { return *(v + m); }
		// std::add_const<reference> operator[](difference_type m) const { return *(v + m); }

		friend reference operator*(const Iterator& it) { return *(it.v); }

		friend Iterator operator+(difference_type diff, const Iterator& it) { return it + diff; }
		friend Iterator operator-(difference_type diff, const Iterator& it) { return it - diff; }

		Iterator& operator++()       { ++v; return *this; }
		Iterator& operator--()       { --v; return *this; }
		Iterator  operator++(int)    { Iterator it(*this); ++v; return it; }
		Iterator  operator--(int)    { Iterator it(*this); --v; return it; }

		Iterator& operator+=(difference_type  n)  { v += n; return *this; }
		Iterator& operator-=(difference_type  n)  { v -= n; return *this; }

		Iterator operator+(difference_type  n)   const { Iterator it(*this); return it += n;}
		Iterator operator-(difference_type  n)   const { Iterator it(*this); return it -= n;}

		difference_type operator-(const Iterator& it) const { return v - it.v; }

		bool operator<(Iterator const& it)  const { return v <  it.v; }
		bool operator<=(Iterator const& it) const { return v <= it.v; }
		bool operator>(Iterator const& it)  const { return v >  it.v; }
		bool operator>=(Iterator const& it) const { return v >= it.v; }
		bool operator!=(const Iterator &it) const { return v != it.v; }
		bool operator==(const Iterator &it) const { return v == it.v; }
	};
}

template<typename T, size_t N = 0, bool s = N != 0>
class vector;

template<typename T, size_t N>
class vector<T, N, true> {

	using value_type = T;
	using reference = value_type&;
	using const_reference = const value_type&;
	using iterator = detail::Iterator<T>;
	using const_iterator = detail::Iterator<T>;

	T *m_begin = buffer;
	T *m_end   = buffer;
	size_t cap = N;
	T buffer[N];

	void deallocate_storage() noexcept(std::is_nothrow_destructible_v<T>)
	{
		clear();
		if (m_begin && m_begin != buffer) {
			std::allocator<T>().deallocate(m_begin, cap);
		}
		m_begin = buffer;
		m_end   = buffer;
		cap     = N;
	}

#if __cpp_lib_allocate_at_least
	using allocation_result = std::allocation_result<T*>;
	static allocation_result allocate(size_t count) {
		return std::allocator<T>().allocate_at_least(count);
	}
#else
	struct allocation_result {
		T *ptr;
		size_t count;
	};
	static allocation_result allocate(size_t count) {
		return { std::allocator<T>().allocate(count), count };
	}
#endif

	void set_storage(const allocation_result &alloc, size_t size) noexcept
	{
		m_begin = alloc.ptr;
		m_end = alloc.ptr + size;
		cap = alloc.count;
	}

	void reallocate(size_t count, size_t size)
	{
		auto alloc = allocate(count);
		if (m_begin) {
			std::memcpy(alloc.ptr, m_begin, this->size() * sizeof(T));
			if (!is_inline())
				std::allocator<T>().deallocate(m_begin, cap);
		}
		set_storage(alloc, size);
	}

	void reallocate(size_t count)
	{
		reallocate(count, count);
	}

	template<size_t X>
	void copy_items_from(vector<T, X> &v)
	{
		const T* src = v.m_begin;
		for (auto* it = m_begin; it != m_end; ++it) {
			*it = *src;
			src++;
		}
	}

	template<size_t X>
	void move_items_from(vector<T, X> &&v)
	{
		T* src = v.m_begin;
		for (auto* it = m_begin; it != m_end; ++it) {
			*it = std::move(*src);
			src++;
		}
	}

	template<size_t X>
	void move_from(vector<T, X> &&v)
	{
		if (v.is_inline()) {
			unitialized_resize(v.size());
			move_items_from(std::forward<vector<T, X>>(v));
			v.deallocate_storage();
		} else {
			std::swap(m_begin, v.m_begin);
			std::swap(m_end, v.m_end);
			std::swap(cap, v.cap);
		}
	}

	void destroy_items(T *begin, T *end) noexcept(std::is_nothrow_destructible_v<T>)
	{
		for (auto* it = begin; it != end; ++it) {
			std::destroy_at(it);
			*it = {};
		};
	}

public:

	constexpr vector() = default;

	template<size_t X>
	constexpr vector(vector<T, X> &v)
	{
		if (v.empty())
			return;
		reserve(v.size());
		m_end = m_begin + v.size();
		copy_items_from(v);
	}

	template<size_t X>
	constexpr vector(vector<T, X> &&v) noexcept
	{
		move_from(std::forward<vector<T, X>>(v));
	}

	template<size_t X>
	vector& operator=(vector<T, X> &v)
	{
		if (this != &v) {
			clear();
			unitialized_resize(v.size());
			copy_items_from(v);
		}
		return *this;
	}

	template<size_t X>
	vector& operator=(vector<T, X> &&v)
	{
		if (this != &v) {
			clear();
			move_from(std::forward<vector<T, X>>(v));
		}
		return *this;
	}

	constexpr explicit vector(size_t s)
	{
		resize(s);
	}

	~vector() noexcept(std::is_nothrow_destructible_v<T>)
	{
		deallocate_storage();
	}

	void clear() noexcept(std::is_nothrow_destructible_v<T>)
	{
		if constexpr (!std::is_trivially_destructible_v<T>) {
			for (auto* it = m_begin; it != m_end; ++it) {
			std::destroy_at(it);
			}
		}
		m_end = m_begin;
	}

	void confirm(size_t s) // deprecated
	{
		m_end = m_begin + s;
	}

	void reserve(size_t s)
	{
		if (s <= cap || s == 0)
			return;
		reallocate(s, size());
	}

	void resize(size_t s)
	{
		size_t cs = size();
		if (s < cs) {
			if constexpr (!std::is_trivially_destructible_v<T>) {
				auto *old = m_end;
				m_end = m_begin + s;
				destroy_items(m_end, old);
			}
			else
				m_end = m_begin + s;
		}
		else if (s > cs) {
			T* it;
			if (s > cap) {
				reallocate(s);
				it = m_begin + cs;
			}
			else {
				it = m_end;
				m_end = m_begin + s;
			}
			for (; it != m_end; ++it)
				std::construct_at(it);
		}
	}

	void unitialized_resize(size_t s)
	{
		if constexpr (!std::is_trivially_destructible_v<T>) {
			resize(s);
			return;
		}

		size_t cs = size();
		if (s < cs)
			m_end = m_begin + s;
		else if (s > cs) {
			if (s > cap)
				reallocate(s);
			else
				m_end = m_begin + s;
		}
	}

	constexpr bool is_inline() const noexcept
	{
		return m_begin == buffer;
	}

	size_t size() const noexcept
	{
		return m_end - m_begin;
	}

	size_t capacity() const noexcept
	{
		return cap;
	}

	constexpr size_t buffer_capacity() const
	{
		return N;
	}

	constexpr bool empty() const noexcept
	{
		return m_begin == m_end;
	}

	constexpr T *data() noexcept
	{
		return m_begin;
	}

	constexpr const T *data() const noexcept
	{
		return m_begin;
	}

	iterator begin() noexcept
	{
		return iterator{m_begin};
	}

	const_iterator begin() const noexcept
	{
		return const_iterator{m_begin};
	}

	iterator end() noexcept
	{
		return iterator{m_end};
	}
	
	const_iterator end() const noexcept
	{
		return const_iterator{m_end};
	}

	constexpr reference operator[](size_t n) noexcept
	{
		assert(n < size() && "vector[] index out of bounds");
		return m_begin[n];
	}
	constexpr const_reference operator[](size_t n) const noexcept
	{
		assert(n < size() && "vector[] index out of bounds");
		return m_begin[n];
	}

	constexpr reference at(size_t n)
	{
		if (n >= size())
			throw std::out_of_range("vector");
		return m_begin[n];
	}
	constexpr const_reference at(size_t n) const
	{
		if (n >= size())
			throw std::out_of_range("vector");
		return m_begin[n];
	}

	constexpr reference front() noexcept
	{
		assert(!empty() && "front() called on an empty vector");
		return *m_begin;
	}
	constexpr const_reference front() const noexcept
	{
		assert(!empty() && "front() called on an empty vector");
		return *m_begin;
	}

	constexpr reference back() noexcept
	{
		assert(!empty() && "back() called on an empty vector");
		return *(m_end - 1);
	}

	constexpr const_reference back() const noexcept
	{
		assert(!empty() && "back() called on an empty vector");
		return *(m_end - 1);
	}
};

template<typename T, size_t N>
class vector<T, N, false> {

	using value_type = T;
	using reference = value_type&;
	using const_reference = const value_type&;
	using iterator = detail::Iterator<T>;
	using const_iterator = detail::Iterator<T>;

	T *m_begin = {};
	T *m_end   = {};
	size_t cap = 0;

	void deallocate_storage() noexcept(std::is_nothrow_destructible_v<T>)
	{
		if (m_begin) {
			clear();
			std::allocator<T>().deallocate(m_begin, cap);
			m_begin = {};
			m_end   = {};
			cap     = 0;
		}
	}

#if __cpp_lib_allocate_at_least
	using allocation_result = std::allocation_result<T*>;
	static allocation_result allocate(size_t count) {
		return std::allocator<T>().allocate_at_least(count);
	}
#else
	struct allocation_result {
		T *ptr;
		size_t count;
	};
	static allocation_result allocate(size_t count) {
		return { std::allocator<T>().allocate(count), count };
	}
#endif

	void set_storage(const allocation_result &alloc, size_t size) noexcept
	{
		m_begin = alloc.ptr;
		m_end = alloc.ptr + size;
		cap = alloc.count;
	}

	void reallocate(size_t count, size_t size)
	{
		auto alloc = allocate(count);
		if (m_begin) {
			std::memcpy(alloc.ptr, m_begin, this->size() * sizeof(T));
			std::allocator<T>().deallocate(m_begin, cap);
		}
		set_storage(alloc, size);
	}

	void reallocate(size_t count)
	{
		reallocate(count, count);
	}

	template<size_t X>
	void copy_items_from(vector<T, X> &v)
	{
		const T* src = v.m_begin;
		for (auto* it = m_begin; it != m_end; ++it) {
			*it = *src;
			src++;
		}
	}

	template<size_t X>
	void move_items_from(vector<T, X> &&v)
	{
		T* src = v.m_begin;
		for (auto* it = m_begin; it != m_end; ++it) {
			*it = std::move(*src);
			src++;
		}
	}

	template<size_t X>
	void move_from(ector<T, X> &&v)
	{
		if (v.is_inline()) {
			unitialized_resize(v.size());
			move_items_from(std::forward<vector<T, X>>(v));
			v.deallocate_storage();
		} else {
			std::swap(m_begin, v.m_begin);
			std::swap(m_end, v.m_end);
			std::swap(cap, v.cap);
		}
	}

	void destroy_items(T *begin, T *end) noexcept(std::is_nothrow_destructible_v<T>)
	{
		for (auto* it = begin; it != end; ++it) {
			std::destroy_at(it);
			*it = {};
		};
	}

public:

	constexpr Vector() = default;

	template<size_t X>
	constexpr Vector(vector<T, X> &v)
	{
		if (v.empty())
			return;
		reserve(v.size());
		m_end = m_begin + v.size();
		copy_items_from(v);
	}

	template<size_t X>
	constexpr Vector(Vector<T, X> &&v) noexcept
	{
		move_from(std::forward<Vector<T, X>>(v));
	}

	template<size_t X>
	Vector& operator=(Vector<T, X> &v)
	{
		if (this != &v) {
			clear();
			unitialized_resize(v.size());
			copy_items_from(v);
		}
		return *this;
	}

	template<size_t X>
	Vector& operator=(Vector<T, X> &&v)
	{
		if (this != &v) {
			clear();
			move_from(std::forward<Vector<T, X>>(v));
		}
		return *this;
	}

	constexpr explicit Vector(size_t s)
	{
		resize(s);
	}

	~Vector() noexcept(std::is_nothrow_destructible_v<T>)
	{
		deallocate_storage();
	}

	void clear() noexcept(std::is_nothrow_destructible_v<T>)
	{
		if constexpr (!std::is_trivially_destructible_v<T>)
			destroy_items(m_begin, m_end);
		m_end = m_begin;
	}

	void reserve(size_t s)
	{
		if (s <= cap || s == 0)
			return;
		reallocate(s, size());
	}

	void resize(size_t s)
	{
		size_t cs = size();
		if (s < cs) {
			if constexpr (!std::is_trivially_destructible_v<T>) {
				auto *old = m_end;
				m_end = m_begin + s;
				destroy_items(m_end, old);
			}
			else
				m_end = m_begin + s;
		}
		else if (s > cs) {
			T* it;
			if (s > cap) {
				reallocate(s);
				it = m_begin + cs;
			}
			else {
				it = m_end;
				m_end = m_begin + s;
			}
			for (; it != m_end; ++it)
				std::construct_at(it);
		}
	}

	void unitialized_resize(size_t s)
	{
		if constexpr (!std::is_trivially_destructible_v<T>) {
			resize(s);
			return;
		}

		size_t cs = size();
		if (s < cs)
			m_end = m_begin + s;
		else if (s > cs) {
			if (s > cap)
				reallocate(s);
			else
				m_end = m_begin + s;
		}
	}

	constexpr bool is_inline() const noexcept
	{
		return false;
	}

	size_t size() const noexcept
	{
		return m_end - m_begin;
	}

	void confirm(size_t s) // deprecated
	{
		m_end = m_begin + s;
	}

	size_t capacity() const noexcept
	{
		return cap;
	}

	constexpr size_t buffer_capacity() const
	{
		return N;
	}

	constexpr bool empty() const noexcept
	{
		return m_begin == m_end;
	}

	constexpr T *data() noexcept
	{
		return m_begin;
	}

	constexpr const T *data() const noexcept
	{
		return m_begin;
	}

	iterator begin() noexcept
	{
		return iterator{m_begin};
	}

	const_iterator begin() const noexcept
	{
		return const_iterator{m_begin};
	}

	iterator end() noexcept
	{
		return iterator{m_end};
	}

	const_iterator end() const noexcept
	{
		return const_iterator{m_end};
	}

	constexpr reference operator[](size_t n) noexcept
	{
		assert(n < size() && "vector[] index out of bounds");
		return m_begin[n];
	}

	constexpr const_reference operator[](size_t n) const noexcept
	{
		assert(n < size() && "vector[] index out of bounds");
		return m_begin[n];
	}

	constexpr reference at(size_t n)
	{
		if (n >= size())
			throw std::out_of_range("vector");
		return m_begin[n];
	}

	constexpr const_reference at(size_t n) const
	{
		if (n >= size())
			throw std::out_of_range("vector");
		return m_begin[n];
	}

	constexpr reference front() noexcept
	{
		assert(!empty() && "front() called on an empty vector");
		return *m_begin;
	}

	constexpr const_reference front() const noexcept
	{
		assert(!empty() && "front() called on an empty vector");
		return *m_begin;
	}

	constexpr reference back() noexcept
	{
		assert(!empty() && "back() called on an empty vector");
		return *(m_end - 1);
	}

	constexpr const_reference back() const noexcept
	{
		assert(!empty() && "back() called on an empty vector");
		return *(m_end - 1);
	}
};
#endif


// vector class
// author: PCJohn (peciva at fit.vut.cz)
template<typename Type>
class vector {
protected:
	Type* _data;
	size_t _size;
public:
	vector() noexcept : _data(nullptr), _size(0)  {}
	vector(size_t size) : _data(new Type[size]), _size(size)  {}
	vector(Type* data, size_t size) noexcept : _data(data), _size(size)  {}
	~vector() noexcept  { delete[] _data; }

	vector(const vector& other);
	vector(vector&& other) noexcept : _data(other._data), _size(other._size)  { other._data = nullptr; other._size = 0; }
	vector& operator=(const vector& rhs);
	vector& operator=(vector&& rhs) noexcept  { delete[] _data; _data = rhs._data; _size = rhs._size; rhs._data = nullptr; rhs._size = 0; return *this; }

	Type& operator[](size_t index)  { return _data[index]; }
	const Type& operator[](size_t index) const  { return _data[index]; }
	Type* data()  { return _data; }
	const Type* data() const  { return _data; }
	size_t size() const  { return _size; }
	bool empty() const  { return _size == 0; }

	void clear() noexcept  { if(_data == nullptr) return; delete[] _data; _data = nullptr; _size = 0; }
	void alloc(size_t size)  { delete[] _data; _data = nullptr; _size = 0; if(size) { _data = new Type[size]; _size = size; } }
	bool alloc_noThrow(size_t size) noexcept  { delete[] _data; if(size == 0) { _data = nullptr; _size = 0; return true; } _data = new(std::nothrow) Type[size]; if(!_data) { _size = 0; return false; } _size = size; return true; }
	void resize(size_t newSize);
	bool resize_noThrow(size_t newSize) noexcept;
};


template<typename Type>
vector<Type>::vector(const vector& other)
	: _data(reinterpret_cast<Type*>(::operator new[](sizeof(Type) * other._size)))
	, _size(other._size)
{
	size_t i=0;
	try {
		for(size_t c=other._size; i<c; i++)
			new(&_data[i]) Type(other._data[i]);
	} catch(...) {
		while(i > 0) {
			i--;
			_data[i].~Type();
		}
		::operator delete[](_data);
		_data = nullptr;
		_size = 0;
		throw;
	}
}


template<typename Type>
vector<Type>& vector<Type>::operator=(const vector& rhs)
{
	if(_size == rhs._size)
		for(size_t i=0,c=_size; i<c; i++)
			_data[i] = rhs._data[i];
	else {
		delete[] _data;
		_data = nullptr;
		_size = 0;
		_data = ::operator new(sizeof(Type) * _size);
		size_t i=0;
		try {
			for(; i<rhs._size; i++)
				new(&_data[i]) Type(rhs._data[i]);
		} catch(...) {
			while(i > 0) {
				i--;
				_data[i].~Type();
			}
			::operator delete[](_data);
			_data = nullptr;
			throw;
		}
		_size = rhs._size;
	}
	return *this;
}


template<typename Type>
void vector<Type>::resize(size_t newSize)
{
	if(newSize > size()) {
		Type* m = reinterpret_cast<Type*>(::operator new(sizeof(Type) * newSize));
		size_t i = 0;
		size_t s = size();
		size_t j = s;
		try {
			for(; i<s; i++)
				new(&m[i]) Type(static_cast<Type&&>(_data[i]));
			for(; j<newSize; j++)
				new(&m[j]) Type();
		} catch(...) {
			while(j != s) {
				j--;
				m[j].~Type();
			}
			while(i != 0) {
				i--;
				m[i].~Type();
			}
			::operator delete[](m);
			throw;
		}
		Type* tmp = _data;
		_data = m;
		_size = newSize;
		delete[] tmp;
	}
	else if(newSize < size())
	{
		if(newSize == 0)
			clear();
		else {
			Type* m = reinterpret_cast<Type*>(::operator new(sizeof(Type) * newSize));
			size_t i = 0;
			try {
				for(size_t c=newSize; i<c; i++)
					new(&m[i]) Type(static_cast<Type&&>(_data[i]));
			} catch(...) {
				while(i != 0) {
					i--;
					m[i].~Type();
				}
				::operator delete[](m);
				throw;
			}
			Type* tmp = _data;
			_data = m;
			_size = newSize;
			delete[] tmp;
		}
	}
}


template<typename Type>
bool vector<Type>::resize_noThrow(size_t newSize) noexcept
{
	if(newSize > size()) {
		Type* m = reinterpret_cast<Type*>(::operator new(sizeof(Type) * newSize, std::nothrow));
		if(!m)  return false;
		size_t i = 0;
		size_t s = size();
		size_t j = s;
		try {
			for(; i<s; i++)
				new(&m[i]) Type(static_cast<Type&&>(_data[i]));
			for(; j<newSize; j++)
				new(&m[j]) Type();
		} catch(...) {
			while(j != s) {
				j--;
				m[j].~Type();
			}
			while(i != 0) {
				i--;
				m[i].~Type();
			}
			::operator delete[](m);
			clear();
			return false;
		}
		Type* tmp = _data;
		_data = m;
		_size = newSize;
		delete[] tmp;
	}
	else if(newSize < size())
	{
		if(newSize == 0)
			clear();
		else {
			Type* m = reinterpret_cast<Type*>(::operator new(sizeof(Type) * newSize, std::nothrow));
			if(!m)  return false;
			size_t i = 0;
			try {
				for(size_t c=newSize; i<c; i++)
					new(&m[i]) Type(static_cast<Type&&>(_data[i]));
			} catch(...) {
				while(i != 0) {
					i--;
					m[i].~Type();
				}
				::operator delete[](m);
				clear();
				return false;
			}
			Type* tmp = _data;
			_data = m;
			_size = newSize;
			delete[] tmp;
		}
	}
	return true;
}


// span class
// author: PCJohn (peciva at fit.vut.cz)
template<typename Type>
class span {
protected:
	Type* _data;
	size_t _size;
public:
	span() : _data(nullptr), _size(0)  {}
	span(Type* data, size_t size) : _data(data), _size(size)  {}
	
	span(const span& other) : _data(other._data), _size(other._size)  {}
	span(span&& other) : _data(other._data), _size(other.size)  {}
	span& operator=(const span& rhs)  { _data = rhs._data; _size = rhs._size; }
	span& operator=(span&& rhs)  { _data = rhs._data; _size = rhs._size; }

	const Type* data() const  { return _data; }
	Type* data()  { return _data; }
	size_t size() const  { return _size; }

	void set(Type* data, size_t size)  { _data = data; _size = size; }
	void clear()  { _data = nullptr; _size = 0; }

};


// string_view class
// author: PCJohn (peciva at fit.vut.cz)
class string_view {
protected:
	const char* _data;
	size_t _size;
public:
	constexpr string_view() noexcept  : _data(nullptr), _size(0) {}
	constexpr string_view(const char* s)  : _data(s), _size(sizeof(*s)-1) {}
	constexpr string_view(const char* s, size_t count)  : _data(s), _size(count) {}
	constexpr string_view(const string_view& other) noexcept = default;
	constexpr string_view& operator=(const string_view& rhs) noexcept = default;
	constexpr const char& operator[](size_t pos) const  { return _data[pos]; }
	constexpr const char* data() const noexcept  { return _data; }
	constexpr size_t size() const noexcept  { return _size; }
	constexpr size_t length() const noexcept  { return _size; }
	constexpr bool empty() const noexcept  { return _size == 0; }
};



// conversions
// author: PCJohn (peciva at fit.vut.cz)
const char* to_cstr(PhysicalDeviceType v);
string_view to_string_view(PhysicalDeviceType v);

// resultToString() returns Span that contains pointer to const char array
// and size of the string excluding terminating zero character
span<const char> resultToString(Result result);
// int32ToString converts int32_t to text that is stored in the buffer pointed by bufferAtLeast12BytesLong;
// the buffer of 12 bytes is long enough to store all digits of int32, sign mark and terminating zero character;
// the buffer can be smaller if the user is sure that the value can be stored there;
// returned value is the length of the string stored in bufferAtLeast12BytesLong excluding terminating zero character
size_t int32ToString(int32_t value, char* bufferAtLeast12BytesLong);



// exceptions
// author: PCJohn (peciva at fit.vut.cz)
void throwResultExceptionWithMessage(Result result, const char* message);
void throwResultException(Result result, const char* funcName);
inline void checkForSuccessValue(Result result, const char* funcName)  { if(result != vk::Result::eSuccess) throwResultException(result, funcName); }
inline void checkSuccess(Result result, const char* funcName)  { if(int32_t(result) < 0) throwResultException(result, funcName); }

class Error {
protected:
	char* _msg = nullptr;
public:
	Error() noexcept = default;
	Error(const char* message) noexcept  { if(message==nullptr) { _msg=nullptr; return; }  size_t n=strlen(message)+1; _msg = reinterpret_cast<char*>(malloc(n)); if(_msg) strncpy(_msg, message, n); }
	Error(const char* msgHeader, const char* msgBody) noexcept;
	Error(const char* funcName, Result result) noexcept;
	Error(const Error& other) noexcept  { delete[] _msg; if(other._msg==nullptr) { _msg=nullptr; return; } size_t n=strlen(other._msg)+1; _msg = reinterpret_cast<char*>(malloc(n)); if(_msg) strncpy(_msg, other._msg, n); }
	Error& operator=(const Error& rhs) noexcept  { if(rhs._msg==nullptr) { _msg=nullptr; return *this; } size_t n=strlen(rhs._msg)+1; _msg = reinterpret_cast<char*>(malloc(n)); if(_msg) strncpy(_msg, rhs._msg, n); return *this; }
	virtual ~Error() noexcept  { delete[] _msg; }
	virtual const char* what() const noexcept  { return _msg ? _msg : "Unknown exception"; }
};

class SuccessResult : public Error { public: using Error::Error; };
class NotReadyResult : public Error { public: using Error::Error; };
class TimeoutResult : public Error { public: using Error::Error; };
class EventSetResult : public Error { public: using Error::Error; };
class EventResetResult : public Error { public: using Error::Error; };
class IncompleteResult : public Error { public: using Error::Error; };
class OutOfHostMemoryError : public Error { public: using Error::Error; };
class OutOfDeviceMemoryError : public Error { public: using Error::Error; };
class InitializationFailedError : public Error { public: using Error::Error; };
class DeviceLostError : public Error { public: using Error::Error; };
class MemoryMapFailedError : public Error { public: using Error::Error; };
class LayerNotPresentError : public Error { public: using Error::Error; };
class ExtensionNotPresentError : public Error { public: using Error::Error; };
class FeatureNotPresentError : public Error { public: using Error::Error; };
class IncompatibleDriverError : public Error { public: using Error::Error; };
class TooManyObjectsError : public Error { public: using Error::Error; };
class FormatNotSupportedError : public Error { public: using Error::Error; };
class FragmentedPoolError : public Error { public: using Error::Error; };
class UnknownError : public Error { public: using Error::Error; };

// version 1.1
class OutOfPoolMemoryError : public Error { public: using Error::Error; };

// version 1.1
class InvalidExternalHandleError : public Error { public: using Error::Error; };

// version 1.2
class FragmentationError : public Error { public: using Error::Error; };

// version 1.2
class InvalidOpaqueCaptureAddressError : public Error { public: using Error::Error; };


// vkg errors
class VkgError : public Error { public: using Error::Error; };



// functions class
// author: PCJohn (peciva at fit.vut.cz)
template<typename T> inline T getGlobalProcAddr(const char* name) noexcept  { return reinterpret_cast<T>(funcs.vkGetInstanceProcAddr(nullptr, name)); }
template<typename T> inline T getInstanceProcAddr(const char* name) noexcept  { return reinterpret_cast<T>(funcs.vkGetInstanceProcAddr(detail::_instance.handle(), name)); }
template<typename T> inline T getDeviceProcAddr(const char* name) noexcept  { return reinterpret_cast<T>(funcs.vkGetDeviceProcAddr(detail::_device.handle(), name)); }
inline bool isExtensionSupported(const vector<ExtensionProperties>& extensionList, const char* extensionName) noexcept
{
	for(size_t i=0,c=extensionList.size(); i<c; i++)
		if(strcmp(extensionList[i].extensionName, extensionName) == 0)
			return true;
	return false;
}

vector<ExtensionProperties> enumerateInstanceExtensionProperties_throw(const char* pLayerName);
Result enumerateInstanceExtensionProperties_noThrow(const char* pLayerName, vector<ExtensionProperties>& extensionList) noexcept;
inline vector<ExtensionProperties> enumerateInstanceExtensionProperties(const char* pLayerName)  { return enumerateInstanceExtensionProperties_throw(pLayerName); }

vector<LayerProperties> enumerateInstanceLayerProperties_throw();
Result enumerateInstanceLayerProperties_noThrow(vector<LayerProperties>& properties) noexcept;
inline vector<LayerProperties> enumerateInstanceLayerProperties()  { return enumerateInstanceLayerProperties_throw(); }

inline void destroyInstance(Instance::HandleType instanceHandle) noexcept  { funcs.vkDestroyInstance(instanceHandle, nullptr); }
inline void destroy(Instance::HandleType instanceHandle) noexcept  { funcs.vkDestroyInstance(instanceHandle, nullptr); }


vector<PhysicalDevice> enumeratePhysicalDevices_throw();
Result enumeratePhysicalDevices_noThrow(vector<PhysicalDevice>& physicalDevices) noexcept;
inline vector<PhysicalDevice> enumeratePhysicalDevices()  { return enumeratePhysicalDevices_throw(); }

vector<ExtensionProperties> enumerateDeviceExtensionProperties_throw(PhysicalDevice pd, const char* pLayerName);
Result enumerateDeviceExtensionProperties_noThrow(PhysicalDevice pd, const char* pLayerName, vector<ExtensionProperties>& physicalDevices) noexcept;
inline vector<ExtensionProperties> enumerateDeviceExtensionProperties(PhysicalDevice pd, const char* pLayerName)  { return enumerateDeviceExtensionProperties_throw(pd, pLayerName); }
inline vector<ExtensionProperties> enumerateDeviceExtensionProperties_throw(const char* pLayerName)  { return enumerateDeviceExtensionProperties_throw(physicalDevice(), pLayerName); }
inline Result enumerateDeviceExtensionProperties_noThrow(const char* pLayerName, vector<ExtensionProperties>& physicalDevices) noexcept  { return enumerateDeviceExtensionProperties_noThrow(physicalDevice(), pLayerName, physicalDevices); }
inline vector<ExtensionProperties> enumerateDeviceExtensionProperties(const char* pLayerName)  { return enumerateDeviceExtensionProperties_throw(physicalDevice(), pLayerName); }

inline PhysicalDeviceProperties getPhysicalDeviceProperties(PhysicalDevice pd) noexcept  { PhysicalDeviceProperties p; funcs.vkGetPhysicalDeviceProperties(pd.handle(), &p); return p; }
inline PhysicalDeviceProperties getPhysicalDeviceProperties() noexcept  { return getPhysicalDeviceProperties(physicalDevice()); }
inline void getPhysicalDeviceProperties2(PhysicalDevice pd, PhysicalDeviceProperties2& pProperties) noexcept  { funcs.vkGetPhysicalDeviceProperties2(pd.handle(), &pProperties); }
inline void getPhysicalDeviceProperties2(PhysicalDeviceProperties2& pProperties) noexcept  { getPhysicalDeviceProperties2(physicalDevice(), pProperties); }

inline PhysicalDeviceFeatures getPhysicalDeviceFeatures(PhysicalDevice pd) noexcept  { PhysicalDeviceFeatures f; funcs.vkGetPhysicalDeviceFeatures(pd.handle(), &f); return f; }
inline PhysicalDeviceFeatures getPhysicalDeviceFeatures() noexcept  { return getPhysicalDeviceFeatures(physicalDevice()); }
inline void getPhysicalDeviceFeatures2(PhysicalDevice pd, PhysicalDeviceFeatures2& pFeatures) noexcept  { funcs.vkGetPhysicalDeviceFeatures2(pd.handle(), &pFeatures); }
inline void getPhysicalDeviceFeatures2(PhysicalDeviceFeatures2& pFeatures) noexcept  { getPhysicalDeviceFeatures2(physicalDevice(), pFeatures); }

inline FormatProperties getPhysicalDeviceFormatProperties(PhysicalDevice pd, Format f) noexcept  { FormatProperties p; funcs.vkGetPhysicalDeviceFormatProperties(pd.handle(), f, &p); return p; }
inline FormatProperties getPhysicalDeviceFormatProperties(Format f) noexcept  { return getPhysicalDeviceFormatProperties(physicalDevice(), f); }

inline ImageFormatProperties getPhysicalDeviceImageFormatProperties_throw(PhysicalDevice pd, Format f, ImageType type, ImageTiling tiling, ImageUsageFlags usage, ImageCreateFlags flags)  { ImageFormatProperties p; Result r = funcs.vkGetPhysicalDeviceImageFormatProperties(pd.handle(), f, type, tiling, usage, flags, &p); checkForSuccessValue(r, "vkGetPhysicalDeviceImageFormatProperties"); return p; }
inline Result getPhysicalDeviceImageFormatProperties_noThrow(PhysicalDevice pd, Format f, ImageType type, ImageTiling tiling, ImageUsageFlags usage, ImageCreateFlags flags, ImageFormatProperties& imageFormatProperties) noexcept  { return funcs.vkGetPhysicalDeviceImageFormatProperties(pd.handle(), f, type, tiling, usage, flags, &imageFormatProperties); }
inline ImageFormatProperties getPhysicalDeviceImageFormatProperties(PhysicalDevice pd, Format f, ImageType type, ImageTiling tiling, ImageUsageFlags usage, ImageCreateFlags flags)  { return getPhysicalDeviceImageFormatProperties_throw(pd, f, type, tiling, usage, flags); }
inline ImageFormatProperties getPhysicalDeviceImageFormatProperties_throw(Format f, ImageType type, ImageTiling tiling, ImageUsageFlags usage, ImageCreateFlags flags)  { return getPhysicalDeviceImageFormatProperties_throw(physicalDevice(), f, type, tiling, usage, flags); }
inline Result getPhysicalDeviceImageFormatProperties_noThrow(Format f, ImageType type, ImageTiling tiling, ImageUsageFlags usage, ImageCreateFlags flags, ImageFormatProperties& imageFormatProperties) noexcept  { return getPhysicalDeviceImageFormatProperties_noThrow(physicalDevice(), f, type, tiling, usage, flags, imageFormatProperties); }
inline ImageFormatProperties getPhysicalDeviceImageFormatProperties(Format f, ImageType type, ImageTiling tiling, ImageUsageFlags usage, ImageCreateFlags flags)  { return getPhysicalDeviceImageFormatProperties_throw(physicalDevice(), f, type, tiling, usage, flags); }

inline PhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties(PhysicalDevice pd) noexcept  { PhysicalDeviceMemoryProperties p; funcs.vkGetPhysicalDeviceMemoryProperties(pd.handle(), &p); return p; }
inline PhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties() noexcept  { return getPhysicalDeviceMemoryProperties(physicalDevice()); }
inline void getPhysicalDeviceMemoryProperties2(PhysicalDevice pd, PhysicalDeviceMemoryProperties2& pMemoryProperties) noexcept  { funcs.vkGetPhysicalDeviceMemoryProperties2(pd.handle(), &pMemoryProperties); }
inline void getPhysicalDeviceMemoryProperties2(PhysicalDeviceMemoryProperties2& pMemoryProperties) noexcept  { getPhysicalDeviceMemoryProperties2(physicalDevice(), pMemoryProperties); }

vector<QueueFamilyProperties> getPhysicalDeviceQueueFamilyProperties_throw(PhysicalDevice pd);
Result getPhysicalDeviceQueueFamilyProperties_noThrow(PhysicalDevice pd, vector<QueueFamilyProperties>& queueFamilyList) noexcept;
inline vector<QueueFamilyProperties> getPhysicalDeviceQueueFamilyProperties(PhysicalDevice pd)  { return getPhysicalDeviceQueueFamilyProperties_throw(pd); }
inline vector<QueueFamilyProperties> getPhysicalDeviceQueueFamilyProperties_throw()  { return getPhysicalDeviceQueueFamilyProperties_throw(physicalDevice()); }
inline Result getPhysicalDeviceQueueFamilyProperties_noThrow(vector<QueueFamilyProperties>& queueFamilyList) noexcept  { return getPhysicalDeviceQueueFamilyProperties_noThrow(physicalDevice(), queueFamilyList); }
inline vector<QueueFamilyProperties> getPhysicalDeviceQueueFamilyProperties()  { return getPhysicalDeviceQueueFamilyProperties_throw(physicalDevice()); }

vector<QueueFamilyProperties2> getPhysicalDeviceQueueFamilyProperties2_throw(PhysicalDevice pd);
Result getPhysicalDeviceQueueFamilyProperties2_noThrow(PhysicalDevice pd, vector<QueueFamilyProperties2>& queueFamilyProperties) noexcept;
inline vector<QueueFamilyProperties2> getPhysicalDeviceQueueFamilyProperties2(PhysicalDevice pd)  { return getPhysicalDeviceQueueFamilyProperties2_throw(pd); }
inline vector<QueueFamilyProperties2> getPhysicalDeviceQueueFamilyProperties2_throw()  { return getPhysicalDeviceQueueFamilyProperties2_throw(physicalDevice()); }
inline Result getPhysicalDeviceQueueFamilyProperties2_noThrow(vector<QueueFamilyProperties2>& queueFamilyProperties) noexcept  { return getPhysicalDeviceQueueFamilyProperties2_noThrow(physicalDevice(), queueFamilyProperties); }
inline vector<QueueFamilyProperties2> getPhysicalDeviceQueueFamilyProperties2()  { return getPhysicalDeviceQueueFamilyProperties2_throw(physicalDevice()); }

namespace detail {
	struct GetPhysicalDeviceQueueFamilyProperties2_struct {
		uint8_t* pStruct;
		size_t structSize;
	};
	template<typename T>
	GetPhysicalDeviceQueueFamilyProperties2_struct
	getPhysicalDeviceQueueFamilyProperties2_throw_helper(uint32_t n, vector<T>& v, bool useV)
	{
		if(useV) {
			v.alloc(n);
			return GetPhysicalDeviceQueueFamilyProperties2_struct{
				.pStruct = reinterpret_cast<uint8_t*>(v.data()),
				.structSize = sizeof(T),
			};
		}
		else
			return {nullptr, 0};
	}
	template<typename T>
	GetPhysicalDeviceQueueFamilyProperties2_struct
	getPhysicalDeviceQueueFamilyProperties2_noThrow_helper(uint32_t n, vector<T>& v, bool useV) noexcept
	{
		if(useV) {
			if(!v.alloc_noThrow(n))
				return GetPhysicalDeviceQueueFamilyProperties2_struct{
					.pStruct = reinterpret_cast<uint8_t*>(~uint64_t(0)),
				};
			return GetPhysicalDeviceQueueFamilyProperties2_struct{
				.pStruct = reinterpret_cast<uint8_t*>(v.data()),
				.structSize = sizeof(T),
			};
		}
		else
			return {nullptr, 0};
	}
	template<typename T, typename... ArgPack>
	GetPhysicalDeviceQueueFamilyProperties2_struct
	getPhysicalDeviceQueueFamilyProperties2_throw_helper(uint32_t n, vector<T>& v, bool useV, ArgPack&&... argPack)
	{
		GetPhysicalDeviceQueueFamilyProperties2_struct s =
			getPhysicalDeviceQueueFamilyProperties2_helper(n, argPack...);
		if(useV) {
			v.alloc(n);
			if(s.pStruct) {
				uint8_t* p = s.pStruct;
				for(size_t i=0; i<n; i++) {
					v[i].pNext = p;
					p += s.structSize;
				}
			}
			return {reinterpret_cast<uint8_t*>(v.data()), sizeof(T)};
		}
		else
			return s;
	}
	template<typename T, typename... ArgPack>
	GetPhysicalDeviceQueueFamilyProperties2_struct
	getPhysicalDeviceQueueFamilyProperties2_noThrow_helper(uint32_t n, vector<T>& v, bool useV, ArgPack&&... argPack) noexcept
	{
		GetPhysicalDeviceQueueFamilyProperties2_struct s =
			getPhysicalDeviceQueueFamilyProperties2_helper(n, argPack...);
		if(s.pStruct == reinterpret_cast<uint8_t*>(~uint64_t(0)))
			return s;
		if(useV) {
			if(!v.alloc_noThrow(n))
				return GetPhysicalDeviceQueueFamilyProperties2_struct{
					.pStruct = reinterpret_cast<uint8_t*>(~uint64_t(0)),
				};
			if(s.pStruct) {
				uint8_t* p = s.pStruct;
				for(size_t i=0; i<n; i++) {
					v[i].pNext = p;
					p += s.structSize;
				}
			}
			return {reinterpret_cast<uint8_t*>(v.data()), sizeof(T)};
		}
		else
			return s;
	}
}
template<typename... ArgPack>
vector<QueueFamilyProperties2> getPhysicalDeviceQueueFamilyProperties2_throw(PhysicalDevice pd, ArgPack&&... argPack)
{
	// get num queue families
	uint32_t n;
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2(pd.handle(), &n, nullptr);
	if(n == 0)
		return {};

	// alloc and link pNext structures
	detail::GetPhysicalDeviceQueueFamilyProperties2_struct s =
		detail::getPhysicalDeviceQueueFamilyProperties2_throw_helper(n, argPack...);

	// QueueFamilyProperties2 list
	vector<QueueFamilyProperties2> queueFamilyProperties;
	queueFamilyProperties.alloc(n);  // this might throw
	if(s.pStruct) {
		uint8_t* p = s.pStruct;
		for(size_t i=0; i<n; i++) {
			queueFamilyProperties[i].pNext = p;
			p += s.structSize;
		}
	}

	// enumerate physical devices
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2(pd.handle(), &n, queueFamilyProperties.data());
	return queueFamilyProperties;
}
template<typename... ArgPack>
Result getPhysicalDeviceQueueFamilyProperties2_noThrow(PhysicalDevice pd, vector<QueueFamilyProperties2>& queueFamilyProperties, ArgPack&&... argPack) noexcept
{
	// get num queue families
	uint32_t n;
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2(pd.handle(), &n, nullptr);

	// alloc and link pNext structures
	detail::GetPhysicalDeviceQueueFamilyProperties2_struct s =
		detail::getPhysicalDeviceQueueFamilyProperties2_noThrow_helper(n, argPack...);
	if(s.pStruct == reinterpret_cast<uint8_t*>(~uint64_t(0)))
		return Result::eErrorOutOfHostMemory;

	// QueueFamilyProperties2 list
	if(!queueFamilyProperties.alloc_noThrow(n))
		return Result::eErrorOutOfHostMemory;
	if(s.pStruct) {
		uint8_t* p = s.pStruct;
		for(size_t i=0; i<n; i++) {
			queueFamilyProperties[i].pNext = p;
			p += s.structSize;
		}
	}

	// enumerate physical devices
	funcs.vkGetPhysicalDeviceQueueFamilyProperties2(pd.handle(), &n, queueFamilyProperties.data());
	return Result::eSuccess;
}
template<typename... ArgPack>
inline vector<QueueFamilyProperties2> getPhysicalDeviceQueueFamilyProperties2(PhysicalDevice pd, ArgPack&&... argPack)  { return getPhysicalDeviceQueueFamilyProperties2_throw(pd, argPack...); }
template<typename T>
inline vector<QueueFamilyProperties2> getPhysicalDeviceQueueFamilyProperties2_throw(PhysicalDevice pd, vector<T>& t)  { return getPhysicalDeviceQueueFamilyProperties2(pd, t, true); }
template<typename T>
inline Result getPhysicalDeviceQueueFamilyProperties2_noThrow(PhysicalDevice pd, vector<QueueFamilyProperties2>& queueFamilyProperties, vector<T>& t) noexcept  { return getPhysicalDeviceQueueFamilyProperties2_noThrow(pd, queueFamilyProperties, t, true); }

template<typename... ArgPack>
inline vector<QueueFamilyProperties2> getPhysicalDeviceQueueFamilyProperties2_throw(ArgPack&&... argPack)  { return getPhysicalDeviceQueueFamilyProperties2_throw(physicalDevice(), argPack...); }
template<typename... ArgPack>
inline Result getPhysicalDeviceQueueFamilyProperties2_noThrow(vector<QueueFamilyProperties2>& queueFamilyProperties, ArgPack&&... argPack) noexcept  { return getPhysicalDeviceQueueFamilyProperties2_noThrow(physicalDevice(), queueFamilyProperties, argPack...); }
template<typename... ArgPack>
inline vector<QueueFamilyProperties2> getPhysicalDeviceQueueFamilyProperties2(ArgPack&&... argPack)  { return getPhysicalDeviceQueueFamilyProperties2_throw(physicalDevice(), argPack...); }
template<typename T>
inline vector<QueueFamilyProperties2> getPhysicalDeviceQueueFamilyProperties2_throw(vector<T>& t)  { return getPhysicalDeviceQueueFamilyProperties2(physicalDevice(), t, true); }
template<typename T>
inline Result getPhysicalDeviceQueueFamilyProperties2_noThrow(vector<QueueFamilyProperties2>& queueFamilyProperties, vector<T>& t) noexcept  { return getPhysicalDeviceQueueFamilyProperties2_noThrow(physicalDevice(), queueFamilyProperties, t, true); }


inline Queue getDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex) noexcept  { Queue::HandleType h; funcs.vkGetDeviceQueue(detail::_device.handle(), queueFamilyIndex, queueIndex, &h); return h; }

template<typename T> void processResult(Result r, T& handle, const char* functionName)  { if(r > Result::eSuccess) { destroy(handle); handle = nullptr; } if(r != Result::eSuccess) throwResultException(r, functionName); }

inline CommandPool createCommandPool_throw(const CommandPoolCreateInfo& createInfo)  { CommandPool::HandleType h; Result r = funcs.vkCreateCommandPool(detail::_device.handle(), &createInfo, nullptr, &h); processResult(r, h, "vkCreateCommandPool"); return h; }
inline Result createCommandPool_noThrow(const CommandPoolCreateInfo& createInfo, CommandPool& v) noexcept  { return funcs.vkCreateCommandPool(detail::_device.handle(), &createInfo, nullptr, reinterpret_cast<CommandPool::HandleType*>(&v)); }
inline CommandPool createCommandPool(const CommandPoolCreateInfo& createInfo)  { return createCommandPool_throw(createInfo); }
inline UniqueCommandPool createCommandPoolUnique_throw(const CommandPoolCreateInfo& createInfo)  { return UniqueCommandPool(createCommandPool_throw(createInfo)); }
inline Result createCommandPoolUnique_noThrow(const CommandPoolCreateInfo& createInfo, UniqueCommandPool& u) noexcept  { u.reset(); return funcs.vkCreateCommandPool(detail::_device.handle(), &createInfo, nullptr, reinterpret_cast<CommandPool::HandleType*>(&u)); }
inline UniqueCommandPool createCommandPoolUnique(const CommandPoolCreateInfo& createInfo)  { return createCommandPoolUnique_throw(createInfo); }

inline void destroyCommandPool(CommandPool commandPool) noexcept  { funcs.vkDestroyCommandPool(detail::_device.handle(), commandPool.handle(), nullptr); }
inline void destroy(CommandPool commandPool) noexcept  { funcs.vkDestroyCommandPool(detail::_device.handle(), commandPool.handle(), nullptr); }

inline void resetCommandPool_throw(CommandPool commandPool, CommandPoolResetFlags flags)  { Result r = funcs.vkResetCommandPool(detail::_device.handle(), commandPool.handle(), flags); checkForSuccessValue(r, "vkResetCommandPool"); }
inline Result resetCommandPool_noThrow(CommandPool commandPool, CommandPoolResetFlags flags) noexcept  { return funcs.vkResetCommandPool(detail::_device.handle(), commandPool.handle(), flags); }
inline void resetCommandPool(CommandPool commandPool, CommandPoolResetFlags flags)  { resetCommandPool_throw(commandPool, flags); }

inline CommandBuffer allocateCommandBuffer_throw(const CommandBufferAllocateInfo& allocateInfo)  { if(allocateInfo.commandBufferCount != 1) throw OutOfHostMemoryError("vk::allocateCommandBuffer_throw(const CommandBufferAllocateInfo&): CommandBufferAllocateInfo::commandBufferCount must be 1."); CommandBuffer::HandleType h; Result r = funcs.vkAllocateCommandBuffers(detail::_device.handle(), &allocateInfo, &h); if(r > Result::eSuccess) funcs.vkFreeCommandBuffers(detail::_device.handle(), allocateInfo.commandPool.handle(), 1, &h); checkForSuccessValue(r, "vkAllocateCommandBuffers"); return h; }
inline Result allocateCommandBuffer_noThrow(const CommandBufferAllocateInfo& allocateInfo, CommandBuffer& commandBuffer) noexcept  { return funcs.vkAllocateCommandBuffers(detail::_device.handle(), &allocateInfo, reinterpret_cast<CommandBuffer::HandleType*>(&commandBuffer)); }
inline CommandBuffer allocateCommandBuffer(const CommandBufferAllocateInfo& allocateInfo)  { return allocateCommandBuffer_throw(allocateInfo); }
inline vector<CommandBuffer> allocateCommandBuffers_throw(const CommandBufferAllocateInfo& allocateInfo)  { vector<CommandBuffer> l; if(!l.alloc_noThrow(allocateInfo.commandBufferCount)) throw OutOfHostMemoryError("allocateCommandBuffers_throw(): Out of host memory."); Result r = funcs.vkAllocateCommandBuffers(detail::_device.handle(), &allocateInfo, reinterpret_cast<CommandBuffer::HandleType*>(l.data())); if(r > Result::eSuccess) funcs.vkFreeCommandBuffers(detail::_device.handle(), allocateInfo.commandPool.handle(), 1, reinterpret_cast<CommandBuffer::HandleType*>(l.data())); checkForSuccessValue(r, "vkAllocateCommandBuffers"); return l; }
inline Result allocateCommandBuffers_noThrow(const CommandBufferAllocateInfo& allocateInfo, vector<CommandBuffer>& commandBufferList) noexcept  { if(!commandBufferList.alloc_noThrow(allocateInfo.commandBufferCount)) return vk::Result::eErrorOutOfHostMemory; return funcs.vkAllocateCommandBuffers(detail::_device.handle(), &allocateInfo, reinterpret_cast<CommandBuffer::HandleType*>(commandBufferList.data())); }
inline vector<CommandBuffer> allocateCommandBuffers(const CommandBufferAllocateInfo& allocateInfo)  { return allocateCommandBuffers_throw(allocateInfo); }

inline void freeCommandBuffer(CommandPool commandPool, const CommandBuffer commandBuffer) noexcept  { funcs.vkFreeCommandBuffers(detail::_device.handle(), commandPool.handle(), 1, reinterpret_cast<const CommandBuffer::HandleType*>(&commandBuffer)); }
inline void free(CommandPool commandPool, const CommandBuffer commandBuffer) noexcept  { funcs.vkFreeCommandBuffers(detail::_device.handle(), commandPool.handle(), 1, reinterpret_cast<const CommandBuffer::HandleType*>(&commandBuffer)); }

inline void beginCommandBuffer_throw(CommandBuffer commandBuffer, const CommandBufferBeginInfo& beginInfo)  { Result r = funcs.vkBeginCommandBuffer(commandBuffer.handle(), &beginInfo); checkForSuccessValue(r, "vkBeginCommandBuffer"); }
inline Result beginCommandBuffer_noThrow(CommandBuffer commandBuffer, const CommandBufferBeginInfo& beginInfo) noexcept  { return funcs.vkBeginCommandBuffer(commandBuffer.handle(), &beginInfo); }
inline void beginCommandBuffer(CommandBuffer commandBuffer, const CommandBufferBeginInfo& beginInfo)  { beginCommandBuffer_throw(commandBuffer, beginInfo); }

inline void endCommandBuffer(CommandBuffer commandBuffer) noexcept  { funcs.vkEndCommandBuffer(commandBuffer.handle()); }

inline void resetCommandBuffer_throw(CommandBuffer commandBuffer, CommandBufferResetFlags flags)  { Result r = funcs.vkResetCommandBuffer(commandBuffer.handle(), flags); checkForSuccessValue(r, "vkResetCommandBuffer"); }
inline Result resetCommandBuffer_noThrow(CommandBuffer commandBuffer, CommandBufferResetFlags flags) noexcept  { return funcs.vkResetCommandBuffer(commandBuffer.handle(), flags); }
inline void resetCommandBuffer(CommandBuffer commandBuffer, CommandBufferResetFlags flags)  { resetCommandBuffer_throw(commandBuffer, flags); }

inline QueryPool createQueryPool_throw(const QueryPoolCreateInfo& createInfo)  { QueryPool::HandleType h; Result r = funcs.vkCreateQueryPool(detail::_device.handle(), &createInfo, nullptr, &h); processResult(r, h, "vkCreateQueryPool"); return h; }
inline Result createQueryPool_noThrow(const QueryPoolCreateInfo& createInfo, QueryPool& v) noexcept  { return funcs.vkCreateQueryPool(detail::_device.handle(), &createInfo, nullptr, reinterpret_cast<QueryPool::HandleType*>(&v)); }
inline QueryPool createQueryPool(const QueryPoolCreateInfo& createInfo)  { return createQueryPool_throw(createInfo); }
inline UniqueQueryPool createQueryPoolUnique_throw(const QueryPoolCreateInfo& createInfo)  { return UniqueQueryPool(createQueryPool_throw(createInfo)); }
inline Result createQueryPoolUnique_noThrow(const QueryPoolCreateInfo& createInfo, UniqueQueryPool& u) noexcept  { u.reset(); return funcs.vkCreateQueryPool(detail::_device.handle(), &createInfo, nullptr, reinterpret_cast<QueryPool::HandleType*>(&u)); }
inline UniqueQueryPool createQueryPoolUnique(const QueryPoolCreateInfo& createInfo)  { return createQueryPoolUnique_throw(createInfo); }

inline void destroyQueryPool(QueryPool queryPool) noexcept  { funcs.vkDestroyQueryPool(detail::_device.handle(), queryPool.handle(), nullptr); }
inline void destroy(QueryPool queryPool) noexcept  { funcs.vkDestroyQueryPool(detail::_device.handle(), queryPool.handle(), nullptr); }

inline void getQueryPoolResults_throw(QueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, DeviceSize stride, QueryResultFlags flags)  { Result r = funcs.vkGetQueryPoolResults(detail::_device.handle(), queryPool.handle(), firstQuery, queryCount, dataSize, pData, stride, flags); checkForSuccessValue(r, "vkGetQueryPoolResults"); }
inline Result getQueryPoolResults_noThrow(QueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, DeviceSize stride, QueryResultFlags flags) noexcept  { return funcs.vkGetQueryPoolResults(detail::_device.handle(), queryPool.handle(), firstQuery, queryCount, dataSize, pData, stride, flags); }
inline void getQueryPoolResults(QueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, DeviceSize stride, QueryResultFlags flags)  { return getQueryPoolResults_throw(queryPool, firstQuery, queryCount, dataSize, pData, stride, flags); }

inline Fence createFence_throw(const FenceCreateInfo& createInfo)  { Fence::HandleType h; Result r = funcs.vkCreateFence(detail::_device.handle(), &createInfo, nullptr, &h); processResult(r, h, "vkCreateFence"); return h; }
inline Result createFence_noThrow(const FenceCreateInfo& createInfo, Fence& fence) noexcept  { return funcs.vkCreateFence(detail::_device.handle(), &createInfo, nullptr, reinterpret_cast<Fence::HandleType*>(&fence)); }
inline Fence createFence(const FenceCreateInfo& createInfo)  { return createFence_throw(createInfo); }
inline UniqueFence createFenceUnique_throw(const FenceCreateInfo& createInfo)  { return UniqueFence(createFence_throw(createInfo)); }
inline Result createFence_noThrow(const FenceCreateInfo& createInfo, UniqueFence& fence) noexcept  { fence.reset(); return funcs.vkCreateFence(detail::_device.handle(), &createInfo, nullptr, reinterpret_cast<Fence::HandleType*>(&fence)); }
inline UniqueFence createFenceUnique(const FenceCreateInfo& createInfo)  { return createFenceUnique_throw(createInfo); }

inline void destroyFence(Fence fence) noexcept  { funcs.vkDestroyFence(detail::_device.handle(), fence.handle(), nullptr); }
inline void destroy(Fence fence) noexcept  { funcs.vkDestroyFence(detail::_device.handle(), fence.handle(), nullptr); }

inline void resetFences_throw(uint32_t fenceCount, const Fence* pFences)  { Result r = funcs.vkResetFences(detail::_device.handle(), fenceCount, reinterpret_cast<const Fence::HandleType*>(pFences)); checkForSuccessValue(r, "vkResetFences"); }
inline Result resetFences_noThrow(uint32_t fenceCount, const Fence* pFences) noexcept  { return funcs.vkResetFences(detail::_device.handle(), fenceCount, reinterpret_cast<const Fence::HandleType*>(pFences)); }
inline void resetFences(uint32_t fenceCount, const Fence* pFences)  { resetFences_throw(fenceCount, pFences); }
inline void resetFences_throw(uint32_t fenceCount, const UniqueFence* pFences)  { resetFences_throw(fenceCount, reinterpret_cast<const Fence*>(pFences)); }
inline Result resetFences_noThrow(uint32_t fenceCount, const UniqueFence* pFences) noexcept  { return resetFences_noThrow(fenceCount, reinterpret_cast<const Fence*>(pFences)); }
inline void resetFences(uint32_t fenceCount, const UniqueFence* pFences)  { resetFences_throw(fenceCount, reinterpret_cast<const Fence*>(pFences)); }
inline void resetFence_throw(Fence fence)  { resetFences_throw(1, &fence); }
inline Result resetFence_noThrow(Fence fence) noexcept  { return resetFences_noThrow(1, &fence); }
inline void resetFence(Fence fence)  { resetFences(1, &fence); }

inline void getFenceStatus_throw(Fence fence)  { Result r = funcs.vkGetFenceStatus(detail::_device.handle(), fence.handle()); checkForSuccessValue(r, "vkGetFenceStatus"); }
inline Result getFenceStatus_noThrow(Fence fence) noexcept  { return funcs.vkGetFenceStatus(detail::_device.handle(), fence.handle()); }
inline void getFenceStatus(Fence fence)  { getFenceStatus_throw(fence); }

inline void waitForFences_throw(uint32_t fenceCount, const Fence* pFences, Bool32 waitAll, uint64_t timeout)  { Result r = funcs.vkWaitForFences(detail::_device.handle(), fenceCount, reinterpret_cast<const Fence::HandleType*>(pFences), waitAll, timeout); checkForSuccessValue(r, "vkWaitForFences"); }
inline Result waitForFences_noThrow(uint32_t fenceCount, const Fence* pFences, Bool32 waitAll, uint64_t timeout) noexcept  { return funcs.vkWaitForFences(detail::_device.handle(), fenceCount, reinterpret_cast<const Fence::HandleType*>(pFences), waitAll, timeout); }
inline void waitForFences(uint32_t fenceCount, const Fence* pFences, Bool32 waitAll, uint64_t timeout)  { waitForFences_throw(fenceCount, pFences, waitAll, timeout); }
inline void waitForFences_throw(uint32_t fenceCount, const UniqueFence* pFences, Bool32 waitAll, uint64_t timeout)  { Result r = funcs.vkWaitForFences(detail::_device.handle(), fenceCount, reinterpret_cast<const Fence::HandleType*>(pFences), waitAll, timeout); checkForSuccessValue(r, "vkWaitForFences"); }
inline Result waitForFences_noThrow(uint32_t fenceCount, const UniqueFence* pFences, Bool32 waitAll, uint64_t timeout) noexcept  { return funcs.vkWaitForFences(detail::_device.handle(), fenceCount, reinterpret_cast<const Fence::HandleType*>(pFences), waitAll, timeout); }
inline void waitForFences(uint32_t fenceCount, const UniqueFence* pFences, Bool32 waitAll, uint64_t timeout)  { waitForFences_throw(fenceCount, reinterpret_cast<const Fence*>(pFences), waitAll, timeout); }
inline void waitForFence_throw(const Fence fence, uint64_t timeout)  { waitForFences_throw(1, &fence, vk::False, timeout); }
inline Result waitForFence_noThrow(const Fence fence, uint64_t timeout) noexcept  { return waitForFences_noThrow(1, &fence, vk::False, timeout); }
inline void waitForFence(const Fence fence, uint64_t timeout)  { waitForFences_throw(1, &fence, vk::False, timeout); }

inline void queueSubmit_throw(Queue queue, uint32_t submitCount, const SubmitInfo* pSubmits, Fence fence)  { Result r = funcs.vkQueueSubmit(queue.handle(), submitCount, pSubmits, fence.handle()); checkForSuccessValue(r, "vkQueueSubmit"); }
inline Result queueSubmit_noThrow(Queue queue, uint32_t submitCount, const SubmitInfo* pSubmits, Fence fence) noexcept  { return funcs.vkQueueSubmit(queue.handle(), submitCount, pSubmits, fence.handle()); }
inline void queueSubmit(Queue queue, uint32_t submitCount, const SubmitInfo* pSubmits, Fence fence)  { queueSubmit_throw(queue, submitCount, pSubmits, fence); }
inline void queueSubmit_throw(Queue queue, const SubmitInfo& submit, Fence fence)  { queueSubmit_throw(queue, 1, &submit, fence); }
inline Result queueSubmit_noThrow(Queue queue, const SubmitInfo& submit, Fence fence) noexcept  { return queueSubmit_noThrow(queue, 1, &submit, fence); }
inline void queueSubmit(Queue queue, const SubmitInfo& submit, Fence fence)  { queueSubmit_throw(queue, 1, &submit, fence); }

inline void queueWaitIdle_throw(Queue queue)  { Result r = funcs.vkQueueWaitIdle(queue.handle()); checkForSuccessValue(r, "vkQueueWaitIdle"); }
inline Result queueWaitIdle_noThrow(Queue queue) noexcept { return funcs.vkQueueWaitIdle(queue.handle()); }
inline void queueWaitIdle(Queue queue)  { queueWaitIdle(queue); }

inline void deviceWaitIdle_throw(Device device)  { Result r = funcs.vkDeviceWaitIdle(device.handle()); checkForSuccessValue(r, "vkDeviceWaitIdle"); }
inline Result deviceWaitIdle_noThrow(Device device) noexcept { return funcs.vkDeviceWaitIdle(device.handle()); }
inline void deviceWaitIdle(Device device)  { deviceWaitIdle(device); }
inline void deviceWaitIdle_throw()  { deviceWaitIdle_throw(device()); }
inline Result deviceWaitIdle_noThrow() noexcept { return deviceWaitIdle_noThrow(device()); }
inline void deviceWaitIdle()  { deviceWaitIdle(device()); }

inline ShaderModule createShaderModule_throw(const ShaderModuleCreateInfo& createInfo)  { ShaderModule::HandleType h; Result r = funcs.vkCreateShaderModule(detail::_device.handle(), &createInfo, nullptr, &h); processResult(r, h, "vkCreateShaderModule"); return h; }
inline Result createShaderModule_noThrow(const ShaderModuleCreateInfo& createInfo, ShaderModule& shaderModule) noexcept  { return funcs.vkCreateShaderModule(detail::_device.handle(), &createInfo, nullptr, reinterpret_cast<ShaderModule::HandleType*>(&shaderModule)); }
inline ShaderModule createShaderModule(const ShaderModuleCreateInfo& createInfo)  { return createShaderModule_throw(createInfo); }
inline UniqueShaderModule createShaderModuleUnique_throw(const ShaderModuleCreateInfo& createInfo)  { return UniqueShaderModule(createShaderModule_throw(createInfo)); }
inline Result createShaderModuleUnique_noThrow(const ShaderModuleCreateInfo& createInfo, UniqueShaderModule& shaderModule) noexcept  { shaderModule.reset(); return funcs.vkCreateShaderModule(detail::_device.handle(), &createInfo, nullptr, reinterpret_cast<ShaderModule::HandleType*>(&shaderModule)); }
inline UniqueShaderModule createShaderModuleUnique(const ShaderModuleCreateInfo& createInfo)  { return createShaderModuleUnique_throw(createInfo); }

inline void destroyShaderModule(ShaderModule shaderModule) noexcept  { funcs.vkDestroyShaderModule(detail::_device.handle(), shaderModule.handle(), nullptr); }
inline void destroy(ShaderModule shaderModule) noexcept  { funcs.vkDestroyShaderModule(detail::_device.handle(), shaderModule.handle(), nullptr); }

inline PipelineLayout createPipelineLayout_throw(const vk::PipelineLayoutCreateInfo& createInfo)  { PipelineLayout::HandleType h; Result r = funcs.vkCreatePipelineLayout(detail::_device.handle(), &createInfo, nullptr, &h); processResult(r, h, "vkCreatePipelineLayout"); return h; }
inline Result createPipelineLayout_noThrow(const vk::PipelineLayoutCreateInfo& createInfo, PipelineLayout& pipelineLayout) noexcept  { return funcs.vkCreatePipelineLayout(detail::_device.handle(), &createInfo, nullptr, reinterpret_cast<PipelineLayout::HandleType*>(&pipelineLayout)); }
inline PipelineLayout createPipelineLayout(const vk::PipelineLayoutCreateInfo& createInfo)  { return createPipelineLayout_throw(createInfo); }
inline UniquePipelineLayout createPipelineLayoutUnique_throw(const vk::PipelineLayoutCreateInfo& createInfo)  { return UniquePipelineLayout(createPipelineLayout_throw(createInfo)); }
inline Result createPipelineLayoutUnique_noThrow(const vk::PipelineLayoutCreateInfo& createInfo, UniquePipelineLayout& pipelineLayout) noexcept  { pipelineLayout.reset(); return funcs.vkCreatePipelineLayout(detail::_device.handle(), &createInfo, nullptr, reinterpret_cast<PipelineLayout::HandleType*>(&pipelineLayout)); }
inline UniquePipelineLayout createPipelineLayoutUnique(const vk::PipelineLayoutCreateInfo& createInfo)  { return createPipelineLayoutUnique_throw(createInfo); }

inline void destroyPipelineLayout(PipelineLayout pipelineLayout) noexcept  { funcs.vkDestroyPipelineLayout(detail::_device.handle(), pipelineLayout.handle(), nullptr); }
inline void destroy(PipelineLayout pipelineLayout) noexcept  { funcs.vkDestroyPipelineLayout(detail::_device.handle(), pipelineLayout.handle(), nullptr); }

inline Pipeline createComputePipeline_throw(PipelineCache pipelineCache, const ComputePipelineCreateInfo& createInfo)  { Pipeline::HandleType h; Result r = funcs.vkCreateComputePipelines(detail::_device.handle(), pipelineCache.handle(), 1, &createInfo, nullptr, &h); processResult(r, h, "vkCreateComputePipelines"); return h; }
inline Result createComputePipeline_noThrow(PipelineCache pipelineCache, const ComputePipelineCreateInfo& createInfo, Pipeline& pipeline) noexcept  { return funcs.vkCreateComputePipelines(detail::_device.handle(), pipelineCache.handle(), 1, &createInfo, nullptr, reinterpret_cast<Pipeline::HandleType*>(&pipeline)); }
inline Pipeline createComputePipeline(PipelineCache pipelineCache, const ComputePipelineCreateInfo& createInfo)  { return createComputePipeline_throw(pipelineCache, createInfo); }
inline UniquePipeline createComputePipelineUnique_throw(PipelineCache pipelineCache, const ComputePipelineCreateInfo& createInfo)  { return UniquePipeline(createComputePipeline_throw(pipelineCache, createInfo)); }
inline Result createComputePipelineUnique_noThrow(PipelineCache pipelineCache, const ComputePipelineCreateInfo& createInfo, UniquePipeline& pipeline) noexcept  { pipeline.reset(); return funcs.vkCreateComputePipelines(detail::_device.handle(), pipelineCache.handle(), 1, &createInfo, nullptr, reinterpret_cast<Pipeline::HandleType*>(&pipeline)); }
inline UniquePipeline createComputePipelineUnique(PipelineCache pipelineCache, const ComputePipelineCreateInfo& createInfo)  { return createComputePipelineUnique_throw(pipelineCache, createInfo); }

inline vector<Pipeline> createComputePipelines_throw(PipelineCache pipelineCache, uint32_t createInfoCount, const ComputePipelineCreateInfo* pCreateInfos)  { vector<Pipeline> l; if(!l.alloc_noThrow(createInfoCount)) throw OutOfHostMemoryError("createComputePipelines_throw(): Out of host memory."); Result r = funcs.vkCreateComputePipelines(detail::_device.handle(), pipelineCache.handle(), createInfoCount, pCreateInfos, nullptr, reinterpret_cast<Pipeline::HandleType*>(l.data())); if(r != Result::eSuccess) { for(size_t i=0; i<createInfoCount; i++) funcs.vkDestroyPipeline(detail::_device.handle(), l[i].handle(), nullptr); throwResultException(r, "vkCreateComputePipelines"); } return l; }
inline Result createComputePipelines_noThrow(PipelineCache pipelineCache, uint32_t createInfoCount, const ComputePipelineCreateInfo* pCreateInfos, Pipeline* pPipelines) noexcept  { return funcs.vkCreateComputePipelines(detail::_device.handle(), pipelineCache.handle(), createInfoCount, pCreateInfos, nullptr, reinterpret_cast<Pipeline::HandleType*>(pPipelines)); }
inline vector<Pipeline> createComputePipelines(PipelineCache pipelineCache, uint32_t createInfoCount, const ComputePipelineCreateInfo* pCreateInfos)  { return createComputePipelines_throw(pipelineCache, createInfoCount, pCreateInfos); }
inline vector<UniquePipeline> createComputePipelinesUnique_throw(PipelineCache pipelineCache, uint32_t createInfoCount, const ComputePipelineCreateInfo* pCreateInfos)  { vector<UniquePipeline> l; if(!l.alloc_noThrow(createInfoCount)) throw OutOfHostMemoryError("createComputePipelinesUnique_throw(): Out of host memory."); Result r = funcs.vkCreateComputePipelines(detail::_device.handle(), pipelineCache.handle(), createInfoCount, pCreateInfos, nullptr, reinterpret_cast<Pipeline::HandleType*>(l.data())); if(r != Result::eSuccess) throwResultException(r, "vkCreateComputePipelines"); return l; }
inline Result createComputePipelinesUnique_noThrow(PipelineCache pipelineCache, uint32_t createInfoCount, const ComputePipelineCreateInfo* pCreateInfos, UniquePipeline* pPipelines) noexcept  { for(uint32_t i=0; i<createInfoCount; i++) pPipelines[i].reset(); return funcs.vkCreateComputePipelines(detail::_device.handle(), pipelineCache.handle(), createInfoCount, pCreateInfos, nullptr, reinterpret_cast<Pipeline::HandleType*>(pPipelines)); }
inline vector<UniquePipeline> createComputePipelinesUnique(PipelineCache pipelineCache, uint32_t createInfoCount, const ComputePipelineCreateInfo* pCreateInfos)  { return createComputePipelinesUnique_throw(pipelineCache, createInfoCount, pCreateInfos); }

inline void destroyPipeline(Pipeline pipeline) noexcept  { funcs.vkDestroyPipeline(detail::_device.handle(), pipeline.handle(), nullptr); }
inline void destroy(Pipeline pipeline) noexcept  { funcs.vkDestroyPipeline(detail::_device.handle(), pipeline.handle(), nullptr); }

inline void cmdPushConstants(CommandBuffer commandBuffer, PipelineLayout layout, ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) noexcept  { funcs.vkCmdPushConstants(commandBuffer.handle(), layout, stageFlags, offset, size, pValues); }
inline void cmdBeginRenderPass(CommandBuffer commandBuffer, const RenderPassBeginInfo& pRenderPassBegin, SubpassContents contents) noexcept  { funcs.vkCmdBeginRenderPass(commandBuffer.handle(), &pRenderPassBegin, contents); }
inline void cmdNextSubpass(CommandBuffer commandBuffer, SubpassContents contents) noexcept  { funcs.vkCmdNextSubpass(commandBuffer.handle(), contents); }
inline void cmdEndRenderPass(CommandBuffer commandBuffer) noexcept  { funcs.vkCmdEndRenderPass(commandBuffer.handle()); }
inline void cmdExecuteCommands(CommandBuffer commandBuffer, uint32_t commandBufferCount, const CommandBuffer* pCommandBufferHandles) noexcept  { funcs.vkCmdExecuteCommands(commandBuffer.handle(), commandBufferCount, reinterpret_cast<const CommandBuffer::HandleType*>(pCommandBufferHandles)); }
inline void cmdBindPipeline(CommandBuffer commandBuffer, PipelineBindPoint pipelineBindPoint, Pipeline pipeline) noexcept  { funcs.vkCmdBindPipeline(commandBuffer.handle(), pipelineBindPoint, pipeline.handle()); }
inline void cmdSetViewport(CommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const Viewport* pViewports) noexcept  { funcs.vkCmdSetViewport(commandBuffer.handle(), firstViewport, viewportCount, pViewports); }
inline void cmdSetScissor(CommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const Rect2D* pScissors) noexcept  { funcs.vkCmdSetScissor(commandBuffer.handle(), firstScissor, scissorCount, pScissors); }
inline void cmdSetLineWidth(CommandBuffer commandBuffer, float lineWidth) noexcept  { funcs.vkCmdSetLineWidth(commandBuffer.handle(), lineWidth); }
inline void cmdSetLineStippleEXT(CommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) noexcept  { funcs.vkCmdSetLineStippleEXT(commandBuffer.handle(), lineStippleFactor, lineStipplePattern); }
inline void cmdSetDepthBias(CommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) noexcept  { funcs.vkCmdSetDepthBias(commandBuffer.handle(), depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor); }
inline void cmdSetBlendConstants(CommandBuffer commandBuffer, const float blendConstants[4]) noexcept  { funcs.vkCmdSetBlendConstants(commandBuffer.handle(), blendConstants); }
inline void cmdSetDepthBounds(CommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) noexcept  { funcs.vkCmdSetDepthBounds(commandBuffer.handle(), minDepthBounds, maxDepthBounds); }
inline void cmdSetStencilCompareMask(CommandBuffer commandBuffer, StencilFaceFlags faceMask, uint32_t compareMask) noexcept  { funcs.vkCmdSetStencilCompareMask(commandBuffer.handle(), faceMask, compareMask); }
inline void cmdSetStencilWriteMask(CommandBuffer commandBuffer, StencilFaceFlags faceMask, uint32_t writeMask) noexcept  { funcs.vkCmdSetStencilWriteMask(commandBuffer.handle(), faceMask, writeMask); }
inline void cmdSetStencilReference(CommandBuffer commandBuffer, StencilFaceFlags faceMask, uint32_t reference) noexcept  { funcs.vkCmdSetStencilReference(commandBuffer.handle(), faceMask, reference); }
inline void cmdBindDescriptorSets(CommandBuffer commandBuffer, PipelineBindPoint pipelineBindPoint, PipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const DescriptorSet::HandleType* pDescriptorSetHandles, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) noexcept  { funcs.vkCmdBindDescriptorSets(commandBuffer.handle(), pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSetHandles, dynamicOffsetCount, pDynamicOffsets); }
inline void cmdBindIndexBuffer(CommandBuffer commandBuffer, Buffer bufferHandle, DeviceSize offset, IndexType indexType) noexcept  { funcs.vkCmdBindIndexBuffer(commandBuffer.handle(), bufferHandle.handle(), offset, indexType); }
inline void cmdBindVertexBuffers(CommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const Buffer* pBufferHandles, const DeviceSize* pOffsets) noexcept  { funcs.vkCmdBindVertexBuffers(commandBuffer.handle(), firstBinding, bindingCount, reinterpret_cast<const Buffer::HandleType*>(pBufferHandles), pOffsets); }
inline void cmdDraw(CommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) noexcept  { funcs.vkCmdDraw(commandBuffer.handle(), vertexCount, instanceCount, firstVertex, firstInstance); }
inline void cmdDrawIndexed(CommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) noexcept  { funcs.vkCmdDrawIndexed(commandBuffer.handle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance); }
inline void cmdDrawIndirect(CommandBuffer commandBuffer, Buffer bufferHandle, DeviceSize offset, uint32_t drawCount, uint32_t stride) noexcept  { funcs.vkCmdDrawIndirect(commandBuffer.handle(), bufferHandle.handle(), offset, drawCount, stride); }
inline void cmdDrawIndexedIndirect(CommandBuffer commandBuffer, Buffer bufferHandle, DeviceSize offset, uint32_t drawCount, uint32_t stride) noexcept  { funcs.vkCmdDrawIndexedIndirect(commandBuffer.handle(), bufferHandle.handle(), offset, drawCount, stride); }
inline void cmdDispatch(CommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) noexcept  { funcs.vkCmdDispatch(commandBuffer.handle(), groupCountX, groupCountY, groupCountZ); }
inline void cmdDispatchIndirect(CommandBuffer commandBuffer, Buffer bufferHandle, DeviceSize offset) noexcept  { funcs.vkCmdDispatchIndirect(commandBuffer.handle(), bufferHandle.handle(), offset); }
inline void cmdDispatchBase(CommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) noexcept  { funcs.vkCmdDispatchBase(commandBuffer.handle(), baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ); }
inline void cmdCopyBuffer(CommandBuffer commandBuffer, Buffer srcBufferHandle, Buffer dstBufferHandle, uint32_t regionCount, const BufferCopy* pRegions) noexcept  { funcs.vkCmdCopyBuffer(commandBuffer.handle(), srcBufferHandle.handle(), dstBufferHandle.handle(), regionCount, pRegions); }
inline void cmdCopyImage(CommandBuffer commandBuffer, Image srcImageHandle, ImageLayout srcImageLayout, Image dstImageHandle, ImageLayout dstImageLayout, uint32_t regionCount, const ImageCopy* pRegions) noexcept  { funcs.vkCmdCopyImage(commandBuffer.handle(), srcImageHandle.handle(), srcImageLayout, dstImageHandle.handle(), dstImageLayout, regionCount, pRegions); }
inline void cmdBlitImage(CommandBuffer commandBuffer, Image srcImageHandle, ImageLayout srcImageLayout, Image dstImageHandle, ImageLayout dstImageLayout, uint32_t regionCount, const ImageBlit* pRegions, Filter filter) noexcept  { funcs.vkCmdBlitImage(commandBuffer.handle(), srcImageHandle.handle(), srcImageLayout, dstImageHandle.handle(), dstImageLayout, regionCount, pRegions, filter); }
inline void cmdCopyBufferToImage(CommandBuffer commandBuffer, Buffer srcBufferHandle, Image dstImageHandle, ImageLayout dstImageLayout, uint32_t regionCount, const BufferImageCopy* pRegions) noexcept  { funcs.vkCmdCopyBufferToImage(commandBuffer.handle(), srcBufferHandle.handle(), dstImageHandle.handle(), dstImageLayout, regionCount, pRegions); }
inline void cmdCopyImageToBuffer(CommandBuffer commandBuffer, Image srcImageHandle, ImageLayout srcImageLayout, Buffer dstBufferHandle, uint32_t regionCount, const BufferImageCopy* pRegions) noexcept  { funcs.vkCmdCopyImageToBuffer(commandBuffer.handle(), srcImageHandle.handle(), srcImageLayout, dstBufferHandle.handle(), regionCount, pRegions); }
inline void cmdUpdateBuffer(CommandBuffer commandBuffer, Buffer dstBufferHandle, DeviceSize dstOffset, DeviceSize dataSize, const void* pData) noexcept  { funcs.vkCmdUpdateBuffer(commandBuffer.handle(), dstBufferHandle.handle(), dstOffset, dataSize, pData); }
inline void cmdFillBuffer(CommandBuffer commandBuffer, Buffer dstBufferHandle, DeviceSize dstOffset, DeviceSize size, uint32_t data) noexcept  { funcs.vkCmdFillBuffer(commandBuffer.handle(), dstBufferHandle.handle(), dstOffset, size, data); }
inline void cmdClearColorImage(CommandBuffer commandBuffer, Image imageHandle, ImageLayout imageLayout, const ClearColorValue* pColor, uint32_t rangeCount, const ImageSubresourceRange* pRanges) noexcept  { funcs.vkCmdClearColorImage(commandBuffer.handle(), imageHandle.handle(), imageLayout, pColor, rangeCount, pRanges); }
inline void cmdClearDepthStencilImage(CommandBuffer commandBuffer, Image imageHandle, ImageLayout imageLayout, const ClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const ImageSubresourceRange* pRanges) noexcept  { funcs.vkCmdClearDepthStencilImage(commandBuffer.handle(), imageHandle.handle(), imageLayout, pDepthStencil, rangeCount, pRanges); }
inline void cmdClearAttachments(CommandBuffer commandBuffer, uint32_t attachmentCount, const ClearAttachment* pAttachments, uint32_t rectCount, const ClearRect* pRects) noexcept  { funcs.vkCmdClearAttachments(commandBuffer.handle(), attachmentCount, pAttachments, rectCount, pRects); }
inline void cmdResolveImage(CommandBuffer commandBuffer, Image srcImageHandle, ImageLayout srcImageLayout, Image dstImageHandle, ImageLayout dstImageLayout, uint32_t regionCount, const ImageResolve* pRegions) noexcept  { funcs.vkCmdResolveImage(commandBuffer.handle(), srcImageHandle.handle(), srcImageLayout, dstImageHandle.handle(), dstImageLayout, regionCount, pRegions); }
inline void cmdSetEvent(CommandBuffer commandBuffer, Event eventHandle, PipelineStageFlags stageMask) noexcept  { funcs.vkCmdSetEvent(commandBuffer.handle(), eventHandle.handle(), stageMask); }
inline void cmdResetEvent(CommandBuffer commandBuffer, Event eventHandle, PipelineStageFlags stageMask) noexcept  { funcs.vkCmdResetEvent(commandBuffer.handle(), eventHandle.handle(), stageMask); }
inline void cmdWaitEvents(CommandBuffer commandBuffer, uint32_t eventCount, const Event* pEventHandles, PipelineStageFlags srcStageMask, PipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const MemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const BufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const ImageMemoryBarrier* pImageMemoryBarriers) noexcept  { funcs.vkCmdWaitEvents(commandBuffer.handle(), eventCount, reinterpret_cast<const Event::HandleType*>(pEventHandles), srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers); }
inline void cmdPipelineBarrier(CommandBuffer commandBuffer, PipelineStageFlags srcStageMask, PipelineStageFlags dstStageMask, DependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const MemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const BufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const ImageMemoryBarrier* pImageMemoryBarriers) noexcept  { funcs.vkCmdPipelineBarrier(commandBuffer.handle(), srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers); }
inline void cmdBeginQuery(CommandBuffer commandBuffer, QueryPool queryPoolHandle, uint32_t query, QueryControlFlags flags) noexcept  { funcs.vkCmdBeginQuery(commandBuffer.handle(), queryPoolHandle.handle(), query, flags); }
inline void cmdEndQuery(CommandBuffer commandBuffer, QueryPool queryPoolHandle, uint32_t query) noexcept  { funcs.vkCmdEndQuery(commandBuffer.handle(), queryPoolHandle.handle(), query); }
inline void cmdResetQueryPool(CommandBuffer commandBuffer, QueryPool queryPoolHandle, uint32_t firstQuery, uint32_t queryCount) noexcept  { funcs.vkCmdResetQueryPool(commandBuffer.handle(), queryPoolHandle.handle(), firstQuery, queryCount); }
inline void cmdWriteTimestamp(CommandBuffer commandBuffer, PipelineStageFlagBits pipelineStage, QueryPool queryPoolHandle, uint32_t query) noexcept  { funcs.vkCmdWriteTimestamp(commandBuffer.handle(), pipelineStage, queryPoolHandle.handle(), query); }
inline void cmdCopyQueryPoolResults(CommandBuffer commandBuffer, QueryPool queryPoolHandle, uint32_t firstQuery, uint32_t queryCount, Buffer dstBufferHandle, DeviceSize dstOffset, DeviceSize stride, QueryResultFlags flags) noexcept  { funcs.vkCmdCopyQueryPoolResults(commandBuffer.handle(), queryPoolHandle.handle(), firstQuery, queryCount, dstBufferHandle.handle(), dstOffset, stride, flags); }

}
