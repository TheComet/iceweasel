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

Finger::Finger(Urho3D::Context* context) :
        Urho3D::LogicComponent(context)
{
}

void Finger::Start()
{
    fingerNode_ = new Urho3D::Node(
        context_
    );
    node_->AddChild(
            fingerNode_, 0
    );

    Urho3D::Quaternion rotation = Urho3D::Quaternion();
    rotation.FromAngleAxis(80, Urho3D::Vector3(0, 1, 0));
    fingerNode_->SetTransform(Urho3D::Vector3(0.5, -0.5, 1.0), rotation, 1.2);

    Urho3D::ResourceCache* cache = GetSubsystem<Urho3D::ResourceCache>();
    model_ = cache->GetResource<Urho3D::Model>("Models/Hand_01.mdl");
    if (!model_)
    {
        URHO3D_LOGERROR("Model not found, cannot initialize example scene");
        return;
    }

    Urho3D::StaticModel* staticModel = fingerNode_->CreateComponent<Urho3D::StaticModel>();
    staticModel->SetModel(model_);
}

void Finger::Stop()
{
    fingerNode_.Get()->Remove();
}