#pragma once

#include <Urho3D/Scene/LogicComponent.h>

namespace Urho3D {
    class Context;
    class Node;
    class Input;
}

class CameraControllerRotation : public Urho3D::LogicComponent
{
    URHO3D_OBJECT(CameraControllerRotation, Urho3D::LogicComponent);

public:
    CameraControllerRotation(Urho3D::Context* context);

private:
    virtual void Start() override;
    virtual void Update(float timeStep) override;

    Urho3D::SharedPtr<Urho3D::Input> input_;

    float angleX_;
    float angleY_;
};
