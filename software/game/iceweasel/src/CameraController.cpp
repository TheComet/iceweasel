#include "iceweasel/CameraController.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IceWeaselMods/Gravity.h>
#include <Urho3D/Math/Matrix2.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>


#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>

using namespace Urho3D;


// ----------------------------------------------------------------------------
float Wrap(float angle)
{
    while(angle < -180.0f)
        angle += 360.0f;
    while(angle > 180.0f)
        angle -= 360.0f;
    return angle;
}


// ----------------------------------------------------------------------------
CameraController::CameraController(Context* context,
                                   Node* moveNode,
                                   Node* rotateNode,
                                   Mode mode) :
    LogicComponent(context),
    moveNode_(moveNode),
    rotateNode_(rotateNode),
    downVelocity_(0.0f),
    playerParameters_({80.0f, 1.8f, 0.8f, 5.0f, 1.25f}), // weight, height, width, jump force, jump speed boost factor
    // TODO Read these values from an XML file
    // TODO Auto-reload XML file when it is edited
    mode_(mode)
{
}

// ----------------------------------------------------------------------------
void CameraController::SetMode(CameraController::Mode mode)
{
    // Clean up if switching from FPS mode
    if(mode_ == FPS)
    {
        moveNode_->RemoveComponent<RigidBody>();
        moveNode_->RemoveComponent<CollisionShape>();
        UnsubscribeFromEvent(E_NODECOLLISION);
    }

    if(mode == FPS)
    {
        // Set up player collision shape and rigid body for FPS mode
        CollisionShape* colShape = moveNode_->CreateComponent<CollisionShape>();
        RigidBody* body = moveNode_->CreateComponent<RigidBody>();
        colShape->SetCapsule(playerParameters_.width,
                            playerParameters_.height,
                            Vector3(0, -playerParameters_.height / 2, 0));
        body->SetAngularFactor(Vector3::ZERO);
        body->SetMass(playerParameters_.mass);
        body->SetFriction(0.0f);

        // Need to listen to node collision events to reset gravity
        SubscribeToEvent(E_NODECOLLISION, URHO3D_HANDLER(CameraController, HandleNodeCollision));

        // Initial physics parameters
        downVelocity_ = 0.0f;
    }

    mode_ = mode;
}

// ----------------------------------------------------------------------------
void CameraController::Start()
{
    input_ = GetSubsystem<Input>();
    physicsWorld_ = GetScene()->GetComponent<PhysicsWorld>();
    gravity_ = GetScene()->GetComponent<Gravity>();

    // Set initial rotation to current camera angle
    const Vector3& cameraRotationEuler = rotateNode_->GetRotation().EulerAngles();
    angleX_ = Wrap(cameraRotationEuler.x_);
    angleY_ = Wrap(cameraRotationEuler.y_);

    // Creates collision shape and rigid body
    SetMode(mode_);

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CameraController, HandleUpdate));

    // Debug text
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    gravityDebugText_ = ui->GetRoot()->CreateChild<Text>();
    gravityDebugText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 12);
    gravityDebugText_->SetHorizontalAlignment(HA_LEFT);
    gravityDebugText_->SetVerticalAlignment(VA_CENTER);
    gravityDebugText_->SetPosition(10, 10);
}

// ----------------------------------------------------------------------------
void CameraController::Stop()
{
    // Clean up the components we added
    moveNode_->RemoveComponent<RigidBody>();
    moveNode_->RemoveComponent<CollisionShape>();

    // Debug text
    UI* ui = GetSubsystem<UI>();
    ui->GetRoot()->RemoveChild(gravityDebugText_);
}

// ----------------------------------------------------------------------------
void CameraController::UpdateCameraRotation()
{
    // Calculate new camera angle according to mouse movement
    float sensitivity = 0.5f;  // TODO load this from an XML config file.
    const IntVector2& mouseMove = input_->GetMouseMove();

    angleX_ = angleX_ + mouseMove.y_ * sensitivity;
    angleY_ = Wrap(angleY_ + mouseMove.x_ * sensitivity);

    if(angleX_ < -90.0f) angleX_ = -90.0f;
    if(angleX_ > 90.0f)  angleX_ = 90.0f;

    rotateNode_->SetRotation(Quaternion(angleX_, angleY_, 0));
}

