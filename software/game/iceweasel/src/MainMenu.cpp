#include "iceweasel/MainMenu.h"
#include "iceweasel/DebugTextScroll.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/Text.h>

using namespace Urho3D;

enum Buttons
{
    BTN_LOCAL,
    BTN_JOIN,
    BTN_OPTIONS,
    BTN_QUIT
};

static const char* g_buttonNames[] = {
    "button_localGame",
    "button_joinServer",
    "button_options",
    "button_quit"
};

// ----------------------------------------------------------------------------
MainMenu::MainMenu(Context* context) :
    MenuBase(context)
{
    LoadUI("UI/JoinServer.xml");

#define CONNECT_BUTTON(BTN, Handler) do {                                    \
        UIElement* button = GetChild(g_buttonNames[BTN], true);              \
        if(button == NULL)                                                   \
        {                                                                    \
            URHO3D_LOGERRORF("Couldn't find button \"%s\" in UI/MainMenu.xml", g_buttonNames[BTN]); \
            break;                                                           \
        }                                                                    \
        SubscribeToEvent(button, E_CLICK, URHO3D_HANDLER(MainMenu, Handler));\
    } while(0)

    CONNECT_BUTTON(BTN_LOCAL, HandleLocalGame);
    CONNECT_BUTTON(BTN_JOIN, HandleJoinServer);
    CONNECT_BUTTON(BTN_OPTIONS, HandleOptions);
    CONNECT_BUTTON(BTN_QUIT, HandleQuit);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleLocalGame(StringHash eventType, VariantMap& eventData)
{
    DebugTextScroll* debug = GetSubsystem<DebugTextScroll>();
    if(debug) debug->Print("Local game not implemented", Color::YELLOW);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleJoinServer(StringHash eventType, VariantMap& eventData)
{
}

// ----------------------------------------------------------------------------
void MainMenu::HandleOptions(StringHash eventType, VariantMap& eventData)
{
    DebugTextScroll* debug = GetSubsystem<DebugTextScroll>();
    if(debug) debug->Print("Options menu not implemented", Color::YELLOW);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleQuit(StringHash eventType, VariantMap& eventData)
{
    GetSubsystem<Engine>()->Exit();
}
