// Header
#include "../include/characters/player.hpp"
#include "../include/common.hpp"
#include "../include/physics.hpp"

// internal

// stlib
#include <vector>
#include <string>
#include <algorithm>
#include <math.h>
#include <cmath>

Texture Player::player_texture;

bool Player::init(vec2 initialPosition, Physics* physicsHandler)
{
	this->physicsHandler = physicsHandler;
	if (!player_texture.is_valid())
	{
		if (!player_texture.load_from_file(textures_path("player.png")))
		{
			fprintf(stderr, "Failed to load turtle texture!");
			return false;
		}
	}

	// The position corresponds to the center of the texture
	float wr = player_texture.width * 0.5f;
	float hr = player_texture.height * 0.5f;

	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, -0.01f };
	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].position = { +wr, +hr, -0.01f };
	vertices[1].texcoord = { 1.f, 1.f,  };
	vertices[2].position = { +wr, -hr, -0.01f };
	vertices[2].texcoord = { 1.f, 0.f };
	vertices[3].position = { -wr, -hr, -0.01f };
	vertices[3].texcoord = { 0.f, 0.f };

	// counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, vertices, GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;
	
	// Setting initial values
	float scaleFactor = 2.f;
	m_scale.x = -scaleFactor;
	m_scale.y = scaleFactor;
	width = player_texture.width * scaleFactor;
	height = player_texture.height * scaleFactor;
	m_is_alive = true;
	v_direction = Direction::none;
	m_position = initialPosition;
	m_rotation = 0.f;
	m_velocity = {0.0, 0.0};

	isBelowPlatform = false;
	isLeftOfPlatform = false;
	isRightOfPlatform = false;

	return true;
}

// Called on each frame by World::update()
void Player::update(float ms)
{
	physicsHandler->characterAccelerationUpdate(this);
	physicsHandler->characterVelocityUpdate(this);
	if (m_is_alive)	move();
}

void Player::draw(const mat3& projection)
{
	transform_begin();

	transform_translate(m_position);
	transform_scale(m_scale);
	transform_rotate(m_rotation);
	transform_end();

	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	// Getting uniform locations
	GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
	GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");
	GLint light_up_uloc = glGetUniformLocation(effect.program, "light_up");

	// Setting vertices and indices
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(effect.program, "in_texcoord");
	glEnableVertexAttribArray(in_position_loc);
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
	glVertexAttribPointer(in_texcoord_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, player_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);

	// Player Color
	float color[] = { 1.f, 1.f, 1.f };

	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES,6, GL_UNSIGNED_SHORT, nullptr);
}

// Returns the local bounding coordinates scaled by the current size of the player 
vec2 Player::get_bounding_box()const {
	// fabs is to avoid negative scale due to the facing direction
	return { std::fabs(m_scale.x) * player_texture.width, std::fabs(m_scale.y) * player_texture.height };
}

void Player::move() {
	m_position.x += m_velocity.x; m_position.y += m_velocity.y;
}

void Player::set_on_platform(float yPos) {
	isOnPlatform = true;
}

void Player::set_in_free_fall() {
	isOnPlatform = false;
}

void Player::set_direction(int key, int action)
{
	if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_UP: if (isOnPlatform) m_velocity.y += jumpVel; break;
			case GLFW_KEY_LEFT: 
				h_direction = Direction::left; 
				m_scale.x = -std::fabs(m_scale.x);
				break;
			case GLFW_KEY_RIGHT: 
				h_direction = Direction::right; 
				m_scale.x = std::fabs(m_scale.x);
				break;
		}
	}
	else if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_UP: 
				if (v_direction == Direction::up)
					v_direction = Direction::none; break;
			case GLFW_KEY_LEFT: 
				if (h_direction == Direction::left)
					h_direction = Direction::none; break;
			case GLFW_KEY_RIGHT: 
				if (h_direction == Direction::right)
					h_direction = Direction::none; break;
		}
	}
}
