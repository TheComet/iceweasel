#include "iceweasel/LobbyScreen.h"
#include "iceweasel/Util.h"

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>

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

    ListView* mapsList = GetUIChild<ListView>(mapSelect_, "listView_maps");
    if(mapsList)
    SubscribeToEvent(mapsList, E_SELECTIONCHANGED, URHO3D_HANDLER(LobbyScreen, HandleMapSelectionChanged));

    ScanForMaps();

    UpdateLayout();
    const IntVector2& rootSize = GetSubsystem<UI>()->GetRoot()->GetSize();
    const IntVector2& mapSize = mapSelect_->GetSize();
    const IntVector2& charSize = characterSelect_->GetSize();
    int wiggleRoom = (rootSize - mapSize - charSize).x_;
    mapSelect_->SetPosition(mapSize.x_ - wiggleRoom, -rootSize.y_ / 8);
    characterSelect_->SetPosition(wiggleRoom, -rootSize.y_ / 8);
}

// ----------------------------------------------------------------------------
void LobbyScreen::ScanForMaps()
{
    FileSystem* fs = GetSubsystem<FileSystem>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    LineEdit* selectedMap = GetUIChild<LineEdit>(mapSelect_, "lineEdit_selectedMap");
    ListView* mapsList = GetUIChild<ListView>(mapSelect_, "listView_maps");
    if(selectedMap == NULL || mapsList == NULL)
        return;
    mapsList->RemoveAllItems();

    const StringVector& resourceDirs = cache->GetResourceDirs();
    for(StringVector::ConstIterator resourceDir = resourceDirs.Begin();
        resourceDir != resourceDirs.End();
        ++resourceDir)
    {
        const String scenePath = *resourceDir + "Scenes/";
        if(!fs->DirExists(scenePath))
            continue;

        URHO3D_LOGDEBUGF("Scanning for maps in %s", scenePath.CString());
        StringVector sceneList;
        fs->ScanDir(sceneList, scenePath, "*", SCAN_FILES, true);

        for(StringVector::ConstIterator sceneFile = sceneList.Begin();
            sceneFile != sceneList.End();
            ++sceneFile)
        {
            URHO3D_LOGDEBUGF("Found map %s", sceneFile->CString());
            StringVector split = sceneFile->Split('/');
            Text* text = new Text(context_);
            text->SetStyle("FileSelectorListText");
            text->SetText(*(split.End() - 1));
            mapsList->AddItem(text);
        }
    }
}

// ----------------------------------------------------------------------------
void LobbyScreen::HandleMapSelectionChanged(StringHash eventType, VariantMap& eventData)
{
    using namespace SelectionChanged;

    UIElement* elem = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
    LineEdit* selectedMap = GetUIChild<LineEdit>(mapSelect_, "lineEdit_selectedMap");

    if(elem && elem->GetTypeName() == "Text" && selectedMap)
    {
        Text* text = static_cast<Text*>(elem);
        selectedMap->SetText(text->GetText());
    }
}
