#pragma once

#include <Urho3D/Core/Object.h>

#if defined(DEBUG)
#   include <Urho3D/IO/Log.h>
#   define LOG_SCROLL(msg) do {                                \
            DebugTextScroll* debug_##__LINE__ =                \
                    GetSubsystem<DebugTextScroll>();           \
            if(debug_##__LINE__) debug_##__LINE__->Print(msg); \
            URHO3D_LOGDEBUG(msg);                              \
    } while(0)
#else
#   define LOG_SCROLL(msg)
#endif

namespace Urho3D {
    class Text;
}

class DebugTextScroll : public Urho3D::Object
{
    URHO3D_OBJECT(DebugTextScroll, Urho3D::Object);

public:
    DebugTextScroll(Urho3D::Context* context);

    void SetTextCount(unsigned count);
    void Print(const Urho3D::String& str, const Urho3D::Color& color=Urho3D::Color::WHITE);

private:
    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleLogMessage(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    struct TextItem
    {
        Urho3D::SharedPtr<Urho3D::Text> text_;
        float timeOut_;
    };

    Urho3D::Vector<TextItem> items_;
    Urho3D::Vector<TextItem>::Iterator insertIt_;
};
