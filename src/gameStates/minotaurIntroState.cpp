#include "../../include/gameStates/minotaurIntroState.hpp"
#include "../../include/renderManager.hpp"

// stlib
#include <stdio.h>
#include <string.h>
#include <cassert>
#include <sstream>
#include <time.h>
#include <iostream>
#include <fstream>

MinotaurIntroState::MinotaurIntroState(Game *game)
{
    this->game = game;

}

void MinotaurIntroState::init(vec2 screen)
{
    fprintf(stderr, "minotaur minotaur_intro init\n");
    this->m_window = game->m_window;
    name = MINOTAURINTRO;

    // hack used to make sure the display for macOS with retina display issue is consistent with display on other systems
    int testWidth;
    glfwGetFramebufferSize(m_window, &testWidth, nullptr);
    osScaleFactor = testWidth / screen.x;

    m_screen = screen;

    // Load OpenGL function pointers
    gl3w_init();

    set_onKey();

    // Create a frame buffer
    m_frame_buffer = 0;
    glGenFramebuffers(1, &m_frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);
    glViewport(0, 0, w, h);

    initialPosition = vec2({static_cast<float>(w / 2), static_cast<float>(h / 2)});

    minotaur_intro.init(initialPosition);
    m_help_menu.init(initialPosition);
    m_help_menu.set_visibility(false);
    initialize_camera_position(w, h);

    // Shift texture up to fit screen
    minotaur_intro.set_position(cameraCenter);

    m_start_time = glfwGetTime();
}

void MinotaurIntroState::draw()
{
    // Clearing error buffer
    gl_flush_errors();

    // Getting size of window
    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);

    // Updating window title with points
    std::stringstream title_ss;
    title_ss << "Minos' Monster Maze";
    glfwSetWindowTitle(m_window, title_ss.str().c_str());

    render_to_framebuffer_screen();

    mat3 projection_2D = calculate_projection();

    minotaur_intro.draw(projection_2D);

    m_help_menu.draw(projection_2D);
    // Presenting
    glfwSwapBuffers(m_window);
}

bool MinotaurIntroState::update(float elapsed_ms)
{
    if (!should_show()) {
        load_minotaur_level();
    }

    return true;
}

void MinotaurIntroState::on_key(GLFWwindow*, int key, int, int action, int mod)
{
    if (key == GLFW_KEY_ESCAPE)
    {
        load_minotaur_level();
    }
}

bool MinotaurIntroState::is_over()
{
    return glfwWindowShouldClose(m_window);
}

void MinotaurIntroState::destroy()
{
    glDeleteFramebuffers(1, &m_frame_buffer);

    minotaur_intro.destroy();
}

bool MinotaurIntroState::should_show()
{
    return glfwGetTime() - m_start_time < minotaur_intro_length;
}

void MinotaurIntroState::load_minotaur_level()
{
    Level* level = (Level*) game->get_state(LEVEL);

    level->load_select_level(minotaur_level);
    game->set_current_state(level);
    world = level;
    level->reset_pause_start();
}

