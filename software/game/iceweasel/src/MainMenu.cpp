#include "iceweasel/MainMenu.h"
#include "iceweasel/MenuScreen.h"
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
    joinServerScreen_ = new MenuScreen(context_);

    mainScreen_->LoadUI("UI/MainMenu_MainScreen.xml");
    joinServerScreen_->LoadUI("UI/MainMenu_JoinScreen.xml");

    AddChild(mainScreen_);
    AddChild(joinServerScreen_);

    mainScreen_->SetVisible(true);

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

    CONNECT_BUTTON(joinServerScreen_, BTN_JOIN_CANCEL, HandleJoinCancel);
    CONNECT_BUTTON(joinServerScreen_, BTN_JOIN_CONNECT, HandleJoinConnect);
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
    joinServerScreen_->SetVisible(true);
    mainScreen_->SetVisible(false);
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
    joinServerScreen_->SetVisible(false);
    mainScreen_->SetVisible(true);
}

// ----------------------------------------------------------------------------
void MainMenu::HandleJoinConnect(StringHash eventType, VariantMap& eventData)
{

}

