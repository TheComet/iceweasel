#pragma once

#include <Urho3D/Core/Object.h>

namespace Urho3D {
    class Text;
}

class DebugTextScroll : public Urho3D::Object
{
    URHO3D_OBJECT(DebugTextScroll, Urho3D::Object);

public:
    DebugTextScroll(Urho3D::Context* context);

    void SetTextCount(unsigned count);
    void Print(const Urho3D::String& str);

private:
    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    struct TextItem
    {
        Urho3D::SharedPtr<Urho3D::Text> text_;
        float timeOut_;
    };

    Urho3D::Vector<TextItem> items_;
    Urho3D::Vector<TextItem>::Iterator insertIt_;
};
