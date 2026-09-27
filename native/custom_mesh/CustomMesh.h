#ifndef CUSTOM_MESH_HPP
#define CUSTOM_MESH_HPP

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/geometry2d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <mutex>
#include <atomic>
#include <vector>
#include <map>
#include <godot_cpp/classes/time.hpp>

using namespace godot;

class DeformLayer : public RefCounted {
    GDCLASS(DeformLayer, RefCounted);

protected:
    static void _bind_methods();

public:
    PackedVector2Array vertices;
    PackedVector2Array top_left;
    PackedVector2Array top_middle;
    PackedVector2Array top_right;
    PackedVector2Array middle_left;
    PackedVector2Array center;
    PackedVector2Array middle_right;
    PackedVector2Array bottom_left;
    PackedVector2Array bottom_middle;
    PackedVector2Array bottom_right;

    void resize(int n) {
        top_left.resize(n);
        top_middle.resize(n);
        top_right.resize(n);
        middle_left.resize(n);
        center.resize(n);
        middle_right.resize(n);
        bottom_left.resize(n);
        bottom_middle.resize(n);
        bottom_right.resize(n);
    }

    int id = 0;

    void set_id(const int &new_id) { id = new_id; }
    int get_id() const { return id; }


    PackedVector2Array get_vertices() const { return vertices; }
    void set_vertices(const PackedVector2Array &v) { vertices = v; }

    PackedVector2Array get_top_left() const { return top_left; }
    PackedVector2Array get_top_middle() const { return top_middle; }
    PackedVector2Array get_top_right() const { return top_right; }
    PackedVector2Array get_middle_left() const { return middle_left; }
    PackedVector2Array get_center() const { return center; }
    PackedVector2Array get_middle_right() const { return middle_right; }
    PackedVector2Array get_bottom_left() const { return bottom_left; }
    PackedVector2Array get_bottom_middle() const { return bottom_middle; }
    PackedVector2Array get_bottom_right() const { return bottom_right; }

    void set_top_left(const PackedVector2Array &v) { top_left = v; }
    void set_top_middle(const PackedVector2Array &v) { top_middle = v; }
    void set_top_right(const PackedVector2Array &v) { top_right = v; }
    void set_middle_left(const PackedVector2Array &v) { middle_left = v; }
    void set_center(const PackedVector2Array &v) { center = v; }
    void set_middle_right(const PackedVector2Array &v) { middle_right = v; }
    void set_bottom_left(const PackedVector2Array &v) { bottom_left = v; }
    void set_bottom_middle(const PackedVector2Array &v) { bottom_middle = v; }
    void set_bottom_right(const PackedVector2Array &v) { bottom_right = v; }

    float strength = 1.0f;

    float velocity = 0.0f;
    Vector2 external_velocity = Vector2(0.0f, 0.0f);
    Vector2 damping = Vector2(1.0f, 1.0f); 
    Vector2 stiffness = Vector2(12.0f, 12.0f);
    float gravity = 0.0f;
    Vector2 mass = Vector2(1.0f, 1.0f);
    float target_strength = 1.0f;
    

    float strength_v = 0.5f; 
    float velocity_v = 0.0f;  

    float sine_speed = 6.0f;
    float sine_amplitude = 0.1f;

    Vector2 bounce_lerp = Vector2(0,0);

    float get_sine_speed() const { return sine_speed; }
    void set_sine_speed(const float &v) { sine_speed = v; }

    float get_sine_amplitude() const { return sine_amplitude; }
    void set_sine_amplitude(const float &v) { sine_amplitude = v; }

    float noise_speed = 1.5f;
    float noise_scale = 0.2f;

    float get_noise_speed() const { return noise_speed; }
    void set_noise_speed(const float &v) { noise_speed = v; }

    float get_noise_scale() const { return noise_scale; }
    void set_noise_scale(const float &v) { noise_scale = v; }

    float follow_lerp = 0.08f;

    float get_follow_lerp() const { return follow_lerp; }
    void set_follow_lerp(const float &v) { follow_lerp = v; }



    float get_strength() const { return strength; }
    void set_strength(const float &v) { strength = v; }
    

    float get_target_strength() const { return target_strength; }
    void set_target_strength(const float &v) { target_strength = v; }

    Vector2 get_velocity() const { return external_velocity; }
    void set_velocity(const Vector2 &v) { external_velocity = v; }

    Vector2 get_damping() const { return damping; }
    void set_damping(const Vector2 &v) { damping = v; }


