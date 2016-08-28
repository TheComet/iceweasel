#include "iceweasel/CameraControllerFPS.h"
#include "iceweasel/CameraControllerEvents.h"
#include "iceweasel/IceWeaselConfig.h"

#include <Urho3D/IceWeaselMods/GravityManager.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/SceneEvents.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
CameraControllerFPS::CameraControllerFPS(Context* context) :
    LogicComponent(context)
{
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::Start()
{
    input_ = GetSubsystem<Input>();
    gravityManager_ = GetScene()->GetComponent<GravityManager>();
    physicsWorld_ = GetScene()->GetComponent<PhysicsWorld>();
    thisNode_ = node_; // Need to store this so we can remove the components
                       // we attach in Stop()

    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    if(config.playerClass.Size() == 0)
    {
        // TODO refactor this into the player class structure and return a default structure
        URHO3D_LOGERROR("[CameraController] Failed to read player class info from settings");
        return;
    }
    const IceWeaselConfig::Data::PlayerClass& playerClass = config.playerClass[0];

    // Set up player collision shape and rigid body for FPS mode
    CollisionShape* colShape = node_->CreateComponent<CollisionShape>();
    RigidBody* body = node_->CreateComponent<RigidBody>();
    colShape->SetCapsule(playerClass.body.width,
                            playerClass.body.height,
                            Vector3(0, -playerClass.body.height / 2, 0));
    body->SetAngularFactor(Vector3::ZERO);
    body->SetMass(playerClass.body.mass);
    body->SetFriction(0.0f);
    body->SetUseGravity(false);

    // Initial physics parameters
    currentVelocity_ = Vector3::ZERO;
    downVelocity_ = 0.0f;
    node_->SetRotation(Quaternion::IDENTITY);

    // Need to listen to node collision events to reset gravity
    SubscribeToEvent(E_NODECOLLISION, URHO3D_HANDLER(CameraControllerFPS, HandleNodeCollision));
    // WASD depends on the current camera Y angle
    SubscribeToEvent(E_CAMERAANGLECHANGED, URHO3D_HANDLER(CameraControllerFPS, HandleCameraAngleChanged));
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::Stop()
{
    // Clean up the components we added
    thisNode_->RemoveComponent<RigidBody>();
    thisNode_->RemoveComponent<CollisionShape>();
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::FixedUpdate(float timeStep)
{
    RigidBody* body = node_->GetComponent<RigidBody>();
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    if(config.playerClass.Size() == 0)
    {
        // TODO refactor this into the player class structure and return a default structure
        URHO3D_LOGERROR("[CameraController] Failed to read player class info from settings");
        return;
    }
    const IceWeaselConfig::Data::PlayerClass& playerClass = config.playerClass[0];

    //TODO transform into local gravity planeVelocity_ = body->GetLinearVelocity();

    /*
     * Get input direction vector from WASD on keyboard and store in x and z
     * components of a target velocity vector.
     *
     * Player speed is "walk" by default, "run" if control is pressed and
     * "crawl" if shift is pressed (precedence on shift, you can't run and
     * crawl).
     */
    float speed = playerClass.speed.walk;
    Vector3 localTargetPlaneVelocity(Vector3::ZERO);
    if(input_->GetKeyDown(KEY_SHIFT)) speed = playerClass.speed.run;
    if(input_->GetKeyDown(KEY_CTRL))  speed = playerClass.speed.crawl;
    if(input_->GetKeyDown(KEY_W))     localTargetPlaneVelocity.z_ += 1;
    if(input_->GetKeyDown(KEY_S))     localTargetPlaneVelocity.z_ -= 1;
    if(input_->GetKeyDown(KEY_A))     localTargetPlaneVelocity.x_ += 1;
    if(input_->GetKeyDown(KEY_D))     localTargetPlaneVelocity.x_ -= 1;
    if(localTargetPlaneVelocity.x_ != 0 || localTargetPlaneVelocity.z_ != 0)
        localTargetPlaneVelocity = localTargetPlaneVelocity.Normalized() * speed;

    // Rotate input direction by camera angle using a 3D rotation matrix
    localTargetPlaneVelocity = Matrix3(
        -Cos(cameraAngleY_), 0, Sin(cameraAngleY_),
        0, 1, 0,
        Sin(cameraAngleY_), 0, Cos(cameraAngleY_)
    ) * localTargetPlaneVelocity;

    currentVelocity_ = body->GetLinearVelocity();

    /*
     * Controls the player's Y velocity. The velocity is reset to 0.0f when
     * E_NODECOLLISION occurs and the player is on the ground. Allow the player
     * to jump by pressing space while the velocity is 0.0f.
     *
     * NOTE: We can't use GetKeyPress() in FixedUpdate() so we have to track
     * the key press with a flag ourselves.
     */
    if(!jumpKeyPressed_ && input_->GetKeyDown(KEY_SPACE))
    {
        jumpKeyPressed_ = true;
        if(downVelocity_ == 0.0f)
        {
            downVelocity_ = playerClass.jump.force;
            // Give the player a slight speed boost so he moves faster than usual
            // in the air.
            currentVelocity_ *= playerClass.jump.bunnyHopBoost;
        }
    }
    if(jumpKeyPressed_ && input_->GetKeyDown(KEY_SPACE) == false)
        jumpKeyPressed_ = false;

    // TODO limit velocity on slopes?

    // TODO Take upwards velocity into account when bunny hopping (e.g. on ramps)

    // TODO Add a "lifter" collision sphere to handle steps or other sharp edges.

    // TODO Collision feedback needs to affect planeVelocity_ and downVelocity_

    /*
     * Query gravity at player's 3D location, integrate local "down" velocity
     * and rotate the local target velocity vector according to the direction
     * of gravity before applying it to the body. Let bullet handle
     * integration from there.
     */
    Vector3 gravity = gravityManager_->QueryGravity(node_->GetWorldPosition());
    downVelocity_ -= gravity.Length() * timeStep;
    Quaternion gravityRotation(Vector3::DOWN, gravity);
    Vector3 transformedVelocity = gravityRotation.RotationMatrix() * Vector3(localTargetPlaneVelocity.x_, downVelocity_, localTargetPlaneVelocity.z_);

    static const float smoothness = 16.0f;
    currentVelocity_ += (transformedVelocity - currentVelocity_) * timeStep * smoothness;

    body->SetLinearVelocity(currentVelocity_);

    // Smoothly rotate camera to target rotation
    static const float correctCameraAngleSpeed = 5.0f;
    currentRotation_ = currentRotation_.Nlerp(gravityRotation, timeStep * correctCameraAngleSpeed, true);
    node_->SetRotation(-currentRotation_);
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::HandleCameraAngleChanged(StringHash eventType, VariantMap& eventData)
{
    cameraAngleY_ = eventData[CameraAngleChanged::P_ANGLEY].GetFloat();
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    using namespace NodeCollision;
    (void)eventType;
    (void)eventData;

    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    if(config.playerClass.Size() == 0)
    {
        // TODO refactor this into the player class structure and return a default structure
        URHO3D_LOGERROR("[CameraController] Failed to read player class info from settings");
        return;
    }
    const IceWeaselConfig::Data::PlayerClass& playerClass = config.playerClass[0];

    RigidBody* body = node_->GetComponent<RigidBody>();

    // Temporarily disable collision checks for the player's rigid body, so
    // raycasts don't collide with ourselves.
    unsigned int storeCollisionMask = body->GetCollisionMask();
    body->SetCollisionMask(0);

        // Cast a ray down and check if we're on the ground
        PhysicsRaycastResult result;
        float rayCastLength = playerClass.body.height * 1.05;
        Vector3 downDirection = node_->GetRotation() * Vector3::DOWN;
        Ray ray(node_->GetWorldPosition(), downDirection);
        physicsWorld_->RaycastSingle(result, ray, rayCastLength);
        if(result.distance_ < rayCastLength)
            if(downVelocity_ <= 0.0f)
                downVelocity_ = 0.0f;

    // Restore collision mask
    body->SetCollisionMask(storeCollisionMask);
}
