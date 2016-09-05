#include "iceweasel/MainMenu.h"

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/IO/Log.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
MainMenu::MainMenu(Context* context) :
    UIElement(context)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* xmlDefaultStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    SetDefaultStyle(xmlDefaultStyle);
    SetLayout(LM_HORIZONTAL);

    GetSubsystem<Input>()->SetMouseVisible(true);

    xml_ = cache->GetResource<XMLFile>("UI/MainMenu.xml");
    Reload();

    SubscribeToEvent(E_FILECHANGED, URHO3D_HANDLER(MainMenu, HandleFileChanged));
    SubscribeToEvent(E_CLICK, URHO3D_HANDLER(MainMenu, HandleClick));
}

// ----------------------------------------------------------------------------
void MainMenu::Reload()
{
    RemoveAllChildren();
    if(!xml_)
        return;

    String elementXMLFile = xml_->GetRoot().GetAttribute("file");
    xmlElements_ = GetSubsystem<ResourceCache>()->GetResource<XMLFile>(elementXMLFile);
    if(!xmlElements_)
    {
        URHO3D_LOGERRORF("Failed to load XML \"%s\"", elementXMLFile.CString());
        return;
    }
    LoadXML(xmlElements_->GetRoot());

    /*
     * Update our layout so the UI elements defined in the layout are able to
     * calculate their preferred sizes, then position ourselves in the centre
     * of the screen.
     */
    UpdateLayout();
    const IntVector2& rootSize = GetSubsystem<UI>()->GetRoot()->GetSize();
    const IntVector2& thisSize = GetSize();
    SetPosition((rootSize - thisSize) / 2);

    // Map UI element actions to member function pointers.
    LoadActions();
}

// ----------------------------------------------------------------------------
void MainMenu::LoadActions()
{
    /*
     * An action consists of three things.
     *  1) A "target", which contains the name of a user defined action (see
     *     HashToMemberFunctionPointer() for a list of actions). The target
     *     string gets mapped to a member function in this class, or, if it
     *     doesn't exist, the action will instead be ignored.
     *  2) A "source", which contains the name of the UI element that is
     *     supposed to trigger the action.
     *  3) A "type", which is the event name to listen to (such as "Click" or
     *     "Release").
     */

    actionMap_.Clear();
    if(!xml_)
        return;

    for(XMLElement actionNode = xml_->GetRoot().GetChild("action");
        actionNode;
        actionNode = actionNode.GetNext("action"))
    {
        Action action;

        String targetStr = actionNode.GetAttribute("target");
        String typeStr   = actionNode.GetAttribute("type");
        String sourceStr = actionNode.GetAttribute("source");

        action.func_ = HashToMemberFunctionPointer(targetStr);
        action.type_ = typeStr;

        if(action.func_ == NULL)
        {
            URHO3D_LOGERRORF("[MainMenu] Unknown action \"%s\"", targetStr.CString());
            continue;
        }
        if(typeStr == "")
        {
            URHO3D_LOGERRORF("[MainMenu] Type of action wasn't specified!");
            continue;
        }
        if(sourceStr == "")
        {
            URHO3D_LOGERRORF("[MainMenu] Source of action wasn't specified!");
            continue;
        }

        actionMap_[sourceStr] = action;
    }
}

// ----------------------------------------------------------------------------
void MainMenu::HandleFileChanged(StringHash eventType, VariantMap& eventData)
{
    using namespace FileChanged;
    (void)eventType;

    String resourceName = eventData[P_RESOURCENAME].GetString();
    if(xml_         && xml_->GetName()         == resourceName ||
       xmlElements_ && xmlElements_->GetName() == resourceName)
    {
        URHO3D_LOGINFOF("[MainMenu] Reloading %s", xml_->GetName().CString());
        Reload();
    }
}

// ----------------------------------------------------------------------------
void MainMenu::HandleClick(StringHash eventType, VariantMap& eventData)
{
    using namespace Click;
    (void)eventType;

    // Check if the UI element has been hooked up to an action
    UIElement* element = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
    ActionMap::Iterator it = actionMap_.Find(element->GetName());
    if(it == actionMap_.End())
        return;

    // Check if the action is the correct type
    if(it->second_.type_ != E_CLICK)
        return;

    // Action maps to a member function. Call it.
    (this->*it->second_.func_)();
}

// ----------------------------------------------------------------------------
MainMenu::ActionFunc MainMenu::HashToMemberFunctionPointer(Urho3D::StringHash hash)
{
    static const Urho3D::StringHash ACTION_LOCAL_GAME ("LocalGame");
    static const Urho3D::StringHash ACTION_JOIN_SERVER("JoinServer");
    static const Urho3D::StringHash ACTION_OPTIONS    ("Options");
    static const Urho3D::StringHash ACTION_QUIT       ("Quit");

    if(hash == ACTION_LOCAL_GAME)  return &MainMenu::Action_LocalGame;
    if(hash == ACTION_JOIN_SERVER) return &MainMenu::Action_JoinServer;
    if(hash == ACTION_OPTIONS)     return &MainMenu::Action_Options;
    if(hash == ACTION_QUIT)        return &MainMenu::Action_Quit;

    return NULL;
}

// ----------------------------------------------------------------------------
void MainMenu::Action_LocalGame()
{

}

// ----------------------------------------------------------------------------
void MainMenu::Action_JoinServer()
{

}

// ----------------------------------------------------------------------------
void MainMenu::Action_Options()
{

}

// ----------------------------------------------------------------------------
void MainMenu::Action_Quit()
{
    GetSubsystem<Engine>()->Exit();
}
