#pragma once

#include "iceweasel/MenuBase.h"

class MainMenu : public MenuBase
{
    URHO3D_OBJECT(MainMenu, MenuBase)

public:
    MainMenu(Urho3D::Context* context);

private:
    void HandleLocalGame(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleJoinServer(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleOptions(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleQuit(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
};
