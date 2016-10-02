#pragma once

#include <Urho3D/Engine/Application.h>

namespace Urho3D {
    class Context;
    class Scene;
    class ScriptFile;
    class UIElement;
}


class InGameEditorApplication : public Urho3D::Application
{
    URHO3D_OBJECT(InGameEditorApplication, Urho3D::Application)

public:
    InGameEditorApplication(Urho3D::Context* context, Urho3D::Scene* scene);
    ~InGameEditorApplication();

    virtual void Start() override;

private:
    void HandleScriptReloadStarted(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleScriptReloadFinished(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleScriptReloadFailed(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Urho3D::ScriptFile> scriptFile_;
    Urho3D::SharedPtr<Urho3D::Scene> scene_;
};


class InGameEditor : public Urho3D::Object
{
    URHO3D_OBJECT(InGameEditor, Urho3D::Object)

public:
    InGameEditor(Urho3D::Context* context);

    void RunEditor(Urho3D::Scene* scene);
};
