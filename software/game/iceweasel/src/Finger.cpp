//
// Created by FN on 28/08/16.
//

#include "iceweasel/Finger.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/Quaternion.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
Finger::Finger(Context* context) :
        LogicComponent(context),
        storeViewMask_(0)
{
}

// ----------------------------------------------------------------------------
void Finger::SetVisible(bool visible)
{
    StaticModel* model = fingerNode_->GetComponent<StaticModel>();
    if(!model)
        return;

    if(visible)
        model->SetViewMask(storeViewMask_);
    else
        model->SetViewMask(0);
}

// ----------------------------------------------------------------------------
void Finger::Start()
{
    fingerNode_ = new Node(
        context_
    );
    node_->AddChild(
            fingerNode_, 0
    );

    Quaternion rotation = Quaternion();
    rotation.FromAngleAxis(80, Vector3(0, 1, 0));
    fingerNode_->SetTransform(Vector3(0.5, -0.5, 1.0), rotation, 1.2);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    model_ = cache->GetResource<Model>("Models/Hand_01.mdl");
    if (!model_)
    {
        URHO3D_LOGERROR("Model not found, cannot initialize example scene");
        return;
    }

    StaticModel* staticModel = fingerNode_->CreateComponent<StaticModel>();
    staticModel->SetModel(model_);
    storeViewMask_ = staticModel->GetViewMask();
}

// ----------------------------------------------------------------------------
void Finger::Stop()
{
    fingerNode_->Remove();
}
