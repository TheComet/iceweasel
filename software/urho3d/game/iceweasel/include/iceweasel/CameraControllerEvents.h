#pragma once

#include <Urho3D/Core/Object.h>

/// When the camera's X or Y angle changes
URHO3D_EVENT(E_CAMERAANGLECHANGED, CameraAngleChanged)
{
    URHO3D_PARAM(P_ANGLEX, AngleX);                // float
    URHO3D_PARAM(P_ANGLEY, AngleY);                // float
}

URHO3D_EVENT(E_LOCALMOVEMENTVELOCITYCHANGED, LocalMovementVelocityChanged)
{
    URHO3D_PARAM(P_LOCALMOVEMENTVELOCITY, LocalVelocity);  // Vector3
}

URHO3D_EVENT(E_CROUCHSTATECHANGED, CrouchStateChanged)
{
    URHO3D_PARAM(P_CROUCHING, Crouching);          // bool
}
