#include "../../include/menus/minotaurIntro.hpp"

Texture MinotaurIntro::texture;

bool MinotaurIntro::init(vec2 position)
{
    const char* textureFile = textures_path("minotaur-intro.png");

    if (!RenderManager::load_texture(textureFile, &texture, this)) return false;

    if (!RenderManager::set_render_data(&texture, this)) return false;

    // Loading shaders
    if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
        return false;

    set_position(position);

    // Setting initial values, scale is negative to make it face the opposite way
    m_scale.x = 0.51f;
    m_scale.y = 0.51f;
    m_rotation = 0.f;

    return true;

}

void MinotaurIntro::draw(const mat3 &projection)
{
    RenderManager::draw_texture(projection, m_position, m_rotation, m_scale, &texture, color, is_hidden, this);
}

vec2 MinotaurIntro::get_position()
{
    return m_position;
}









