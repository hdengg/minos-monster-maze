#pragma once

#include "fixedComponent.hpp"

class Floor : public FixedComponent
{
public:

    bool init(vec2 position)override;
	bool set_texture_properties(Texture *texture);

    // Renders the component
	void draw(const mat3& projection)override;

};

class Floors : public FixedComponents
{
	static Texture texture;

public:
	bool renderSetup()override;
	bool spawn_floor(vec2 position);
	std::vector<Floor> get_floor_vector();

	// Renders the component
	void draw(const mat3& projection)override;

	void destroy();
private:
	std::vector<Floor> m_floors;
};