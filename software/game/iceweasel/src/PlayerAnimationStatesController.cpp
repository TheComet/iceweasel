#include "iceweasel/PlayerAnimationStatesController.h"
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
PlayerAnimationStatesController::PlayerAnimationStatesController(Context* context) :
    LogicComponent(context)
{
}

// ----------------------------------------------------------------------------
void PlayerAnimationStatesController::Start()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    AnimatedModel* aniModel = node_->GetComponent<AnimatedModel>();
    if(!aniModel)
        return;

    /*
     * Get all animations from the animated model. We assume they are in the
     * same order as in the enum.
     */
    for(unsigned i = 0; i != IceWeaselConfig::NUM_ANIMATIONS; ++i)
    {
        animation_[i] = aniModel->GetAnimationState(i);
        animationFactor_[i].value_ = 0;
        animationFactor_[i].SetTarget(0);
    }

    // Used to control transitions between idle/walk/run/hop animations
    SubscribeToEvent(E_LOCALMOVEMENTVELOCITYCHANGED, URHO3D_HANDLER(PlayerAnimationStatesController, HandleLocalMovementVelocityChanged));
    SubscribeToEvent(E_CROUCHSTATECHANGED, URHO3D_HANDLER(PlayerAnimationStatesController, HandleCrouchStateChanged));
}

// ----------------------------------------------------------------------------
void PlayerAnimationStatesController::Update(float timeStep)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    float velocitySquared = Vector2(currentLocalVelocity_.x_, currentLocalVelocity_.z_).LengthSquared();

    for(unsigned i = 0; i != IceWeaselConfig::NUM_ANIMATIONS; ++i)
        animationFactor_[i].SetTarget(0);

    if(isCrouching_)
        HandleCrouchStates(velocitySquared);
    else
        HandleGroundStates(velocitySquared);

    for(unsigned i = 0; i != IceWeaselConfig::NUM_ANIMATIONS; ++i)
        if(animation_[i])
        {
            animationFactor_[i].Advance(timeStep * config.playerClass(0).animations[i].transitionSpeed);
            animation_[i]->SetWeight(animationFactor_[i].value_);
            animation_[i]->AddTime(timeStep * config.playerClass(0).animations[i].speed);
        }
}

// ----------------------------------------------------------------------------
void PlayerAnimationStatesController::HandleGroundStates(float velocitySquared)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    float walkFastSpeedSquared = config.playerClass(0).speed.walk * config.playerClass(0).speed.walk;
    float walkSlowSpeedSquared = config.playerClass(0).speed.walk / 4; // half the speed of fast
    float runSpeedSquared = config.playerClass(0).speed.run * config.playerClass(0).speed.run;


    animationFactor_[IceWeaselConfig::IDLE].SetTarget(1);
    if(velocitySquared <= walkSlowSpeedSquared)
    {
        float factor = velocitySquared / walkSlowSpeedSquared;
        animationFactor_[IceWeaselConfig::WALK].SetTarget(factor);
        animationFactor_[IceWeaselConfig::RUN].SetTarget(0);
        animationFactor_[IceWeaselConfig::SPRINT].SetTarget(0);
    }
    else if(velocitySquared <= walkFastSpeedSquared)
    {
        float factor = (velocitySquared - walkSlowSpeedSquared) / (walkFastSpeedSquared - walkSlowSpeedSquared);
        animationFactor_[IceWeaselConfig::WALK].SetTarget(1.0f - factor);
        animationFactor_[IceWeaselConfig::RUN].SetTarget(factor);
        animationFactor_[IceWeaselConfig::SPRINT].SetTarget(0);
    }
    else
    {
        float factor = (velocitySquared - walkFastSpeedSquared) / (runSpeedSquared - walkFastSpeedSquared);
        animationFactor_[IceWeaselConfig::WALK].SetTarget(0);
        animationFactor_[IceWeaselConfig::RUN].SetTarget(1.0f - factor);
        animationFactor_[IceWeaselConfig::SPRINT].SetTarget(factor);
    }
}

// ----------------------------------------------------------------------------
void PlayerAnimationStatesController::HandleCrouchStates(float velocitySquared)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    float crouchSpeedSquared = config.playerClass(0).speed.crouch * config.playerClass(0).speed.crouch;

    /*
     * Because crouch-walking is ordered after normal crouching, it has
     * precedence.
     */
    float factor = Min(1.0f, velocitySquared / crouchSpeedSquared);
    animationFactor_[IceWeaselConfig::CROUCH].SetTarget(1);
    animationFactor_[IceWeaselConfig::CROUCH_WALK].SetTarget(factor);
}

// ----------------------------------------------------------------------------
void PlayerAnimationStatesController::HandleLocalMovementVelocityChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace LocalMovementVelocityChanged;
    currentLocalVelocity_ = eventData[P_LOCALMOVEMENTVELOCITY].GetVector3();
}

// ----------------------------------------------------------------------------
void PlayerAnimationStatesController::HandleCrouchStateChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace CrouchStateChanged;
    isCrouching_ = eventData[P_CROUCHING].GetBool();
}
