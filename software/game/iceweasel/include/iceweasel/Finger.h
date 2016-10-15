#pragma once

#include <Urho3D/Scene/LogicComponent.h>

namespace Urho3D {
    class XMLFile;
}

class Finger : public Urho3D::LogicComponent
{
    URHO3D_OBJECT(Finger, Urho3D::LogicComponent)

public:
    Finger(Urho3D::Context* context);

    void SetVisible(bool visible);

private:
    void ReloadGun();

    virtual void Start() override;
    virtual void Stop() override;

    void HandleFileChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Urho3D::XMLFile> xml_;
    Urho3D::SharedPtr<Urho3D::Node> fingerNode_;

    //int storeViewMask_;
};
