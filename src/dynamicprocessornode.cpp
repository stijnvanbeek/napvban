/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "dynamicprocessornode.h"
#include "vbanstreamsendercomponent.h"
#include "vban/vban.h"

#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::DynamicProcessorNode)
		RTTI_PROPERTY("input", &nap::audio::DynamicProcessorNode::inputs, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


namespace nap
{
	namespace audio
	{
		DynamicProcessorNode::DynamicProcessorNode(NodeManager& nodeManager)
			: Node(nodeManager)
		{
			getNodeManager().registerRootProcess(*this);
            mInputPullResult.reserve(2);
		}


		DynamicProcessorNode::~DynamicProcessorNode()
		{
			getNodeManager().unregisterRootProcess(*this);
		}


		void DynamicProcessorNode::process()
		{
			// get output buffers
			inputs.pull(mInputPullResult);

			int num_samples = mInputPullResult.size() * VBanBitResolutionSize[(VBAN_BITFMT_16_INT & VBAN_BIT_RESOLUTION_MASK)];
			int payload_size = VBAN_DATA_MAX_SIZE / num_samples;
			payload_size = (int) ( payload_size / num_samples ) * num_samples;
			if (payload_size > VBAN_SAMPLES_MAX_NB)
				payload_size = VBAN_SAMPLES_MAX_NB;

			int buffer_size = mInputPullResult[0]->size();
			int sample_count = 0;
			while (sample_count < buffer_size)
			{
				for(auto& buffer : mInputPullResult)
				{
					// convert float to short
					short value = static_cast<short>(buffer->data()[sample_count] * 32768.0f);

					// convert short to two bytes
					char byte_1 = value;
					char byte_2 = value >> 8;

					mBuffer.emplace_back(byte_1);
					mBuffer.emplace_back(byte_2);

					// process buffer when we reach max size of vban packet
					if( mBuffer.size() == payload_size)
					{
						std::vector<char> vban_payload;
						mBuffer.swap(vban_payload);

						for (auto* processor : mProcessors)
						{
							processor->processBuffer(vban_payload);
						}
					}

					assert(mBuffer.size() < payload_size); // buffersize should never grow beyond payload size
				}
				sample_count++;
			}
		}


		void DynamicProcessorNode::registerProcessor(IProcessor * processor)
		{
			getNodeManager().enqueueTask([this, processor]()
			{
				auto it = std::find_if(mProcessors.begin(), mProcessors.end(),
									   [processor](auto& a) { return a == processor; });

				assert(it == mProcessors.end()); // processor already registered

				if (it == mProcessors.end())
				{
					mProcessors.emplace_back(processor);
				}
			});
		}


		void DynamicProcessorNode::removeProcessor(IProcessor * processor)
		{
			getNodeManager().enqueueTask([this, processor]()
			{
				auto it = std::find_if(mProcessors.begin(), mProcessors.end(), [processor](auto& a)
				{
					return a == processor;
				});

				assert(it != mProcessors.end()); // processor not registered

				if(it != mProcessors.end())
				{
					mProcessors.erase(it);
				}
			});
		}
	}
}
