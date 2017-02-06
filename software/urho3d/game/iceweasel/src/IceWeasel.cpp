#include "iceweasel/IceWeasel.h"
#include "iceweasel/IceWeaselConfig.h"

#include "iceweasel/Args.h"
#include "iceweasel/PlayerController.h"
#include "iceweasel/CameraControllerFree.h"
#include "iceweasel/DebugTextScroll.h"
#include "iceweasel/GravityManager.h"
#include "iceweasel/GravityVector.h"
#include "iceweasel/MainMenu.h"

#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/LuaScript/LuaScript.h>
#include <Urho3D/Math/Random.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/AngelScript/ScriptFile.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Window.h>

using namespace Urho3D;

const char* ICEWEASEL_CATEGORY = "IceWeasel Mods";

// ----------------------------------------------------------------------------
void RegisterIceWeaselMods(Urho3D::Context* context)
{
    GravityManager::RegisterObject(context);
    GravityVector::RegisterObject(context);
}

// ----------------------------------------------------------------------------
IceWeasel::IceWeasel(Context* context, Args* args) :
    Application(context),
    args_(args),
    debugDrawMode_(DRAW_NONE),
    gameState_(EMPTY),
    isThirdPerson_(true)
{
}

// ----------------------------------------------------------------------------
void IceWeasel::SwitchState(GameState state)
{
    if(gameState_ == state)
        return;

    CleanupState();

    switch(state)
    {
        case EMPTY     : break;
        case MAIN_MENU : StartState_MainMenu();   break;
        case GAME      : StartState_Game();       break;
    }

    gameState_ = state;
}

// ----------------------------------------------------------------------------
void IceWeasel::CleanupState()
{
    switch(gameState_)
    {
        case EMPTY     : break;
        case MAIN_MENU : CleanupState_MainMenu(); break;
        case GAME      : CleanupState_Game();     break;
    }

    gameState_ = EMPTY;
}

// ----------------------------------------------------------------------------
void IceWeasel::StartState_MainMenu()
{
    mainMenu_ = new MainMenu(context_);
    GetSubsystem<UI>()->GetRoot()->AddChild(mainMenu_);
}

// ----------------------------------------------------------------------------
void IceWeasel::CleanupState_MainMenu()
{
    GetSubsystem<UI>()->GetRoot()->RemoveChild(mainMenu_);
    mainMenu_.Reset();
}

// ----------------------------------------------------------------------------
void IceWeasel::StartState_Game()
{
    CreateScene();
    CreateCamera();

    GetSubsystem<Input>()->SetMouseVisible(true);
}

// ----------------------------------------------------------------------------
void IceWeasel::CleanupState_Game()
{
    StopNetworking();
}

// ----------------------------------------------------------------------------
void IceWeasel::Setup()
{
    // called before engine initialization

    engineParameters_["WindowTitle"] = "IceWeasel";
    engineParameters_["FullScreen"]  = args_->fullscreen_;
    engineParameters_["Headless"]    = args_->server_;
    engineParameters_["Multisample"] = args_->multisample_;
    engineParameters_["VSync"] = args_->vsync_;

    engineParameters_["WindowResizable"] = true;
}

// ----------------------------------------------------------------------------
void IceWeasel::Start()
{
    // configure resource cache
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    cache->SetAutoReloadResources(true);
    for(StringVector::ConstIterator it = args_->resourcePaths_.Begin(); it != args_->resourcePaths_.End(); ++it)
        cache->AddResourceDir(*it);

    RegisterIceWeaselMods(context_);
    RegisterSubsystems();
    RegisterComponents();
    CreateDebugHud();

    GetSubsystem<IceWeaselConfig>()->Load("Config/IceWeaselConfig.xml");

    SwitchState(GAME);

    if(args_->server_)
    {
        StartNetworking();
    }
    else
    {
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(IceWeasel, HandleKeyDown));
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(IceWeasel, HandlePostRenderUpdate));
    }

    SubscribeToEvent(E_FILECHANGED, URHO3D_HANDLER(IceWeasel, HandleFileChanged));

    // Network events
    SubscribeToEvent(E_SERVERCONNECTED, URHO3D_HANDLER(IceWeasel, HandleConnectionStatus));
    SubscribeToEvent(E_SERVERDISCONNECTED, URHO3D_HANDLER(IceWeasel, HandleConnectionStatus));
    SubscribeToEvent(E_CONNECTFAILED, URHO3D_HANDLER(IceWeasel, HandleConnectionStatus));
    SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(IceWeasel, HandleClientConnected));
    SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(IceWeasel, HandleClientDisconnected));
}

// ----------------------------------------------------------------------------
void IceWeasel::Stop()
{
    SwitchState(EMPTY);
}

// ----------------------------------------------------------------------------
void IceWeasel::RegisterSubsystems()
{
    context_->RegisterSubsystem(new Script(context_));
    context_->RegisterSubsystem(new LuaScript(context_));
    context_->RegisterSubsystem(new IceWeaselConfig(context_));
    context_->RegisterSubsystem(new DebugTextScroll(context_));

    GetSubsystem<DebugTextScroll>()->SetTextCount(20);
}

