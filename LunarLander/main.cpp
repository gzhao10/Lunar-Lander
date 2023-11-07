/**
* Author: Gavin Zhao
* Assignment: Lunar Lander
* Date due: 2023-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ACC_OF_GRAVITY -2.5f
#define NUMBER_OF_PLATFORMS 12

#ifdef _WINDOWS
    #include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "Entity.h"
#include <vector>
#include <ctime>
#include "cmath"

// ————— STRUCTS AND ENUMS —————//
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* messages;
    Entity* fuel;
};

// ————— CONSTANTS ————— //
const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED     = 0,
            BG_BLUE    = 0,
            BG_GREEN   = 0,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const char SPRITESHEET_FILEPATH[]   = "assets/Space.png",
           PLATFORM_FILEPATH[]      = "assets/Ground.png",
           ASTEROID_FILEPATH[]      = "assets/Asteroid.png",
           WIN_MESSAGE_FILEPATH[]   = "assets/Win.png",
           LOSE_MESSAGE_FILEPATH[]  = "assets/Lose.png",
           FULL_FUEL_FILEPATH[]     = "assets/FullFuel.png",
           HALF_FUEL_FILEPATH[]     = "assets/HalfFuel.png",
           LOW_FUEL_FILEPATH[]      = "assets/LowFuel.png",
           NO_FUEL_FILEPATH[]       = "assets/NoFuel.png";
           

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL  = 0;
const GLint TEXTURE_BORDER   = 0;

// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_time_accumulator = 0.0f;

// ———— GENERAL FUNCTIONS ———— //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Hello, Lunar Lander!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // ————— PLAYER ————— //
    g_game_state.player = new Entity();
    g_game_state.player->set_entity_type(PLAYER);
    g_game_state.player->set_position(glm::vec3(0.0f, 3.5f, 0.0f));
    g_game_state.player->set_movement(glm::vec3(0.0f));
    g_game_state.player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY * 0.1, 0.0f));
    g_game_state.player->set_speed(1.0f);
    g_game_state.player->m_texture_id = load_texture(SPRITESHEET_FILEPATH);
    

    g_game_state.player->m_walking[g_game_state.player->LEFT]  = new int[2] { 2, 3 };
    g_game_state.player->m_walking[g_game_state.player->RIGHT] = new int[2] { 0, 1 };
    g_game_state.player->m_walking[g_game_state.player->UP]    = new int[2] { 4, 5 };

    g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->UP];
    g_game_state.player->m_animation_time    = 0.0f;
    g_game_state.player->m_animation_frames  = 2;
    g_game_state.player->m_animation_index   = 0;
    g_game_state.player->m_animation_cols    = 6;
    g_game_state.player->m_animation_rows    = 1;
    
    
    
    // ————— PLATFORMS ————— //
    g_game_state.platforms = new Entity[NUMBER_OF_PLATFORMS];

    for (int i = 0; i < 3; i++){
        g_game_state.platforms[i].set_entity_type(WIN_PLATFORM);
        g_game_state.platforms[i].m_texture_id = load_texture(PLATFORM_FILEPATH);
        g_game_state.platforms[i].set_position(glm::vec3(i + 1.75f, -3.75f, 0.0f));
        g_game_state.platforms[i].update(0.0f, NULL, 0);
    }
    
    
    g_game_state.platforms[3].set_position (glm::vec3(-4.0f,  0.0f, 0.0f));
    g_game_state.platforms[4].set_position (glm::vec3(-3.0f,  1.0f, 0.0f));
    g_game_state.platforms[5].set_position (glm::vec3(-3.0f, -3.0f, 0.0f));
    g_game_state.platforms[6].set_position (glm::vec3(-2.0f,  3.0f, 0.0f));
    g_game_state.platforms[7].set_position (glm::vec3(-0.5f, -2.5f, 0.0f));
    g_game_state.platforms[8].set_position (glm::vec3( 0.0f,  0.0f, 0.0f));
    g_game_state.platforms[9].set_position (glm::vec3( 2.0f,  0.5f, 0.0f));
    g_game_state.platforms[10].set_position(glm::vec3( 3.0f,  2.0f, 0.0f));
    g_game_state.platforms[11].set_position(glm::vec3( 4.0f, -2.0f, 0.0f));
    
    for (int i = 3; i < 12; i++){
        g_game_state.platforms[i].set_height(0.5f);
        g_game_state.platforms[i].set_width(0.5f);
        g_game_state.platforms[i].set_entity_type(LOSE_PLATFORM);
        g_game_state.platforms[i].m_texture_id = load_texture(ASTEROID_FILEPATH);
        g_game_state.platforms[i].update(0.0f, NULL, 0);
    }
    
    // ————— MESSAGES ————— //
    g_game_state.messages = new Entity[2];
    g_game_state.messages[0].m_texture_id = load_texture(WIN_MESSAGE_FILEPATH);
    g_game_state.messages[1].m_texture_id = load_texture(LOSE_MESSAGE_FILEPATH);
    
    for (int i = 0; i < 2; i++){
        g_game_state.messages[i].set_position(glm::vec3(0.0f));
        g_game_state.messages[i].set_entity_type(MESSAGE);
        g_game_state.messages[i].update(0.0f, NULL, 0);
        g_game_state.messages[i].deactivate();
    }
    
    // ————— FUEL ————— //
    g_game_state.fuel = new Entity[4];
    
    g_game_state.fuel[0].m_texture_id = load_texture(FULL_FUEL_FILEPATH);
    g_game_state.fuel[1].m_texture_id = load_texture(HALF_FUEL_FILEPATH);
    g_game_state.fuel[2].m_texture_id = load_texture(LOW_FUEL_FILEPATH);
    g_game_state.fuel[3].m_texture_id = load_texture(NO_FUEL_FILEPATH);
    
    for (int i = 0; i < 4; i++){
        g_game_state.fuel[i].set_position(glm::vec3(-3.5f, 2.0f, 0.0f));
        g_game_state.fuel[i].set_entity_type(FUEL);
        g_game_state.fuel[i].update(0.0f, NULL, 0);
        g_game_state.fuel[i].deactivate();
    }
    
    g_game_state.fuel[0].activate();
    
    // ————— GENERAL ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_game_state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q: g_game_is_running = false;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT]){
        g_game_state.player->m_is_boosting = true;
        g_game_state.player->m_fuel--;
        if (g_game_state.player->m_fuel > 0){
            g_game_state.player->boost_left();
        }
        g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT]){
        g_game_state.player->m_is_boosting = true;
        g_game_state.player->m_fuel--;
        if (g_game_state.player->m_fuel > 0){
            g_game_state.player->boost_right();
        }
        g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->RIGHT];
    }
    if (key_state[SDL_SCANCODE_UP]){
        g_game_state.player->m_is_boosting = true;
        g_game_state.player->m_fuel--;
        if (g_game_state.player->m_fuel > 0){
            g_game_state.player->boost_up();
        }
        g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->UP];
    }
    
    
    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
    {
        g_game_state.player->set_movement(glm::normalize(g_game_state.player->get_movement()));
    }
}

void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_time_accumulator;

    if (delta_time < FIXED_TIMESTEP){
        g_time_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP){
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.platforms, NUMBER_OF_PLATFORMS);
        delta_time -= FIXED_TIMESTEP;
    }

    g_time_accumulator = delta_time;
    
    if (g_game_state.player->m_win){
        g_game_state.messages[0].activate();
    }
    else if (g_game_state.player->m_lose){
        g_game_state.messages[1].activate();
    }
    
    if (g_game_state.player->m_fuel < 0){
        g_game_state.fuel[2].deactivate();
        g_game_state.fuel[3].activate();
    }
    else if (g_game_state.player->m_fuel < 350){
        g_game_state.fuel[1].deactivate();
        g_game_state.fuel[2].activate();
    }
    else if (g_game_state.player->m_fuel < 750){
        g_game_state.fuel[0].deactivate();
        g_game_state.fuel[1].activate();
    }
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    g_game_state.player->render(&g_shader_program);
    
    for (int i = 0; i < NUMBER_OF_PLATFORMS; i++){
        g_game_state.platforms[i].render(&g_shader_program);
    }
    for (int i = 0; i < 2; i++){
        g_game_state.messages[i].render(&g_shader_program);
    }
    for (int i = 0; i < 4; i++){
        g_game_state.fuel[i].render(&g_shader_program);
    }
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

// ————— DRIVER GAME LOOP ————— /
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
