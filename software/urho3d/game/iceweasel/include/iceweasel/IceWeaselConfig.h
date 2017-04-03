#pragma once

#include "iceweasel/PlayerAnimation.h"
#include <Urho3D/Core/Object.h>


namespace Urho3D {
    class Context;
    class XMLFile;
}


class IceWeaselConfig : public Urho3D::Object
{
    URHO3D_OBJECT(IceWeaselConfig, Urho3D::Object)

public:

    struct Data {
        struct Input {
            struct Mouse {
                float sensitivity;
            } mouse;
        } input;
        struct PlayerClass {
            Urho3D::String className;
            struct Body {
                float width;
                float height;
                float mass;
                float crouchWidth;
                float crouchHeight;
            } body;
            struct Jump {
                float force, bunnyHopBoost;
            } jump;
            struct Speed {
                float crouch;
                float walk;
                float run;
                float crouchTransitionSpeed;
                float jounceSpeed;
                float accelerateSpeed;
            } speed;
            struct Turn {
                float speed;
            } turn;
            struct Lean {
                float amount;
                float speed;
            } lean;
            struct Animations {
                float speed;
                float transitionSpeed;
            } animations[PlayerAnimation::NUM_ANIMATIONS];
        };
        Urho3D::Vector<PlayerClass> playerClassContainer;
        const PlayerClass& playerClass(unsigned index) const;

        struct Camera {
            struct Transition {
                float speed;
                float distance;
            } transition;
        } camera;

        struct FreeCam {
            struct Speed {
                float normal, fast, smoothness;
            } speed;
        } freeCam;
    };

    IceWeaselConfig(Urho3D::Context* context);

    void Load(Urho3D::String fileName);
    void LoadXML(Urho3D::XMLFile* xml);
    void Reload();

    const Data& GetConfig() const;

private:
    void HandleFileChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::SharedPtr<Urho3D::XMLFile> xml_;
    Data data_;
};
