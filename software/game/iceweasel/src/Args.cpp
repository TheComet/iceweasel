#include "iceweasel/Args.h"

// ----------------------------------------------------------------------------
Args::Args() :
    networkAddress_("127.0.0.1"),
    networkPort_(1834),
    editor_(false),
    server_(false),
    fullscreen_(false),
    vsync_(false),
    multisample_(2)
{
}
