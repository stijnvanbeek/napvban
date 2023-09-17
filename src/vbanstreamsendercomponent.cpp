/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "vbanstreamsendercomponent.h"
#include "udppacket.h"
#include "dynamicprocessornode.h"

#include "vban/vban.h"

#include <entity.h>
#include <audio/service/audioservice.h>
#include <audio/node/outputnode.h>

RTTI_BEGIN_CLASS(nap::audio::VbanStreamSenderComponent)
RTTI_PROPERTY("UdpClient", &nap::audio::VbanStreamSenderComponent::mUdpClient, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("Input", &nap::audio::VbanStreamSenderComponent::mInput, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("StreamName", &nap::audio::VbanStreamSenderComponent::mStreamName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::VbanStreamSenderComponentInstance)
		RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component &)
RTTI_END_CLASS

using namespace nap::audio;

namespace nap
{
	void VbanStreamSenderComponentInstance::onDestroy()
	{
        mDynamicProcessorNode->removeProcessor(this);
	}


	bool VbanStreamSenderComponentInstance::init(utility::ErrorState& errorState)
	{
		VbanStreamSenderComponent* resource = getComponent<VbanStreamSenderComponent>();
		mResource = resource;

		mAudioService = getEntityInstance()->getCore()->getService<AudioService>();
		auto& nodeManager = mAudioService->getNodeManager();

		mChannelRouting = resource->mChannelRouting;
		if (mChannelRouting.empty())
		{
			for (auto channel = 0; channel < mInput->getChannelCount(); ++channel)
				mChannelRouting.emplace_back(channel);
		}

		for (auto channel = 0; channel < mChannelRouting.size(); ++channel)
			if (mChannelRouting[channel] >= mInput->getChannelCount())
			{
				errorState.fail("%s: Trying to rout input channel that is out of bounds.", resource->mID.c_str());
				return false;
			}

        mDynamicProcessorNode = nodeManager.makeSafe<DynamicProcessorNode>(nodeManager);
        mDynamicProcessorNode->registerProcessor(this);

		for (auto channel = 0; channel < mChannelRouting.size(); ++channel)
		{
			if (mChannelRouting[channel] < 0)
				continue;

			mDynamicProcessorNode->inputs.connect(*mInput->getOutputForChannel(mChannelRouting[channel]));
		}

		mStreamName = mResource->mStreamName;
		mUdpClient = mResource->mUdpClient.get();

		return true;
	}


    void VbanStreamSenderComponentInstance::processBuffer(const std::vector<SampleBuffer*>& buffer)
    {

    }


	void VbanStreamSenderComponentInstance::processBuffer(const std::vector<char>& vbanBuffer)
	{
		std::vector<nap::uint8> buffer;
		buffer.resize(VBAN_HEADER_SIZE + VBAN_SAMPLES_MAX_NB);

		// init header
		auto* const hdr = (struct VBanHeader*)(&buffer[0]);
		hdr->vban       = *(int32_t*)("VBAN");
		hdr->format_nbc = mChannelRouting.size() - 1;
		hdr->format_SR  = 16; // 44100
		hdr->format_bit = VBAN_BITFMT_16_INT;
		strncpy(hdr->streamname, mStreamName.c_str(), VBAN_STREAM_NAME_SIZE-1);
		hdr->nuFrame    = mFrameCount;
		hdr->format_nbs = (vbanBuffer.size() / ((hdr->format_nbc+1) * VBanBitResolutionSize[(hdr->format_bit & VBAN_BIT_RESOLUTION_MASK)])) - 1;

		// copy bytes
		std::memcpy(&buffer[VBAN_HEADER_SIZE], &vbanBuffer[0], VBAN_SAMPLES_MAX_NB);

		// create udp packet & send
		UDPPacket packet(std::move(buffer));
		mUdpClient->send(packet);

		mFrameCount++;
	}
}