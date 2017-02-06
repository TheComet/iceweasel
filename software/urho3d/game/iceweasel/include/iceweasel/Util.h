#include <Urho3D/UI/UIElement.h>

/*!
 * @brief Recursively searches for a child with the specified name, and safely
 * up-casts it to the correct type. If anything fails, NULL is returned.
 */
template <class T>
T* GetUIChild(Urho3D::UIElement* ui, const Urho3D::String& name)
{
    Urho3D::UIElement* elem = ui->GetChild(name, true);
    if(elem && elem->GetType() == T::GetTypeStatic())
        return static_cast<T*>(elem);
    return NULL;
}
