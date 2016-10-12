#pragma once

#include <Urho3D/UI/UIElement.h>

namespace Urho3D {
    class UIElement;
}

class MainMenu : public Urho3D::UIElement
{
    URHO3D_OBJECT(MainMenu, Urho3D::UIElement)

public:
    MainMenu(Urho3D::Context* context);
    virtual ~MainMenu();

private:
    void SetupUI();

    Urho3D::SharedPtr<Urho3D::UIElement> root_;

    bool mouseWasVisible_;
};