// ----------------------------------------------------------------------------
void IceWeasel::RegisterComponents()
{
}

// ----------------------------------------------------------------------------
void IceWeasel::StartNetworking()
{
    Network* network = GetSubsystem<Network>();

    if(args_->server_)
        network->StartServer(args_->networkPort_);
    else
        network->Connect(args_->networkAddress_, args_->networkPort_, scene_);
}

// ----------------------------------------------------------------------------
void IceWeasel::StopNetworking()
{
    Network* network = GetSubsystem<Network>();

    if(args_->server_)
        network->StopServer();
    else
        network->Disconnect();
}

// ----------------------------------------------------------------------------
void IceWeasel::CreateCamera()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Renderer* renderer = GetSubsystem<Renderer>();
    if(renderer == NULL)
        return;

    /*
     * The camera is attached to a "rotate node", which is in turn attached to
     * a "move" node. The rotation controller is separate from the movement
     * controller.
     */
    cameraMoveNode_   = scene_->CreateChild("Camera Move", LOCAL);
    cameraOffsetNode_ = cameraMoveNode_->CreateChild("Camera Offset", LOCAL);
    cameraRotateNode_ = cameraOffsetNode_->CreateChild("Camera Rotate", LOCAL);
    Camera* camera = cameraRotateNode_->CreateComponent<Camera>(LOCAL);
    camera->SetFarClip(300.0f);

    cameraMoveNode_->AddComponent(new PlayerController(context_, cameraMoveNode_, cameraOffsetNode_, cameraRotateNode_), 0, LOCAL);
    cameraMoveNode_->GetComponent<PlayerController>()->SetMode(PlayerController::THIRD_PERSON);

    // Give the camera a viewport
    Viewport* viewport = new Viewport(context_, scene_, camera);
    viewport->SetDrawDebug(true);
    renderer->SetViewport(0, viewport);

    // Configure render path
    SharedPtr<RenderPath> effectRenderPath(new RenderPath);
    effectRenderPath->Load(cache->GetResource<XMLFile>("RenderPaths/IceWeasel.xml"));
/*
    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA3.xml"));

    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
    effectRenderPath->SetShaderParameter("BloomHDRMix", Vector2(1.0, 0.3));
    effectRenderPath->SetShaderParameter("BloomHDRThreshold", 0.3);

    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/GammaCorrection.xml"));
    effectRenderPath->SetEnabled("GammaCorrection", false);

    viewport->SetRenderPath(effectRenderPath);
    renderer->SetHDRRendering(true);*/
}

// ----------------------------------------------------------------------------
void IceWeasel::CreateScene()
{
    // Scene object should always exist, regardless if client or server
    scene_ = new Scene(context_);

    scene_->CreateComponent<Octree>(LOCAL);
    scene_->CreateComponent<PhysicsWorld>(LOCAL);

    // If client, don't load anything
    //if(args_->server_ == false)
    //    return;

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    String mapName = "Scenes/TestMap.xml";
    if(args_->sceneName_.Length() != 0)
        mapName = args_->sceneName_;

    // load scene from XML
    if(mapName.Length() == 0)
        mapName = "Scenes/TestMap.xml";
    xmlScene_ = cache->GetResource<XMLFile>(mapName);
    if(xmlScene_)
        scene_->LoadXML(xmlScene_->GetRoot());
    else
        ErrorExit("Failed to load scene \"" + mapName + "\" - did you spell it correctly?");

    /*
    // HACK Add a small random offset to all gravity vectors so the
    // triangulation can avoid degenerate tetrahedrons.
    PODVector<Node*> gravityVectorNodes;
    scene_->GetChildrenWithComponent<GravityVector>(gravityVectorNodes, true);
    // Don't forget to check this node's components as well
    if(scene_->GetComponent<GravityVector>())
        gravityVectorNodes.Push(scene_);
    PODVector<Node*>::Iterator it = gravityVectorNodes.Begin();
    for(; it != gravityVectorNodes.End(); ++it)
        (*it)->SetPosition((*it)->GetPosition() + Vector3(Random()*2-1, Random()*2-1, Random()*2-1));*/
}

// ----------------------------------------------------------------------------
void IceWeasel::CreateDebugHud()
{
#ifdef DEBUG
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    debugHud_ = engine_->CreateDebugHud();
    if(debugHud_)
        debugHud_->SetDefaultStyle(style);
#endif
}

// ----------------------------------------------------------------------------
void IceWeasel::SwitchCameraToFreeCam()
{
    cameraMoveNode_->RemoveComponent<PlayerController>();
    cameraMoveNode_->AddComponent(new CameraControllerFree(context_), 0, LOCAL);
}

// ----------------------------------------------------------------------------
void IceWeasel::SwitchCameraToFPSCam()
{
    cameraMoveNode_->RemoveComponent<CameraControllerFree>();

}

