#include "iceweasel/MainMenu.h"

#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/Text.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
MainMenu::MainMenu(Context* context) :
    UIElement(context)
{
    Input* input = GetSubsystem<Input>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    mouseWasVisible_ = input->IsMouseVisible();
    input->SetMouseVisible(true);

    root_ = GetSubsystem<UI>()->GetRoot()->CreateChild<UIElement>("MainMenu");
    root_->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

    root_->LoadXML();

}
// ----------------------------------------------------------------------------
MainMenu::~MainMenu()
{
    GetSubsystem<UI>()->GetRoot()->RemoveChild(root_);

    GetSubsystem<Input>()->SetMouseVisible(mouseWasVisible_);
}

// ----------------------------------------------------------------------------
void MainMenu::SetupUI()
{
    Text* text_joinServer = new Text(context_);
    text_joinServer->SetText("Join Server");

    Text* text_options = new Text(context_);
    text_options->SetText("Options");

    Text* text_quit = new Text(context_);
    text_quit->SetText("Quit");

    Button* button_joinServer = new Button(context_);
    button_joinServer->AddChild(text_joinServer);

    Button* button_options = new Button(context_);
    button_options->AddChild(text_options);

    Button* button_quit = new Button(context_);
    button_quit->AddChild(text_quit);

    Window* window = new Window(context_);
    window->AddChild(button_joinServer);
    window->AddChild(button_options);
    window->AddChild(button_quit);

    root_->AddChild(window);
}
