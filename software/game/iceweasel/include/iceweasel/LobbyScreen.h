#pragma once

#include "iceweasel/MenuScreen.h"

class LobbyScreen : public MenuScreen
{
    URHO3D_OBJECT(LobbyScreen, MenuScreen)

public:
    LobbyScreen(Urho3D::Context* context);

private:
    void ScanForMaps();

    void HandleMapSelectionChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<MenuScreen> mapSelect_;
    Urho3D::SharedPtr<MenuScreen> characterSelect_;
};
