#pragma once

#include <Urho3D/UI/UIElement.h>

class MenuScreens : public Urho3D::UIElement
{
    URHO3D_OBJECT(MenuScreens, Urho3D::UIElement)

public:
    MenuScreens(Urho3D::Context* context);

    void LoadMenuFromFile(Urho3D::String fileName);

private:
    void Reload();
    bool SwitchScreenTo(Urho3D::StringHash screenName);

    void HandleFileChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleClick(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    typedef void (MenuScreens::*ActionFunc)();
    static ActionFunc HashToMemberFunctionPoiter(Urho3D::StringHash hash);

    void Action_LocalGame();
    void Action_JoinServer();
    void Action_Options();
    void Action_Quit();

    struct Screen
    {
        Urho3D::SharedPtr<Urho3D::XMLFile> xml;
        Urho3D::SharedPtr<Urho3D::UIElement> ui;
    };

    Urho3D::SharedPtr<Urho3D::UIElement> activeScreen_;
    Urho3D::SharedPtr<Urho3D::XMLFile> topXML_;
    Urho3D::HashMap<Urho3D::StringHash, Screen> screens_;
    Urho3D::HashMap<Urho3D::StringHash, Urho3D::StringHash> actionMap_;
};
