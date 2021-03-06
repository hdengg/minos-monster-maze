#include <stack>
#include <chrono>

#include "../include/characters/harpy.hpp"
#include "../include/common.hpp"
#include "../include/physics.hpp"
#include "../include/gameAI.hpp"

// Put implementation for Harpy enemies here

using Clock = std::chrono::high_resolution_clock;
Texture Harpy::harpy_texture;

bool Harpy::init(vec2 initialPosition, Physics * physicsHandler)
{
    Smart::init(initialPosition, physicsHandler);

    const char* textureFile = textures_path("bat-sprite-sheet.png");
	if (!RenderManager::load_texture(textureFile, &harpy_texture, this)) {
        return false;
    }

	float spriteSheetWidth = 5.0f;
	float spriteSheetHeight = 3.0f;
    int horizontalTrim = 12;
    int verticalTrim = 19;

	spriteSheet.init(&harpy_texture, { spriteSheetWidth, spriteSheetHeight }, this);

	spriteSheet.set_render_data(this, 0);

	initStateTree();
	set_properties(initialPosition, 3.0f, speed);
	m_frozen = false;
	set_dimensions(&harpy_texture, spriteSheetWidth, spriteSheetHeight, horizontalTrim, verticalTrim);
	characterState->changeState(idle);

    path_to_follow = stack<vec2>();
    cycle_start = Clock::now();
    next_node = initialPosition;
	return true;
}

void Harpy::update(float ms)
{
	if (is_alive() && !m_frozen)
	{
        bool update_path = checkTimeElapsed();
        if (path_to_follow.empty() || update_path)
        {
            // call AI to get a path to the player   
            path_to_follow = GameAI::followPlayer(m_position, SearchMethod::aStar);
            if (path_to_follow.empty()) {
                characterState->changeState(idle);
                return;
            } else {
                vec2 first = path_to_follow.top();
                next_node = first;
            }
        } 
        moveAlongPath();
	}

}

void Harpy::moveAlongPath(){
    // check to see if already reached the next node in the path. If not, continue to move there
    switch (direction)
    {
        case Direction::up:
            if (m_position.y > next_node.y) {
                move();
                return;
            }
            break;
        case Direction::down:
            if (m_position.y < next_node.y) {
                move();
                return;
            }
            break;
        case Direction::left:
            if (m_position.x > next_node.x) {
                move();
                return;
            }
            break;
        case Direction::right:
            if (m_position.x < next_node.x) {
                move();
                return;
            }
            break;
        default:
            break;
    }
    // if we've reached the next node in the path, get a new next node and move towards it
    next_node = path_to_follow.top();
    path_to_follow.pop();
    if (abs(next_node.y - m_position.y) >= maxVerticalSpeed) {
        if (next_node.y < m_position.y) {
            set_direction(Direction::up);
            characterState->changeState(rising);
        } else {
            set_direction(Direction::down);
            characterState->changeState(falling);
        }
    } else if (abs(next_node.x - m_position.x) >= maxHorzSpeed) {
        if (next_node.x < m_position.x) {
            set_direction(Direction::left);
            m_scale.x = -std::fabs(m_scale.x);
        } else {
            set_direction(Direction::right);
            m_scale.x = std::fabs(m_scale.x);
        }
        characterState->changeState(running);
    } 
    updateVelocity();
    move();

}

void Harpy::updateVelocity() {
    // if the character is idle, then the velocity is 0
    if (characterState->currentState == idle) {
        m_velocity.x = 0.f;
        m_velocity.y = 0.f;
        return;
    }

    // otherwise, we move update the velocity based on the direction the character is going
    switch (direction)
    {
        case Direction::up:
            m_velocity.x = 0.f;
            m_velocity.y = - maxVerticalSpeed;
            break;
        case Direction::down:
            m_velocity.x = 0.f;
            m_velocity.y = maxVerticalSpeed;
            break;
        case Direction::left:
            m_velocity.x = - maxHorzSpeed;
            m_velocity.y = 0.f;
            break;
        case Direction::right:
            m_velocity.x = maxHorzSpeed;
            m_velocity.y = 0.f;
            break;
        default:
            break;
    }
}

bool Harpy::checkTimeElapsed() {
    auto now = Clock::now();
	float elapsed_sec = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - cycle_start)).count() / 1000;
    if (elapsed_sec >= path_cycle_time) {
        resetCycleStart();
        return true;
    }
    return false;
}

void Harpy::resetCycleStart() {
    cycle_start = Clock::now();
}

void Harpy::draw(const mat3& projection)
{
    set_animation();
	RenderManager::draw_texture(projection, m_position, m_rotation, m_scale, &harpy_texture, color, is_hidden, this);

}

void Harpy::set_animation()
{
    int numTiles;
    int tileIndex;
    bool isRepeat = true;

    float animSpeed = 0.2f;

    if (is_alive())
    {
        is_anim_once = false;
        switch (characterState->currentState) {
            case idle:
            case running:
            case rising:
            case falling:
                numTiles = 5;
                tileIndex = 0;
                break;
            default:
                numTiles = 1;
                tileIndex = 0;

        }
    } else {
        isRepeat = false;

        if (is_anim_once)
        {
            numTiles = 1;
            tileIndex = 14;
        } else
        {
            numTiles = 5;
            tileIndex = 10;
        }
    }

    // Increment animation time
    m_animTime += animSpeed;

    // Apply animation
    tileIndex = tileIndex + (int)m_animTime % numTiles;

    // do not repeat death animation
    if (!isRepeat && tileIndex == 14) is_anim_once = true;

    spriteSheet.update_render_data(this, tileIndex);
}

bool Harpies::spawn_harpy_enemy(vec2 position)
{
	Harpy enemy;
	if (enemy.init(position, physicsHandler))
	{
		m_harpies.emplace_back(enemy);
		return true;
	}
	fprintf(stderr, "Failed to spawn harpy");
	return false;
}

std::vector<Harpy> Harpies::get_harpy_vector()
{
	return m_harpies;
}

void Harpies::draw(const mat3 & projection)
{
	for (auto& harpy : m_harpies)
		harpy.draw(projection);
}

void Harpies::reset()
{
	for (auto& harpy : m_harpies) {
		harpy.freeze();
		harpy.reset_position();
		harpy.unfreeze();
	};
}

void Harpies::freeze()
{
	for (auto& h : m_harpies) h.freeze();
}

void Harpies::unfreeze()
{
	for (auto& h : m_harpies) h.unfreeze();
}

void Harpies::update(float elapsed_ms)
{
	for (auto& h : m_harpies) h.update(elapsed_ms);
}

void Harpies::setHarpyProperties(size_t index, vec2 position, vec2 velocity, vec2 scale)
{
	m_harpies[index].set_position(position);
	m_harpies[index].set_velocity(velocity);
	m_harpies[index].set_scale(scale);
}

void Harpies::destroy()
{
	for (auto& harpy : m_harpies)
		harpy.destroy();
}
