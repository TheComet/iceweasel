#pragma once

#include <Urho3D/Scene/LogicComponent.h>

namespace Urho3D {
    class Input;
}

class CameraControllerFree : public Urho3D::LogicComponent
{
    URHO3D_OBJECT(CameraControllerFree, Urho3D::LogicComponent)
public:
    CameraControllerFree(Urho3D::Context* context);

private:
    virtual void Start() override;
    virtual void Update(float timeStep) override;

    void HandleCameraAngleChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Urho3D::Input> input_;
    Urho3D::Vector3 currentVelocity_;
    float cameraAngleY_;
};
