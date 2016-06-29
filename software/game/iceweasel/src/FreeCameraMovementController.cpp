#include "iceweasel/FreeCameraMovementController.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Math/Matrix2.h>
#include <Urho3D/Scene/Node.h>


using namespace Urho3D;

// ----------------------------------------------------------------------------
FreeCameraMovementController::FreeCameraMovementController(Context* context,
                                                           Node* moveNode,
                                                           Node* rotateNode) :
    Component(context),
    moveNode_(moveNode),
    rotateNode_(rotateNode)
{
    input_ = GetSubsystem<Input>();

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FreeCameraMovementController, HandleUpdate));
}

// ----------------------------------------------------------------------------
void FreeCameraMovementController::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    (void)eventType;

    double timeStep = eventData[P_TIMESTEP].GetDouble();

    // get input direction
    float speed = 0.3f;
    float smoothness = 4.0f;
    Vector3 targetDirection(Vector3::ZERO);
    if(input_->GetKeyDown(KEY_SHIFT)) speed *= 5; // speed boost
    if(input_->GetKeyDown(KEY_W))     targetDirection.z_ += 1;
    if(input_->GetKeyDown(KEY_S))     targetDirection.z_ -= 1;
    if(input_->GetKeyDown(KEY_A))     targetDirection.x_ += 1;
    if(input_->GetKeyDown(KEY_D))     targetDirection.x_ -= 1;
    if(input_->GetKeyDown(KEY_E)) targetDirection.y_ += 1;
    if(input_->GetKeyDown(KEY_Q))  targetDirection.y_ -= 1;
    if(targetDirection.x_ != 0 || targetDirection.y_ != 0 || targetDirection.z_ != 0)
        targetDirection = targetDirection.Normalized() * speed;

    // rotate input direction by camera angle using a 3D rotation matrix
    float cameraYAngle = rotateNode_->GetRotation().EulerAngles().y_;
    targetDirection = Matrix3(-Cos(cameraYAngle), 0, Sin(cameraYAngle),
                              0, 1, 0,
                              Sin(cameraYAngle), 0, Cos(cameraYAngle)) * targetDirection;

    // smoothly approach target direction
    actualDirection_ += (targetDirection - actualDirection_) * timeStep * smoothness;

    // update camera position
    moveNode_->Translate(actualDirection_, Urho3D::TS_WORLD);
}
