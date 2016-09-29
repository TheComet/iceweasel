#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Graphics/Model.h>

class Finger : public Urho3D::LogicComponent {
    URHO3D_OBJECT(Finger, Urho3D::LogicComponent)

public:
    Finger(Urho3D::Context* context);

    void SetVisible(bool visible);

private:
    virtual void Start() override;
    virtual void Stop() override;

    Urho3D::SharedPtr<Urho3D::Model> model_;
    Urho3D::SharedPtr<Urho3D::Node> fingerNode_;

    int storeViewMask_;
};
