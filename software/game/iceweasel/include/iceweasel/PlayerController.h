#pragma once

#include "iceweasel/Curves.h"
#include <Urho3D/Scene/LogicComponent.h>

namespace Urho3D {
    class Node;
    class AnimationState;
}

class GravityManager;

class PlayerController : public Urho3D::LogicComponent
{
    URHO3D_OBJECT(PlayerController, Urho3D::LogicComponent)

    enum Mode
    {
        FIRST_PERSON,
        THIRD_PERSON,
        FREE_CAM
    };

    enum Animation
    {
        IDLE,
        WALK_SLOW,
        WALK_FAST,
        RUN,


        NUM_ANIMATIONS
    };

public:
    PlayerController(Urho3D::Context* context, Urho3D::Node* moveNode, Urho3D::Node* offsetNode, Urho3D::Node* rotateNode);

    void SetMode(Mode mode);

protected:
    virtual void Start() override;
    virtual void Stop() override;
    virtual void Update(float timeStep) override;

private:
    void HandleLocalMovementVelocityChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    void CreateComponents();
    void DestroyComponents();

    Urho3D::SharedPtr<Urho3D::Node> moveNode_;
    Urho3D::SharedPtr<Urho3D::Node> offsetNode_;
    Urho3D::SharedPtr<Urho3D::Node> rotateNode_;
    Urho3D::SharedPtr<Urho3D::Node> modelNode_;
    Urho3D::SharedPtr<Urho3D::AnimationState> animation_[NUM_ANIMATIONS];

    Urho3D::Vector3 currentLocalVelocity_;
    Urho3D::Vector3 newLocalVelocity_;

    Urho3D::Vector2 currentAcceleration_;

    float currentYAngle_;
    ExponentialCurve cameraOffset_;
    ExponentialCurve animationWalkFactor_;

    Mode mode_;
};
