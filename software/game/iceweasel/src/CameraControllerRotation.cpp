#include "iceweasel/CameraControllerRotation.h"
#include "iceweasel/CameraControllerEvents.h"
#include "iceweasel/IceWeaselConfig.h"

#include <Urho3D/Input/Input.h>
#include <Urho3D/Scene/Node.h>

using namespace Urho3D;


// ----------------------------------------------------------------------------
float Wrap(float angle)
{
    while(angle < -180.0f)
        angle += 360.0f;
    while(angle > 180.0f)
        angle -= 360.0f;
    return angle;
}


// ----------------------------------------------------------------------------
CameraControllerRotation::CameraControllerRotation(Urho3D::Context* context) :
    LogicComponent(context)
{
}

// ----------------------------------------------------------------------------
void CameraControllerRotation::Start()
{
    input_ = GetSubsystem<Input>();
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    // Set initial rotation to current camera angle
    const Vector3& cameraRotationEuler = node_->GetRotation().EulerAngles();
    angleX_ = Wrap(cameraRotationEuler.x_);
    angleY_ = Wrap(cameraRotationEuler.y_);
}

// ----------------------------------------------------------------------------
void CameraControllerRotation::Update(float timeStep)
{
    // Calculate new camera angle according to mouse movement
    float sensitivity = GetSubsystem<IceWeaselConfig>()->GetConfig().input.mouse.sensitivity;
    const IntVector2& mouseMove = input_->GetMouseMove();

    angleX_ = angleX_ + mouseMove.y_ * sensitivity;
    angleY_ = Wrap(angleY_ + mouseMove.x_ * sensitivity);

    if(angleX_ < -90.0f) angleX_ = -90.0f;
    if(angleX_ > 90.0f)  angleX_ = 90.0f;

    node_->SetRotation(Quaternion(angleX_, angleY_, 0));

    // Emit event if angle changed
    if(mouseMove.x_ != 0 || mouseMove.y_ != 0)
    {
        using namespace CameraAngleChanged;
        VariantMap& eventData = GetEventDataMap();
        eventData[P_ANGLEX] = angleX_;
        eventData[P_ANGLEY] = angleY_;
        SendEvent(E_CAMERAANGLECHANGED, eventData);
    }
}
