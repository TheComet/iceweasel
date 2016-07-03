#include "iceweasel/FPSCameraRotateController.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Scene/Node.h>


using namespace Urho3D;


float Wrap(float angle)
{
    while(angle < -180.0f)
        angle += 360.0f;
    while(angle > 180.0f)
        angle -= 360.0f;
    return angle;
}


// ----------------------------------------------------------------------------
FPSCameraRotateController::FPSCameraRotateController(Context* context, Node* cameraNode) :
    Component(context),
    cameraNode_(cameraNode)
{
    input_ = GetSubsystem<Input>();

    // Set initial rotation to current camera angle
    const Vector3& cameraRotationEuler = cameraNode_->GetRotation().EulerAngles();
    angleX_ = Wrap(cameraRotationEuler.x_);
    angleY_ = Wrap(cameraRotationEuler.y_);

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FPSCameraRotateController, HandleUpdate));
}

// ----------------------------------------------------------------------------
void FPSCameraRotateController::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if(!cameraNode_)
        return;

    (void)eventType;
    (void)eventData;


    // Calculate new camera angle according to mouse movement
    float sensitivity = 0.5f;  // TODO load this from an XML config file.
    const IntVector2& mouseMove = input_->GetMouseMove();

    angleX_ = angleX_ + mouseMove.y_ * sensitivity;
    angleY_ = Wrap(angleY_ + mouseMove.x_ * sensitivity);

    if(angleX_ < -90.0f) angleX_ = -90.0f;
    if(angleX_ > 90.0f)  angleX_ = 90.0f;

    cameraNode_->SetRotation(Quaternion(angleX_, angleY_, 0));
}