// ----------------------------------------------------------------------------
void CameraController::UpdateFPSCameraMovement(float timeStep)
{
    RigidBody* body = moveNode_->GetComponent<RigidBody>();
    //TODO transform into local gravity planeVelocity_ = body->GetLinearVelocity();

    // Get input direction vector
    float speed = 8.0f; // TODO read this from an XML config file
    Vector3 targetPlaneVelocity(Vector3::ZERO);
    if(input_->GetKeyDown(KEY_W))     targetPlaneVelocity.z_ += 1;
    if(input_->GetKeyDown(KEY_S))     targetPlaneVelocity.z_ -= 1;
    if(input_->GetKeyDown(KEY_A))     targetPlaneVelocity.x_ += 1;
    if(input_->GetKeyDown(KEY_D))     targetPlaneVelocity.x_ -= 1;
    if(targetPlaneVelocity.x_ != 0 || targetPlaneVelocity.z_ != 0)
        targetPlaneVelocity = targetPlaneVelocity.Normalized() * speed;

    // Rotate input direction by camera angle using a 3D rotation matrix
    targetPlaneVelocity = Matrix3(-Cos(angleY_), 0, Sin(angleY_),
                              0, 1, 0,
                              Sin(angleY_), 0, Cos(angleY_)) * targetPlaneVelocity;

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

    // TODO Take upwards velocity into account when bunny hopping (e.g. on ramps)

    // TODO Add a "lifter" collision sphere to handle steps or other sharp edges.

    // TODO Collision feedback needs to affect planeVelocity_ and downVelocity_

    // TODO Limit bunny hopping speed.

    // smoothly approach target direction if we're on the ground. Otherwise
    // just maintain whatever plane velocity we had previously.
    float smoothness = 16.0f;
    if(downVelocity_ == 0.0f)
        planeVelocity_ += (targetPlaneVelocity - planeVelocity_) * timeStep * smoothness;


    // Get gravity and rotate our velocity vector according to the direction of gravity
    Vector3 gravity = gravity_->QueryGravity(node_->GetWorldPosition());
    float gravityForce = gravity.Length();
    Quaternion gravityRotation(Vector3::DOWN, gravity);
    Vector3 velocity = gravityRotation.RotationMatrix() * Vector3(planeVelocity_.x_, downVelocity_, planeVelocity_.z_);

    // TODO Fix sliding! This is ugly as shit. PhysicsWorld gravity is temporarily set to 0 until this is fixed
    if(downVelocity_ != 0.0f)
        velocity += gravity * timeStep;

    // Integrate velocity
    downVelocity_ -= gravityForce * timeStep;

    // update camera position
    body->SetLinearVelocity(velocity);

    // Smoothly rotate camera to target rotation
    gravityRotation_ = gravityRotation_.Nlerp(gravityRotation, Min(1.0f, 8.0f * timeStep), true);
    moveNode_->SetRotation(-gravityRotation_);
}

// ----------------------------------------------------------------------------
void CameraController::UpdateFreeCameraMovement(float timeStep)
{
    // get input direction
    float speed = 0.3f; // TODO Load these from an XML config file.
    float smoothness = 4.0f;
    Vector3 targetPlaneVelocity(Vector3::ZERO);
    if(input_->GetKeyDown(KEY_SHIFT)) speed *= 5; // speed boost
    if(input_->GetKeyDown(KEY_W))     targetPlaneVelocity.z_ += 1;
    if(input_->GetKeyDown(KEY_S))     targetPlaneVelocity.z_ -= 1;
    if(input_->GetKeyDown(KEY_A))     targetPlaneVelocity.x_ += 1;
    if(input_->GetKeyDown(KEY_D))     targetPlaneVelocity.x_ -= 1;
    if(input_->GetKeyDown(KEY_SPACE)) targetPlaneVelocity.y_ += 1;
    if(input_->GetKeyDown(KEY_CTRL))  targetPlaneVelocity.y_ -= 1;
    if(targetPlaneVelocity.x_ != 0 || targetPlaneVelocity.y_ != 0 || targetPlaneVelocity.z_ != 0)
        targetPlaneVelocity = targetPlaneVelocity.Normalized() * speed;

    // rotate input direction by camera angle using a 3D rotation matrix
    targetPlaneVelocity = Matrix3(-Cos(angleY_), 0, Sin(angleY_),
                              0, 1, 0,
                              Sin(angleY_), 0, Cos(angleY_)) * targetPlaneVelocity;

    // smoothly approach target direction
    planeVelocity_ += (targetPlaneVelocity - planeVelocity_) * timeStep * smoothness;

    // update camera position
    moveNode_->Translate(planeVelocity_, Urho3D::TS_WORLD);
}

// ----------------------------------------------------------------------------
void CameraController::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    (void)eventType;

    double timeStep = eventData[P_TIMESTEP].GetDouble();

    if(gravity_)
    {
        Vector3 gravity = gravity_->QueryGravity(node_->GetWorldPosition());
        gravityDebugText_->SetText(String("Gravity: ") + String(gravity.x_) + "," + String(gravity.y_) + "," + String(gravity.z_));
    }

    UpdateCameraRotation();

    if(mode_ == FPS)
        return UpdateFPSCameraMovement(timeStep);
    if(mode_ == FREE)
        return UpdateFreeCameraMovement(timeStep);
}

#include <Urho3D/Graphics/DebugRenderer.h>

// ----------------------------------------------------------------------------
void CameraController::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
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
        Vector3 downDirection = moveNode_->GetRotation() * Vector3::DOWN;
        Ray ray(moveNode_->GetWorldPosition(), downDirection);
        physicsWorld_->RaycastSingle(result, ray, rayCastLength);
        if(result.distance_ < rayCastLength)
            // Reset player's Y velocity
            downVelocity_ = 0.0f;

    // Restore collision mask
    body->SetCollisionMask(storeCollisionMask);

    // Mark where the ray is being cast to
    DebugRenderer* r = GetScene()->GetComponent<DebugRenderer>();
    if(r)
        r->AddSphere(Sphere(moveNode_->GetWorldPosition() + downDirection, 0.1), Color::RED);
}
