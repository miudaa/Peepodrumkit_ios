#ifdef __APPLE__

#include "audio_common.h"

// Conflict between MacTypes.h and core_types.h
#define Rect AppleRect
#define Component AppleComponent
#include <AudioToolbox/AudioToolbox.h>
#undef Rect
#undef Component

#include "audio_backend.h"
#include <atomic>
#include <vector>

namespace Audio
{
	struct CoreAudioBackend::Impl
	{
		AudioUnit audioUnit = nullptr;
		BackendRenderCallback renderCallback = nullptr;
		std::atomic<b8> isOpenRunning{ false };
		std::vector<i16> renderBuffer;
		u32 channelCount = 0;

		static OSStatus RenderCallback(void *inRefCon,
									   AudioUnitRenderActionFlags *ioActionFlags,
									   const AudioTimeStamp *inTimeStamp,
									   UInt32 inBusNumber,
									   UInt32 inNumberFrames,
									   AudioBufferList *ioData)
		{
			auto impl = static_cast<CoreAudioBackend::Impl *>(inRefCon);
			if (!impl->isOpenRunning.load() || !impl->renderCallback)
			{
				for (UInt32 i = 0; i < ioData->mNumberBuffers; ++i)
					memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);
				return noErr;
			}

			// Use the pre-allocated buffer. 
			// Note: we assume renderBuffer is large enough based on OpenStartStream pre-allocation.
			impl->renderCallback(impl->renderBuffer.data(), inNumberFrames, impl->channelCount);

			if (ioData->mNumberBuffers > 0)
			{
				memcpy(ioData->mBuffers[0].mData, impl->renderBuffer.data(), inNumberFrames * impl->channelCount * sizeof(i16));
			}

