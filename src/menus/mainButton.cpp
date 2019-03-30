#include "../../include/menus/mainButton.hpp"


bool MainButton::init(vec2 position, const char* textureFile, button name)
{

    m_texture.id = 0;

    if (!RenderManager::load_texture(textureFile, &m_texture, this)) return false;

    if (!RenderManager::set_render_data(&m_texture, this)) return false;

    // Loading shaders
    if (!effect.load_from_file(shader_path("menu.vs.glsl"), shader_path("menu.fs.glsl")))
        return false;

    set_position(position);

    // Setting initial values, scale is negative to make it face the opposite way
    m_scale.x = 0.7f;
    m_scale.y = 0.7f;
    m_rotation = 0.f;
    buttonName = name;
    is_hidden = false;

    return true;

}

void MainButton::draw(const mat3 &projection)
{
    RenderManager::init_drawing_data(m_position, m_rotation, m_scale, this);

    // Getting uniform locations for glUniform* calls
    GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
    GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
    GLint projection_uloc = glGetUniformLocation(effect.program, "projection");

    // Input data location as in the vertex buffer
    GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
    GLint in_texcoord_loc = glGetAttribLocation(effect.program, "in_texcoord");
    glEnableVertexAttribArray(in_position_loc);
    glEnableVertexAttribArray(in_texcoord_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
    glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));

    // Enabling and binding texture to slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture.id);

    // Set opacity
    GLint is_hidden_uloc = glGetUniformLocation(effect.program, "is_hidden");
    glUniform1f(is_hidden_uloc, is_hidden);

    // Setting uniform values to the currently bound program
    glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform);
    if (isSelected) {
        float color[] = { 0.f, 0.f, 0.f };
        glUniform3fv(color_uloc, 1, color);
    } else {
        float color[] = { 1.f, 1.f, 1.f };
        glUniform3fv(color_uloc, 1, color);
    }

    glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

    // magnifies texture to avoid it being blurry when scaled
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Drawing!
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void MainButton::set_visibility(bool is_visible)
{
    is_hidden = !is_visible;
}










