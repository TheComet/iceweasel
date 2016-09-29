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
    LogicComponent(context),
    state_(ON_GROUND)
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
        animationWeight_[i].value_ = 0;
        animationWeight_[i].SetTarget(0);
    }

    // jump animations are not looped
    if(animation_[IceWeaselConfig::JUMP_OFF])
        animation_[IceWeaselConfig::JUMP_OFF]->SetLooped(false);
    if(animation_[IceWeaselConfig::JUMP_LAND])
        animation_[IceWeaselConfig::JUMP_LAND]->SetLooped(false);

    // Used to control transitions between idle/walk/run/hop animations
    SubscribeToEvent(E_LOCALMOVEMENTVELOCITYCHANGED, URHO3D_HANDLER(PlayerAnimationStatesController, HandleLocalMovementVelocityChanged));
    SubscribeToEvent(E_CROUCHSTATECHANGED, URHO3D_HANDLER(PlayerAnimationStatesController, HandleCrouchStateChanged));
}

// ----------------------------------------------------------------------------
void PlayerAnimationStatesController::Update(float timeStep)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    float velocitySquared = Vector2(currentLocalVelocity_.x_, currentLocalVelocity_.z_).LengthSquared();

    /*
     * Reset all of the animation weights every time we update, so we don't
     * have to care about resetting it during transitions.
     */
    for(unsigned i = 0; i != IceWeaselConfig::NUM_ANIMATIONS; ++i)
        animationWeight_[i].SetTarget(0);

    switch(state_)
    {
        case ON_GROUND:
            HandleGroundWeights(velocitySquared);

            if(currentLocalVelocity_.y_ > 0.0f)
                state_ = JUMP_BEGIN;
            if(currentLocalVelocity_.y_ < 0.0f)
                state_ = JUMP_FALL;

            break;

        case CROUCHING:
            HandleCrouchWeights(velocitySquared);

            if(currentLocalVelocity_.y_ > 0.0f)
                state_ = JUMP_BEGIN;
            if(currentLocalVelocity_.y_ < 0.0f)
                state_ = JUMP_FALL;

            break;

        case JUMP_BEGIN:
            if(animation_[IceWeaselConfig::JUMP_OFF])
                animation_[IceWeaselConfig::JUMP_OFF]->SetTime(0);
            state_ = JUMP_OFF;

            // fall through on purpose

        case JUMP_OFF:
            animationWeight_[IceWeaselConfig::JUMP_OFF].SetTarget(1);

            if(currentLocalVelocity_.y_ <= 0.0f) // now falling
                state_ = JUMP_FALL;

            break;

        case JUMP_FALL:
            animationWeight_[IceWeaselConfig::JUMP_OFF].SetTarget(1);
            animationWeight_[IceWeaselConfig::JUMP_LAND].SetTarget(1);

            // keep landing animation in reset pose, looks like falling
            if(animation_[IceWeaselConfig::JUMP_LAND])
                    animation_[IceWeaselConfig::JUMP_LAND]->SetTime(0);

            if(currentLocalVelocity_.y_ == 0.0f) // landed
                state_ = JUMP_LAND;

            break;

        case JUMP_LAND:
            animationWeight_[IceWeaselConfig::JUMP_LAND].SetTarget(1);
            animationWeight_[IceWeaselConfig::JUMP_OFF].value_ = 0;
            if(animation_[IceWeaselConfig::JUMP_LAND] == NULL ||
                animation_[IceWeaselConfig::JUMP_LAND]->GetTime() >= animation_[IceWeaselConfig::JUMP_LAND]->GetLength())
            {
                state_ = ON_GROUND;
            }

            break;
    }

    for(unsigned i = 0; i != IceWeaselConfig::NUM_ANIMATIONS; ++i)
        if(animation_[i])
        {
            animationWeight_[i].Advance(timeStep * config.playerClass(0).animations[i].transitionSpeed);
            animation_[i]->SetWeight(animationWeight_[i].value_);
            animation_[i]->AddTime(timeStep * config.playerClass(0).animations[i].speed);
        }
}

// ----------------------------------------------------------------------------
void PlayerAnimationStatesController::HandleGroundWeights(float velocitySquared)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    float walkFastSpeedSquared = config.playerClass(0).speed.walk * config.playerClass(0).speed.walk;
    float walkSlowSpeedSquared = config.playerClass(0).speed.walk / 4; // half the speed of fast
    float runSpeedSquared = config.playerClass(0).speed.run * config.playerClass(0).speed.run;


    animationWeight_[IceWeaselConfig::IDLE].SetTarget(1);
    if(velocitySquared <= walkSlowSpeedSquared)
    {
        float factor = velocitySquared / walkSlowSpeedSquared;
        animationWeight_[IceWeaselConfig::WALK].SetTarget(factor);
        animationWeight_[IceWeaselConfig::RUN].SetTarget(0);
        animationWeight_[IceWeaselConfig::SPRINT].SetTarget(0);
    }
    else if(velocitySquared <= walkFastSpeedSquared)
    {
        float factor = (velocitySquared - walkSlowSpeedSquared) / (walkFastSpeedSquared - walkSlowSpeedSquared);
        animationWeight_[IceWeaselConfig::WALK].SetTarget(1.0f - factor);
        animationWeight_[IceWeaselConfig::RUN].SetTarget(factor);
        animationWeight_[IceWeaselConfig::SPRINT].SetTarget(0);
    }
    else
    {
        float factor = (velocitySquared - walkFastSpeedSquared) / (runSpeedSquared - walkFastSpeedSquared);
        animationWeight_[IceWeaselConfig::WALK].SetTarget(0);
        animationWeight_[IceWeaselConfig::RUN].SetTarget(1.0f - factor);
        animationWeight_[IceWeaselConfig::SPRINT].SetTarget(factor);
    }
}

// ----------------------------------------------------------------------------
void PlayerAnimationStatesController::HandleCrouchWeights(float velocitySquared)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();
    float crouchSpeedSquared = config.playerClass(0).speed.crouch * config.playerClass(0).speed.crouch;

    /*
     * Because crouch-walking is ordered after normal crouching, it has
     * precedence.
     */
    float factor = Min(1.0f, velocitySquared / crouchSpeedSquared);
    animationWeight_[IceWeaselConfig::CROUCH].SetTarget(1);
    animationWeight_[IceWeaselConfig::CROUCH_WALK].SetTarget(factor);
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
    if(eventData[P_CROUCHING].GetBool())
        state_ = CROUCHING;
    else
        state_ = ON_GROUND;
}
