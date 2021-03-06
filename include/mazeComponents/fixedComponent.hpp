#pragma once

#include "mazeComponent.hpp"
#include <math.h>

// subclass of mazeComponent for all objects unaffected by gravity
// examples include walls/platforms, spikes fixed to platforms, etc
// (I think because of rotation, we should just treat walls as platforms)

class FixedComponent : public MazeComponent
{

public:
	// sets the vertex coordinates of the object relative to the world
	void set_world_vertex_coord();

	void transform_vertex_coord();

	void set_collision_properties();
	std::vector<vec2> get_vertex_coord();
};

class FixedComponents : public MazeComponents
{
public:
	bool init(float tile_width, float tile_height);
	virtual bool renderSetup() = 0;
protected:
	float m_tile_width;
	float m_tile_height;
};