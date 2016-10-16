#include "iceweasel/MainMenu.h"
#include "iceweasel/MenuScreen.h"
#include "iceweasel/LobbyScreen.h"
#include "iceweasel/DebugTextScroll.h"
#include "iceweasel/Util.h"

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
    BTN_JOIN_CONNECT,

    // connection failed screen
    BTN_CON_FAILED_OK
};

static const char* g_buttonNames[] = {
    // main screen
    "button_main_localGame",
    "button_main_joinServer",
    "button_main_options",
    "button_main_quit",

    // join screen
    "button_join_cancel",
    "button_join_connect",

    // connection failed
    "button_ok"
};

// ----------------------------------------------------------------------------
MainMenu::MainMenu(Context* context) :
    UIElement(context)
{
    for(unsigned i = 0; i != NUM_SCREENS; ++i)
    {
        switch(i)
        {
            case SCREEN_LOBBY:
                screens_[i] = new LobbyScreen(context_);
                break;

            default:
                screens_[i] = new MenuScreen(context_);
                break;
        }

        AddChild(screens_[i]);
    }

    screens_[SCREEN_MAIN]->LoadUI("UI/MainMenu_Main.xml");
    screens_[SCREEN_JOIN_SERVER]->LoadUI("UI/MainMenu_JoinServer.xml");
    screens_[SCREEN_CONNECTING]->LoadUI("UI/MainMenu_Connecting.xml");
    screens_[SCREEN_CONNECTION_FAILED]->LoadUI("UI/MainMenu_ConnectionFailed.xml");

    /*
     * Update our layout so the UI elements defined in the layout are able to
     * calculate their preferred sizes, then position ourselves in the centre
     * of the screen.
     */
    UpdateLayout();
    for(unsigned i = 0; i != NUM_SCREENS; ++i)
    {
        const IntVector2& rootSize = GetSubsystem<UI>()->GetRoot()->GetSize();
        const IntVector2& elementSize = screens_[i]->GetSize();
        screens_[i]->SetPosition((rootSize - elementSize) / 2);
    }

    SwitchToScreen(SCREEN_MAIN);

    // initial address
    if(screens_[SCREEN_JOIN_SERVER])
    {
        LineEdit* line = GetUIChild<LineEdit>(screens_[SCREEN_JOIN_SERVER], "lineEdit_address");
        if(line)
            line->SetText("127.0.0.1");
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

    CONNECT_BUTTON(screens_[SCREEN_MAIN], BTN_MAIN_LOCAL, HandleMainLocalGame);
    CONNECT_BUTTON(screens_[SCREEN_MAIN], BTN_MAIN_JOIN, HandleMainJoinServer);
    CONNECT_BUTTON(screens_[SCREEN_MAIN], BTN_MAIN_OPTIONS, HandleMainOptions);
    CONNECT_BUTTON(screens_[SCREEN_MAIN], BTN_MAIN_QUIT, HandleMainQuit);

    CONNECT_BUTTON(screens_[SCREEN_JOIN_SERVER], BTN_JOIN_CANCEL, HandleJoinCancel);
    CONNECT_BUTTON(screens_[SCREEN_JOIN_SERVER], BTN_JOIN_CONNECT, HandleJoinConnect);

    CONNECT_BUTTON(screens_[SCREEN_CONNECTION_FAILED], BTN_CON_FAILED_OK, HandleConnectionFailedOK);

    SubscribeToEvent(E_SERVERCONNECTED, URHO3D_HANDLER(MainMenu, HandleServerConnected));
    SubscribeToEvent(E_SERVERDISCONNECTED, URHO3D_HANDLER(MainMenu, HandleServerDisconnected));
    SubscribeToEvent(E_CONNECTFAILED, URHO3D_HANDLER(MainMenu, HandleConnectionFailed));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(MainMenu, HandleKeyDown));
}

// ----------------------------------------------------------------------------
void MainMenu::SwitchToScreen(MainMenu::Screen screen)
{
    for(unsigned i = 0; i != NUM_SCREENS; ++i)
        screens_[i]->SetVisible(false);

    screens_[screen]->SetVisible(true);
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
    LineEdit* line = GetUIChild<LineEdit>(screens_[SCREEN_JOIN_SERVER], "lineEdit_address");
    if(line)
        address = line->GetText();

    // Split address + port
    StringVector addressPort = address.Split(':');
    if(addressPort.Size() > 1)
        port = ToUInt(addressPort[1]);
    address = addressPort[0];

    // Attempt to connect
    SwitchToScreen(SCREEN_CONNECTING);
    Network* network = GetSubsystem<Network>();
    network->Connect(address, port, NULL);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleConnectionFailedOK(StringHash eventType, VariantMap& eventData)
{
    SwitchToScreen(SCREEN_JOIN_SERVER);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleServerConnected(StringHash eventType, VariantMap& eventData)
{
    SwitchToScreen(SCREEN_LOBBY);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleServerDisconnected(StringHash eventType, VariantMap& eventData)
{
    SwitchToScreen(SCREEN_JOIN_SERVER);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleConnectionFailed(StringHash eventType, VariantMap& eventData)
{
    SwitchToScreen(SCREEN_CONNECTION_FAILED);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;

    if(screens_[SCREEN_JOIN_SERVER]->IsVisible())
    {
        if(eventData[P_KEY].GetInt() == KEY_RETURN)
        {
            /*
             * This isn't really a good solution, but as long as the handler
             * doesn't use any of the event data we can get away with calling
             * the handler directly.
             */
            HandleJoinConnect(0, GetEventDataMap());
        }
    }
}
