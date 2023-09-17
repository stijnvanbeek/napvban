/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <renderservice.h>
#include <imguiservice.h>
#include <app.h>
#include <spheremesh.h>
#include <font.h>
#include <imagefromfile.h>
#include <audio/component/levelmetercomponent.h>
#include <audio/component/playbackcomponent.h>

#include <Spatial/Gui/AudioDeviceSettingsGui.h>

namespace nap
{
	using namespace rtti;

	class VbanSenderApp : public App
	{
		RTTI_ENABLE(App)
	public:
		VbanSenderApp(nap::Core& core) : App(core)	{ }

		/**
		 *	Initialize app specific data structures
		 */
		bool init(utility::ErrorState& error) override;
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all renderable objects to the GPU
		 */
		void render() override;

		/**
		 *	Forwards the received window event to the render service
		 */
		void windowMessageReceived(WindowEventPtr windowEvent) override;
		
		/**
		 *  Forwards the received input event to the input service
		 */
		void inputMessageReceived(InputEventPtr inputEvent) override;
		
		/**
		 *	Called when loop finishes
		 */
		int shutdown() override;

	private:
		// Nap Services
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< Manages gui related update / draw calls
		ObjectPtr<RenderWindow> mRenderWindow;							//< Pointer to the render window		
		ObjectPtr<EntityInstance> mVbanEntity;
        ObjectPtr<audio::PlaybackComponentInstance> mPlaybackComponent;

		std::vector<audio::ControllerValue> mPlotvalues = { };  //< Output of the analysis will be stored chronologically in this factor, so we can draw a plot of the data
		uint32 mTickIdx = 0;

        std::unique_ptr<audio::AudioDeviceSettingsGui> mAudioSettingsGui;
	};
}
