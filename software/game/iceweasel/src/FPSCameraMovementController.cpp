#include "iceweasel/FPSCameraMovementController.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Math/Matrix2.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>


using namespace Urho3D;

// ----------------------------------------------------------------------------
FPSCameraMovementController::FPSCameraMovementController(Context* context,
                                                         Node* moveNode,
                                                         Node* rotateNode) :
    LogicComponent(context),
    moveNode_(moveNode),
    rotateNode_(rotateNode),
    downVelocity_(0.0f)
{
}

// ----------------------------------------------------------------------------
void FPSCameraMovementController::Start()
{
    input_ = GetSubsystem<Input>();
    physicsWorld_ = GetScene()->GetComponent<PhysicsWorld>();

    CollisionShape* colShape = moveNode_->CreateComponent<CollisionShape>();
    RigidBody* body = moveNode_->CreateComponent<RigidBody>();

    colShape->SetCapsule(0.8f, 1.8f, Vector3(0, -0.9f, 0));
    body->SetAngularFactor(Vector3::ZERO);
    body->SetMass(80);
    body->SetFriction(0.0f);

    position_ = moveNode_->GetPosition();
    oldPosition_ = position_;

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FPSCameraMovementController, HandleUpdate));
}

// ----------------------------------------------------------------------------
void FPSCameraMovementController::Stop()
{
    moveNode_->RemoveComponent<RigidBody>();
    moveNode_->RemoveComponent<CollisionShape>();
}

// ----------------------------------------------------------------------------
void FPSCameraMovementController::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    (void)eventType;

    double timeStep = eventData[P_TIMESTEP].GetDouble();

    RigidBody* body = moveNode_->GetComponent<RigidBody>();

    // get input direction
    float speed = 8.0f;
    float smoothness = 16.0f;
    Vector3 targetDirection(Vector2::ZERO);
    if(input_->GetKeyDown(KEY_W))     targetDirection.z_ += 1;
    if(input_->GetKeyDown(KEY_S))     targetDirection.z_ -= 1;
    if(input_->GetKeyDown(KEY_A))     targetDirection.x_ += 1;
    if(input_->GetKeyDown(KEY_D))     targetDirection.x_ -= 1;
    if(targetDirection.x_ != 0 || targetDirection.z_ != 0)
        targetDirection = targetDirection.Normalized() * speed;

    // rotate input direction by camera angle using a 3D rotation matrix
    float cameraYAngle = rotateNode_->GetRotation().EulerAngles().y_;
    targetDirection = Matrix3(-Cos(cameraYAngle), 0, Sin(cameraYAngle),
                              0, 1, 0,
                              Sin(cameraYAngle), 0, Cos(cameraYAngle)) * targetDirection;

    unsigned int storeCollisionMask = body->GetCollisionMask();
    body->SetCollisionMask(0);

    PhysicsRaycastResult result;
    Ray ray(moveNode_->GetWorldPosition(), Vector3::DOWN);
    physicsWorld_->RaycastSingle(result, ray, 2.0f);
    if(result.distance_ < 2.0f) // player height is 1.8
        if(input_->GetKeyDown(KEY_SPACE))
            body->ApplyImpulse(Vector3::UP * 80 * 4);

    body->SetCollisionMask(storeCollisionMask);

    // TODO limit velocity on slopes

    // smoothly approach target direction
    actualDirection_ += (targetDirection - actualDirection_) * timeStep * smoothness;
    Vector3 velocity = Vector3(actualDirection_.x_, body->GetLinearVelocity().y_, actualDirection_.z_);

    // update camera position
    body->SetLinearVelocity(velocity);
}

// ----------------------------------------------------------------------------
bool FPSCameraMovementController::IsGroundUnderneath() const
{
    PhysicsRaycastResult result;
    Ray ray(moveNode_->GetWorldPosition(), Vector3::DOWN);
    physicsWorld_->RaycastSingle(result, ray, 2.0f); // height is 1.8, cast down slightly more
    return (result.distance_ < 2.0f);
}

// ----------------------------------------------------------------------------
void FPSCameraMovementController::ApplyGravity(float timeStep)
{
    float gravity = -9.81;
    downVelocity_ += gravity * timeStep;
    position_.y_ += downVelocity_ * timeStep;
}

// ----------------------------------------------------------------------------
void FPSCameraMovementController::DoCollision()
{
    PhysicsRaycastResult result;
    Ray ray(oldPosition_, position_ - Vector3(0, 1, 0));  // cast in direction of heading
    float dist = 2;
    physicsWorld_->SphereCast(result, ray, 10, dist);
    if(result.distance_ < dist)
    {
        position_ = ray.origin_ + ray.direction_ * result.distance_;
        downVelocity_ = 0;
    }
}
