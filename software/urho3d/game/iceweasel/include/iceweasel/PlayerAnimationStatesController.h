#pragma once

#include "iceweasel/Curves.h"
#include "iceweasel/PlayerAnimation.h"
#include <Urho3D/Scene/LogicComponent.h>

namespace Urho3D {
    class AnimationState;
    class AnimatedModel;
}

class GravityManager;

class PlayerAnimationStatesController : public Urho3D::LogicComponent
{
    URHO3D_OBJECT(PlayerAnimationStatesController, Urho3D::LogicComponent)

public:

    enum State
    {
        ON_GROUND,
        CROUCHING,
        JUMP_BEGIN,
        JUMP_OFF,
        JUMP_FALL,
        JUMP_LAND,
        SWIMMING
    };

    PlayerAnimationStatesController(Urho3D::Context* context);

protected:
    virtual void Start() override;
    virtual void Update(float timeStep) override;

private:
    void HandleGroundWeights(float velocitySquared);
    void HandleCrouchWeights(float velocitySquared);
    void HandleAirWeights();

    void HandleLocalMovementVelocityChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleCrouchStateChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleDownVelocityChange(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Urho3D::AnimationState> animation_[PlayerAnimation::NUM_ANIMATIONS];
    ExponentialCurve<float> animationWeight_[PlayerAnimation::NUM_ANIMATIONS];

    State state_;

    Urho3D::Vector3 currentLocalVelocity_;
    bool isCrouching_;
};
