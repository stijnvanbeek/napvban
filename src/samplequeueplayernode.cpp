/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "samplequeueplayernode.h"

#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::SampleQueuePlayerNode)
RTTI_PROPERTY("audioOutput", &nap::audio::SampleQueuePlayerNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{

	namespace audio
	{

		SampleQueuePlayerNode::SampleQueuePlayerNode(NodeManager& manager, float latency, int channels) : Node(manager)
		{
            mLatency = latency * manager.getSamplesPerMillisecond();
			int circularBufferSize	=  mLatency * 2;
			mCircularBuffer.resize(circularBufferSize, 0.f);

			mWritePosition = mLatency;
			mReadPosition = 0;
            mMinimumReserve = mLatency / 2;
		}


		void SampleQueuePlayerNode::queueSamples(const float* samples, size_t numSamples)
		{
			// copy samples to buffer
			if (!mQueue.enqueue_bulk(samples, numSamples))
			{
				nap::Logger::error("Failed to allocate memory for queue buffer");
			}
		}


		void SampleQueuePlayerNode::process()
		{
			// Write samples from queue to buffer
            SampleValue sample;
            while (mWritePosition != mReadPosition && mQueue.try_dequeue(sample))
            {
                mCircularBuffer[mWritePosition++] = sample;
                if (mWritePosition == mCircularBuffer.size())
                    mWritePosition = 0;
            }

            // Read samples from buffer to output
			auto& outputBuffer = getOutputBuffer(audioOutput);
			for (auto i = 0; i < outputBuffer.size(); i++)
            {
                auto& bufferSample = mCircularBuffer[mReadPosition++];
				outputBuffer[i] = bufferSample;
                bufferSample = 0.f;
                if (mReadPosition == mCircularBuffer.size())
                    mReadPosition = 0;
                if (mReadPosition == mWritePosition)
                {
                    Logger::warn("SampleQueuePlayerNode: read position overtakes write position");
                    for (auto i = 0; i < mLatency; ++i)
                    {
                        mCircularBuffer[mWritePosition++] = 0.f;
                        if (mWritePosition == mCircularBuffer.size())
                            mWritePosition = 0;
                    }
                }
			}
		}
	}

}
