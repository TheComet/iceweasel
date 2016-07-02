#include "iceweasel/IceWeasel.h"
#include "iceweasel/FPSCameraRotateController.h"
#include "iceweasel/FPSCameraMovementController.h"
#include "iceweasel/FreeCameraMovementController.h"

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
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/Text.h>

#include <iostream>

using namespace Urho3D;

// ----------------------------------------------------------------------------
IceWeasel::IceWeasel(Context* context) :
    Application(context),
    drawDebugGeometry_(false),
    cameraModeIsFreeCam_(true)
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

    context_->RegisterSubsystem(new Script(context_));

    CreateDebugHud();
    CreateUI();
    CreateScene();
    CreateCamera();

    // Shows mouse and allows it to exit the window boundaries
    //GetSubsystem<Input>()->SetMouseVisible(true);

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
void IceWeasel::CreateUI()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    UI* ui = GetSubsystem<UI>();
    UIElement* root = ui->GetRoot();

    XMLFile* xmlDefaultStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    root->SetDefaultStyle(xmlDefaultStyle);

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

    root->AddChild(window);
    window->AddChild(button);
    window->AddChild(titleBar);
    titleBar->AddChild(windowTitle);
    button->AddChild(buttonText);

    window->SetStyleAuto();
    button->SetStyleAuto();
    windowTitle->SetStyleAuto();
    buttonText->SetStyleAuto();
}

// ----------------------------------------------------------------------------
void IceWeasel::CreateCamera()
{
    cameraMoveNode_ = scene_->CreateChild("Camera Move");
    cameraRotateNode_ = cameraMoveNode_->CreateChild("Camera Rotate");
    Camera* camera = cameraRotateNode_->CreateComponent<Camera>(Urho3D::LOCAL);
    camera->SetFarClip(300.0f);
    cameraMoveNode_->SetPosition(Vector3(0.0f, 5.0f, -0.0f));
    cameraRotateNode_->SetPosition(Vector3::ZERO);

    Viewport* viewport = new Viewport(context_, scene_, camera);
    viewport->SetDrawDebug(true);
    GetSubsystem<Renderer>()->SetViewport(0, viewport);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SharedPtr<RenderPath> effectRenderPath = viewport->GetRenderPath()->Clone();
    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/Bloom.xml"));
    effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));

    effectRenderPath->SetShaderParameter("BloomMix", Vector2(5.0f, 5.0f));
    effectRenderPath->SetEnabled("Bloom", true);
    effectRenderPath->SetEnabled("FXAA2", true);
    viewport->SetRenderPath(effectRenderPath);

    cameraRotateNode_->AddComponent(new FPSCameraRotateController(context_, cameraRotateNode_), 0, Urho3D::LOCAL);

    SwitchCameraToFreeCam();
}

// ----------------------------------------------------------------------------
void IceWeasel::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // load scene, delete XML file after use
    scene_ = new Scene(context_);
    XMLFile* xmlScene = cache->GetResource<XMLFile>("Scenes/TestMap.xml");
    if(xmlScene)
        scene_->LoadXML(xmlScene->GetRoot());
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
    if(cameraMoveNode_->HasComponent<FPSCameraMovementController>())
        cameraMoveNode_->RemoveComponent<FPSCameraMovementController>();

    cameraMoveNode_->AddComponent(
        new FreeCameraMovementController(context_, cameraMoveNode_, cameraRotateNode_),
        0, Urho3D::LOCAL
    );
}

// ----------------------------------------------------------------------------
void IceWeasel::SwitchCameraToFPSCam()
{
    if(cameraMoveNode_->HasComponent<FreeCameraMovementController>())
        cameraMoveNode_->RemoveComponent<FreeCameraMovementController>();

    cameraMoveNode_->AddComponent(
        new FPSCameraMovementController(context_, cameraMoveNode_, cameraRotateNode_),
        0, Urho3D::LOCAL
    );
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
    if(key == KEY_F1)
        drawDebugGeometry_ = !drawDebugGeometry_;

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
        cameraModeIsFreeCam_ = !cameraModeIsFreeCam_;
        if(cameraModeIsFreeCam_)
            SwitchCameraToFreeCam();
        else
            SwitchCameraToFPSCam();
    }
#endif
}

// ----------------------------------------------------------------------------
void IceWeasel::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    (void)eventType;
    (void)eventData;
    if(!drawDebugGeometry_)
        return;

    PhysicsWorld* phy = scene_->GetComponent<PhysicsWorld>();
    DebugRenderer* r = scene_->GetComponent<DebugRenderer>();
    if(!phy)
        return;
    phy->DrawDebugGeometry(true);
}

// ----------------------------------------------------------------------------
URHO3D_DEFINE_APPLICATION_MAIN(IceWeasel)
