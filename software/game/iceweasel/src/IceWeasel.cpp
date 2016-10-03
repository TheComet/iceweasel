#include "iceweasel/IceWeasel.h"
#include "iceweasel/IceWeaselConfig.h"

#include "iceweasel/PlayerController.h"
#include "iceweasel/CameraControllerFree.h"
#include "iceweasel/DebugTextScroll.h"
#include "iceweasel/GravityManager.h"
#include "iceweasel/GravityVector.h"
#include "iceweasel/InGameEditor.h"
#include "iceweasel/MenuScreens.h"

#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/LuaScript/LuaScript.h>
#include <Urho3D/Math/Random.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
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
IceWeasel::IceWeasel(Context* context) :
    Application(context),
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
    mainMenu_ = new MenuScreens(context_);
    mainMenu_->LoadMenuFromFile("UI/MainMenu.xml");
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

    //GetSubsystem<Input>()->SetMouseVisible(true);
}

// ----------------------------------------------------------------------------
void IceWeasel::CleanupState_Game()
{
}

// ----------------------------------------------------------------------------
void IceWeasel::Setup()
{
    // called before engine initialization

    engineParameters_["WindowTitle"] = "IceWeasel";
    engineParameters_["FullScreen"]  = false;
    engineParameters_["Headless"]    = false;
    engineParameters_["Multisample"] = 2;
    engineParameters_["VSync"] = true;
}

// ----------------------------------------------------------------------------
void IceWeasel::Start()
{
    // configure resource cache
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    cache->SetAutoReloadResources(true);

    RegisterIceWeaselMods(context_);
    RegisterSubsystems();
    RegisterComponents();
    CreateDebugHud();

    GetSubsystem<IceWeaselConfig>()->Load("Config/IceWeaselConfig.xml");

    SwitchState(GAME);

    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(IceWeasel, HandleKeyDown));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(IceWeasel, HandlePostRenderUpdate));
}

// ----------------------------------------------------------------------------
void IceWeasel::Stop()
{
    cameraMoveNode_.Reset();
    cameraRotateNode_.Reset();

    scene_.Reset();
}

// ----------------------------------------------------------------------------
void IceWeasel::RegisterSubsystems()
{
    context_->RegisterSubsystem(new Script(context_));
    context_->RegisterSubsystem(new LuaScript(context_));
    context_->RegisterSubsystem(new IceWeaselConfig(context_));
    context_->RegisterSubsystem(new InGameEditor(context_));
    context_->RegisterSubsystem(new DebugTextScroll(context_));

    GetSubsystem<DebugTextScroll>()->SetTextCount(20);
}

// ----------------------------------------------------------------------------
void IceWeasel::RegisterComponents()
{
}

// ----------------------------------------------------------------------------
void IceWeasel::CreateCamera()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Renderer* renderer = GetSubsystem<Renderer>();

    /*
     * The camera is attached to a "rotate node", which is in turn attached to
     * a "move" node. The rotation controller is separate from the movement
     * controller.
     */
    cameraMoveNode_   = scene_->CreateChild("Camera Move");
    cameraOffsetNode_ = cameraMoveNode_->CreateChild("Camera Offset");
    cameraRotateNode_ = cameraOffsetNode_->CreateChild("Camera Rotate");
    Camera* camera = cameraRotateNode_->CreateComponent<Camera>(LOCAL);
    camera->SetFarClip(300.0f);

    cameraMoveNode_->AddComponent(new PlayerController(context_, cameraMoveNode_, cameraOffsetNode_, cameraRotateNode_), 0, LOCAL);
    cameraMoveNode_->GetComponent<PlayerController>()->SetMode(PlayerController::THIRD_PERSON);

    // Give the camera a viewport
    Viewport* viewport = new Viewport(context_, scene_, camera);
    viewport->SetDrawDebug(true);
    renderer->SetViewport(0, viewport);

    // Configure render path
    /*
    SharedPtr<RenderPath> effectRenderPath(new RenderPath);
    effectRenderPath->Load(cache->GetResource<XMLFile>("RenderPaths/Deferred.xml"));
    //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/AutoExposure.xml"));
    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));

    //effectRenderPath->SetEnabled("AutoExposure", true);
    effectRenderPath->SetShaderParameter("BloomHDRMix", Vector2(1.0f, 0.3f));
    effectRenderPath->SetShaderParameter("BloomHDRThreshold", float(0.3));
    effectRenderPath->SetEnabled("BloomHDR", true);
    effectRenderPath->SetEnabled("FXAA2", true);

    viewport->SetRenderPath(effectRenderPath);
    renderer->SetHDRRendering(true);*/
}

// ----------------------------------------------------------------------------
void IceWeasel::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // load scene from XML
    scene_ = new Scene(context_);
    //XMLFile* xmlScene = cache->GetResource<XMLFile>("Scenes/GravityMeshTest.xml");
    XMLFile* xmlScene = cache->GetResource<XMLFile>("Scenes/TestMap.xml");
    if(xmlScene)
        scene_->LoadXML(xmlScene->GetRoot());

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
        InGameEditor* editor = GetSubsystem<InGameEditor>();
        if(editor)
        {
            if(editor->IsOpen() == false)
                editor->OpenEditor(scene_);
        }
    }

    // Toggle debug geometry
#ifdef DEBUG
    if(key == KEY_1)
    {
        switch(debugDrawMode_)
        {
            case DRAW_NONE    : debugDrawMode_ = DRAW_PHYSICS; break;
            case DRAW_PHYSICS : debugDrawMode_ = DRAW_GRAVITY; break;
            case DRAW_GRAVITY : debugDrawMode_ = DRAW_NONE;    break;
        }
    }

    // Toggle debug HUD
    if(key == KEY_2)
    {
        if(debugHud_->GetMode() == DEBUGHUD_SHOW_NONE)
            debugHud_->SetMode(DEBUGHUD_SHOW_ALL);
        else if(debugHud_->GetMode() == DEBUGHUD_SHOW_ALL)
            debugHud_->SetMode(DEBUGHUD_SHOW_MEMORY);
        else
            debugHud_->SetMode(DEBUGHUD_SHOW_NONE);
    }

    // toggle between free-cam and FPS cam
    if(key == KEY_5)
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
URHO3D_DEFINE_APPLICATION_MAIN(IceWeasel)
