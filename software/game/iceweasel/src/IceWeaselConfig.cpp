#include "iceweasel/IceWeaselConfig.h"

#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Resource/ResourceEvents.h>

using namespace Urho3D;


// ----------------------------------------------------------------------------
IceWeaselConfig::IceWeaselConfig(Context* context) :
    Object(context)
{
    SubscribeToEvent(E_FILECHANGED, URHO3D_HANDLER(IceWeaselConfig, HandleFileChanged));
}

// ----------------------------------------------------------------------------
void IceWeaselConfig::Load(String fileName)
{
    xml_ = GetSubsystem<ResourceCache>()->GetResource<XMLFile>(fileName);
    Reload();
}

// ----------------------------------------------------------------------------
void IceWeaselConfig::LoadXML(XMLFile* xml)
{
    xml_ = xml;
    Reload();
}

// ----------------------------------------------------------------------------
void IceWeaselConfig::Reload()
{

    // TODO Is it necessary to get the XML file from the cache every time?
    // what if we just hold our own reference to it? Does it still reload?

    if(!xml_)
    {
        URHO3D_LOGERROR("[IceWeaselSettings] Failed to load XML file");
        return;
    }

    XMLElement root = xml_->GetRoot();

    XMLElement player = root.GetChild("Player");
    data_.playerClass.Clear();
    for(; player; player = player.GetNext("Player"))
    {
        Data::PlayerClass playerClass;
        playerClass.className = player.GetAttribute("Class");

        XMLElement body = player.GetChild("Body");
        playerClass.body.width = body.GetFloat("Width");
        playerClass.body.height = body.GetFloat("Height");
        playerClass.body.mass = body.GetFloat("Mass");

        XMLElement jump = player.GetChild("Jump");
        playerClass.jump.force = jump.GetFloat("Force");
        playerClass.jump.bunnyHopBoost = jump.GetFloat("BunnyHopBoost");

        data_.playerClass.Push(playerClass);
    }
}

// ----------------------------------------------------------------------------
const IceWeaselConfig::Data& IceWeaselConfig::GetConfig() const
{
    return data_;
}

// ----------------------------------------------------------------------------
void IceWeaselConfig::HandleFileChanged(StringHash eventType, VariantMap& eventData)
{
    using namespace FileChanged;
    (void)eventType;


    if(xml_ && xml_->GetName() == eventData[P_RESOURCENAME].GetString())
    {
        URHO3D_LOGINFO("[IceWeaselSettings] Reloading config");
        Reload();
    }
}
