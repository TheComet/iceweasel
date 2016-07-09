#include "iceweasel/IceWeasel.h"
#include "iceweasel/CameraController.h"

#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/IceWeaselMods/Gravity.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>
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

#include "iceweasel/Tetrahedron.h"

#include <iostream>


using namespace Urho3D;

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
    engineParameters_["FullScreen"]  = true;
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

    playerLocationText_ = root->CreateChild<Text>();
    playerLocationText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 12);
    playerLocationText_->SetPosition(0, 0);

#ifdef DEBUG
    instructionText_ = root->CreateChild<Text>();
    instructionText_->SetText(
        "Use WASD and mouse to move, Space/CTRL to move up/down in freecam mode\n"
        "Press 1 to toggle debug geometry\n"
        "Press 2 to toggle profiling information\n"
        "Press 5 to toggle camera modes\n"
        "Press 0 to toggle this text"
    );
    instructionText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
    instructionText_->SetTextAlignment(HA_CENTER);
    instructionText_->SetHorizontalAlignment(HA_CENTER);
    instructionText_->SetVerticalAlignment(VA_CENTER);
    instructionText_->SetPosition(0, -root->GetHeight() / 4);
#endif
/*
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
    buttonText->SetStyleAuto();*/
}

// ----------------------------------------------------------------------------
void IceWeasel::CreateCamera()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Renderer* renderer = GetSubsystem<Renderer>();

    // The camera is attached to a "rotate node", which is in turn attached to
    // a "move" node.
    cameraMoveNode_ = scene_->CreateChild("Camera Move");
    cameraRotateNode_ = cameraMoveNode_->CreateChild("Camera Rotate");
    Camera* camera = cameraRotateNode_->CreateComponent<Camera>(Urho3D::LOCAL);
    camera->SetFarClip(300.0f);
    cameraMoveNode_->SetPosition(Vector3(0.0f, 5.0f, -0.0f));
    cameraRotateNode_->SetPosition(Vector3::ZERO);
    cameraMoveNode_->AddComponent(
        new CameraController(context_, cameraMoveNode_, cameraRotateNode_),
        0,
        Urho3D::LOCAL
    );

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
    CameraController* controller = cameraMoveNode_->GetComponent<CameraController>();
    controller->SetMode(CameraController::FREE);
}

// ----------------------------------------------------------------------------
void IceWeasel::SwitchCameraToFPSCam()
{
    CameraController* controller = cameraMoveNode_->GetComponent<CameraController>();
    controller->SetMode(CameraController::FPS);
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

    // toggle instruction text
    if(key == KEY_0)
        instructionText_->SetVisible(!instructionText_->IsVisible());
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

    // Tetrahedron test stuff, will be removed
    Tetrahedron t(
        Vector3(0, 10, 0),
        Vector3(0, 0, 0),
        Vector3(-5, 5, 10),
        Vector3(5, 5, 10)
    );
    Vector3 playerPos = cameraMoveNode_->GetWorldPosition() + cameraRotateNode_->GetDirection();
    debugRenderer->AddSphere(Sphere(playerPos, 0.04), Color::RED, depthTest);
    t.DrawDebugGeometry(debugRenderer, depthTest, Color::GRAY);
    if(t.PointLiesInside(playerPos))
        t.DrawDebugGeometry(debugRenderer, depthTest, Color::BLUE);

    Tetrahedron t1(
        Vector3(0, 10, 20),
        Vector3(0, 0, 20),
        Vector3(-5, 5, 30),
        Vector3(5, 5, 30)
    );
    if(t1.PointLiesInside(playerPos))
    {
        t1.DrawDebugGeometry(debugRenderer, depthTest, Color::BLUE);
        Vector4 bary = t1.TransformToBarycentric(playerPos);
        Vector3 back = t1.TransformToCartesian(bary);
        debugRenderer->AddSphere(Sphere(back, 0.1), Color::RED, depthTest);
        debugRenderer->AddLine(playerPos, back, Color::RED, depthTest);

        String location;
        location.AppendWithFormat(
            "Barycentric coordinates: %f,%f,%f,%f\n"
            "Inside: %s", bary.x_, bary.y_, bary.z_, bary.w_, t1.PointLiesInside(playerPos) ? "Yes" : "No");
        playerLocationText_->SetText(location);
        playerLocationText_->SetVisible(true);
    }
    else
    {
        t1.DrawDebugGeometry(debugRenderer, depthTest, Color::GRAY);
        playerLocationText_->SetVisible(false);
    }

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
            Gravity* gravity = scene_->GetComponent<Gravity>();
            if(gravity)
                gravity->DrawDebugGeometry(debugRenderer, depthTest);
            break;
        }
    }
}

// ----------------------------------------------------------------------------
URHO3D_DEFINE_APPLICATION_MAIN(IceWeasel)
