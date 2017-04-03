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
    respawnDistance_(100.0f),
    jumpKeyPressed_(false),
    crouchKeyPressed_(false),
    isSwimming_(false)
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
    // Cache frequently used subsystems/scene components
    input_ = GetSubsystem<Input>();
    gravityManager_ = GetScene()->GetOrCreateComponent<GravityManager>();
    physicsWorld_ = GetScene()->GetOrCreateComponent<PhysicsWorld>();

    // Set up things
    CreateComponents();

    // Initial physics parameters
    moveNode_->SetRotation(Quaternion::IDENTITY);

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
    Update_Ground(timeStep);
    //Update_Water(timeStep);
}

// ----------------------------------------------------------------------------
void MovementController::FixedUpdate(float timeStep)
{
    FixedUpdate_Ground(timeStep);
    //FixedUpdate_Water(timeStep);
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
void MovementController::Update_Ground(float timeStep)
{
    (void)timeStep;

    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

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
void MovementController::FixedUpdate_Ground(float timeStep)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    const IceWeaselConfig::Data::PlayerClass& playerClass = config.playerClass(0);

    /*
     * Some notes on the following code.
     *  + Through experimenting I have found the best method of controlling a
     *    physics body is to modify its current linear velocity. There are
     *    other implementations that act on its acceleration instead, but those
     *    seem to be less stable and harder to control.
     *
     *  + The directional input (e.g. WASD) is assumed to be jerk/jounce (i.e.
     *    the fourth derivative of location). This is integrated once using a
     *    negative exponential function to get acceleration, which in turn is
     *    integrated a second time and added to the current linear velocity of
     *    the physics body. The reasons for this double-integration approach
     *    are:
     *      - The X/Z rotation code for tilting the player in the direction of
     *        acceleration is a lot smoother this way.
     *      - It's a little more realistic.
     */

    // Change collision shape depending on whether the player is crouching
    // This needs to happen in FixedUpdate().
    if(input_->GetKeyDown(KEY_CTRL))
    {
        if(crouchKeyPressed_ == false)
        {
            crouchKeyPressed_ = true;
            collisionShapeCrouch_->SetEnabled(true);
            collisionShapeUpright_->SetEnabled(false);

            NotifyCrouchStateChange(true);
        }
    }
    else
        crouchKeyPressed_ = false;

    if(IsCrouching() && crouchKeyPressed_ == false)
    {
        if(CanStandUp())
        {
            collisionShapeCrouch_->SetEnabled(false);
            collisionShapeUpright_->SetEnabled(true);

            NotifyCrouchStateChange(false);
        }
    }

    /*
     * Do some raycasts to figure out if we are on the ground (or hitting our
     * head on something). If so, reset the down velocity to 0.0f.
     *
     * A down velocity of 0.0f means we are on the ground.
     */
    bool isOnGround = ResetDownVelocityIfOnGround();

    /*
     * Get input direction vector from WASD on keyboard and store in x and z
     * components. This vector is normalized.
     *
     * Player speed is "walk" by default, "run" if control is pressed and
     * "crawl" if shift is pressed (precedence on shift, you can't run and
     * crawl).
     */
    float speed = playerClass.speed.walk;
    Vector3 jounce(Vector3::ZERO);
    if(input_->GetKeyDown(KEY_SHIFT))
        speed = playerClass.speed.run;
    if(input_->GetKeyDown(KEY_CTRL) || IsCrouching())
        speed = playerClass.speed.crouch;
    if(input_->GetKeyDown(KEY_W))     jounce.z_ += 1;
    if(input_->GetKeyDown(KEY_S))     jounce.z_ -= 1;
    if(input_->GetKeyDown(KEY_A))     jounce.x_ += 1;
    if(input_->GetKeyDown(KEY_D))     jounce.x_ -= 1;
    if(jounce.x_ != 0 || jounce.z_ != 0)
        jounce = jounce.Normalized();

    // Rotate input direction by camera angle using a 3D rotation matrix
    jounce = Matrix3(
        -Cos(cameraAngle_.y_), 0, Sin(cameraAngle_.y_),
        0, 1, 0,
        Sin(cameraAngle_.y_), 0, Cos(cameraAngle_.y_)
    ) * jounce;

    // Approach acceleration smoothly using the input vector.
    localPlaneAcceleration_.SetTarget(jounce * speed);
    localPlaneAcceleration_.Advance(timeStep * playerClass.speed.jounceSpeed);

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
    ExponentialCurve<Vector3> localPlaneVelocity(
        velocityTransform.Inverse() * body_->GetLinearVelocity(), Vector3::ZERO);

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
        if(isOnGround)
        {
            downVelocity_ = playerClass.jump.force;
            // Give the player a slight speed boost so he moves faster than usual
            // in the air.
            localPlaneVelocity.value_ *= playerClass.jump.bunnyHopBoost;
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
    localPlaneVelocity.SetTarget(localPlaneAcceleration_.value_);
    if(downVelocity_ == 0.0f)
        localPlaneVelocity.Advance(timeStep * playerClass.speed.accelerateSpeed);
    else
        localPlaneVelocity.Advance(timeStep);

    NotifyLocalMovementVelocityChange(Vector3(
        localPlaneVelocity.value_.x_,
        downVelocity_,
        localPlaneVelocity.value_.z_
    ));

    // Integrate downwards velocity if in air and apply velocity back to body.
    // We don't integrate if on the ground so the player doesn't slide slowly.
    if(isOnGround == false)
        downVelocity_ -= gravity.Length() * timeStep;
    body_->SetLinearVelocity(velocityTransform *
            Vector3(
                localPlaneVelocity.value_.x_,
                downVelocity_,
                localPlaneVelocity.value_.z_
            )
    );

    // Smoothly rotate to target orientation
    static const float correctCameraAngleSpeed = 5.0f;
    currentRotation_ = currentRotation_.Nlerp(gravityRotation, timeStep * correctCameraAngleSpeed, true);
    moveNode_->SetRotation(-currentRotation_);
}

// ----------------------------------------------------------------------------
void MovementController::Update_Water(float timeStep)
{
}

// ----------------------------------------------------------------------------
void MovementController::FixedUpdate_Water(float timeStep)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    const IceWeaselConfig::Data::PlayerClass& playerClass = config.playerClass(0);

    /*
     * Get input direction vector from WASD on keyboard and store in x and z
     * components. This vector is normalized.
     *
     * Player speed is "walk" by default, "run" if control is pressed and
     * "crawl" if shift is pressed (precedence on shift, you can't run and
     * crawl).
     */
    float speed = playerClass.speed.walk;
    Vector3 jounce(Vector3::ZERO);
    if(input_->GetKeyDown(KEY_SHIFT))
        speed = playerClass.speed.run;
    if(input_->GetKeyDown(KEY_CTRL) || IsCrouching())
        speed = playerClass.speed.crouch;
    if(input_->GetKeyDown(KEY_W))     jounce.z_ += 1;
    if(input_->GetKeyDown(KEY_S))     jounce.z_ -= 1;
    if(input_->GetKeyDown(KEY_A))     jounce.x_ += 1;
    if(input_->GetKeyDown(KEY_D))     jounce.x_ -= 1;
    if(jounce.x_ != 0 || jounce.z_ != 0)
        jounce = jounce.Normalized();

    // Rotate input direction by camera angle using a 3D rotation matrix
    jounce = Matrix3(
        -Cos(cameraAngle_.y_), 0, Sin(cameraAngle_.y_),
        0, 1, 0,
        Sin(cameraAngle_.y_), 0, Cos(cameraAngle_.y_)
    ) * Matrix3(
        1, 0, 0,
        0, Cos(cameraAngle_.x_), -Sin(cameraAngle_.x_),
        0, Sin(cameraAngle_.x_), Cos(cameraAngle_.x_)
    ) * jounce;

    // Approach acceleration smoothly using the input vector.
    localPlaneAcceleration_.SetTarget(jounce * speed);
    localPlaneAcceleration_.Advance(timeStep * playerClass.speed.jounceSpeed);

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
    ExponentialCurve<Vector3> localPlaneVelocity(
        velocityTransform.Inverse() * body_->GetLinearVelocity(), Vector3::ZERO);

    // Under water, down velocity is always constant
    downVelocity_ = -0.04f * timeStep;
    /*
     * X/Z movement of the player is only possible when on the ground. If the
     * player is in the air just maintain whatever velocity he currently has,
     * but also add the target velocity on top of that so the player can
     * slightly control his movement in the air.
     */
    localPlaneVelocity.SetTarget(localPlaneAcceleration_.value_);
    localPlaneVelocity.Advance(timeStep * playerClass.speed.accelerateSpeed);

    NotifyLocalMovementVelocityChange(localPlaneVelocity.value_);

    // Integrate downwards velocity and apply velocity back to body.
    downVelocity_ -= gravity.Length() * timeStep;
    body_->SetLinearVelocity(velocityTransform * localPlaneVelocity.value_);

    // Smoothly rotate to target orientation
    static const float correctCameraAngleSpeed = 5.0f;
    currentRotation_ = currentRotation_.Nlerp(gravityRotation, timeStep * correctCameraAngleSpeed, true);
    moveNode_->SetRotation(-currentRotation_);
}

// ----------------------------------------------------------------------------
bool MovementController::ResetDownVelocityIfOnGround()
{
    using namespace NodeCollision;

    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    const IceWeaselConfig::Data::PlayerClass& playerClass = config.playerClass(0);

    bool isOnGround = false;

    /*
     * Temporarily disable collision checks for the player's rigid body, so
     * raycasts don't collide with ourselves.
     * */
    unsigned int storeCollisionMask = body_->GetCollisionMask();
    body_->SetCollisionMask(0);

        // Ray length depends on whether player is standing or crouching
        float bodyHeight = playerClass.body.height;
        float bodyRadius = playerClass.body.width / 2;
        if(IsCrouching())
            bodyHeight = playerClass.body.crouchHeight;
        if(IsCrouching())
            bodyRadius = playerClass.body.crouchWidth / 2;

        // Radius slightly less than body width/2
        float castRadius = bodyRadius / 1.1f;

        // Because we're sphere casting, and the ray length goes to the center
        // of the sphere and not to the edge, subtract cast width from the
        // length
        float rayCastLength = bodyHeight - castRadius;

        // Cast slightly beyond
        const float extendFactor = 1.1f;
        rayCastLength *= extendFactor;

        // Cast a ray down and check if we're on the ground
        PhysicsRaycastResult result;
        Vector3 downDirection = moveNode_->GetRotation() * Vector3::DOWN;
        Vector3 rayOrigin(moveNode_->GetWorldPosition() - downDirection * bodyHeight); // cast from top of capsule
        Ray ray(rayOrigin, downDirection);
        physicsWorld_->SphereCast(result, ray, castRadius, rayCastLength);
        if(result.distance_ < rayCastLength)
        {
            isOnGround = true;

            if(downVelocity_ <= 0.0f)
                downVelocity_ = 0.0f;

            /*
             * When the player is running down a slope, he will start
             * oscillating between falling/landing. Since we know we're
             * touching the ground, just translate the player downwards so he's
             * actually touching the ground.
             * Note that this shouldn't be done if the player's velocity is
             * moving upwards.
             */
            if(downVelocity_ <= 0.0f)
            {
                physicsWorld_->RaycastSingle(result, ray, bodyHeight*1.1f);
                if(result.distance_ < bodyHeight * 1.1f)
                    moveNode_->SetPosition(result.position_);
            }
        }

        // Cast a ray up and check if we're hitting anything with our head
        ray = Ray(moveNode_->GetWorldPosition(), -downDirection);
        physicsWorld_->RaycastSingle(result, ray, rayCastLength);
        if(result.distance_ < rayCastLength)
            if(downVelocity_ >= 0.0f)
                downVelocity_ = 0.0f;

    // Restore collision mask
    body_->SetCollisionMask(storeCollisionMask);

    return isOnGround;
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

// ----------------------------------------------------------------------------
void MovementController::HandleCameraAngleChanged(StringHash /*eventType*/, VariantMap& eventData)
{
    cameraAngle_.x_ = eventData[CameraAngleChanged::P_ANGLEX].GetFloat();
    cameraAngle_.y_ = eventData[CameraAngleChanged::P_ANGLEY].GetFloat();
}

// ----------------------------------------------------------------------------
void MovementController::HandleConfigReloaded(StringHash /*eventType*/, VariantMap& /*eventData*/)
{
    UpdatePhysicsSettings();
}

// ----------------------------------------------------------------------------
void MovementController::NotifyLocalMovementVelocityChange(const Urho3D::Vector3& localPlaneVelocity)
{
    using namespace LocalMovementVelocityChanged;
    VariantMap& eventData = GetEventDataMap();
    eventData[P_LOCALMOVEMENTVELOCITY] = localPlaneVelocity;
    SendEvent(E_LOCALMOVEMENTVELOCITYCHANGED, eventData);
}

// ----------------------------------------------------------------------------
void MovementController::NotifyCrouchStateChange(bool isCrouching)
{
    using namespace CrouchStateChanged;
    VariantMap& eventData = GetEventDataMap();
    eventData[P_CROUCHING] = isCrouching;
    SendEvent(E_CROUCHSTATECHANGED, eventData);
}
