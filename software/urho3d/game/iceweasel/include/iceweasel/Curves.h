#pragma once

#include "iceweasel/Curves.hxx"
#include <Urho3D/Math/MathDefs.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
template <class T>
ICurve<T>::ICurve(T initialValue) :
    value_(initialValue)
{
}

// ============================================================================
template <class T>
ExponentialCurve<T>::ExponentialCurve() :
    ICurve<T>(T())
{
}

// ----------------------------------------------------------------------------
template <class T>
ExponentialCurve<T>::ExponentialCurve(T initialValue, T targetValue) :
    ICurve<T>(initialValue),
    targetValue_(targetValue)
{
}

// ----------------------------------------------------------------------------
template <class T>
void ExponentialCurve<T>::SetTarget(T targetValue)
{
    targetValue_ = targetValue;
}

// ----------------------------------------------------------------------------
template <class T>
T ExponentialCurve<T>::Advance(float step)
{
    this->value_ += (targetValue_ - this->value_) * Min(1.0f, step);
    return this->value_;
}
