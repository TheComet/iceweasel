#pragma once

#include <Urho3D/Core/Object.h>

namespace Urho3D {
    class Context;
    class Scene;
    class ScriptFile;
}


class URHO3D_API InGameEditor : public Urho3D::Object
{
    URHO3D_OBJECT(InGameEditor, Urho3D::Object)

public:
    InGameEditor(Urho3D::Context* context);

    void OpenEditor(Urho3D::Scene* scene);
    void CloseEditor();
    void ToggleEditor(Urho3D::Scene* scene);
    bool IsOpen();

private:
    void HandleScriptReloadStarted(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleScriptReloadFinished(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleScriptReloadFailed(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Urho3D::ScriptFile> scriptFile_;
    Urho3D::SharedPtr<Urho3D::Scene> scene_;
};
