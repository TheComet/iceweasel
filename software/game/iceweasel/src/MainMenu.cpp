#include "iceweasel/MainMenu.h"
#include "iceweasel/MenuScreen.h"
#include "iceweasel/DebugTextScroll.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Window.h>

using namespace Urho3D;

enum Buttons
{
    // main screen
    BTN_MAIN_LOCAL,
    BTN_MAIN_JOIN,
    BTN_MAIN_OPTIONS,
    BTN_MAIN_QUIT,

    // join screen
    BTN_JOIN_CANCEL,
    BTN_JOIN_CONNECT
};

static const char* g_buttonNames[] = {
    // main screen
    "button_main_localGame",
    "button_main_joinServer",
    "button_main_options",
    "button_main_quit",

    // join screen
    "button_join_cancel",
    "button_join_connect"
};

// ----------------------------------------------------------------------------
MainMenu::MainMenu(Context* context) :
    UIElement(context)
{
    mainScreen_ = new MenuScreen(context_);
    joinScreen_ = new MenuScreen(context_);
    lobbyScreen_ = new MenuScreen(context_);

    mainScreen_->LoadUI("UI/MainMenu_MainScreen.xml");
    joinScreen_->LoadUI("UI/MainMenu_JoinScreen.xml");
    lobbyScreen_->LoadUI("UI/MainMenu_Lobby.xml");

    AddChild(mainScreen_);
    AddChild(joinScreen_);
    AddChild(lobbyScreen_);

    SwitchToScreen(SCREEN_MAIN);

    // initial address
    if(joinScreen_)
    {
        UIElement* elem = joinScreen_->GetChild("lineEdit_address", true);
        if(elem && elem->GetTypeName() == "LineEdit")
        {
            static_cast<LineEdit*>(elem)->SetText("mak89.ch");
        }
    }

#define CONNECT_BUTTON(screen, BTN, handler) do {                            \
        if(screen == NULL || screen->ui_ == NULL) {                          \
            URHO3D_LOGERROR("Failed to load screen");                        \
            break;                                                           \
        }                                                                    \
        UIElement* button = screen->ui_->GetChild(g_buttonNames[BTN], true); \
        if(button == NULL) {                                                 \
            URHO3D_LOGERRORF("Couldn't find button \"%s\" in UI/MainMenu.xml", g_buttonNames[BTN]); \
            break;                                                           \
        }                                                                    \
        SubscribeToEvent(button, E_CLICK, URHO3D_HANDLER(MainMenu, handler));\
    } while(0)

    CONNECT_BUTTON(mainScreen_, BTN_MAIN_LOCAL, HandleMainLocalGame);
    CONNECT_BUTTON(mainScreen_, BTN_MAIN_JOIN, HandleMainJoinServer);
    CONNECT_BUTTON(mainScreen_, BTN_MAIN_OPTIONS, HandleMainOptions);
    CONNECT_BUTTON(mainScreen_, BTN_MAIN_QUIT, HandleMainQuit);

    CONNECT_BUTTON(joinScreen_, BTN_JOIN_CANCEL, HandleJoinCancel);
    CONNECT_BUTTON(joinScreen_, BTN_JOIN_CONNECT, HandleJoinConnect);

    SubscribeToEvent(E_SERVERCONNECTED, URHO3D_HANDLER(MainMenu, HandleServerConnected));
    SubscribeToEvent(E_SERVERDISCONNECTED, URHO3D_HANDLER(MainMenu, HandleServerDisconnected));
    SubscribeToEvent(E_CONNECTFAILED, URHO3D_HANDLER(MainMenu, HandleConnectionFailed));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(MainMenu, HandleKeyDown));
}

// ----------------------------------------------------------------------------
void MainMenu::SwitchToScreen(MainMenu::Screen screen)
{
    switch(screen)
    {
        case SCREEN_MAIN:
            mainScreen_->SetVisible(true);
            joinScreen_->SetVisible(false);
            lobbyScreen_->SetVisible(false);
            break;

        case SCREEN_JOIN_SERVER:
            mainScreen_->SetVisible(false);
            joinScreen_->SetVisible(true);
            lobbyScreen_->SetVisible(false);
            break;

        case SCREEN_LOBBY:
            mainScreen_->SetVisible(false);
            joinScreen_->SetVisible(false);
            lobbyScreen_->SetVisible(true);
            break;
    }
}

// ----------------------------------------------------------------------------
void MainMenu::HandleMainLocalGame(StringHash eventType, VariantMap& eventData)
{
    DebugTextScroll* debug = GetSubsystem<DebugTextScroll>();
    if(debug) debug->Print("Local game not implemented", Color::YELLOW);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleMainJoinServer(StringHash eventType, VariantMap& eventData)
{
    SwitchToScreen(SCREEN_JOIN_SERVER);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleMainOptions(StringHash eventType, VariantMap& eventData)
{
    DebugTextScroll* debug = GetSubsystem<DebugTextScroll>();
    if(debug) debug->Print("Options menu not implemented", Color::YELLOW);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleMainQuit(StringHash eventType, VariantMap& eventData)
{
    GetSubsystem<Engine>()->Exit();
}

// ----------------------------------------------------------------------------
void MainMenu::HandleJoinCancel(StringHash eventType, VariantMap& eventData)
{
    SwitchToScreen(SCREEN_MAIN);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleJoinConnect(StringHash eventType, VariantMap& eventData)
{
    String address = "127.0.0.1";
    unsigned short port = 1834;

    // Get the text entered into the line edit
    UIElement* elem = joinScreen_->GetChild("lineEdit_address", true);
    if(elem && elem->GetTypeName() == "LineEdit")
    {
        address = static_cast<LineEdit*>(elem)->GetText();
    }

    // Split address + port
    StringVector addressPort = address.Split(':');
    if(addressPort.Size() > 1)
        port = ToUInt(addressPort[1]);
    address = addressPort[0];

    // Attempt to connect
    DebugTextScroll* debug = GetSubsystem<DebugTextScroll>();
    if(debug) debug->Print(String("Connecting to \"") + address + "\" on port " + String(port));

    Network* network = GetSubsystem<Network>();
    network->Connect(address, port, NULL);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleServerConnected(StringHash eventType, VariantMap& eventData)
{
    DebugTextScroll* debug = GetSubsystem<DebugTextScroll>();
    if(debug) debug->Print("Connected to server");

    SwitchToScreen(SCREEN_LOBBY);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleServerDisconnected(StringHash eventType, VariantMap& eventData)
{
    DebugTextScroll* debug = GetSubsystem<DebugTextScroll>();
    if(debug) debug->Print("Disconnected");

    SwitchToScreen(SCREEN_JOIN_SERVER);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleConnectionFailed(StringHash eventType, VariantMap& eventData)
{
    SwitchToScreen(SCREEN_JOIN_SERVER);
}


// ----------------------------------------------------------------------------
void MainMenu::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;

    if(joinScreen_ && joinScreen_->IsVisible())
    {
        if(eventData[P_KEY].GetInt() == KEY_RETURN)
        {
            /*
             * This isn't really a good solution, but as long as the handler
             * doesn't use any of the event data we can get away with this.
             */
            HandleJoinConnect(0, GetEventDataMap());
        }
    }
}
