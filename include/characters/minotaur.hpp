#pragma once

#include <stack>

// Simple interface for the minotaur. This is the boss enemy!
#include "enemy.hpp"
#include "character.hpp"
#include "smart.hpp"

class Minotaur: public Smart
{
    static Texture minotaur_texture;
    public:
        // Creates all the associated render resources and default transform
        bool init(vec2 initialPosition, Physics* physicsHandler)override;

        // Update player position based on velocity vector
        // ms represents the number of milliseconds elapsed from the previous update() call
        void update(float ms)override;

        void draw(const mat3& projection)override;

        void set_animation()override;

        void set_bound(float bound);

        bool isPlayerClose();

        void initStateTreeMinotaur();

        void setFollowDirection();

        bool atBound();

        void updateVelocity();

        void handleBossRotation();

        float getTimeElapsed();

        void resetCycleStart();

    private:
        float maxVerticalSpeed = 1.f;
        float maxHorzSpeed = 1.f;
        float m_stopBound;
        float detectPlayerRange = 200.f;
        std::chrono::high_resolution_clock::time_point cycle_start;
        const float rotate_cycle_time = 5000.f;
        const float rotate_duration = 1000.f;
        bool rotating = false;
	
};