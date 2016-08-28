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
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    // Cache frequently used subsystems/scene components
    input_ = GetSubsystem<Input>();
    gravityManager_ = GetScene()->GetComponent<GravityManager>();
    physicsWorld_ = GetScene()->GetComponent<PhysicsWorld>();

    // Set up things
    PatchSceneGraph();
    CreateComponents();

    // Initial physics parameters
    downVelocity_ = 0.0f;
    moveNode_->SetRotation(Quaternion::IDENTITY);

    // Need to listen to node collision events to reset gravity
    SubscribeToEvent(E_NODECOLLISION, URHO3D_HANDLER(CameraControllerFPS, HandleNodeCollision));
    // WASD depends on the current camera Y angle
    SubscribeToEvent(E_CAMERAANGLECHANGED, URHO3D_HANDLER(CameraControllerFPS, HandleCameraAngleChanged));
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::Stop()
{
    DestroyComponents();
    UnpatchSceneGraph();
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::Update(float timeStep)
{
    (void)timeStep;

    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    if(input_->GetKeyPress(KEY_CTRL))
    {
        collisionShapeCrouch_->SetEnabled(true);
        collisionShapeUpright_->SetEnabled(false);
    }
    if(collisionShapeCrouch_->IsEnabled() && input_->GetKeyDown(KEY_CTRL) == false)
    {
        if(CanStandUp())
        {
            collisionShapeCrouch_->SetEnabled(false);
            collisionShapeUpright_->SetEnabled(true);
        }
    }

    // Change height offset if crouching
    float targetHeight = config.playerClass(0).body.height;
    if(collisionShapeCrouch_->IsEnabled())
        targetHeight = config.playerClass(0).body.crouchHeight;

    float currentHeight = heightOffsetNode_->GetPosition().y_;
    currentHeight += (targetHeight - currentHeight) *
            Min(1.0f, config.playerClass(0).speed.crouchTransitionSpeed * timeStep);
    heightOffsetNode_->SetPosition(Vector3(0, currentHeight, 0));
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::FixedUpdate(float timeStep)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    const IceWeaselConfig::Data::PlayerClass& playerClass = config.playerClass(0);

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
    if(input_->GetKeyDown(KEY_SHIFT))
        speed = playerClass.speed.run;
    if(input_->GetKeyDown(KEY_CTRL) || collisionShapeCrouch_->IsEnabled())
        speed = playerClass.speed.crouch;
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

    /*
     * Query gravity at player's 3D location and calculate the rotation matrix
     * that would transform our local coordinate system into the gravity's
     * coordinate system (such that "down" correlates with the direction of
     * gravity).
     */
    Vector3 gravity = gravityManager_->QueryGravity(moveNode_->GetWorldPosition());
    Quaternion gravityRotation(Vector3::DOWN, gravity);
    Matrix3 velocityTransform = gravityRotation.RotationMatrix();

    /*
     * Transform the body's current velocity into our local coordinate system.
     */
    Vector3 currentLocalPlaneVelocity = velocityTransform.Inverse() * body_->GetLinearVelocity();

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
            currentLocalPlaneVelocity *= playerClass.jump.bunnyHopBoost;
        }
    }
    if(jumpKeyPressed_ && input_->GetKeyDown(KEY_SPACE) == false)
        jumpKeyPressed_ = false;

    // TODO limit velocity on slopes?

    // TODO Take upwards velocity into account when bunny hopping (e.g. on ramps)

    // TODO Add a "lifter" collision sphere to handle steps or other sharp edges.

    /*
     * X/Z movement of the player is only possible when on the ground. If the
     * player is in the air just maintain whatever velocity he currently has.
     */
    static const float smoothness = 16.0f;
    if(downVelocity_ == 0.0f)
        currentLocalPlaneVelocity += (localTargetPlaneVelocity - currentLocalPlaneVelocity) * timeStep * smoothness;

    // Integrate downwards velocity and apply velocity back to body.
    downVelocity_ -= gravity.Length() * timeStep;
    body_->SetLinearVelocity(velocityTransform * Vector3(currentLocalPlaneVelocity.x_, downVelocity_, currentLocalPlaneVelocity.z_));

    // Smoothly rotate camera to target orientation
    static const float correctCameraAngleSpeed = 5.0f;
    currentRotation_ = currentRotation_.Nlerp(gravityRotation, timeStep * correctCameraAngleSpeed, true);
    moveNode_->SetRotation(-currentRotation_);
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::HandleCameraAngleChanged(StringHash eventType, VariantMap& eventData)
{
    cameraAngleY_ = eventData[CameraAngleChanged::P_ANGLEY].GetFloat();
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::PatchSceneGraph()
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    /*
     * Insert a node by re-parenting the current node. The height offset node
     * is used to set the height of the camera off of the ground while the
     * move node is used to move the camera around in the 3D world.
     */
    moveNode_ = node_->GetParent()->CreateChild("", LOCAL);
    moveNode_->SetPosition(node_->GetPosition());
    heightOffsetNode_ = node_;
    heightOffsetNode_->SetParent(moveNode_);
    heightOffsetNode_->SetPosition(Vector3(0, config.playerClass(0).body.height, 0));
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::UnpatchSceneGraph()
{
    // Revert back the changes we did on the scene graph
    heightOffsetNode_->SetParent(moveNode_->GetParent());
    moveNode_->Remove();
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::CreateComponents()
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    // Set up player collision shapes. One for standing and one for crouching
    collisionShapeUpright_ = moveNode_->CreateComponent<CollisionShape>();
    collisionShapeUpright_->SetCapsule(config.playerClass(0).body.width,
                                       config.playerClass(0).body.height,
                                       Vector3(0, config.playerClass(0).body.height / 2, 0));
    collisionShapeCrouch_ = moveNode_->CreateComponent<CollisionShape>();
    collisionShapeCrouch_->SetCapsule(config.playerClass(0).body.crouchWidth,
                                      config.playerClass(0).body.crouchHeight,
                                      Vector3(0, config.playerClass(0).body.crouchHeight / 2, 0));
    collisionShapeCrouch_->SetEnabled(false);

    // Set up rigid body. Disable angular rotation and disable world gravity
    body_ = moveNode_->CreateComponent<RigidBody>();
    body_->SetAngularFactor(Vector3::ZERO);
    body_->SetMass(config.playerClass(0).body.mass);
    body_->SetFriction(0.0f);
    body_->SetUseGravity(false);
}

// ----------------------------------------------------------------------------
void CameraControllerFPS::DestroyComponents()
{
    // Clean up the components we added
    moveNode_->RemoveComponent<RigidBody>();
    moveNode_->RemoveComponent<CollisionShape>();
    moveNode_->RemoveComponent<CollisionShape>();
}

// ----------------------------------------------------------------------------
bool CameraControllerFPS::CanStandUp() const
{
    bool canStandUp = true;

    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    const IceWeaselConfig::Data::PlayerClass& playerClass = config.playerClass(0);

    /*
     * Temporarily disable collision checks for the player's rigid body, so
     * raycasts don't collide with ourselves.
     */
    unsigned storeCollisionMask = body_->GetCollisionMask();
    body_->SetCollisionMask(0);

        float rayCastLength = playerClass.body.height;
        Vector3 upDirection = moveNode_->GetRotation() * Vector3::UP;
        Vector3 rayOrigin = moveNode_->GetWorldPosition();

        PhysicsRaycastResult result;
        Ray ray(rayOrigin, upDirection);
        physicsWorld_->RaycastSingle(result, ray, rayCastLength);
        if(result.distance_ < rayCastLength)
            canStandUp = false;

    body_->SetCollisionMask(storeCollisionMask);

    return canStandUp;
}


// ----------------------------------------------------------------------------
void CameraControllerFPS::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    using namespace NodeCollision;
    (void)eventType;
    (void)eventData;

    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    const IceWeaselConfig::Data::PlayerClass& playerClass = config.playerClass(0);

    // Temporarily disable collision checks for the player's rigid body, so
    // raycasts don't collide with ourselves.
    unsigned int storeCollisionMask = body_->GetCollisionMask();
    body_->SetCollisionMask(0);

        float rayCastLength = playerClass.body.height;
        if(collisionShapeCrouch_->IsEnabled())
            rayCastLength = playerClass.body.crouchHeight;
        Vector3 downDirection = moveNode_->GetRotation() * Vector3::DOWN;
        Vector3 rayOrigin(moveNode_->GetWorldPosition() + -downDirection * rayCastLength);

        // Cast slightly beyond
        static const float extendFactor = 1.1f;
        rayCastLength *= extendFactor;

        // Cast a ray down and check if we're on the ground
        PhysicsRaycastResult result;
        Ray ray(rayOrigin, downDirection);
        physicsWorld_->RaycastSingle(result, ray, rayCastLength);
        if(result.distance_ < rayCastLength)
            if(downVelocity_ <= 0.0f)
                downVelocity_ = 0.0f;

    // Restore collision mask
    body_->SetCollisionMask(storeCollisionMask);
}
