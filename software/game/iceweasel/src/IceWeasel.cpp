#include "iceweasel/IceWeasel.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Physics/PhysicsWorld.h>
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
    drawDebugGeometry_(false)
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
    engineParameters_["VSync"] = false;
}

// ----------------------------------------------------------------------------
void IceWeasel::Start()
{
    // configure resource cache
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    cache->SetAutoReloadResources(true);

    CreateScene();
    CreateCamera();
    CreateUI();

    // Shows mouse and allows it to exit the window boundaries
    GetSubsystem<Input>()->SetMouseVisible(true);

    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(IceWeasel, HandleKeyDown));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(IceWeasel, HandlePostRenderUpdate));
}

// ----------------------------------------------------------------------------
void IceWeasel::Stop()
{
    cameraNode_.Reset();

    scene_.Reset();
}

// ----------------------------------------------------------------------------
void IceWeasel::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // load scene, delete XML file after use
    scene_ = new Scene(context_);
    XMLFile* xmlScene = cache->GetResource<XMLFile>("Scenes/Test.xml");
    if(xmlScene)
        scene_->LoadXML(xmlScene->GetRoot());
}

// ----------------------------------------------------------------------------
void IceWeasel::CreateCamera()
{
    cameraNode_ = scene_->CreateChild("Camera");
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(300.0f);
    cameraNode_->SetPosition(Vector3(0.0f, 5.0f, -20.0f));

    Viewport* viewport = new Viewport(context_, scene_, camera);
    viewport->SetDrawDebug(true);
    GetSubsystem<Renderer>()->SetViewport(0, viewport);
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
void IceWeasel::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;
    (void)eventType;

    // Check for pressing ESC
    int key = eventData[P_KEY].GetInt();
    if(key == KEY_ESCAPE)
        engine_->Exit();

    // Toggle debug geometry
    if(key == KEY_P)
        drawDebugGeometry_ = !drawDebugGeometry_;
}

// ----------------------------------------------------------------------------
void IceWeasel::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    (void)eventType;
    (void)eventData;
    if(!drawDebugGeometry_)
        return;

    PhysicsWorld* phy = scene_->GetComponent<PhysicsWorld>();
    if(!phy)
        return;
    phy->DrawDebugGeometry(true);
}

// ----------------------------------------------------------------------------
URHO3D_DEFINE_APPLICATION_MAIN(IceWeasel)
