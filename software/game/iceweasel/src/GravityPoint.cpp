#include "iceweasel/GravityPoint.h"

using namespace Urho3D;

// ----------------------------------------------------------------------------
GravityPoint::GravityPoint(const Vector3& vertex, const Vector3& direction, float forceFactor) :
    position_(vertex),
    direction_(direction),
    forceFactor_(forceFactor)
{
}
