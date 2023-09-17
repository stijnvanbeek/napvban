/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <nap/logger.h>

#include "vbanpacketreceiver.h"
#include "vbanstreamplayercomponent.h"

#include "vban/vban.h"

RTTI_BEGIN_CLASS(nap::VBANPacketReceiver)
RTTI_PROPERTY("Server", &nap::VBANPacketReceiver::mServer, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("Channels", &nap::VBANPacketReceiver::mChannels, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

	bool VBANPacketReceiver::init(utility::ErrorState& errorState)
	{
        mServer->packetReceived.connect(mPacketReceivedSlot);
		return true;
	}


	void VBANPacketReceiver::packetReceived(const UDPPacket &packet)
	{
        // Process adding or removing receivers
        mTaskQueue.process();

		utility::ErrorState errorState;
		if (checkPacket(errorState, &packet.data()[0], packet.size()) )
		{
			struct VBanHeader const* const hdr = (struct VBanHeader*)(&packet.data()[0]);
			uint8_t channels 			= hdr->format_nbc + 1;

			int const nb_samples    = hdr->format_nbs + 1;
			int const nb_channels   = hdr->format_nbc + 1;
			size_t sample_size = VBanBitResolutionSize[static_cast<const VBanBitResolution>(hdr->format_bit & VBAN_BIT_RESOLUTION_MASK)];
			size_t payload_size = nb_samples * sample_size * nb_channels;

			// create buffers to push to players
			std::vector<std::vector<float>> buffers;
			int float_buffer_size = ( payload_size / sample_size ) / channels;
			for (int i = 0; i < channels; i++)
			{
				std::vector<float> new_buffer;
				new_buffer.resize(float_buffer_size);
				buffers.emplace_back(new_buffer);
			}

			// convert WAVE PCM to floating point
			int channel = 0;
			for (size_t i = 0; i < payload_size; i += 2)
			{
				char byte_1 = packet.data()[VBAN_HEADER_SIZE + i];
				char byte_2 = packet.data()[VBAN_HEADER_SIZE + i + 1];
				short original_value = ((static_cast<short>(byte_2)) << 8) | (0x00ff & byte_1);

				int buffer_pos = ( i / 2 ) / channels;
				buffers[channel][buffer_pos] = ((float) original_value) / (float) 32768;

				channel++;
				channel%=channels;
			}

			// get stream name, use it to forward buffers to any registered stream audio receivers
			std::string stream_name(hdr->streamname);
			for (auto* receiver : mReceivers)
				if (receiver->getStreamName() == stream_name)
					receiver->pushBuffers(buffers);
		}
        else {
			nap::Logger::warn(*this, errorState.toString());
		}
	}


	bool VBANPacketReceiver::checkPacket(utility::ErrorState& errorState, nap::uint8 const* buffer, size_t size)
	{
		struct VBanHeader const* const hdr = (struct VBanHeader*)(buffer);
		enum VBanProtocol protocol = static_cast<VBanProtocol>(VBAN_PROTOCOL_UNDEFINED_4);
		enum VBanCodec codec = static_cast<VBanCodec>(VBAN_BIT_RESOLUTION_MAX);

		if( !errorState.check(buffer != 0, "buffer is null ptr") )
			return false;

		if( !errorState.check(size > VBAN_HEADER_SIZE, "packet too small") )
			return false;

		if( !errorState.check(hdr->vban == *(int32_t*)("VBAN"), "invalid vban magic fourc"))
			return false;

		if( !errorState.check((hdr->format_bit & VBAN_RESERVED_MASK) == 0, "reserved format bit invalid value") )
			return false;

		// check protocol and codec
		protocol        = static_cast<VBanProtocol>(hdr->format_SR & VBAN_PROTOCOL_MASK);
		codec           = static_cast<VBanCodec>(hdr->format_bit & VBAN_CODEC_MASK);

		if( protocol != VBAN_PROTOCOL_AUDIO )
		{
			switch (protocol)
			{
			case VBAN_PROTOCOL_SERIAL:
			case VBAN_PROTOCOL_TXT:
			case VBAN_PROTOCOL_UNDEFINED_1:
			case VBAN_PROTOCOL_UNDEFINED_2:
			case VBAN_PROTOCOL_UNDEFINED_3:
			case VBAN_PROTOCOL_UNDEFINED_4:
				errorState.fail("protocol not supported yet");
			default:
				errorState.fail("packet with unknown protocol");
			}

			return false;
		}else
		{
			if( !errorState.check(codec == VBAN_CODEC_PCM, "unsupported codec") )
				return false;

			if( !checkPcmPacket(errorState, buffer, size) )
				return false;
		}

		return true;
	}


	bool VBANPacketReceiver::checkPcmPacket(utility::ErrorState& errorState, const nap::uint8* buffer, size_t size)
	{
		// the packet is already a valid vban packet and buffer already checked before
		struct VBanHeader const* const hdr = (struct VBanHeader*)(buffer);
		enum VBanBitResolution const bit_resolution = static_cast<const VBanBitResolution>(hdr->format_bit & VBAN_BIT_RESOLUTION_MASK);
		int const sample_rate   = hdr->format_SR & VBAN_SR_MASK;
		size_t sample_size      = 0;
		size_t payload_size     = 0;

		if( !errorState.check(bit_resolution < VBAN_BIT_RESOLUTION_MAX, "invalid bit resolution") )
			return false;

		if( !errorState.check(sample_rate < VBAN_SR_MAXNUMBER, "invalid sample rate") )
			return false;

		return true;
	}


	void VBANPacketReceiver::registerStreamListener(IVBANStreamListener* receiver)
	{
		mTaskQueue.enqueue([this, receiver]()
		{
			auto it = std::find_if(mReceivers.begin(), mReceivers.end(), [receiver](auto& a) { return a == receiver; });

			assert(it == mReceivers.end()); // receiver already registered

			if (it == mReceivers.end())
			{
				mReceivers.emplace_back(receiver);
			}
		});
	}


	void VBANPacketReceiver::removeStreamListener(IVBANStreamListener* receiver)
	{
		mTaskQueue.enqueue([this, receiver]()
		{
			auto it = std::find_if(mReceivers.begin(), mReceivers.end(), [receiver](auto& a)
			{
				return a == receiver;
			});

			assert(it != mReceivers.end()); // receiver not registered

			if(it != mReceivers.end())
			{
			  	mReceivers.erase(it);
			}
		});
	}

}