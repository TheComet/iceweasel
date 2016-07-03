#pragma once

#include <Urho3D/Scene/LogicComponent.h>

namespace Urho3D {
    class Node;
    class Input;
    class PhysicsWorld;
}


class FPSCameraMovementController : public Urho3D::LogicComponent
{
    URHO3D_OBJECT(FPSCameraMovementController, Urho3D::LogicComponent)

    public:

    /*!
     * @brief Constructs a new free camera controller.
     * @param[in] context Urho3D context object.
     * @param[in] moveNode The node to control movement for. The camera should
     * be attached to a rotation node, which in turn is a child of a movement
     * node.
     * @param[in] rotateNode The node to read rotations from. The camera should
     * be attached to this node.
     */
    FPSCameraMovementController(Urho3D::Context* context,
                                Urho3D::Node* moveNode,
                                Urho3D::Node* rotateNode);

private:
    virtual void Start() override;
    virtual void Stop() override;

    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Urho3D::Node> moveNode_;
    Urho3D::SharedPtr<Urho3D::Node> rotateNode_;
    Urho3D::SharedPtr<Urho3D::Input> input_;
    Urho3D::SharedPtr<Urho3D::PhysicsWorld> physicsWorld_;

    Urho3D::Vector3 planeVelocity_;
    float downVelocity_;

    struct {
        float mass;
        float height;
        float width;
        float jumpForce;
        float jumpSpeedBoostFactor;
    } playerParameters_;
};
