#pragma once

#include <Urho3D/Core/Object.h>

/// Log message event.
URHO3D_EVENT(E_CAMERAANGLECHANGED, CameraAngleChanged)
{
    URHO3D_PARAM(P_ANGLEX, Message);   // float
    URHO3D_PARAM(P_ANGLEY, Level);     // float
}
