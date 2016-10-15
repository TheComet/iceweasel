#pragma once

#include <Urho3D/UI/UIElement.h>

class MenuScreen;

class MainMenu : public Urho3D::UIElement
{
    URHO3D_OBJECT(MainMenu, Urho3D::UIElement)

public:
    MainMenu(Urho3D::Context* context);

private:
    enum Screen
    {
        SCREEN_MAIN,
        SCREEN_JOIN_SERVER,
        SCREEN_CONNECTING,
        SCREEN_CONNECTION_FAILED,
        SCREEN_LOBBY,

        NUM_SCREENS
    };

    void SwitchToScreen(Screen screen);

    // main screen
    void HandleMainLocalGame(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleMainJoinServer(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleMainOptions(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleMainQuit(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    // join screen
    void HandleJoinCancel(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleJoinConnect(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    // connection failed screen
    void HandleConnectionFailedOK(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    // Network events control the UI
    void HandleServerConnected(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleServerDisconnected(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleConnectionFailed(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    // Input events
    void HandleKeyDown(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<MenuScreen> screens_[NUM_SCREENS];
};
