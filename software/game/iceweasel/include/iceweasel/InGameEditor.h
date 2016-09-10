#pragma once

#include <Urho3D/Core/Object.h>

namespace Urho3D {
    class Context;
    class DebugRenderer;
    class GravityVector;
    class GravityHull;
    class GravityMesh;
}


/*!
 * @brief Provides an interface to query the scene for gravitational data.
 *
 * This class manages all of the gravity probes in the scene. A probe
 * defines a (normalized) gravitational direction and a multiplication factor
 * for a specific location in 3D world space. Multiple probes can be placed
 * in the scene at different locations to change the gravity of physics
 * objects.
 *
 * The final gravitational force is a combination of the global gravitational
 * force (defined by this class) and a resulting interpolated vector from
 * nearby gravity probes.
 */
class URHO3D_API InGameEditor : public Urho3D::Object
{
    URHO3D_OBJECT(InGameEditor, Object)

public:

    /*!
     * @brief Creates a new gravity component.
     */
    InGameEditor(Urho3D::Context* context);
};
