#include "iceweasel/CameraControllerRotation.h"
#include "iceweasel/CameraControllerEvents.h"
#include "iceweasel/IceWeaselConfig.h"
#include "iceweasel/Math.h"

#include <Urho3D/Input/Input.h>
#include <Urho3D/Scene/Node.h>

using namespace Urho3D;


// ----------------------------------------------------------------------------
CameraControllerRotation::CameraControllerRotation(Urho3D::Context* context) :
    LogicComponent(context)
{
}

// ----------------------------------------------------------------------------
void CameraControllerRotation::Start()
{
    input_ = GetSubsystem<Input>();

    // Set initial rotation to current camera angle
    const Vector3& cameraRotationEuler = node_->GetRotation().EulerAngles();
    angleX_ = Math::Wrap180(cameraRotationEuler.x_);
    angleY_ = Math::Wrap180(cameraRotationEuler.y_);
}

// ----------------------------------------------------------------------------
void CameraControllerRotation::Update(float timeStep)
{
    // Calculate new camera angle according to mouse movement
    float sensitivity = GetSubsystem<IceWeaselConfig>()->GetConfig().input.mouse.sensitivity;
    const IntVector2& mouseMove = input_->GetMouseMove();

    angleX_ = angleX_ + mouseMove.y_ * sensitivity;
    angleY_ = Math::Wrap180(angleY_ + mouseMove.x_ * sensitivity);

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
