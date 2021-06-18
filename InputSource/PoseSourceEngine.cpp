// Copyright 2014-2017 Oxford University Innovation Limited and the authors of InfiniTAM

#include "PoseSourceEngine.h"

#include "../ORUtils/FileUtils.h"

#include <stdio.h>

using namespace InputSource;

PoseSourceEngine::PoseSourceEngine(const char *poseMask)
{
	strncpy(this->poseMask, poseMask, BUF_SIZE);

	currentFrameNo = 0;

	cached_pose = NULL;
}

void PoseSourceEngine::loadPoseIntoCache(void)
{
	char str[2048]; FILE *f; bool success = false;

	cached_pose = new ORUtils::SE3Pose();
	ORUtils::Matrix4<float> M;


	sprintf(str, poseMask, currentFrameNo);
	f = fopen(str, "r");
	if (f)
	{
		size_t ret = fscanf(f, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
							&M.m00, &M.m01, &M.m02, &M.m03,
							&M.m10, &M.m11, &M.m12, &M.m13,
							&M.m20, &M.m21, &M.m22, &M.m23,
							&M.m30, &M.m31, &M.m32, &M.m33);

		fclose(f);
		cached_pose->SetM(M);
		if (ret == 16) success = true;
	}

	if (!success) {
		delete cached_pose; cached_pose = NULL;
		printf("error reading file '%s'\n", str);
	}
}

bool PoseSourceEngine::hasMorePoses(void)
{
	loadPoseIntoCache();

	return (cached_pose != NULL);
}

void PoseSourceEngine::getPose(ORUtils::SE3Pose *pose)
{
	bool bUsedCache = false;
	
	if (cached_pose != NULL)
	{
		pose->SetM(cached_pose->GetM());
		delete cached_pose;
		cached_pose = NULL;
		bUsedCache = true;
	}

	if (!bUsedCache) this->loadPoseIntoCache();

	++currentFrameNo;
}