			return noErr;
		}
	};

	CoreAudioBackend::CoreAudioBackend() : impl(std::make_unique<Impl>()) {}
	CoreAudioBackend::~CoreAudioBackend() { StopCloseStream(); }

	b8 CoreAudioBackend::OpenStartStream(const BackendStreamParam &param, BackendRenderCallback callback)
	{
		if (impl->isOpenRunning.load())
			return false;

		impl->renderCallback = callback;
		impl->channelCount = param.ChannelCount;

		AudioComponentDescription desc;
		desc.componentType = kAudioUnitType_Output;
#if TARGET_OS_IPHONE
		desc.componentSubType = kAudioUnitSubType_RemoteIO;
#else
		desc.componentSubType = kAudioUnitSubType_DefaultOutput;
#endif
		desc.componentManufacturer = kAudioUnitManufacturer_Apple;
		desc.componentFlags = 0;
		desc.componentFlagsMask = 0;

		AudioComponent comp = AudioComponentFindNext(NULL, &desc);
		if (!comp)
		{
			printf("CoreAudio: Failed to find default output component.\n");
			return false;
		}

		OSStatus status = AudioComponentInstanceNew(comp, &impl->audioUnit);
		if (status != noErr)
		{
			printf("CoreAudio: AudioComponentInstanceNew failed with error %d\n", (int)status);
			return false;
		}

		// Set format: S16 interleaved
		AudioStreamBasicDescription format = { 0 };
		format.mSampleRate = param.SampleRate;
		format.mFormatID = kAudioFormatLinearPCM;
		format.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
		format.mFramesPerPacket = 1;
		format.mChannelsPerFrame = param.ChannelCount;
		format.mBitsPerChannel = 16;
		format.mBytesPerPacket = 2 * param.ChannelCount;
		format.mBytesPerFrame = 2 * param.ChannelCount;

		status = AudioUnitSetProperty(impl->audioUnit,
									  kAudioUnitProperty_StreamFormat,
									  kAudioUnitScope_Input,
									  0,
									  &format,
									  sizeof(format));
		if (status != noErr)
		{
			printf("CoreAudio: Failed to set stream format, error %d\n", (int)status);
			return false;
		}

		// Set callback
		AURenderCallbackStruct cb;
		cb.inputProc = Impl::RenderCallback;
		cb.inputProcRefCon = impl.get();

		status = AudioUnitSetProperty(impl->audioUnit,
									  kAudioUnitProperty_SetRenderCallback,
									  kAudioUnitScope_Input,
									  0,
									  &cb,
									  sizeof(cb));
		if (status != noErr)
		{
			printf("CoreAudio: Failed to set render callback, error %d\n", (int)status);
			return false;
		}

		// Try to set hardware buffer size for lowest latency (macOS only)
#if !TARGET_OS_IPHONE
		{
			AudioDeviceID outputDeviceID;
			UInt32 propertySize = sizeof(AudioDeviceID);
			if (AudioUnitGetProperty(impl->audioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &outputDeviceID, &propertySize) == noErr)
			{
				AudioObjectPropertyAddress propertyAddress = {
					kAudioDevicePropertyBufferFrameSizeRange,
					kAudioObjectPropertyScopeOutput,
					kAudioObjectPropertyElementMain
				};

				AudioValueRange range;
				propertySize = sizeof(range);
				if (AudioObjectGetPropertyData(outputDeviceID, &propertyAddress, 0, NULL, &propertySize, &range) != noErr)
				{
					propertyAddress.mElement = kAudioObjectPropertyElementMaster;
					AudioObjectGetPropertyData(outputDeviceID, &propertyAddress, 0, NULL, &propertySize, &range);
				}

				UInt32 bufferFrameSize = (UInt32)range.mMinimum;

				propertyAddress.mSelector = kAudioDevicePropertyBufferFrameSize;
				propertySize = sizeof(bufferFrameSize);
				if (AudioObjectSetPropertyData(outputDeviceID, &propertyAddress, 0, NULL, propertySize, &bufferFrameSize) != noErr)
				{
					propertyAddress.mElement = kAudioObjectPropertyElementMaster;
					AudioObjectSetPropertyData(outputDeviceID, &propertyAddress, 0, NULL, propertySize, &bufferFrameSize);
				}

				printf("CoreAudio: Hardware buffer size set to %u frames (Hardware Range: %.0f - %.0f)\n",
					   bufferFrameSize, range.mMinimum, range.mMaximum);
			}
		}
#endif

		status = AudioUnitInitialize(impl->audioUnit);
		if (status != noErr)
		{
			printf("CoreAudio: AudioUnitInitialize failed with error %d\n", (int)status);
			return false;
		}

		status = AudioOutputUnitStart(impl->audioUnit);
		if (status != noErr)
		{
			printf("CoreAudio: AudioOutputUnitStart failed with error %d\n", (int)status);
			return false;
		}

		// Pre-allocate buffer to avoid allocation in render thread
		UInt32 maxFrames = 4096;
		UInt32 propSize = sizeof(maxFrames);
		AudioUnitGetProperty(impl->audioUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &maxFrames, &propSize);
		impl->renderBuffer.assign(maxFrames * impl->channelCount, 0);

		impl->isOpenRunning.store(true);
		printf("CoreAudio: Stream started (Rate: %u, Channels: %u)\n", param.SampleRate, param.ChannelCount);
		return true;
	}

	b8 CoreAudioBackend::StopCloseStream()
	{
		if (!impl->isOpenRunning.load())
			return false;

		impl->isOpenRunning.store(false);

		AudioOutputUnitStop(impl->audioUnit);
		AudioUnitUninitialize(impl->audioUnit);
		AudioComponentInstanceDispose(impl->audioUnit);
		impl->audioUnit = nullptr;

		printf("CoreAudio: Stream stopped.\n");
		return true;
	}

	b8 CoreAudioBackend::IsOpenRunning() const { return impl->isOpenRunning.load(); }

	u32 CoreAudioBackend::GetVariantCount() const { return 1; }
	cstr CoreAudioBackend::GetVariantName(u32 index) const { return "CoreAudio"; }
}

#endif
