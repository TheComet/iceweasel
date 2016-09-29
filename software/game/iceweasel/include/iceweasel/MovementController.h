#pragma once

#include "iceweasel/Curves.h"
#include <Urho3D/Scene/LogicComponent.h>

namespace Urho3D {
    class Input;
    class PhysicsWorld;
    class RigidBody;
    class CollisionShape;
}

class GravityManager;

class MovementController : public Urho3D::LogicComponent
{
    URHO3D_OBJECT(MovementController, Urho3D::LogicComponent)

public:
    MovementController(Urho3D::Context* context, Urho3D::Node* moveNode, Urho3D::Node* offsetNode);

    void setRespawnDistance(float distance);

protected:
    virtual void Start() override;
    virtual void Stop() override;
    virtual void Update(float timeStep) override;
    virtual void FixedUpdate(float timeStep) override;

    void CreateComponents();
    void DestroyComponents();
    bool CanStandUp() const;
    bool IsCrouching() const;

private:
    void HandleCameraAngleChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleConfigReloaded(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void UpdatePhysicsSettings();
    void SetInitialPhysicsParameters();

    void NotifyLocalMovementVelocityChange(const Urho3D::Vector3& localPlaneVelocity);
    void NotifyCrouchStateChange(bool isCrouching);

    Urho3D::SharedPtr<Urho3D::Input> input_;
    Urho3D::SharedPtr<Urho3D::PhysicsWorld> physicsWorld_;
    Urho3D::SharedPtr<GravityManager> gravityManager_;
    Urho3D::SharedPtr<Urho3D::RigidBody> body_;
    Urho3D::SharedPtr<Urho3D::CollisionShape> collisionShapeUpright_;
    Urho3D::SharedPtr<Urho3D::CollisionShape> collisionShapeCrouch_;
    Urho3D::SharedPtr<Urho3D::Node> moveNode_;
    Urho3D::SharedPtr<Urho3D::Node> offsetNode_;

    ExponentialCurve<Urho3D::Vector3> localPlaneAcceleration_;

    Urho3D::Quaternion currentRotation_;
    float downVelocity_;
    float cameraAngleY_;
    float respawnDistance_;
    bool jumpKeyPressed_;
};
