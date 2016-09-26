#include "iceweasel/MovementController.h"
#include "iceweasel/CameraControllerRotation.h"
#include "iceweasel/CameraControllerEvents.h"
#include "iceweasel/Finger.h"
#include "iceweasel/IceWeaselConfig.h"
#include "iceweasel/IceWeaselConfigEvents.h"
#include "iceweasel/GravityManager.h"

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
MovementController::MovementController(Context* context, Node* moveNode, Node* offsetNode) :
    LogicComponent(context),
    moveNode_(moveNode),
    offsetNode_(offsetNode),
    respawnDistance_(200.0f)
{
}
// ----------------------------------------------------------------------------
void MovementController::setRespawnDistance(float distance)
{
    respawnDistance_ = distance;
}

// ----------------------------------------------------------------------------
void MovementController::Start()
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    // Cache frequently used subsystems/scene components
    input_ = GetSubsystem<Input>();
    gravityManager_ = GetScene()->GetComponent<GravityManager>();
    physicsWorld_ = GetScene()->GetComponent<PhysicsWorld>();

    // Set up things
    CreateComponents();

    // Initial physics parameters
    moveNode_->SetRotation(Quaternion::IDENTITY);

    // Need to listen to node collision events to reset gravity
    SubscribeToEvent(E_NODECOLLISION, URHO3D_HANDLER(MovementController, HandleNodeCollision));
    // WASD depends on the current camera Y angle
    SubscribeToEvent(E_CAMERAANGLECHANGED, URHO3D_HANDLER(MovementController, HandleCameraAngleChanged));
    // Update physics collision shapes and rigid body parameters
    SubscribeToEvent(E_CONFIGRELOADED, URHO3D_HANDLER(MovementController, HandleConfigReloaded));
}

// ----------------------------------------------------------------------------
void MovementController::Stop()
{
    DestroyComponents();
}

// ----------------------------------------------------------------------------
void MovementController::Update(float timeStep)
{
    (void)timeStep;

    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    // Change collision shape depending on whether the player is crouching
    if(input_->GetKeyPress(KEY_CTRL))
    {
        collisionShapeCrouch_->SetEnabled(true);
        collisionShapeUpright_->SetEnabled(false);
    }
    if(IsCrouching() && input_->GetKeyDown(KEY_CTRL) == false)
    {
        if(CanStandUp())
        {
            collisionShapeCrouch_->SetEnabled(false);
            collisionShapeUpright_->SetEnabled(true);
        }
    }

    // Change height offset if crouching
    float targetHeight = config.playerClass(0).body.height;
    if(IsCrouching())
        targetHeight = config.playerClass(0).body.crouchHeight;

    float currentHeight = offsetNode_->GetPosition().y_;
    currentHeight += (targetHeight - currentHeight) *
            Min(1.0f, config.playerClass(0).speed.crouchTransitionSpeed * timeStep);
    offsetNode_->SetPosition(Vector3(0, currentHeight, 0));

    // Respawn player if he goes too far away
    if(moveNode_->GetWorldPosition().LengthSquared() > respawnDistance_ * respawnDistance_)
    {
        SetInitialPhysicsParameters();
        moveNode_->SetPosition(Vector3::ZERO);
    }
}

// ----------------------------------------------------------------------------
void MovementController::FixedUpdate(float timeStep)
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
    Vector3 targetLocalPlaneVelocity(Vector3::ZERO);
    if(input_->GetKeyDown(KEY_SHIFT))
        speed = playerClass.speed.run;
    if(input_->GetKeyDown(KEY_CTRL) || IsCrouching())
        speed = playerClass.speed.crouch;
    if(input_->GetKeyDown(KEY_W))     targetLocalPlaneVelocity.z_ += 1;
    if(input_->GetKeyDown(KEY_S))     targetLocalPlaneVelocity.z_ -= 1;
    if(input_->GetKeyDown(KEY_A))     targetLocalPlaneVelocity.x_ += 1;
    if(input_->GetKeyDown(KEY_D))     targetLocalPlaneVelocity.x_ -= 1;
    if(targetLocalPlaneVelocity.x_ != 0 || targetLocalPlaneVelocity.z_ != 0)
        targetLocalPlaneVelocity = targetLocalPlaneVelocity.Normalized() * speed;

    // Rotate input direction by camera angle using a 3D rotation matrix
    targetLocalPlaneVelocity = Matrix3(
        -Cos(cameraAngleY_), 0, Sin(cameraAngleY_),
        0, 1, 0,
        Sin(cameraAngleY_), 0, Cos(cameraAngleY_)
    ) * targetLocalPlaneVelocity;

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
    {
        using namespace LocalMovementVelocityChanged;
        VariantMap& eventData = GetEventDataMap();
        eventData[P_LOCALMOVEMENTVELOCITY] = currentLocalPlaneVelocity;
        SendEvent(E_LOCALMOVEMENTVELOCITYCHANGED, eventData);
    }

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
     * player is in the air just maintain whatever velocity he currently has,
     * but also add the target velocity on top of that so the player can
     * slightly control his movement in the air.
     */
    static const float smoothness = 16.0f;
    if(downVelocity_ == 0.0f)
        currentLocalPlaneVelocity += (targetLocalPlaneVelocity - currentLocalPlaneVelocity) * timeStep * smoothness;
    else
        currentLocalPlaneVelocity += targetLocalPlaneVelocity * timeStep;

    Vector3 localVelocity = Vector3(currentLocalPlaneVelocity.x_, downVelocity_, currentLocalPlaneVelocity.z_);

    // Integrate downwards velocity and apply velocity back to body.
    downVelocity_ -= gravity.Length() * timeStep;
    body_->SetLinearVelocity(velocityTransform * localVelocity);

    // Smoothly rotate to target orientation
    static const float correctCameraAngleSpeed = 5.0f;
    currentRotation_ = currentRotation_.Nlerp(gravityRotation, timeStep * correctCameraAngleSpeed, true);
    moveNode_->SetRotation(-currentRotation_);
}

