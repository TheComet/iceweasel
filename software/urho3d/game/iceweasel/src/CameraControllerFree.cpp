#include "iceweasel/CameraControllerFree.h"
#include "iceweasel/IceWeaselConfig.h"
#include "iceweasel/CameraControllerEvents.h"

#include <Urho3D/Input/Input.h>
#include <Urho3D/Scene/Node.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
CameraControllerFree::CameraControllerFree(Context* context) :
    LogicComponent(context)
{
}

// ----------------------------------------------------------------------------
void CameraControllerFree::Start()
{
    input_ = GetSubsystem<Input>();

    node_->SetRotation(Quaternion::IDENTITY);

    // WASD depends on the current camera Y angle
    SubscribeToEvent(E_CAMERAANGLECHANGED, URHO3D_HANDLER(CameraControllerFree, HandleCameraAngleChanged));
}

// ----------------------------------------------------------------------------
void CameraControllerFree::Update(float timeStep)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    // get input direction
    float speed = config.freeCam.speed.normal;
    Vector3 targetPlaneVelocity(Vector3::ZERO);
    if(input_->GetKeyDown(KEY_SHIFT)) speed = config.freeCam.speed.fast;
    if(input_->GetKeyDown(KEY_W))     targetPlaneVelocity.z_ += 1;
    if(input_->GetKeyDown(KEY_S))     targetPlaneVelocity.z_ -= 1;
    if(input_->GetKeyDown(KEY_A))     targetPlaneVelocity.x_ += 1;
    if(input_->GetKeyDown(KEY_D))     targetPlaneVelocity.x_ -= 1;
    if(input_->GetKeyDown(KEY_SPACE)) targetPlaneVelocity.y_ += 1;
    if(input_->GetKeyDown(KEY_CTRL))  targetPlaneVelocity.y_ -= 1;
    if(targetPlaneVelocity.x_ != 0 || targetPlaneVelocity.y_ != 0 || targetPlaneVelocity.z_ != 0)
        targetPlaneVelocity = targetPlaneVelocity.Normalized() * speed;

    // rotate input direction by camera angle using a 3D rotation matrix
    targetPlaneVelocity = Matrix3(-Cos(cameraAngleY_), 0, Sin(cameraAngleY_),
                              0, 1, 0,
                              Sin(cameraAngleY_), 0, Cos(cameraAngleY_)) * targetPlaneVelocity;

    // smoothly approach target direction
    currentVelocity_ += (targetPlaneVelocity - currentVelocity_) * timeStep * config.freeCam.speed.smoothness;

    // update camera position
    node_->Translate(currentVelocity_, Urho3D::TS_WORLD);
}

// ----------------------------------------------------------------------------
void CameraControllerFree::HandleCameraAngleChanged(StringHash eventType, VariantMap& eventData)
{
    cameraAngleY_ = eventData[CameraAngleChanged::P_ANGLEY].GetFloat();
}
