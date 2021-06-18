#pragma once

#include "../ORUtils/SE3Pose.h"

namespace InputSource
{

    class PoseSourceEngine
    {
    private:
        static const int BUF_SIZE = 2048;
        char poseMask[BUF_SIZE];

        ORUtils::SE3Pose *cached_pose;

        void loadPoseIntoCache();
        int currentFrameNo;

    public:
        PoseSourceEngine(const char *poseMask);
        ~PoseSourceEngine() {}

        bool hasMorePoses(void);
        void getPose(ORUtils::SE3Pose *pose);
    };

}
