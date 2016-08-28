#pragma once

#include <Urho3D/Scene/LogicComponent.h>

class Finger : public Urho3D::LogicComponent {
    URHO3D_OBJECT(Finger, Urho3D::LogicComponent)

public:
    Finger(Urho3D::Context* context);
};