// ----------------------------------------------------------------------------
void IceWeasel::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;
    (void)eventType;

    // Pressing ESC opens the editor
    int key = eventData[P_KEY].GetInt();
    if(key == KEY_ESCAPE)
    {
        engine_->Exit();
    }

    // Toggle debug geometry
#ifdef DEBUG
    if(gameState_ != GAME)
        return;

    if(key == KEY_F1)
    {
        switch(debugDrawMode_)
        {
            case DRAW_NONE    : debugDrawMode_ = DRAW_PHYSICS; break;
            case DRAW_PHYSICS : debugDrawMode_ = DRAW_GRAVITY; break;
            case DRAW_GRAVITY : debugDrawMode_ = DRAW_NONE;    break;
        }
    }

    if(key == KEY_B)
    {
        RenderPath* effectRenderPath = GetSubsystem<Renderer>()->GetViewport(0)->GetRenderPath();
        bool enable = !effectRenderPath->GetEnabled("BloomHDR");
        effectRenderPath->SetEnabled("BloomHDR", enable);
        LOG_SCROLL(enable ? "Enabling BloomHDR" : "Disabling BloomHDR");
    }

    if(key == KEY_F)
    {
        RenderPath* effectRenderPath = GetSubsystem<Renderer>()->GetViewport(0)->GetRenderPath();
        bool enable = !effectRenderPath->GetEnabled("FXAA3");
        effectRenderPath->SetEnabled("FXAA3", enable);
        LOG_SCROLL(enable ? "Enabling FXAA3" : "Disabling FXAA3");
    }

    if(key == KEY_G)
    {
        RenderPath* effectRenderPath = GetSubsystem<Renderer>()->GetViewport(0)->GetRenderPath();
        bool enable = !effectRenderPath->GetEnabled("GammaCorrection");
        effectRenderPath->SetEnabled("GammaCorrection", enable);
        LOG_SCROLL(enable ? "Enabling GammaCorrection" : "Disabling GammaCorrection");
    }

    // Toggle debug HUD
    if(key == KEY_F2)
    {
        if(debugHud_->GetMode() == DEBUGHUD_SHOW_NONE)
            debugHud_->SetMode(DEBUGHUD_SHOW_ALL);
        else if(debugHud_->GetMode() == DEBUGHUD_SHOW_ALL)
            debugHud_->SetMode(DEBUGHUD_SHOW_MEMORY);
        else
            debugHud_->SetMode(DEBUGHUD_SHOW_NONE);
    }

    // toggle between free-cam and FPS cam
    if(key == KEY_F5)
    {
        isThirdPerson_ = !isThirdPerson_;
        if(isThirdPerson_)
            cameraMoveNode_->GetComponent<PlayerController>()->SetMode(PlayerController::THIRD_PERSON);
        else
            cameraMoveNode_->GetComponent<PlayerController>()->SetMode(PlayerController::FIRST_PERSON);
    }

    // Toggle mouse visibility (for debugging)
    if(key == KEY_9)
        GetSubsystem<Input>()->SetMouseVisible(!GetSubsystem<Input>()->IsMouseVisible());
#endif
}

// ----------------------------------------------------------------------------
void IceWeasel::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    (void)eventType;
    (void)eventData;

    if(debugDrawMode_ == DRAW_NONE)
        return;
    if(scene_ == NULL)
        return;

    DebugRenderer* debugRenderer = scene_->GetComponent<DebugRenderer>();
    if(!debugRenderer)
        return;
    bool depthTest = true;

    switch(debugDrawMode_)
    {
        case DRAW_NONE: return;

        case DRAW_PHYSICS:
        {
            PhysicsWorld* phy = scene_->GetComponent<PhysicsWorld>();
            if(!phy)
                return;
            phy->DrawDebugGeometry(depthTest);
            break;
        }

        case DRAW_GRAVITY:
        {
            GravityManager* gravity = scene_->GetComponent<GravityManager>();
            if(gravity)
                gravity->DrawDebugGeometry(debugRenderer, depthTest, cameraMoveNode_->GetWorldPosition());
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void IceWeasel::HandleFileChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    if(xmlScene_ && xmlScene_->GetName() == eventData[FileChanged::P_RESOURCENAME].GetString())
    {
        URHO3D_LOGINFO("[IceWeasel] Reloading scene");
        if(scene_)
        {
            scene_->LoadXML(xmlScene_->GetRoot());
            SwitchState(EMPTY);
            SwitchState(GAME);
        }
    }
}

// ----------------------------------------------------------------------------
void IceWeasel::HandleClientConnected(StringHash eventType, VariantMap& eventData)
{
    using namespace ClientConnected;

    // When a client connects, assign to scene to begin scene replication
    Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
    newConnection->SetScene(scene_);
}

// ----------------------------------------------------------------------------
void IceWeasel::HandleClientDisconnected(StringHash eventType, VariantMap& eventData)
{
    using namespace ClientConnected;
}

// ----------------------------------------------------------------------------
void IceWeasel::HandleConnectionStatus(StringHash eventType, VariantMap& eventData)
{
}
