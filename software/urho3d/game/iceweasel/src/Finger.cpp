//
// Created by FN on 28/08/16.
//

#include "iceweasel/Finger.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/Quaternion.h>
#include <Urho3D/Resource/XMLFile.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
Finger::Finger(Context* context) :
        LogicComponent(context)
{
}

// ----------------------------------------------------------------------------
void Finger::SetVisible(bool visible)
{
    if(fingerNode_)
        fingerNode_->SetEnabled(visible);
}

// ----------------------------------------------------------------------------
void Finger::ReloadGun()
{
    // Reset transform, as the script might not define position/rotation/scale
    fingerNode_->SetTransform(Vector3::ZERO, Quaternion::IDENTITY);

    if(xml_)
        fingerNode_->LoadXML(xml_->GetRoot());
}

// ----------------------------------------------------------------------------
void Finger::Start()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // Always create finger node, even if loading XML fails.
    fingerNode_ = node_->CreateChild("Finger", LOCAL);

    // Load finger prefab
    xml_ = cache->GetResource<XMLFile>("Prefabs/FuckYouFinger.xml");
    if(!xml_)
        URHO3D_LOGERROR("Couldn't find Prefabs/FuckYouFinger.xml. Can't load gun.");

    ReloadGun();

    SubscribeToEvent(E_FILECHANGED, URHO3D_HANDLER(Finger, HandleFileChanged));
}

// ----------------------------------------------------------------------------
void Finger::Stop()
{
    if(fingerNode_)
        fingerNode_->Remove();
}

// ----------------------------------------------------------------------------
void Finger::HandleFileChanged(StringHash eventType, VariantMap& eventData)
{
    using namespace FileChanged;
    (void)eventType;

    if(xml_ && xml_->GetName() == eventData[P_RESOURCENAME].GetString())
    {
        ReloadGun();
    }
}
