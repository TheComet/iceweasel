#include "iceweasel/IceWeaselMods.h"
#include "iceweasel/GravityVector.h"
#include "iceweasel/GravityManager.h"

namespace Urho3D {
    class Context;
}

const char* ICEWEASELMODS_CATEGORY = "Ice Weasel";

// ----------------------------------------------------------------------------
void RegisterIceWeaselMods(Urho3D::Context* context)
{
    GravityManager::RegisterObject(context);
    GravityVector::RegisterObject(context);
}

} // namespace Urho3D
