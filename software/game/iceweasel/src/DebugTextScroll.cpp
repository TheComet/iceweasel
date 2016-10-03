#include "iceweasel/DebugTextScroll.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
DebugTextScroll::DebugTextScroll(Urho3D::Context* context) :
    Object(context)
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(DebugTextScroll, HandleUpdate));
}

// ----------------------------------------------------------------------------
void DebugTextScroll::SetTextCount(unsigned count)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

    while(items_.Size() < count)
    {
        TextItem item;
        item.text_ = ui->GetRoot()->CreateChild<Text>();
        item.text_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 9);
        item.text_->SetTextAlignment(HA_LEFT);
        item.text_->SetHorizontalAlignment(HA_LEFT);
        item.text_->SetVerticalAlignment(VA_TOP);
        item.timeOut_ = 0;
        items_.Push(item);
    }

    while(items_.Size() > count)
    {
        items_.Pop();
    }

    insertIt_ = items_.Begin();
}


// ----------------------------------------------------------------------------
void DebugTextScroll::Print(const Urho3D::String& str)
{
    if(items_.Size() == 0)
        return;

    insertIt_->text_->SetText(str);
    insertIt_->text_->SetOpacity(1);
    insertIt_->text_->SetPosition(0, GetSubsystem<UI>()->GetRoot()->GetHeight() - 40);
    insertIt_->timeOut_ = 3;

    if(++insertIt_ == items_.End())
        insertIt_ = items_.Begin();

    for(Vector<TextItem>::Iterator it = items_.Begin(); it != items_.End(); ++it)
    {
        IntVector2 pos = it->text_->GetPosition();
        it->text_->SetPosition(pos.x_, pos.y_ - it->text_->GetHeight() - 2);
    }
}

// ----------------------------------------------------------------------------
void DebugTextScroll::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    for(Vector<TextItem>::Iterator it = items_.Begin(); it != items_.End(); ++it)
    {
        if((it->timeOut_ -= timeStep) < 0)
        {
            float opacity = it->timeOut_ + 1;
            if(opacity > 0)
                it->text_->SetOpacity(opacity);
            else
                it->text_->SetText("");
        }
    }
}
