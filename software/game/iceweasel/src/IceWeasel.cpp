#include "iceweasel/IceWeasel.h"
#include "iceweasel/IceWeaselConfig.h"

#include "iceweasel/CameraControllerRotation.h"
#include "iceweasel/CameraControllerFPS.h"
#include "iceweasel/CameraControllerFree.h"
#include "iceweasel/Finger.h"
#include "iceweasel/GravityManager.h"
#include "iceweasel/GravityVector.h"
#include "iceweasel/MainMenu.h"

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
#include <Urho3D/Math/Random.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
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
    cameraModeIsFreeCam_(false)
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

    GetSubsystem<Input>()->SetMouseVisible(true);

    RegisterSubsystems();
    CreateDebugHud();
    //CreateUI();
    CreateScene();
    CreateCamera();

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
    RegisterIceWeaselMods(context_);

    context_->RegisterSubsystem(new Script(context_));
    context_->RegisterSubsystem(new IceWeaselConfig(context_));

    GetSubsystem<IceWeaselConfig>()->Load("Config/IceWeaselConfig.xml");
}

// ----------------------------------------------------------------------------
void IceWeasel::CreateUI()
{
    GetSubsystem<UI>()->GetRoot()->AddChild(new MainMenu(context_));
/*
    XMLFile* xmlDefaultStyle = GetSubsystem<ResourceCache>()->GetResource<XMLFile>("UI/DefaultStyle.xml");
    GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(xmlDefaultStyle);

    Window* window = new Window(context_);
    window->SetMinWidth(384);
    window->SetMinHeight(100);
    window->SetPosition(8, 8);
    window->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    window->SetName("Window");

    UIElement* titleBar = new UIElement(context_);
    titleBar->SetMinSize(0, 24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);

    Text* windowTitle = new Text(context_);
    windowTitle->SetName("WindowTitle");
    windowTitle->SetText("This is a test!");

    Button* button = new Button(context_);
    button->SetName("TestButton");

    Text* buttonText = new Text(context_);
    buttonText->SetText("button");

    GetSubsystem<UI>()->GetRoot()->AddChild(window);
    window->AddChild(button);
    window->AddChild(titleBar);
    titleBar->AddChild(windowTitle);
    button->AddChild(buttonText);

    window->SetStyleAuto();
    button->SetStyleAuto();
    windowTitle->SetStyleAuto();
    buttonText->SetStyleAuto();*/
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
    cameraMoveNode_ = scene_->CreateChild("Camera Move");
    cameraRotateNode_ = cameraMoveNode_->CreateChild("Camera Rotate");
    Camera* camera = cameraRotateNode_->CreateComponent<Camera>(Urho3D::LOCAL);
    camera->SetFarClip(300.0f);
    cameraMoveNode_->SetPosition(Vector3(0.0f, 5.0f, -0.0f)); // spawn location
    cameraRotateNode_->AddComponent(new CameraControllerRotation(context_), 0, Urho3D::LOCAL);
    SwitchCameraToFPSCam();
    cameraRotateNode_->AddComponent(new Finger(context_), 0, Urho3D::LOCAL);

    // Give the camera a viewport
    Viewport* viewport = new Viewport(context_, scene_, camera);
    viewport->SetDrawDebug(true);
    renderer->SetViewport(0, viewport);

    // Configure render path
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
    renderer->SetHDRRendering(true);
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
    cameraMoveNode_->RemoveComponent<CameraControllerFPS>();
    cameraMoveNode_->AddComponent(new CameraControllerFree(context_), 0, Urho3D::LOCAL);
}

// ----------------------------------------------------------------------------
void IceWeasel::SwitchCameraToFPSCam()
{
    cameraMoveNode_->RemoveComponent<CameraControllerFree>();
    cameraMoveNode_->AddComponent(new CameraControllerFPS(context_), 0, Urho3D::LOCAL);
}

// ----------------------------------------------------------------------------
void IceWeasel::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;
    (void)eventType;

    // Check for pressing ESC
    int key = eventData[P_KEY].GetInt();
    if(key == KEY_ESCAPE)
        engine_->Exit();

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
        cameraModeIsFreeCam_ = !cameraModeIsFreeCam_;
        if(cameraModeIsFreeCam_)
            SwitchCameraToFreeCam();
        else
            SwitchCameraToFPSCam();
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
