#include "iceweasel/FreeCamMovementController.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Math/Matrix2.h>
#include <Urho3D/Scene/Node.h>


using namespace Urho3D;

// ----------------------------------------------------------------------------
FreeCamMovementController::FreeCamMovementController(Context* context, Node* cameraNode) :
    Component(context),
    cameraNode_(cameraNode)
{
    input_ = GetSubsystem<Input>();

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FreeCamMovementController, HandleUpdate));
}

// ----------------------------------------------------------------------------
void FreeCamMovementController::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if(!cameraNode_)
        return;

    using namespace Update;
    (void)eventType;


    double timeStep = eventData[P_TIMESTEP].GetDouble();

    // get input direction
    float speed = 0.1f;
    float smoothness = 4.0f;
    Vector3 targetDirection(Vector3::ZERO);
    if(input_->GetKeyDown(KEY_SHIFT)) speed *= 5; // speed boost
    if(input_->GetKeyDown(KEY_W))     targetDirection.z_ += 1;
    if(input_->GetKeyDown(KEY_S))     targetDirection.z_ -= 1;
    if(input_->GetKeyDown(KEY_A))     targetDirection.x_ += 1;
    if(input_->GetKeyDown(KEY_D))     targetDirection.x_ -= 1;
    if(input_->GetKeyDown(KEY_SPACE)) targetDirection.y_ += 1;
    if(input_->GetKeyDown(KEY_CTRL))  targetDirection.y_ -= 1;
    if(targetDirection.x_ != 0 || targetDirection.y_ != 0 || targetDirection.z_ != 0)
        targetDirection = targetDirection.Normalized() * speed;

    // rotate input direction by camera angle using a 3D rotation matrix
    float cameraYAngle = cameraNode_->GetRotation().EulerAngles().y_;
    targetDirection = Matrix3(-Cos(cameraYAngle), 0, Sin(cameraYAngle),
                              0, 1, 0,
                              Sin(cameraYAngle), 0, Cos(cameraYAngle)) * targetDirection;

    // smoothly approach target direction
    actualDirection_ += (targetDirection - actualDirection_) * timeStep * smoothness;

    // update camera position
    cameraNode_->Translate(actualDirection_, Urho3D::TS_WORLD);
}

/*
// ----------------------------------------------------------------------------
void FreeCamMovementController::UpdatePlayerPosition(double timeStep)
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
    targetDirection = Matrix2(Cos(cameraYAngle), -Sin(cameraYAngle),
                              Sin(cameraYAngle), Cos(cameraYAngle)) * targetDirection;

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
void FreeCamMovementController::UpdatePlayerAngle(double timeStep)
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
