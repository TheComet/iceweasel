#include "iceweasel/IceWeaselConfig.h"
#include "iceweasel/IceWeaselConfigEvents.h"

#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Resource/ResourceEvents.h>

using namespace Urho3D;


// ----------------------------------------------------------------------------
const IceWeaselConfig::Data::PlayerClass& IceWeaselConfig::Data::playerClass(unsigned int index) const
{
    static const Data::PlayerClass defaultPlayerClass = {
        "",
        {1, 1, 1, 1, 1},
        {1, 1},
        {1, 1, 1, 1}
    };
    if(playerClassContainer.Size() <= index)
    {
        URHO3D_LOGERRORF("[IceWeaselConfig] Failed to read player class info \"%d\" from settings", index);
        return defaultPlayerClass;
    }
    return playerClassContainer.At(index);
}


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
    if(!xml_)
    {
        URHO3D_LOGERROR("[IceWeaselSettings] Failed to load XML file");
        return;
    }

    XMLElement root = xml_->GetRoot();

    // Load input settings
    XMLElement input = root.GetChild("Input");
    {
        XMLElement mouse = input.GetChild("Mouse");
        data_.input.mouse.sensitivity = mouse.GetFloat("Sensitivity");
    }

    // Load player class parameters
    XMLElement player = root.GetChild("Player");
    data_.playerClassContainer.Clear();
    for(; player; player = player.GetNext("Player"))
    {
        Data::PlayerClass playerClass;
        playerClass.className = player.GetAttribute("Class");

        XMLElement body = player.GetChild("Body");
        playerClass.body.width        = body.GetFloat("Width");
        playerClass.body.height       = body.GetFloat("Height");
        playerClass.body.mass         = body.GetFloat("Mass");
        playerClass.body.crouchWidth  = body.GetFloat("CrouchWidth");
        playerClass.body.crouchHeight = body.GetFloat("CrouchHeight");

        XMLElement jump = player.GetChild("Jump");
        playerClass.jump.force = jump.GetFloat("Force");
        playerClass.jump.bunnyHopBoost = jump.GetFloat("BunnyHopBoost");

        XMLElement speed = player.GetChild("Speed");
        playerClass.speed.crouch                = speed.GetFloat("Crouch");
        playerClass.speed.walk                  = speed.GetFloat("Walk");
        playerClass.speed.run                   = speed.GetFloat("Run");
        playerClass.speed.crouchTransitionSpeed = speed.GetFloat("CrouchTransitionSpeed");
        playerClass.speed.jounceSpeed           = speed.GetFloat("JounceSpeed");
        playerClass.speed.accelerateSpeed       = speed.GetFloat("AccelerateSpeed");

        XMLElement turn = player.GetChild("Turn");
        playerClass.turn.speed                  = turn.GetFloat("Speed");

        XMLElement lean = player.GetChild("Lean");
        playerClass.lean.amount                 = lean.GetFloat("Amount");
        playerClass.lean.speed                  = lean.GetFloat("Speed");

        XMLElement animations = player.GetChild("Animations");
        for(unsigned i = 0; i != PlayerAnimation::NUM_ANIMATIONS; ++i)
        {
            playerClass.animations[i].speed = animations.GetChild(PlayerAnimation::name[i]).GetFloat("Speed");
            playerClass.animations[i].transitionSpeed = animations.GetChild(PlayerAnimation::name[i]).GetFloat("TransitionSpeed");

            if(playerClass.animations[i].speed == 0.0f)
                playerClass.animations[i].speed = 1.0f;

            if(playerClass.animations[i].transitionSpeed == 0.0f)
                playerClass.animations[i].transitionSpeed = animations.GetFloat("DefaultTransitionSpeed");
        }

        data_.playerClassContainer.Push(playerClass);
    }

    // Load camera parameters
    XMLElement camera = root.GetChild("Camera");
    {
        XMLElement transition = camera.GetChild("Transition");
        data_.camera.transition.speed    = transition.GetFloat("Speed");
        data_.camera.transition.distance = transition.GetFloat("Distance");
    }

    // Load freecam parameters
    XMLElement freeCam = root.GetChild("FreeCam");
    {
        XMLElement speed = freeCam.GetChild("Speed");
        data_.freeCam.speed.normal     = speed.GetFloat("Normal");
        data_.freeCam.speed.fast       = speed.GetFloat("Fast");
        data_.freeCam.speed.smoothness = speed.GetFloat("Smoothness");
    }

    SendEvent(E_CONFIGRELOADED, GetEventDataMap());
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
