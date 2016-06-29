#pragma once

#include <Urho3D/Scene/Component.h>

namespace Urho3D {
    class Node;
    class Input;
}


class FPSCameraRotateController : public Urho3D::Component
{
    URHO3D_OBJECT(FPSCameraRotateController, Urho3D::Component)

    public:

    /*!
     * @brief Constructs a new free camera controller.
     * @param[in] context Urho3D context object.
     * @param[in] cameraNode The node to control (to which the camera is
     * attached).
     */
    FPSCameraRotateController(Urho3D::Context* context, Urho3D::Node* cameraNode);

private:
    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Urho3D::Node> cameraNode_;
    Urho3D::SharedPtr<Urho3D::Input> input_;

    float angleX_;
    float angleY_;
};
