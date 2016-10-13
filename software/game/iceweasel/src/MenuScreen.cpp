#include "iceweasel/MenuScreen.h"

#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/Text.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
MenuScreen::MenuScreen(Context* context) :
    UIElement(context)
{
    Input* input = GetSubsystem<Input>();
    mouseWasVisible_ = input->IsMouseVisible();
    input->SetMouseVisible(true);

    SetLayout(LM_HORIZONTAL);

    SubscribeToEvent(E_FILECHANGED, URHO3D_HANDLER(MenuScreen, HandleFileChanged));
}

// ----------------------------------------------------------------------------
MenuScreen::~MenuScreen()
{
    GetSubsystem<Input>()->SetMouseVisible(mouseWasVisible_);
}

// ----------------------------------------------------------------------------
void MenuScreen::LoadUI(const String& xmlFileName)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    xml_ = cache->GetResource<XMLFile>(xmlFileName);
    if(!xml_)
    {
        URHO3D_LOGERRORF("Failed to load XML file \"%s\"", xmlFileName.CString());
        return;
    }

    ReloadUI();
    SetVisible(false);
}

// ----------------------------------------------------------------------------
void MenuScreen::ReloadUI()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    if(ui_)
        RemoveChild(ui_);
    ui_ = LoadChildXML(
        xml_->GetRoot(),
        cache->GetResource<XMLFile>("UI/DefaultStyle.xml")
    );

    /*
     * Update our layout so the UI elements defined in the layout are able to
     * calculate their preferred sizes, then position ourselves in the centre
     * of the screen.
     */
    UpdateLayout();
    const IntVector2& rootSize = GetSubsystem<UI>()->GetRoot()->GetSize();
    const IntVector2& thisSize = GetSize();
    SetPosition((rootSize - thisSize) / 2);
}

// ----------------------------------------------------------------------------
void MenuScreen::HandleFileChanged(StringHash eventType, VariantMap& eventData)
{
    using namespace FileChanged;

    if(xml_ && xml_->GetName() == eventData[P_RESOURCENAME].GetString())
    {
        ReloadUI();
    }
}
