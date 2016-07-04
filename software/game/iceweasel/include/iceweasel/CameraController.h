#pragma once

#include <Urho3D/Scene/LogicComponent.h>

namespace Urho3D {
    class Gravity;
    class Input;
    class Node;
    class PhysicsWorld;
    class Text;
}


class CameraController : public Urho3D::LogicComponent
{
    URHO3D_OBJECT(CameraController, Urho3D::LogicComponent)

public:

    enum Mode
    {
        FREE,
        FPS
    };

    /*!
     * @brief Constructs a new FPS camera controller.
     * @param[in] context Urho3D context object.
     * @param[in] moveNode The node to control movement for. The camera should
     * be attached to a rotation node, which in turn is a child of a movement
     * node.
     * @param[in] rotateNode The node to read rotations from. The camera should
     * be attached to this node.
     * @param[in] mode The initial mode to use. Defaults to FPS.
     */
    CameraController(Urho3D::Context* context,
                     Urho3D::Node* moveNode,
                     Urho3D::Node* rotateNode,
                     Mode mode=FPS);

    void SetMode(Mode mode);

private:
    virtual void Start() override;
    virtual void Stop() override;

    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    void UpdateCameraRotation();
    void UpdateFPSCameraMovement(float timeStep);
    void UpdateFreeCameraMovement(float timeStep);

    Urho3D::SharedPtr<Urho3D::Node> moveNode_;
    Urho3D::SharedPtr<Urho3D::Node> rotateNode_;

    Urho3D::SharedPtr<Urho3D::Input> input_;
    Urho3D::SharedPtr<Urho3D::PhysicsWorld> physicsWorld_;
    Urho3D::SharedPtr<Urho3D::Gravity> gravity_;

    Urho3D::Vector3 planeVelocity_;
    Urho3D::Quaternion gravityRotation_;
    float downVelocity_;

    float angleX_;
    float angleY_;

    struct {
        float mass;
        float height;
        float width;
        float jumpForce;
        float jumpSpeedBoostFactor;
    } playerParameters_;

    Mode mode_;

    Urho3D::SharedPtr<Urho3D::Text> gravityDebugText_;
};
