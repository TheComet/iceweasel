#pragma once

#include <Urho3D/UI/UIElement.h>

namespace Urho3D {
    class UIElement;
}

class MenuBase : public Urho3D::UIElement
{
    URHO3D_OBJECT(MenuBase, Urho3D::UIElement)

public:
    MenuBase(Urho3D::Context* context);
    virtual ~MenuBase();

protected:
    void LoadUI(const Urho3D::String& xmlFileName);

    Urho3D::SharedPtr<Urho3D::XMLFile> xml_;
    Urho3D::SharedPtr<Urho3D::UIElement> root_;
private:
    void ReloadUI();
    void HandleFileChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    bool mouseWasVisible_;
};
