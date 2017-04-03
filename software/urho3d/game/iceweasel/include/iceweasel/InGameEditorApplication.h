#pragma once

#include <Urho3D/Engine/Application.h>

namespace Urho3D {
    class Context;
    class Scene;
    class ScriptFile;
}
class Args;

class InGameEditorApplication : public Urho3D::Application
{
    URHO3D_OBJECT(InGameEditorApplication, Urho3D::Object)

public:
    InGameEditorApplication(Urho3D::Context* context, Args* args);

    virtual void Setup() override;
    virtual void Start() override;

private:
    void RegisterSubsystems();

    void HandleScriptReloadStarted(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleScriptReloadFinished(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleScriptReloadFailed(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Args> args_;
    Urho3D::SharedPtr<Urho3D::ScriptFile> script_;
};
