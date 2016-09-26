#include "iceweasel/PlayerController.h"
#include "iceweasel/CameraControllerRotation.h"
#include "iceweasel/CameraControllerEvents.h"
#include "iceweasel/MovementController.h"
#include "iceweasel/Math.h"
#include "iceweasel/Finger.h"
#include "iceweasel/IceWeaselConfig.h"
#include "iceweasel/IceWeaselConfigEvents.h"
#include "iceweasel/GravityManager.h"

#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/SceneEvents.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
PlayerController::PlayerController(Context* context, Node* moveNode, Node* offsetNode, Node* rotateNode) :
    LogicComponent(context),
    moveNode_(moveNode),
    offsetNode_(offsetNode),
    rotateNode_(rotateNode),
    currentYAngle_(0.0f)
{
}

// ----------------------------------------------------------------------------
void PlayerController::SetMode(Mode mode)
{
    mode_ = mode;
}

// ----------------------------------------------------------------------------
void PlayerController::Start()
{
    CreateComponents();

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* modelNodeXML = cache->GetResource<XMLFile>("Models/Kangaroo.xml");
    modelNode_ = moveNode_->CreateChild("Player");
    modelNode_->LoadXML(modelNodeXML->GetRoot());
    AnimatedModel* aniModel = modelNode_->GetComponent<AnimatedModel>();
    if(aniModel)
        for(unsigned i = 0; i != NUM_ANIMATIONS; ++i)
            animation_[i] = aniModel->GetAnimationState(i);

    // Initial camera offset
    cameraOffset_.SetTarget(-4);

    SubscribeToEvent(E_LOCALMOVEMENTVELOCITYCHANGED, URHO3D_HANDLER(PlayerController, HandleLocalMovementVelocityChanged));
}

// ----------------------------------------------------------------------------
void PlayerController::Stop()
{
    modelNode_->Remove();

    DestroyComponents();
}

// ----------------------------------------------------------------------------
void PlayerController::CreateComponents()
{
    // Add the movement controller to the movement node
    moveNode_->AddComponent(new MovementController(context_, moveNode_, offsetNode_), 0, LOCAL);

    // The mouse controls the angle of camera node
    offsetNode_->AddComponent(new CameraControllerRotation(context_), 0, LOCAL);

    // Attach hand model
    rotateNode_->AddComponent(new Finger(context_), 0, LOCAL);
}

// ----------------------------------------------------------------------------
void PlayerController::DestroyComponents()
{
    // Clean up the components we added
    rotateNode_->RemoveComponent<Finger>();
    offsetNode_->RemoveComponent<CameraControllerRotation>();
    rotateNode_->RemoveComponent<MovementController>();
}

// ----------------------------------------------------------------------------
void PlayerController::Update(float timeStep)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    // Control camera offset
    static const float cameraSmooth = 5;
    rotateNode_->SetPosition(Vector3(0, 0, cameraOffset_.Advance(timeStep * cameraSmooth)));

    // X and Z rotate model depending on acceleration
    Vector2 targetAcceleration = (
        Vector2(currentLocalVelocity_.x_, currentLocalVelocity_.z_) -
        Vector2(newLocalVelocity_.x_, newLocalVelocity_.z_)) / Max(M_EPSILON, timeStep);
    targetAcceleration *= config.playerClass(0).lean.amount;
    currentAcceleration_ = (targetAcceleration - currentAcceleration_) * Min(1.0f, config.playerClass(0).lean.speed * timeStep);

    Quaternion targetRotation = Quaternion(currentAcceleration_.x_, Vector3::FORWARD);
    targetRotation = targetRotation * Quaternion(currentAcceleration_.y_, Vector3::LEFT);

    // Y rotate model in local space towards the direction it is moving
    float newAngle = Atan2(currentLocalVelocity_.x_, currentLocalVelocity_.z_);
    if(Abs(currentLocalVelocity_.x_) + Abs(currentLocalVelocity_.z_) > M_EPSILON)
        targetRotation = targetRotation * Quaternion(newAngle, Vector3::UP);

    modelNode_->SetRotation(modelNode_->GetRotation().Nlerp(targetRotation, Min(1.0f, config.playerClass(0).turn.speed * timeStep), true));

    animationWalkFactor_.SetTarget(Vector2(currentLocalVelocity_.x_, currentLocalVelocity_.z_).LengthSquared() /
            (config.playerClass(0).speed.walk * config.playerClass(0).speed.walk));
    animationWalkFactor_.Advance(timeStep * 12.0f);
    if(animation_[WALK_FAST])
        animation_[WALK_FAST]->SetWeight(animationWalkFactor_.GetValue());
    if(animation_[IDLE])
        animation_[IDLE]->SetWeight(1.0f - animationWalkFactor_.GetValue());

    for(unsigned i = 0; i != NUM_ANIMATIONS; ++i)
        if(animation_[i])
            animation_[i]->AddTime(timeStep);

    currentLocalVelocity_ = newLocalVelocity_;

}

// ----------------------------------------------------------------------------
void PlayerController::HandleLocalMovementVelocityChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace LocalMovementVelocityChanged;
    newLocalVelocity_ = eventData[P_LOCALMOVEMENTVELOCITY].GetVector3();
}
