enum EntityType{ PLAYER, WIN_PLATFORM, LOSE_PLATFORM, MESSAGE, FUEL };

class Entity{
private:
    EntityType m_entity_type;
    
    bool m_is_active = true;
    
    int *m_animation_right = NULL, // move to the right
        *m_animation_left  = NULL, // move to the left
        *m_animation_up    = NULL; // move upwards
    
    // ––––– PHYSICS (GRAVITY) ––––– //
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;
    
    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::mat4 m_model_matrix;
    float     m_speed;
    
    float m_width = .75;
    float m_height = .75;
    
public:
    // ————— STATIC VARIABLES ————— //
    static const int SECONDS_PER_FRAME = 4;
    static const int LEFT  = 0,
                     RIGHT = 1,
                     UP    = 2;
    
    // ————— ANIMATION ————— //
    int **m_walking = new int*[3]
    {
        m_animation_left,
        m_animation_right,
        m_animation_up
    };
    
    int m_animation_frames = 0,
        m_animation_index  = 0,
        m_animation_cols   = 0,
        m_animation_rows   = 0;
    
    int  *m_animation_indices = NULL;
    float m_animation_time    = 0.0f;
    
    // ––––– BOOSTING ––––– //
    bool  m_is_boosting     = false;
    int   m_fuel            = 1000;
    
    // ––––– PHYSICS (COLLISIONS) ––––– //
    bool m_win = false;
    bool m_lose = false;
    
    // ————— TEXTURES ————— //
    GLuint    m_texture_id;

    // ————— METHODS ————— //
    Entity();
    ~Entity();
    
    bool const check_collision(Entity* other) const;
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    
    
    void draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id, int index);
    void update(float delta_time, Entity* collidable_entities, int collidable_entity_count);
    void render(ShaderProgram *program);
    
    void boost_up()      { m_velocity.y += 0.008f; };
    void boost_left()      { m_acceleration.x -= 0.35f; };
    void boost_right()      { m_acceleration.x += 0.35f; }
    
    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };
    
    // ————— GETTERS ————— //
    glm::vec3 const get_position()      const { return m_position;   };
    glm::vec3 const get_velocity()      const { return m_velocity; };
    glm::vec3 const get_movement()      const { return m_movement;   };
    glm::vec3 const get_acceleration()  const { return m_acceleration; };
    float     const get_speed()         const { return m_speed;      };
    int       const get_width()         const { return m_width; };
    int       const get_height()        const { return m_height; };
    EntityType const get_entity_type()  const { return m_entity_type; };
    
    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position)         { m_position = new_position; };
    void const set_velocity(glm::vec3 new_velocity)         { m_velocity = new_velocity; };
    void const set_acceleration(glm::vec3 new_position)     { m_acceleration = new_position; };
    void const set_movement(glm::vec3 new_movement)         { m_movement = new_movement; };
    void const set_speed(float new_speed)                   { m_speed = new_speed; };
    void const set_width(float new_width)                   { m_width = new_width; };
    void const set_height(float new_height)                 { m_height = new_height; };
    void const set_entity_type(EntityType new_type)         { m_entity_type = new_type; };
};
