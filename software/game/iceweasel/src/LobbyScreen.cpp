#include "iceweasel/LobbyScreen.h"

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UI.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
LobbyScreen::LobbyScreen(Context* context) :
    MenuScreen(context),
    mapSelect_(new MenuScreen(context)),
    characterSelect_(new MenuScreen(context))
{
    AddChild(mapSelect_);
    AddChild(characterSelect_);

    mapSelect_->LoadUI("UI/MainMenu_Lobby_MapSelect.xml");
    characterSelect_->LoadUI("UI/MainMenu_Lobby_CharacterSelect.xml");

    UpdateLayout();
    const IntVector2& rootSize = GetSubsystem<UI>()->GetRoot()->GetSize();
    const IntVector2& mapSize = mapSelect_->GetSize();
    const IntVector2& charSize = characterSelect_->GetSize();
    int wiggleRoom = (rootSize - mapSize - charSize).x_;
    mapSelect_->SetPosition(mapSize.x_ - wiggleRoom, -rootSize.y_ / 8);
    characterSelect_->SetPosition(wiggleRoom, -rootSize.y_ / 8);
}
