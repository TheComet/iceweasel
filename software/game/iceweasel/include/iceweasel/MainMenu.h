#pragma once

#include <Urho3D/UI/UIElement.h>

class MainMenu : public Urho3D::UIElement
{
    URHO3D_OBJECT(MainMenu, Urho3D::UIElement)

public:
    MainMenu(Urho3D::Context* context);

private:
    void Reload();
    void LoadActions();

    void HandleFileChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleClick(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    typedef void (MainMenu::*ActionFunc)();
    struct Action
    {
        Urho3D::StringHash type_;
        ActionFunc func_;
    };
    typedef Urho3D::HashMap<Urho3D::StringHash, Action> ActionMap;
    ActionMap actionMap_;

    static ActionFunc HashToMemberFunctionPointer(Urho3D::StringHash hash);
    void Action_Quit();
    void Action_Options();
    void Action_JoinServer();

    Urho3D::XMLFile* xml_;
};