// ----------------------------------------------------------------------------
void MovementController::HandleCameraAngleChanged(StringHash /*eventType*/, VariantMap& eventData)
{
    cameraAngleY_ = eventData[CameraAngleChanged::P_ANGLEY].GetFloat();
}

// ----------------------------------------------------------------------------
void MovementController::CreateComponents()
{
    // Set up player collision shapes. One for standing and one for crouching
    collisionShapeUpright_ = moveNode_->CreateComponent<CollisionShape>();
    collisionShapeCrouch_ = moveNode_->CreateComponent<CollisionShape>();
    collisionShapeCrouch_->SetEnabled(false);

    // Set up rigid body. Disable angular rotation and disable world gravity
    body_ = moveNode_->CreateComponent<RigidBody>();
    body_->SetAngularFactor(Vector3::ZERO);
    body_->SetFriction(0.0f);
    body_->SetUseGravity(false);

    UpdatePhysicsSettings();
}

// ----------------------------------------------------------------------------
void MovementController::DestroyComponents()
{
    // Clean up the components we added
    moveNode_->RemoveComponent<RigidBody>();
    moveNode_->RemoveComponent<CollisionShape>();
    moveNode_->RemoveComponent<CollisionShape>();
}

// ----------------------------------------------------------------------------
bool MovementController::CanStandUp() const
{
    /*
     * This is called when the player is crouching, but the user has let go of
     * the crouch button. The player should only be able to stand up again if
     * there are no obstacles in the way.
     */

    bool canStandUp = true;

    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    const IceWeaselConfig::Data::PlayerClass& playerClass = config.playerClass(0);

    /*
     * Temporarily disable collision checks for the player's rigid body, so
     * raycasts don't collide with ourselves.
     */
    unsigned storeCollisionMask = body_->GetCollisionMask();
    body_->SetCollisionMask(0);

        // Cast a ray that is as long as the player's standing height
        float rayCastLength = playerClass.body.height;
        Vector3 upDirection = moveNode_->GetRotation() * Vector3::UP;
        Vector3 rayOrigin = moveNode_->GetWorldPosition();

        // Cast upwards. If it hits anything then we can't stand up.
        PhysicsRaycastResult result;
        Ray ray(rayOrigin, upDirection);
        physicsWorld_->RaycastSingle(result, ray, rayCastLength);
        if(result.distance_ < rayCastLength)
            canStandUp = false;

    body_->SetCollisionMask(storeCollisionMask);

    return canStandUp;
}

// ----------------------------------------------------------------------------
bool MovementController::IsCrouching() const
{
    return collisionShapeCrouch_->IsEnabled();
}

// ----------------------------------------------------------------------------
void MovementController::HandleNodeCollision(StringHash /*eventType*/, VariantMap& /*eventData*/)
{
    /*
     * The point of this event handler is to reset the player's downVelocity to
     * 0.0f if the player hits anything along the axis of the "down velocity
     * vector" (so either the ground or the roof). This axis changes depending
     * on the current gravity vector.
     *
     * Other code relies on downVelocity_ == 0.0f (such as initiating a jump).
     */

    using namespace NodeCollision;

    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    const IceWeaselConfig::Data::PlayerClass& playerClass = config.playerClass(0);

    /*
     * Temporarily disable collision checks for the player's rigid body, so
     * raycasts don't collide with ourselves.
     * */
    unsigned int storeCollisionMask = body_->GetCollisionMask();
    body_->SetCollisionMask(0);

        // Ray length depends on whether player is standing or crouching
        float rayCastLength = playerClass.body.height;
        if(IsCrouching())
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

        // Cast a ray up and check if we're hitting anything with our head
        ray = Ray(moveNode_->GetWorldPosition(), -downDirection);
        physicsWorld_->RaycastSingle(result, ray, rayCastLength);
        if(result.distance_ < rayCastLength)
            if(downVelocity_ >= 0.0f)
                downVelocity_ = 0.0f;

    // Restore collision mask
    body_->SetCollisionMask(storeCollisionMask);
}

// ----------------------------------------------------------------------------
void MovementController::HandleConfigReloaded(StringHash /*eventType*/, VariantMap& /*eventData*/)
{
    UpdatePhysicsSettings();
}

// ----------------------------------------------------------------------------
void MovementController::UpdatePhysicsSettings()
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    // Update collision shapes
    collisionShapeUpright_->SetCapsule(config.playerClass(0).body.width,
                                       config.playerClass(0).body.height,
                                       Vector3(0, config.playerClass(0).body.height / 2, 0));
    collisionShapeCrouch_->SetCapsule(config.playerClass(0).body.crouchWidth,
                                      config.playerClass(0).body.crouchHeight,
                                      Vector3(0, config.playerClass(0).body.crouchHeight / 2, 0));

    // Update rigid body
    body_->SetMass(config.playerClass(0).body.mass);

    SetInitialPhysicsParameters();
}

// ----------------------------------------------------------------------------
void MovementController::SetInitialPhysicsParameters()
{
    downVelocity_ = 0.0f;
}
