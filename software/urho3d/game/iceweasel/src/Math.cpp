#include "iceweasel/Math.h"

// ----------------------------------------------------------------------------
float Math::Wrap180(float angle)
{
    while(angle < -180.0f)
        angle += 360.0f;
    while(angle > 180.0f)
        angle -= 360.0f;
    return angle;
}
