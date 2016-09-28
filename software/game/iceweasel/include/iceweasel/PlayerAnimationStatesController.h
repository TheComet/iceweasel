#pragma once

#include "iceweasel/Curves.h"
#include "iceweasel/IceWeaselConfig.h"
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

    PlayerAnimationStatesController(Urho3D::Context* context);

protected:
    virtual void Start() override;
    virtual void Update(float timeStep) override;

private:
    void HandleGroundStates(float velocitySquared);
    void HandleCrouchStates(float velocitySquared);
    void HandleLocalMovementVelocityChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleCrouchStateChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Urho3D::AnimationState> animation_[IceWeaselConfig::NUM_ANIMATIONS];
    ExponentialCurve<float> animationFactor_[IceWeaselConfig::NUM_ANIMATIONS];

    Urho3D::Vector3 currentLocalVelocity_;
    bool isCrouching_;
};
