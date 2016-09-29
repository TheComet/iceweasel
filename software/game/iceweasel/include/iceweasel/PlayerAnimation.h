#pragma once

class PlayerAnimation
{
public:

    enum Animation
    {
        IDLE,
        WALK,
        RUN,
        SPRINT,
        CROUCH,
        CROUCH_WALK,
        JUMP_OFF,
        JUMP_LAND,

        NUM_ANIMATIONS
    };

    static const char* name[];
};
