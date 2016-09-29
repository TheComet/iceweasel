#include "iceweasel/MenuScreens.h"

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/IO/Log.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
MenuScreens::MenuScreens(Context* context) :
    UIElement(context)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* xmlDefaultStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    SetDefaultStyle(xmlDefaultStyle);
    SetLayout(LM_HORIZONTAL);

    GetSubsystem<Input>()->SetMouseVisible(true);

    SubscribeToEvent(E_FILECHANGED, URHO3D_HANDLER(MenuScreens, HandleFileChanged));
    SubscribeToEvent(E_CLICK, URHO3D_HANDLER(MenuScreens, HandleClick));
}

// ----------------------------------------------------------------------------
void MenuScreens::LoadMenuFromFile(String fileName)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    topXML_ = cache->GetResource<XMLFile>(fileName);
    if(!topXML_)
        URHO3D_LOGERRORF("Failed to load XML \"%s\"", fileName.CString());

    Reload();
}

// ----------------------------------------------------------------------------
void MenuScreens::Reload()
{
    // Clean up
    activeScreen_.Reset();
    screens_.Clear();
    actionMap_.Clear();
    RemoveAllChildren();
    if(!topXML_)
        return;

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    XMLElement screenNode = topXML_->GetRoot().GetChild("menu");
    for(; screenNode; screenNode = screenNode.GetNext("menu"))
    {
        String screenFileName = screenNode.GetAttribute("file");
        String screenName = screenNode.GetAttribute("name");

        Screen screen;
        screen.xml = cache->GetResource<XMLFile>(screenFileName);
        if(!screen.xml)
            continue;
        screen.ui = new UIElement(context_);
        AddChild(screen.ui);
        screen.ui->SetLayout(LM_HORIZONTAL);
        screen.ui->LoadXML(screen.xml->GetRoot());
        screen.ui->SetVisible(false);
        screens_.Insert(Pair<StringHash, Screen>(screenName, screen));

        XMLElement actionNode = screenNode.GetChild("action");
        for(; actionNode; actionNode = actionNode.GetNext("action"))
        {
            String sourceName = actionNode.GetAttribute("source");
            String targetName = actionNode.GetAttribute("target");
            actionMap_.Insert(Pair<StringHash, StringHash>(sourceName, targetName));
        }
    }

    SwitchScreenTo(topXML_->GetRoot().GetAttribute("initial"));

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
bool MenuScreens::SwitchScreenTo(StringHash screenName)
{
    HashMap<StringHash, Screen>::Iterator it = screens_.Find(screenName);
    if(it == screens_.End())
        return false;

    if(activeScreen_)
        activeScreen_->SetVisible(false);

    activeScreen_ = it->second_.ui;
    activeScreen_->SetVisible(true);
    return true;
}

// ----------------------------------------------------------------------------
void MenuScreens::HandleFileChanged(StringHash eventType, VariantMap& eventData)
{
    using namespace FileChanged;
    (void)eventType;

    String resourceName = eventData[P_RESOURCENAME].GetString();
    if(topXML_ && topXML_->GetName() == resourceName)
    {
        URHO3D_LOGINFOF("[MenuScreen] Reloading %s", topXML_->GetName().CString());
        Reload();
    }
}

// ----------------------------------------------------------------------------
void MenuScreens::HandleClick(StringHash eventType, VariantMap& eventData)
{
    using namespace Click;
    (void)eventType;

    // Check if the UI element has been hooked up to an action
    UIElement* element = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
    HashMap<StringHash, StringHash>::Iterator action = actionMap_.Find(element->GetName());
    if(action == actionMap_.End())
        return;

    // Action could be a screen name, in which case switch screens to it
    if(SwitchScreenTo(action->second_))
        return;
}

// ----------------------------------------------------------------------------
void MenuScreens::Action_LocalGame()
{

}

// ----------------------------------------------------------------------------
void MenuScreens::Action_JoinServer()
{

}

// ----------------------------------------------------------------------------
void MenuScreens::Action_Options()
{

}

// ----------------------------------------------------------------------------
void MenuScreens::Action_Quit()
{
    GetSubsystem<Engine>()->Exit();
}
