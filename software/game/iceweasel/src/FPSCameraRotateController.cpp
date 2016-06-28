#include "iceweasel/FPSCameraRotateController.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Scene/Node.h>


using namespace Urho3D;

// ----------------------------------------------------------------------------
FPSCameraRotateController::FPSCameraRotateController(Context* context, Node* cameraNode) :
    Component(context),
    cameraNode_(cameraNode)
{
    input_ = GetSubsystem<Input>();

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
    float sensitivity = 0.5f;
    const IntVector2& mouseMove = input_->GetMouseMove();
    const Vector3& cameraRotationEuler = cameraNode_->GetRotation().EulerAngles();
    cameraNode_->SetRotation(
        Quaternion(
            cameraRotationEuler.x_ + mouseMove.y_ * sensitivity,
            cameraRotationEuler.y_ + mouseMove.x_ * sensitivity,
            0
        )
    );
}

/*
// ----------------------------------------------------------------------------
void FPSCameraRotateController::UpdatePlayerPosition(double timeStep)
{
    double speed = config_.walkSpeed_;

    // get input direction
    Vector2 targetDirection(0, 0);
    if(input_->GetKeyDown(KEY_W)) targetDirection.y_ += 1;
    if(input_->GetKeyDown(KEY_S)) targetDirection.y_ -= 1;
    if(input_->GetKeyDown(KEY_A)) targetDirection.x_ -= 1;
    if(input_->GetKeyDown(KEY_D)) targetDirection.x_ += 1;
    if(targetDirection.x_ != 0 || targetDirection.y_ != 0)
        targetDirection = targetDirection.Normalized() * speed;

    // rotate input direction by camera angle using a 2D rotation matrix
    targetDirection = Matrix2(Cos(cameraAngle_), -Sin(cameraAngle_),
                              Sin(cameraAngle_), Cos(cameraAngle_)) * targetDirection;

    // smoothly approach target direction
    actualDirection_ += (targetDirection - actualDirection_) * timeStep /
            config_.accelerationSmoothness_;

    // update player position using target direction
    //node_->SetPosition(node_->GetPosition() + Vector3(actualDirection_.x_, 0, actualDirection_.y_) * timeStep);
    playerNode_->GetComponent<RigidBody>()->SetLinearVelocity(
        Vector3(actualDirection_.x_,
                playerNode_->GetComponent<RigidBody>()->GetLinearVelocity().y_,
                actualDirection_.y_));
}

// ----------------------------------------------------------------------------
void FPSCameraRotateController::UpdatePlayerAngle(double timeStep)
{
    // From the actual direction vector, calculate the Y angle the player
    // should be facing. We only do this if the player is actually moving.
    // If the player is not moving, we instead let the target angle approach
    // the actual angle to smoothly stop rotating the player.
    double speed = actualDirection_.Length();
    if(speed > config_.walkSpeed_ * 0.1)
    {
        float dotProduct = actualDirection_.y_;  // with Vector2(0, 1)
        float determinant = actualDirection_.x_; // with Vector2(0, 1)
        targetAngle_ = Atan2(determinant, dotProduct);
    }
    else
    {
        targetAngle_ += (actualAngle_ - targetAngle_) * timeStep /
                config_.rotationSmoothness_;
    }

    // always approach target angle with shortest angle (max 180°) to avoid
    // flipping
    if(actualAngle_ - targetAngle_ > 180)
        actualAngle_ -= 360;
    if(actualAngle_ - targetAngle_ < -180)
        actualAngle_ += 360;
    double delta = targetAngle_ - actualAngle_;
    actualAngle_ += delta * timeStep * speed / config_.rotationSmoothness_;

    // apply actual angle to player angle
    playerNode_->SetRotation(Quaternion(actualAngle_, Vector3::UP));
}
*/
