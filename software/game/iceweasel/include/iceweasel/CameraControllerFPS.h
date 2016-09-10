#pragma once

#include <Urho3D/Scene/LogicComponent.h>

namespace Urho3D {
    class Input;
    class PhysicsWorld;
    class RigidBody;
    class CollisionShape;
}

class GravityManager;

class CameraControllerFPS : public Urho3D::LogicComponent
{
    URHO3D_OBJECT(CameraControllerFPS, Urho3D::LogicComponent)

public:
    CameraControllerFPS(Urho3D::Context* context);

private:
    virtual void Start() override;
    virtual void Stop() override;
    virtual void Update(float timeStep) override;
    virtual void FixedUpdate(float timeStep) override;

    void PatchSceneGraph();
    void UnpatchSceneGraph();
    void CreateComponents();
    void DestroyComponents();
    bool CanStandUp() const;

    void HandleCameraAngleChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleConfigReloaded(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void ConfigurePhysicsParameters();

    Urho3D::SharedPtr<Urho3D::Input> input_;
    Urho3D::SharedPtr<Urho3D::PhysicsWorld> physicsWorld_;
    Urho3D::SharedPtr<GravityManager> gravityManager_;
    Urho3D::SharedPtr<Urho3D::RigidBody> body_;
    Urho3D::SharedPtr<Urho3D::CollisionShape> collisionShapeUpright_;
    Urho3D::SharedPtr<Urho3D::CollisionShape> collisionShapeCrouch_;
    Urho3D::SharedPtr<Urho3D::Node> moveNode_;
    Urho3D::SharedPtr<Urho3D::Node> heightOffsetNode_;

    Urho3D::Quaternion currentRotation_;
    float downVelocity_;
    float cameraAngleY_;
    bool jumpKeyPressed_;
};
