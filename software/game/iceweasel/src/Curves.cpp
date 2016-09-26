#include "iceweasel/Curves.h"
#include <Urho3D/Math/MathDefs.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
ICurve::ICurve(float initialValue) :
    value_(initialValue)
{
}

// ----------------------------------------------------------------------------
float ICurve::GetValue() const
{
    return value_;
}

// ----------------------------------------------------------------------------
ExponentialCurve::ExponentialCurve() :
    ICurve(0)
{
}

// ----------------------------------------------------------------------------
ExponentialCurve::ExponentialCurve(float initialValue, float targetValue) :
    ICurve(initialValue),
    targetValue_(targetValue)
{
}

// ----------------------------------------------------------------------------
void ExponentialCurve::SetTarget(float targetValue)
{
    targetValue_ = targetValue;
}

// ----------------------------------------------------------------------------
float ExponentialCurve::Advance(float step)
{
    value_ += (targetValue_ - value_) * Min(1.0f, step);
    return value_;
}