    Vector2 get_stiffness() const { return stiffness; }
    void set_stiffness(const Vector2 &v) { stiffness = v; }

    float get_gravity() const { return gravity; }
    void set_gravity(const float &v) { gravity = v; }

    Vector2 get_mass() const { return mass; }
    void set_mass(const Vector2 &v) { mass = v; }

    enum class MotionType : int {
        SPRING = 0,
        SINE,
        NOISE,
        FOLLOW,
        BOUNCY,
        CUSTOM,
        
    };

    MotionType motion = MotionType::CUSTOM;

    int get_motion() const {
        return static_cast<int>(motion);
    }

    void set_motion(int v) {
        motion = static_cast<MotionType>(v);
    }
};


class GlueGroup : public RefCounted {
    GDCLASS(GlueGroup, RefCounted);

protected:
    static void _bind_methods();

public:
    String glue_name = "New Glue";
    PackedInt32Array indices;     // vertices this glue affects
    PackedVector2Array weights;   // optional influence per vertex
    Vector2 last_position = Vector2();

    int id = 0;

    GlueGroup() {}

    void set_id(const int &new_id) { id = new_id; }
    int get_id() const { return id; }

    void set_glue_name(const String &new_name) { glue_name = new_name; }
    String get_glue_name() const { return glue_name; }

    void set_indices(const PackedInt32Array &p) { indices = p; }
    PackedInt32Array get_indices() const { return indices; }

    void set_weights(const PackedVector2Array &p) { weights = p; }
    PackedVector2Array get_weights() const { return weights; }

    void set_last_position(const Vector2 &p) { last_position = p; }
    Vector2 get_last_position() const { return last_position; }
};




class CustomMesh : public Node2D {
    GDCLASS(CustomMesh, Node2D);

protected:
    static void _bind_methods();
    void _notification(int p_what); // Added for Registry handling

private:
    Vector2 deform_velocity = Vector2(0, 0);
    double damping = 0.15;

    float last_u = 0.0f;
    float last_v = 0.0f;
    Vector2 final_pos = Vector2();
    Vector2 last_deform_pos = Vector2();
    std::atomic_bool _vertices_dirty{true};
    PackedVector2Array _cached_tri_points;
    PackedVector2Array _cached_tri_uvs;
    PackedVector2Array _last_vertices_used;
    PackedVector2Array _temp_tri_uvs;
    Vector2 _last_tex_size = Vector2();
    static std::recursive_mutex deform_mutex;

public:
    CustomMesh();
    ~CustomMesh();
    
    static std::vector<CustomMesh*> mesh_registry;


    Ref<Texture2D> texture;
    Node *actor = nullptr;
    bool editable = false;
    bool show_deformed_mesh = true;
    int selected_vertex = -1;
    float deform_x = 0.0f;
    float deform_y = 0.5f;

    int mesh_id = 0;
    bool is_warp_mesh = false;

    PackedVector2Array original_vertices;
    PackedVector2Array base_vertices;
    PackedVector2Array deformed_vertices;
    PackedInt32Array triangles;
    PackedVector2Array internal_vertices;
    PackedVector2Array interpolated_vertices;
    

    std::vector<Ref<DeformLayer>> deform_layers;
    Ref<DeformLayer> get_layer(int index) const;
    void set_layer(int index, const Ref<DeformLayer> &layer);


    Array glues;
    Array warps;


    void set_mesh_id(int id) { mesh_id = id; }
    int get_mesh_id() { return mesh_id; }

    void set_is_warp_mesh(bool val) { is_warp_mesh = val; }
    bool get_is_warp_mesh() { return is_warp_mesh; }



    Array get_warp();
    void set_warp(const Array &arr);


    Array get_glue();
    void set_glue(const Array &arr);


    Array get_layers() const;
    void set_layers(const Array &arr);

    void set_texture(const Ref<Texture2D> &p_texture);
    Ref<Texture2D> get_texture() const;

    void set_actor(Node *p_actor);
    Node *get_actor() const;

    void set_editable(bool p);
    bool get_editable() const;

    void set_show_deformed_mesh(bool p);
    bool get_show_deformed_mesh() const;

    void set_selected_vertex(int p);
    int get_selected_vertex() const;

    void set_deform_x(float p);
    float get_deform_x() const;

    void set_deform_y(float p);
    float get_deform_y() const;
    void sample_target_mesh(const CustomMesh* target_mesh,const Vector2& position,float radius,int max_neighbors,Vector2& out_delta);

    void set_original_vertices(const PackedVector2Array &p);
    PackedVector2Array get_original_vertices() const;

