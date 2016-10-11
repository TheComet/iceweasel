#include "iceweasel/PlayerController.h"
#include "iceweasel/PlayerAnimationStatesController.h"
#include "iceweasel/CameraControllerRotation.h"
#include "iceweasel/CameraControllerEvents.h"
#include "iceweasel/MovementController.h"
#include "iceweasel/Math.h"
#include "iceweasel/Finger.h"
#include "iceweasel/IceWeaselConfig.h"
#include "iceweasel/IceWeaselConfigEvents.h"
#include "iceweasel/GravityManager.h"

#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/SceneEvents.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
PlayerController::PlayerController(Context* context, Node* moveNode, Node* offsetNode, Node* rotateNode) :
    LogicComponent(context),
    moveNode_(moveNode),
    offsetNode_(offsetNode),
    rotateNode_(rotateNode),
    currentYAngle_(0.0f),
    storeViewMask_(0)
{
    SetMode(FIRST_PERSON);
}

// ----------------------------------------------------------------------------
void PlayerController::SetMode(Mode mode)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    mode_ = mode;

    switch(mode_)
    {
        case FIRST_PERSON:
            cameraOffset_.SetTarget(0);
            break;

        case THIRD_PERSON:
            cameraOffset_.SetTarget(-config.camera.transition.distance);
            break;

        case FREE_CAM:
            URHO3D_LOGERROR("FreeCam is currently not implemented");
            break;
    }
}

// ----------------------------------------------------------------------------
void PlayerController::SetPlayerVisible(bool visible)
{
    AnimatedModel* model = modelNode_->GetComponent<AnimatedModel>();
    if(!model)
        return;

    if(visible)
        model->SetViewMask(storeViewMask_);
    else
        model->SetViewMask(0);
}

// ----------------------------------------------------------------------------
void PlayerController::Start()
{
    CreateComponents();

    // Needs to always exist
    modelNode_ = moveNode_->CreateChild("Player", LOCAL);

    /*
     * The player is created in the editor and saved (with animation states)
     * as a node. Load it into the model node.
     */
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* modelNodeXML = cache->GetResource<XMLFile>("Models/Binky.xml");
    if(!modelNodeXML)
        return;
    modelNode_->LoadXML(modelNodeXML->GetRoot());
    AnimatedModel* model = modelNode_->GetComponent<AnimatedModel>();
    if(model)
        storeViewMask_ = model->GetViewMask();

    // Let the animation controller component set the different animation state
    // weights
    modelNode_->AddComponent(new PlayerAnimationStatesController(context_), 0, LOCAL);

    SubscribeToEvent(E_LOCALMOVEMENTVELOCITYCHANGED, URHO3D_HANDLER(PlayerController, HandleLocalMovementVelocityChanged));
}

// ----------------------------------------------------------------------------
void PlayerController::Stop()
{
    modelNode_->Remove();

    DestroyComponents();
}

// ----------------------------------------------------------------------------
void PlayerController::CreateComponents()
{
    // Add the movement controller to the movement node
    moveNode_->AddComponent(new MovementController(context_, moveNode_, offsetNode_), 0, LOCAL);

    // The mouse controls the angle of camera node
    offsetNode_->AddComponent(new CameraControllerRotation(context_), 0, LOCAL);

    // Attach hand model
    rotateNode_->AddComponent(new Finger(context_), 0, LOCAL);
}

// ----------------------------------------------------------------------------
void PlayerController::DestroyComponents()
{
    // Clean up the components we added
    rotateNode_->RemoveComponent<Finger>();
    offsetNode_->RemoveComponent<CameraControllerRotation>();
    rotateNode_->RemoveComponent<MovementController>();
}

// ----------------------------------------------------------------------------
void PlayerController::Update(float timeStep)
{
    const IceWeaselConfig::Data& config = GetSubsystem<IceWeaselConfig>()->GetConfig();

    // Control camera offset
    rotateNode_->SetPosition(Vector3(0, 0, cameraOffset_.Advance(timeStep * config.camera.transition.speed)));
    float factor = -cameraOffset_.value_ / config.camera.transition.distance;
    if(factor < 0.1)
    {
        SetPlayerVisible(false);
        rotateNode_->GetComponent<Finger>()->SetVisible(true);
    }
    else
    {
        SetPlayerVisible(true);
        rotateNode_->GetComponent<Finger>()->SetVisible(false);
    }

    // X and Z rotate model depending on acceleration
    acceleration_.SetTarget((
            Vector2(oldLocalVelocity_.x_, oldLocalVelocity_.z_) -
            Vector2(currentLocalVelocity_.x_, currentLocalVelocity_.z_)
        ) / Max(M_EPSILON, timeStep) * config.playerClass(0).lean.amount);
    acceleration_.Advance(timeStep * config.playerClass(0).lean.speed);

    Quaternion targetRotation = Quaternion(acceleration_.value_.x_, Vector3::FORWARD);
    targetRotation = targetRotation * Quaternion(acceleration_.value_.y_, Vector3::LEFT);

    // Y rotate model in local space towards the direction it is moving
    if(Abs(currentLocalVelocity_.x_) + Abs(currentLocalVelocity_.z_) > M_EPSILON*100)
        currentYAngle_ = Atan2(currentLocalVelocity_.x_, currentLocalVelocity_.z_);
    targetRotation = targetRotation * Quaternion(currentYAngle_, Vector3::UP);

    modelNode_->SetRotation(modelNode_->GetRotation().Nlerp(targetRotation, Min(1.0f, config.playerClass(0).turn.speed * timeStep), true));

    oldLocalVelocity_ = currentLocalVelocity_;
}

// ----------------------------------------------------------------------------
void PlayerController::HandleLocalMovementVelocityChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace LocalMovementVelocityChanged;
    currentLocalVelocity_ = eventData[P_LOCALMOVEMENTVELOCITY].GetVector3();
}
