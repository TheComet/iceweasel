#pragma once

#include <Urho3D/Engine/Application.h>

namespace Urho3D {
    class Node;
    class Scene;
    class Text;
    class XMLFile;
}

class Args;
class MainMenu;

/// Defines the category under which iceweasel specific components can be found in the editor.
extern const char* ICEWEASEL_CATEGORY;

/*!
 * Registers all Ice Weasel modifications.
 */
void RegisterIceWeaselMods(Urho3D::Context* context);


class IceWeasel : public Urho3D::Application
{
public:
    enum GameState
    {
        EMPTY = 0,
        MAIN_MENU,
        GAME
    };

    IceWeasel(Urho3D::Context* context, Args* args);

    void SwitchState(GameState state);

    virtual void Setup() override;
    virtual void Start() override;
    virtual void Stop() override;

private:

    void CleanupState();
    void StartState_MainMenu();
    void CleanupState_MainMenu();
    void StartState_Game();
    void CleanupState_Game();

    void RegisterSubsystems();
    void RegisterComponents();
    void StartNetworking();
    void StopNetworking();
    void CreateCamera();
    void CreateScene();
    void CreateDebugHud();

    void SwitchCameraToFreeCam();
    void SwitchCameraToFPSCam();

    void HandleKeyDown(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleFileChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    void HandleClientConnected(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleClientDisconnected(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleConnectionStatus(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Args> args_;
    Urho3D::SharedPtr<MainMenu> mainMenu_;
    Urho3D::SharedPtr<Urho3D::Scene> scene_;
    Urho3D::SharedPtr<Urho3D::XMLFile> xmlScene_;
    Urho3D::SharedPtr<Urho3D::Node> cameraMoveNode_;
    Urho3D::SharedPtr<Urho3D::Node> cameraOffsetNode_;
    Urho3D::SharedPtr<Urho3D::Node> cameraRotateNode_;
    Urho3D::SharedPtr<Urho3D::DebugHud> debugHud_;

    enum DebugDrawMode
    {
        DRAW_NONE,
        DRAW_PHYSICS,
        DRAW_GRAVITY
    };

    DebugDrawMode debugDrawMode_;
    GameState gameState_;
    bool isThirdPerson_;
};
