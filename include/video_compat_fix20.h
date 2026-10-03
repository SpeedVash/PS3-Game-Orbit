#pragma once
// The pinned ps3aqua SDK uses videoOut* names for the same video output APIs.
#include <sysutil/video_out.h>
using videoConfiguration = videoOutConfiguration;
using videoState = videoOutState;
using videoResolution = videoOutResolution;
#define VIDEO_PRIMARY VIDEO_OUT_PRIMARY
#define VIDEO_RESOLUTION_720 VIDEO_OUT_RESOLUTION_720
#define VIDEO_BUFFER_FORMAT_XRGB VIDEO_OUT_BUFFER_FORMAT_XRGB
#define VIDEO_ASPECT_AUTO VIDEO_OUT_ASPECT_AUTO
#define videoGetResolutionAvailability videoOutGetResolutionAvailability
#define videoGetResolution videoOutGetResolution
#define videoConfigure videoOutConfigure
#define videoGetState videoOutGetState
#define videoGetConfiguration videoOutGetConfiguration
