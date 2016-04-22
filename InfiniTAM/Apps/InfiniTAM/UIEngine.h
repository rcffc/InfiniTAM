// Copyright 2014-2015 Isis Innovation Limited and the authors of InfiniTAM

#pragma once

#include "../../Engine/ImageSourceEngine.h"
#include "../../Engine/IMUSourceEngine.h"
#include "../../Engine/FFMPEGWriter.h"
#include "../../ITMLib/Core/ITMMainEngine.h"
#include "../../ITMLib/Utils/ITMLibSettings.h"
#include "../../Utils/FileUtils.h"
#include "../../Utils/NVTimer.h"

#include <vector>

namespace InfiniTAM
{
	namespace Engine
	{
		class UIEngine
		{
			static UIEngine* instance;

			enum MainLoopAction
			{
				PROCESS_PAUSED, PROCESS_FRAME, PROCESS_VIDEO, EXIT, SAVE_TO_DISK
			}mainLoopAction;

			struct UIColourMode {
				const char *name;
				ITMLib::ITMMainEngine::GetImageType type;
				UIColourMode(const char *_name, ITMLib::ITMMainEngine::GetImageType _type)
				 : name(_name), type(_type)
				{}
			};
			std::vector<UIColourMode> colourModes;
			int currentColourMode;

			ImageSourceEngine *imageSource;
			IMUSourceEngine *imuSource;
			ITMLib::ITMLibSettings internalSettings;
			ITMLib::ITMMainEngine *mainEngine;

			StopWatchInterface *timer_instant;
			StopWatchInterface *timer_average;

		private: // For UI layout
			static const int NUM_WIN = 3;
			Vector4f winReg[NUM_WIN]; // (x1, y1, x2, y2)
			Vector2i winSize;
			uint textureId[NUM_WIN];
			ITMUChar4Image *outImage[NUM_WIN];
			ITMLib::ITMMainEngine::GetImageType outImageType[NUM_WIN];

			ITMUChar4Image *inputRGBImage; ITMShortImage *inputRawDepthImage;
			ITMLib::ITMIMUMeasurement *inputIMUMeasurement;

			bool freeviewActive;
			bool intergrationActive;
			ORUtils::SE3Pose freeviewPose;
			ITMLib::ITMIntrinsics freeviewIntrinsics;

			int mouseState;
			Vector2i mouseLastClick;

			int currentFrameNo; bool isRecording;
			FFMPEGWriter *rgbVideoWriter;
			FFMPEGWriter *depthVideoWriter;
		public:
			static UIEngine* Instance(void) {
				if (instance == NULL) instance = new UIEngine();
				return instance;
			}

			static void glutDisplayFunction();
			static void glutIdleFunction();
			static void glutKeyUpFunction(unsigned char key, int x, int y);
			static void glutMouseButtonFunction(int button, int state, int x, int y);
			static void glutMouseMoveFunction(int x, int y);
			static void glutMouseWheelFunction(int button, int dir, int x, int y);

			const Vector2i & getWindowSize(void) const
			{ return winSize; }

			float processedTime;
			int processedFrameNo;
			char *outFolder;
			bool needsRefresh;
			ITMUChar4Image *saveImage;

			void Initialise(int & argc, char** argv, ImageSourceEngine *imageSource, IMUSourceEngine *imuSource, ITMLib::ITMMainEngine *mainEngine,
				const char *outFolder, ITMLib::ITMLibSettings::DeviceType deviceType);
			void Shutdown();

			void Run();
			void ProcessFrame();
			
			void GetScreenshot(ITMUChar4Image *dest) const;
			void SaveScreenshot(const char *filename) const;
		};
	}
}