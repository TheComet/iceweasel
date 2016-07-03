#include "iceweasel/FPSCameraMovementController.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Math/Matrix2.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/PhysicsEvents.h>
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
    downVelocity_(0.0f),
    playerParameters_({80.0f, 1.8f, 0.8f, 5.0f, 1.25f}) // weight, height, width, jump force, jump speed boost factor
    // TODO Read these values from an XML file
    // TODO Auto-reload XML file when it is edited
{
}

// ----------------------------------------------------------------------------
void FPSCameraMovementController::Start()
{
    input_ = GetSubsystem<Input>();
    physicsWorld_ = GetScene()->GetComponent<PhysicsWorld>();

    CollisionShape* colShape = moveNode_->CreateComponent<CollisionShape>();
    RigidBody* body = moveNode_->CreateComponent<RigidBody>();

    colShape->SetCapsule(playerParameters_.width,
                         playerParameters_.height,
                         Vector3(0, -playerParameters_.height / 2, 0));
    body->SetAngularFactor(Vector3::ZERO);
    body->SetMass(playerParameters_.mass);
    body->SetFriction(0.0f);

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FPSCameraMovementController, HandleUpdate));
    SubscribeToEvent(E_NODECOLLISION, URHO3D_HANDLER(FPSCameraMovementController, HandleNodeCollision));
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

    // Get input direction vector
    float speed = 8.0f;
    Vector3 targetPlaneVelocity(Vector2::ZERO);
    if(input_->GetKeyDown(KEY_W))     targetPlaneVelocity.z_ += 1;
    if(input_->GetKeyDown(KEY_S))     targetPlaneVelocity.z_ -= 1;
    if(input_->GetKeyDown(KEY_A))     targetPlaneVelocity.x_ += 1;
    if(input_->GetKeyDown(KEY_D))     targetPlaneVelocity.x_ -= 1;
    if(targetPlaneVelocity.x_ != 0 || targetPlaneVelocity.z_ != 0)
        targetPlaneVelocity = targetPlaneVelocity.Normalized() * speed;

    // Rotate input direction by camera angle using a 3D rotation matrix
    float cameraYAngle = rotateNode_->GetRotation().EulerAngles().y_;
    targetPlaneVelocity = Matrix3(-Cos(cameraYAngle), 0, Sin(cameraYAngle),
                              0, 1, 0,
                              Sin(cameraYAngle), 0, Cos(cameraYAngle)) * targetPlaneVelocity;

    // Controls the player's Y velocity. The velocity is reset to 0.0f when
    // E_NODECOLLISION occurs and the player is on the ground. Allow the player
    // to jump by pressing space while the velocity is 0.0f.
    if(input_->GetKeyDown(KEY_SPACE) && downVelocity_ == 0.0f)
    {
        downVelocity_ = playerParameters_.jumpForce;
        // Give the player a slight speed boost so he moves faster than usual
        // in the air.
        planeVelocity_ *= playerParameters_.jumpSpeedBoostFactor;
    }

    // TODO limit velocity on slopes?

    // smoothly approach target direction if we're on the ground. Otherwise
    // just maintain whatever plane velocity we had previously.
    float smoothness = 16.0f;
    if(downVelocity_ == 0.0f)
        planeVelocity_ += (targetPlaneVelocity - planeVelocity_) * timeStep * smoothness;

    // Integrate gravity to get Y velocity
    downVelocity_ += physicsWorld_->GetGravity().y_ * timeStep;

    // update camera position
    Vector3 velocity(planeVelocity_.x_, downVelocity_, planeVelocity_.z_);
    body->SetLinearVelocity(velocity);
}

// ----------------------------------------------------------------------------
void FPSCameraMovementController::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    using namespace NodeCollision;
    (void)eventType;
    (void)eventData;

    RigidBody* body = moveNode_->GetComponent<RigidBody>();

    // Temporarily disable collision checks for the player's rigid body, so
    // raycasts don't collide with ourselves.
    unsigned int storeCollisionMask = body->GetCollisionMask();
    body->SetCollisionMask(0);

        // Cast a ray down and check if we're on the ground
        PhysicsRaycastResult result;
        float rayCastLength = playerParameters_.height * 1.05;
        Ray ray(moveNode_->GetWorldPosition(), Vector3::DOWN);
        physicsWorld_->RaycastSingle(result, ray, rayCastLength);
        if(result.distance_ < rayCastLength)
            // Reset player's Y velocity and boost speed
            downVelocity_ = 0.0f;

    // Restore collision mask
    body->SetCollisionMask(storeCollisionMask);
}
