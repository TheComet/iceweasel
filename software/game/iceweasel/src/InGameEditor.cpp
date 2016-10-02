#include "iceweasel/InGameEditor.h"
#include "iceweasel/IceWeasel.h"

#include <Urho3D/AngelScript/ScriptFile.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/LuaScript/LuaScript.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
InGameEditor::InGameEditor(Urho3D::Context* context) :
    Object(context)
{
}

// ----------------------------------------------------------------------------
void InGameEditor::RunEditor(Urho3D::Scene* scene)
{
    URHO3D_LOGINFO("Opening Editor");

    Urho3D::Context* editorContext = new Urho3D::Context;
    InGameEditorApplication* editorApplication = new InGameEditorApplication(editorContext, scene);
    RegisterIceWeaselMods(editorContext);
    editorContext->RegisterSubsystem(new Script(editorContext));
    editorContext->RegisterSubsystem(new LuaScript(editorContext));
    editorApplication->Run();

    URHO3D_LOGINFO("Closing Editor");
}

// ----------------------------------------------------------------------------
InGameEditorApplication::InGameEditorApplication(Context* context, Scene* scene) :
    Application(context),
    scene_(scene)
{
}

// ----------------------------------------------------------------------------
InGameEditorApplication::~InGameEditorApplication()
{
    if(scriptFile_ && scriptFile_->GetFunction("void Stop()"))
        scriptFile_->Execute("void Stop()");
}

// ----------------------------------------------------------------------------
void InGameEditorApplication::Start()
{
    VariantVector args;
    args.Push(Variant(scene_));

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    scriptFile_ = cache->GetResource<ScriptFile>("Scripts/Editor.as");
    if(scriptFile_ && scriptFile_->Execute("void Start(Scene@)", args))
    {
        // Subscribe to script's reload event to allow live-reload of the application
        SubscribeToEvent(scriptFile_, E_RELOADSTARTED, URHO3D_HANDLER(InGameEditorApplication, HandleScriptReloadStarted));
        SubscribeToEvent(scriptFile_, E_RELOADFINISHED, URHO3D_HANDLER(InGameEditorApplication, HandleScriptReloadFinished));
        SubscribeToEvent(scriptFile_, E_RELOADFAILED, URHO3D_HANDLER(InGameEditorApplication, HandleScriptReloadFailed));
    }
    else
        ErrorExit();
}

// ----------------------------------------------------------------------------
void InGameEditorApplication::HandleScriptReloadStarted(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGINFO("Reloading editor script");

    if(scriptFile_ && scriptFile_->GetFunction("void Stop()"))
        scriptFile_->Execute("void Stop()");
}

// ----------------------------------------------------------------------------
void InGameEditorApplication::HandleScriptReloadFailed(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGERROR("Failed reloading editor script");
    scriptFile_.Reset();
}

// ----------------------------------------------------------------------------
void InGameEditorApplication::HandleScriptReloadFinished(StringHash eventType, VariantMap& eventData)
{
    VariantVector args;
    args.Push(Variant(scene_));

    if(scriptFile_ && !scriptFile_->Execute("void Start(Scene@)"))
    {
        URHO3D_LOGERROR("Failed to call Start() function in editor script");
        scriptFile_.Reset();
    }
    else
    {
        URHO3D_LOGINFO("Finished reloading editor script");
    }
}
