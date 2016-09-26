#include "iceweasel/InGameEditor.h"

#include <Urho3D/AngelScript/ScriptFile.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
InGameEditor::InGameEditor(Urho3D::Context* context) :
    Object(context)
{
}

// ----------------------------------------------------------------------------
void InGameEditor::OpenEditor(Urho3D::Scene* scene)
{
    if(scriptFile_)
        return;

    scene_ = scene;
    URHO3D_LOGINFO("Opening Editor");

    VariantVector args;
    args.Push(Variant(scene_));

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    scriptFile_ = cache->GetResource<ScriptFile>("Scripts/Editor.as");
    if(scriptFile_ && scriptFile_->Execute("void Start(Scene@)", args))
    {
        // Subscribe to script's reload event to allow live-reload of the application
        SubscribeToEvent(scriptFile_, E_RELOADSTARTED, URHO3D_HANDLER(InGameEditor, HandleScriptReloadStarted));
        SubscribeToEvent(scriptFile_, E_RELOADFINISHED, URHO3D_HANDLER(InGameEditor, HandleScriptReloadFinished));
        SubscribeToEvent(scriptFile_, E_RELOADFAILED, URHO3D_HANDLER(InGameEditor, HandleScriptReloadFailed));
    }
}

// ----------------------------------------------------------------------------
void InGameEditor::CloseEditor()
{
    if(!scriptFile_)
        return;

    if(scriptFile_->GetFunction("void Stop()"))
        scriptFile_->Execute("void Stop()");
    scriptFile_.Reset();

    GetSubsystem<UI>()->Clear();
}

// ----------------------------------------------------------------------------
void InGameEditor::ToggleEditor(Scene* scene)
{
    if(IsOpen())
        CloseEditor();
    else
        OpenEditor(scene);
}

// ----------------------------------------------------------------------------
bool InGameEditor::IsOpen()
{
    return scriptFile_ != NULL;
}

// ----------------------------------------------------------------------------
void InGameEditor::HandleScriptReloadStarted(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGINFO("Reloading editor script");

    if(scriptFile_->GetFunction("void Stop()"))
        scriptFile_->Execute("void Stop()");
}

// ----------------------------------------------------------------------------
void InGameEditor::HandleScriptReloadFailed(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGERROR("Failed reloading editor script");
    scriptFile_.Reset();
}

// ----------------------------------------------------------------------------
void InGameEditor::HandleScriptReloadFinished(StringHash eventType, VariantMap& eventData)
{
    VariantVector args;
    args.Push(Variant(scene_));

    if(!scriptFile_->Execute("void Start(Scene@)"))
    {
        URHO3D_LOGERROR("Failed to call Start() function in editor script");
        scriptFile_.Reset();
    }
    else
    {
        URHO3D_LOGINFO("Finished reloading editor script");
    }
}
