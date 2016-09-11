#include "iceweasel/InGameEditor.h"

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/AngelScript/ScriptFile.h>

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

    VariantVector args;
    args.Push(Variant(scene));

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    ScriptFile* scriptFile = cache->GetResource<ScriptFile>("Scripts/Editor.as");
    if(scriptFile && scriptFile->Execute("void Start(Scene@)", args))
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
}

// ----------------------------------------------------------------------------
void InGameEditor::HandleScriptReloadStarted(StringHash eventType, VariantMap& eventData)
{
    if(scriptFile_->GetFunction("void Stop()"))
        scriptFile_->Execute("void Stop()");
}

// ----------------------------------------------------------------------------
void InGameEditor::HandleScriptReloadFailed(StringHash eventType, VariantMap& eventData)
{
    if(!scriptFile_->Execute("void Start()"))
        scriptFile_.Reset();
}

// ----------------------------------------------------------------------------
void InGameEditor::HandleScriptReloadFinished(StringHash eventType, VariantMap& eventData)
{
    scriptFile_.Reset();
}
