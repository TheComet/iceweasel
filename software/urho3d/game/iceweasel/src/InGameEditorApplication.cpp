#include "iceweasel/InGameEditorApplication.h"
#include "iceweasel/IceWeasel.h"
#include "iceweasel/Args.h"

#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/LuaScript/LuaScript.h>
#include <Urho3D/AngelScript/ScriptFile.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Graphics/Graphics.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
InGameEditorApplication::InGameEditorApplication(Urho3D::Context* context, Args* args) :
    Application(context),
    args_(args)
{
}

// ----------------------------------------------------------------------------
void InGameEditorApplication::Setup()
{
    engineParameters_["WindowTitle"] = "IceWeasel";
    engineParameters_["FullScreen"]  = args_->fullscreen_;
    engineParameters_["Headless"]    = args_->server_;
    engineParameters_["Multisample"] = args_->multisample_;
    engineParameters_["VSync"] = args_->vsync_;

    engineParameters_["WindowResizable"] = true;
}

// ----------------------------------------------------------------------------
void InGameEditorApplication::Start()
{
    URHO3D_LOGINFO("Opening Editor");

    RegisterIceWeaselMods(context_);
    RegisterSubsystems();

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    for(StringVector::ConstIterator it = args_->resourcePaths_.Begin(); it != args_->resourcePaths_.End(); ++it)
        cache->AddResourceDir(*it);

    script_ = cache->GetResource<ScriptFile>("Scripts/Editor.as");
    if(script_ && script_->Execute("void Start()"))
    {
        // Subscribe to script's reload event to allow live-reload of the application
        SubscribeToEvent(script_, E_RELOADSTARTED, URHO3D_HANDLER(InGameEditorApplication, HandleScriptReloadStarted));
        SubscribeToEvent(script_, E_RELOADFINISHED, URHO3D_HANDLER(InGameEditorApplication, HandleScriptReloadFinished));
        SubscribeToEvent(script_, E_RELOADFAILED, URHO3D_HANDLER(InGameEditorApplication, HandleScriptReloadFailed));
    }
}

// ----------------------------------------------------------------------------
void InGameEditorApplication::RegisterSubsystems()
{
    context_->RegisterSubsystem(new Script(context_));
    context_->RegisterSubsystem(new LuaScript(context_));
}

// ----------------------------------------------------------------------------
void InGameEditorApplication::HandleScriptReloadStarted(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGINFO("Reloading editor script");

    if(script_ && script_->GetFunction("void Stop()"))
        script_->Execute("void Stop()");
}

// ----------------------------------------------------------------------------
void InGameEditorApplication::HandleScriptReloadFailed(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGERROR("Failed reloading editor script");
}

// ----------------------------------------------------------------------------
void InGameEditorApplication::HandleScriptReloadFinished(StringHash eventType, VariantMap& eventData)
{
    if(script_ && !script_->Execute("void Start()"))
    {
        URHO3D_LOGERROR("Failed to call Start() function in editor script");
    }
    else
    {
        URHO3D_LOGINFO("Finished reloading editor script");
    }
}
