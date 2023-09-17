/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>

// Nap includes
#include <audio/utility/safeptr.h>
#include <utility/threading.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>

namespace nap
{
	namespace audio
	{

        /**
         * Node that allows the queueing of samples from another thread before they are being send through the output pin.
         *
         */
		class NAPAPI SampleQueuePlayerNode : public Node
		{
			RTTI_ENABLE(Node)

		public:
			SampleQueuePlayerNode(NodeManager& manager, float latency, int channels);

            /**
             * Connect output to this pin
             */
			OutputPin audioOutput = {this};

            /**
             * Queue any amount of samples from another thread to be played back through the outpu pin.
             * @param samples Pointer to floating point data of sample data
             * @param numSamples Number of samples to be queued
             */
			void queueSamples(const float* samples, size_t numSamples);

		private:
			// Inherited from Node
			void process() override;

            SampleBuffer mCircularBuffer; // Circular read/write buffer where queued samples will be stored before being played
			int mReadPosition = 0;        // Read position within the circular buffer of the next sample, to be played
			int mWritePosition = 0;       // Write position within the circular buffer where the next queued sample will be stored
            int mLatency = 0;             // Latency in samples
            int mMinimumReserve = 0;      // Minimum reserve of samples in the buffer before an underflow is signalled
			moodycamel::ConcurrentQueue<float> mQueue;  // New samples are queued here from a different thread.
		};

	}
}
