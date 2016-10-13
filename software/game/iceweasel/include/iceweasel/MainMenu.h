#pragma once

#include <Urho3D/UI/UIElement.h>

class MenuScreen;

class MainMenu : public Urho3D::UIElement
{
    URHO3D_OBJECT(MainMenu, Urho3D::UIElement)

public:
    MainMenu(Urho3D::Context* context);

private:
    // main screen
    void HandleMainLocalGame(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleMainJoinServer(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleMainOptions(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleMainQuit(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    // join screen
    void HandleJoinCancel(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleJoinConnect(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<MenuScreen> mainScreen_;
    Urho3D::SharedPtr<MenuScreen> joinServerScreen_;
};
