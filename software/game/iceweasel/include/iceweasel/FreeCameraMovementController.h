#pragma once

#include <Urho3D/Scene/Component.h>

namespace Urho3D {
    class Node;
    class Input;
}


class FreeCameraMovementController : public Urho3D::Component
{
    URHO3D_OBJECT(FreeCameraMovementController, Urho3D::Component)

    public:

    /*!
     * @brief Constructs a new free camera controller.
     * @param[in] moveNode The node to control movement for. The camera should
     * be attached to a rotation node, which in turn is a child of a movement
     * node.
     * @param[in] rotateNode The node to read rotations from. The camera should
     * be attached to this node.
     */
    FreeCameraMovementController(Urho3D::Context* context,
                                 Urho3D::Node* moveNode,
                                 Urho3D::Node* rotateNode);

private:
    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Urho3D::Node> moveNode_;
    Urho3D::SharedPtr<Urho3D::Node> rotateNode_;
    Urho3D::SharedPtr<Urho3D::Input> input_;

    Urho3D::Vector3 actualDirection_;
};