    void set_interpolated_vertices(const PackedVector2Array &p);
    PackedVector2Array get_interpolated_vertices() const;


    void set_base_vertices(const PackedVector2Array &p);
    PackedVector2Array get_base_vertices() const;

    void set_deformed_vertices(const PackedVector2Array &p);
    PackedVector2Array get_deformed_vertices() const;

    void set_triangles(const PackedInt32Array &p);
    PackedInt32Array get_triangles() const;

    void set_internal_vertices(const PackedVector2Array &p);
    PackedVector2Array get_internal_vertices() const;

    Ref<DeformLayer> add_deform_layer();
    void remove_deform_layer(int index);
    int get_layer_count() const;

    void compute_interpolated_vertices(double u_in, double v_in, bool redraw);

    void deformations_3x3(double u, double v, bool redraw);
    void sync_deformation_arrays();
    void toggle_mesh_view();

    void add_internal_point(Vector2 p);
    bool remove_nearest_internal_point(Vector2 p, double max_dist = 12.0);

    bool is_triangle_valid(const Vector2 &a, const Vector2 &b, const Vector2 &c) const;
    bool is_inside_base(const Vector2 &p) const;

    Vector2 apply_wobble_to_deformer(const Vector2 &wobble, double delta, const Vector2 &amp, double lerp_speed);

    void _draw() override;

    float simulate_physics(
        float current,
        float target,
        float &velocity,
        float dt,
        float stiffness,
        float damping
    ) {
        float force = (target - current) * stiffness;
        velocity += force * dt;
        velocity *= damping;
        return current + velocity * dt;
    }


    void update_physics(double delta, bool preview_physics);

    static std::vector<Ref<GlueGroup>> glue_groups;

    static Ref<GlueGroup> add_glue_group();
    static void remove_glue_group(int index);
    static Array get_glue_groups();
    static void set_glue_groups(const Array &arr);

private:
    static double move_toward_double(double current, double target, double max_delta);

    bool sample_layer_point(
        const DeformLayer &layer,
        int index,
        float u,
        float v,
        Vector2 &out
    ) ;

inline void apply_motion_modifier( DeformLayer &layer,Vector2 &target, double dt) {
    switch (layer.motion) {

        case DeformLayer::MotionType::SINE: {
            float t = Time::get_singleton()->get_ticks_msec() * 0.001f;
            target.x += sin(t * layer.sine_speed) * layer.sine_amplitude;
            target.y += cos(t * layer.sine_speed) * layer.sine_amplitude;
        } break;

        case DeformLayer::MotionType::NOISE: {
            float t = Time::get_singleton()->get_ticks_msec() * 0.001f;
            target.x += Math::sin(t * layer.noise_speed) * layer.noise_scale;
            target.y += Math::cos(t * layer.noise_speed) * layer.noise_scale;
        } break;

        case DeformLayer::MotionType::FOLLOW: {
            target += (layer.external_velocity) * layer.follow_lerp;
        } break;

        default:
            break;
    }
}


inline void apply_bounce_modifier(DeformLayer &layer, double dt) {
    if (layer.motion != DeformLayer::MotionType::BOUNCY)
        return;

    const Vector2 rest(0.5f, 0.5f);

    Vector2 displacement = layer.bounce_lerp - rest;

    Vector2 accel;
    accel.x = (-layer.stiffness.x * displacement.x) / std::max(layer.mass.x, 1e-6f);
    accel.y = (-layer.stiffness.y * displacement.y) / std::max(layer.mass.y, 1e-6f);

    accel.y += layer.gravity;

    layer.velocity   += accel.x * float(dt);
    layer.velocity_v += accel.y * float(dt);

    layer.velocity   *= std::exp(-layer.damping.x * dt);
    layer.velocity_v *= std::exp(-layer.damping.y * dt);

    layer.bounce_lerp.x += layer.velocity * float(dt);
    layer.bounce_lerp.y += layer.velocity_v * float(dt);

    if (layer.sine_amplitude > 0.0f) {
        float t = Time::get_singleton()->get_ticks_msec() * 0.001f;
        float wobble = sinf(t * layer.sine_speed) * layer.sine_amplitude;
        layer.bounce_lerp += Vector2(wobble, wobble);
    }

    layer.bounce_lerp.x = Math::clamp(layer.bounce_lerp.x, 0.0f, 1.0f);
    layer.bounce_lerp.y = Math::clamp(layer.bounce_lerp.y, 0.0f, 1.0f);
}



};

#endif