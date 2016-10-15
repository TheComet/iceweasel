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

    /*
    StaticModel* model = fingerNode_->GetComponent<StaticModel>();
    if(!model)
        return;

    if(visible)
        model->SetViewMask(storeViewMask_);
    else
        model->SetViewMask(0);*/
}

// ----------------------------------------------------------------------------
void Finger::Start()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    SubscribeToEvent(E_FILECHANGED, URHO3D_HANDLER(Finger, HandleFileChanged));

    xml_ = cache->GetResource<XMLFile>("Prefabs/HEP-R34.xml");
    if(!xml_)
    {
        URHO3D_LOGERROR("Couldn't find Prefabs/HEP-R34.xml. Can't load gun.");
        return;
    }

    fingerNode_ = node_->CreateChild("Finger", LOCAL);
    fingerNode_->LoadXML(xml_->GetRoot());

    /*
    Quaternion rotation = Quaternion();
    rotation.FromAngleAxis(80, Vector3(0, 1, 0));
    fingerNode_->SetTransform(Vector3(0.5, -0.5, 1.0), rotation, 1.2);

    model_ = cache->GetResource<Model>("Models/Hand_01.mdl");
    if (!model_)
    {
        URHO3D_LOGERROR("Model not found, cannot initialize example scene");
        return;
    }

    StaticModel* staticModel = fingerNode_->CreateComponent<StaticModel>();
    staticModel->SetModel(model_);
    storeViewMask_ = staticModel->GetViewMask();*/
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
        if(fingerNode_)
            fingerNode_->LoadXML(xml_->GetRoot());
    }
}
