namespace Urho3D {
    class Context;
}

/// Defines the category under which iceweasel specific components can be found in the editor.
extern const char* ICEWEASELMODS_CATEGORY;

/*!
 * Registers all Ice Weasel modifications.
 */
void RegisterIceWeaselMods(Urho3D::Context* context);
