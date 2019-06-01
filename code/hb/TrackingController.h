#ifndef TRACKING_CONTROLLER_H
#define TRACKING_CONTROLLER_H

#include "hec.h"

struct TrackingController
{
    EntityHandle target;
    float p = -0.96f;
    float d = -0.2605f;
}

#endif
