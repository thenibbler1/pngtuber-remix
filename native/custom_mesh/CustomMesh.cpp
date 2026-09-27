#include "CustomMesh.h"

#include <godot_cpp/classes/geometry2d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <chrono>
#include <algorithm>
#include <cstdint>



using namespace godot;

    static inline bool has_point(const Vector2 *arr, int size, int i) {
        return arr && i < size;
    }


    std::recursive_mutex CustomMesh::deform_mutex;
    std::vector<Ref<GlueGroup>> CustomMesh::glue_groups;
    
    std::vector<CustomMesh*>  CustomMesh::mesh_registry;


    void GlueGroup::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_indices","value"), &GlueGroup::set_indices);
        ClassDB::bind_method(D_METHOD("get_indices"), &GlueGroup::get_indices);
        ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY,"indices"), "set_indices","get_indices");


        ClassDB::bind_method(D_METHOD("set_id","new_id"), &GlueGroup::set_id);
        ClassDB::bind_method(D_METHOD("get_id"), &GlueGroup::get_id);
        ADD_PROPERTY(PropertyInfo(Variant::INT,"id"), "set_id","get_id");

        ClassDB::bind_method(D_METHOD("set_glue_name","new_name"), &GlueGroup::set_glue_name);
        ClassDB::bind_method(D_METHOD("get_glue_name"), &GlueGroup::get_glue_name);
        ADD_PROPERTY(PropertyInfo(Variant::STRING,"glue_name"), "set_glue_name","get_glue_name");

        ClassDB::bind_method(D_METHOD("set_weights","value"), &GlueGroup::set_weights);
        ClassDB::bind_method(D_METHOD("get_weights"), &GlueGroup::get_weights);
        ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY,"weights"), "set_weights","get_weights");

        ClassDB::bind_method(D_METHOD("set_last_position","value"), &GlueGroup::set_last_position);
        ClassDB::bind_method(D_METHOD("get_last_position"), &GlueGroup::get_last_position);
        ADD_PROPERTY(PropertyInfo(Variant::VECTOR2,"last_position"), "set_last_position","get_last_position");
    }

    void DeformLayer::_bind_methods() {


        ClassDB::bind_method(D_METHOD("set_id","new_id"), &DeformLayer::set_id);
        ClassDB::bind_method(D_METHOD("get_id"), &DeformLayer::get_id);
        ADD_PROPERTY(PropertyInfo(Variant::INT,"id"), "set_id","get_id");


        ClassDB::bind_method(D_METHOD("resize", "n"), &DeformLayer::resize);

        #define BIND_VECTOR2_ARRAY_DEFORM(prop) \
            ClassDB::bind_method(D_METHOD("set_"#prop,"value"), &DeformLayer::set_##prop); \
            ClassDB::bind_method(D_METHOD("get_"#prop), &DeformLayer::get_##prop); \
            ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY,#prop), "set_"#prop, "get_"#prop)

        #define BIND_INT32_ARRAY_DEFORM(prop) \
            ClassDB::bind_method(D_METHOD("set_"#prop,"value"), &DeformLayer::set_##prop); \
            ClassDB::bind_method(D_METHOD("get_"#prop), &DeformLayer::get_##prop); \
            ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY,#prop), "set_"#prop, "get_"#prop)


        ClassDB::bind_method(D_METHOD("set_strength","value"), &DeformLayer::set_strength);
        ClassDB::bind_method(D_METHOD("get_strength"), &DeformLayer::get_strength);

        ClassDB::bind_method(D_METHOD("set_target_strength","value"), &DeformLayer::set_target_strength);
        ClassDB::bind_method(D_METHOD("get_target_strength"), &DeformLayer::get_target_strength);

        ClassDB::bind_method(D_METHOD("set_velocity","value"), &DeformLayer::set_velocity);
        ClassDB::bind_method(D_METHOD("get_velocity"), &DeformLayer::get_velocity);

        ClassDB::bind_method(D_METHOD("set_damping","value"), &DeformLayer::set_damping);
        ClassDB::bind_method(D_METHOD("get_damping"), &DeformLayer::get_damping);

        ClassDB::bind_method(D_METHOD("set_stiffness","value"), &DeformLayer::set_stiffness);
        ClassDB::bind_method(D_METHOD("get_stiffness"), &DeformLayer::get_stiffness);

        ClassDB::bind_method(D_METHOD("set_gravity","value"), &DeformLayer::set_gravity);
        ClassDB::bind_method(D_METHOD("get_gravity"), &DeformLayer::get_gravity);

        ClassDB::bind_method(D_METHOD("set_mass","value"), &DeformLayer::set_mass);
        ClassDB::bind_method(D_METHOD("get_mass"), &DeformLayer::get_mass);

        ClassDB::bind_method(D_METHOD("set_sine_speed","value"), &DeformLayer::set_sine_speed);
        ClassDB::bind_method(D_METHOD("get_sine_speed"), &DeformLayer::get_sine_speed);

        ClassDB::bind_method(D_METHOD("set_sine_amplitude","value"), &DeformLayer::set_sine_amplitude);
        ClassDB::bind_method(D_METHOD("get_sine_amplitude"), &DeformLayer::get_sine_amplitude);


        ClassDB::bind_method(D_METHOD("set_noise_speed","value"), &DeformLayer::set_noise_speed);
        ClassDB::bind_method(D_METHOD("get_noise_speed"), &DeformLayer::get_noise_speed);

        ClassDB::bind_method(D_METHOD("set_noise_scale","value"), &DeformLayer::set_noise_scale);
        ClassDB::bind_method(D_METHOD("get_noise_scale"), &DeformLayer::get_noise_scale);

        ClassDB::bind_method(D_METHOD("set_follow_lerp","value"), &DeformLayer::set_follow_lerp);
        ClassDB::bind_method(D_METHOD("get_follow_lerp"), &DeformLayer::get_follow_lerp);
        ClassDB::bind_method(D_METHOD("set_motion","value"), &DeformLayer::set_motion);
        ClassDB::bind_method(D_METHOD("get_motion"), &DeformLayer::get_motion);   

        ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"strength"), "set_strength","get_strength");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"target_strength"), "set_target_strength","get_target_strength");

        ADD_PROPERTY(PropertyInfo(Variant::VECTOR2,"external_velocity"), "set_velocity","get_velocity");
        ADD_PROPERTY(PropertyInfo(Variant::VECTOR2,"damping"), "set_damping","get_damping");
        ADD_PROPERTY(PropertyInfo(Variant::VECTOR2,"stiffness"), "set_stiffness","get_stiffness");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"gravity"), "set_gravity","get_gravity");
        ADD_PROPERTY(PropertyInfo(Variant::VECTOR2,"mass"), "set_mass","get_mass");

        ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"sine_speed"), "set_sine_speed","get_sine_speed");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"sine_amplitude"), "set_sine_amplitude","get_sine_amplitude");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"noise_speed"), "set_noise_speed","get_noise_speed");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"noise_scale"), "set_noise_scale","get_noise_scale");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"follow_lerp"), "set_follow_lerp","get_follow_lerp");

        ADD_PROPERTY(
            PropertyInfo(
                Variant::INT,
                "motion",
                PROPERTY_HINT_ENUM,
                "Spring,Sine,Noise,Follow,Custom"
            ),
            "set_motion",
            "get_motion"
        );

        BIND_VECTOR2_ARRAY_DEFORM(vertices);
        BIND_VECTOR2_ARRAY_DEFORM(top_left);
        BIND_VECTOR2_ARRAY_DEFORM(middle_left);
        BIND_VECTOR2_ARRAY_DEFORM(bottom_left);

        BIND_VECTOR2_ARRAY_DEFORM(top_middle);
        BIND_VECTOR2_ARRAY_DEFORM(center);
        BIND_VECTOR2_ARRAY_DEFORM(bottom_middle);

        BIND_VECTOR2_ARRAY_DEFORM(top_right);
        BIND_VECTOR2_ARRAY_DEFORM(middle_right);
        BIND_VECTOR2_ARRAY_DEFORM(bottom_right);

    }

    static inline Vector2 fast_lerp(const Vector2 &a, const Vector2 &b, float t) {
        return Vector2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
    }

    static inline bool fast_tri_valid(const Vector2 &a, const Vector2 &b, const Vector2 &c) {
        float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        return Math::abs(cross) > 0.001f;
    }

    static inline double move_toward_fast(double current, double target, double max_delta) {
        double diff = target - current;
        if (Math::abs(diff) <= max_delta) return target;
        return current + (diff > 0.0 ? max_delta : -max_delta);
    }

    CustomMesh::CustomMesh() {
        _cached_tri_points.resize(3);
        _temp_tri_uvs.resize(3);
        _cached_tri_uvs.resize(0);
        _last_vertices_used.resize(0);
        _vertices_dirty.store(true, std::memory_order_relaxed);
        mesh_registry.push_back(this);

    }

    CustomMesh::~CustomMesh() {
    mesh_registry.erase(
        std::remove(mesh_registry.begin(), mesh_registry.end(), this),
        mesh_registry.end()
    );
    }

    void CustomMesh::_notification(int p_what) {
    }

    void CustomMesh::_bind_methods() {

        ClassDB::bind_method(D_METHOD("remove_deform_layer", "index"), &CustomMesh::remove_deform_layer);
        ClassDB::bind_method(D_METHOD("add_deform_layer"), &CustomMesh::add_deform_layer);
        ClassDB::bind_method(D_METHOD("get_layer_count"), &CustomMesh::get_layer_count);
        ClassDB::bind_method(D_METHOD("deformations_3x3","u","v"), &CustomMesh::compute_interpolated_vertices);
        ClassDB::bind_method(D_METHOD("sync_deformation_arrays"), &CustomMesh::sync_deformation_arrays);
        ClassDB::bind_method(D_METHOD("toggle_mesh_view"), &CustomMesh::toggle_mesh_view);
        ClassDB::bind_method(D_METHOD("add_internal_point","p"), &CustomMesh::add_internal_point);
        ClassDB::bind_method(D_METHOD("remove_nearest_internal_point","p","max_dist"), &CustomMesh::remove_nearest_internal_point);
        ClassDB::bind_method(D_METHOD("apply_wobble_to_deformer","wobble","delta","amp","lerp_speed"), &CustomMesh::apply_wobble_to_deformer);
        ClassDB::bind_method(D_METHOD("is_triangle_valid","a","b","c"), &CustomMesh::is_triangle_valid);
        ClassDB::bind_method(D_METHOD("is_inside_base","p"), &CustomMesh::is_inside_base);
        ClassDB::bind_method(D_METHOD("update_physics", "delta"), &CustomMesh::update_physics);

        ClassDB::bind_method(D_METHOD("get_layer", "index"), &CustomMesh::get_layer);
        ClassDB::bind_method(D_METHOD("set_layer", "index", "layer"), &CustomMesh::set_layer);
        ClassDB::bind_method(D_METHOD("get_layers"), &CustomMesh::get_layers);
        ClassDB::bind_method(D_METHOD("set_layers", "layers"), &CustomMesh::set_layers);

        ClassDB::bind_method(D_METHOD("set_texture","texture"), &CustomMesh::set_texture);
        ClassDB::bind_method(D_METHOD("get_texture"), &CustomMesh::get_texture);
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"texture",PROPERTY_HINT_RESOURCE_TYPE,"Texture2D"), "set_texture","get_texture");

        ClassDB::bind_method(D_METHOD("set_actor","actor"), &CustomMesh::set_actor);
        ClassDB::bind_method(D_METHOD("get_actor"), &CustomMesh::get_actor);
        ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"actor",PROPERTY_HINT_RESOURCE_TYPE,"Node"), "set_actor","get_actor");

        ClassDB::bind_method(D_METHOD("set_editable","editable"), &CustomMesh::set_editable);
        ClassDB::bind_method(D_METHOD("get_editable"), &CustomMesh::get_editable);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL,"editable"), "set_editable","get_editable");

        ClassDB::bind_method(D_METHOD("set_show_deformed_mesh","value"), &CustomMesh::set_show_deformed_mesh);
        ClassDB::bind_method(D_METHOD("get_show_deformed_mesh"), &CustomMesh::get_show_deformed_mesh);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL,"show_deformed_mesh"), "set_show_deformed_mesh","get_show_deformed_mesh");

        ClassDB::bind_method(D_METHOD("set_selected_vertex","value"), &CustomMesh::set_selected_vertex);
        ClassDB::bind_method(D_METHOD("get_selected_vertex"), &CustomMesh::get_selected_vertex);
        ADD_PROPERTY(PropertyInfo(Variant::INT,"selected_vertex"), "set_selected_vertex","get_selected_vertex");

        ClassDB::bind_method(D_METHOD("set_deform_x","value"), &CustomMesh::set_deform_x);
        ClassDB::bind_method(D_METHOD("get_deform_x"), &CustomMesh::get_deform_x);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"deform_x"), "set_deform_x","get_deform_x");

        ClassDB::bind_method(D_METHOD("set_deform_y","value"), &CustomMesh::set_deform_y);
        ClassDB::bind_method(D_METHOD("get_deform_y"), &CustomMesh::get_deform_y);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT,"deform_y"), "set_deform_y","get_deform_y");


        // Glue Groups
        ClassDB::bind_static_method("CustomMesh", D_METHOD("add_glue_group"), &CustomMesh::add_glue_group);
        ClassDB::bind_static_method("CustomMesh", D_METHOD("remove_glue_group", "index"), &CustomMesh::remove_glue_group);
        ClassDB::bind_static_method("CustomMesh", D_METHOD("get_glue_groups"), &CustomMesh::get_glue_groups);
        ClassDB::bind_static_method("CustomMesh", D_METHOD("set_glue_groups", "groups"), &CustomMesh::set_glue_groups);


        ClassDB::bind_method(D_METHOD("set_mesh_id", "id"), &CustomMesh::set_mesh_id);
        ClassDB::bind_method(D_METHOD("get_mesh_id"), &CustomMesh::get_mesh_id);
        ClassDB::bind_method(D_METHOD("set_is_warp_mesh", "value"), &CustomMesh::set_is_warp_mesh);
        ClassDB::bind_method(D_METHOD("get_is_warp_mesh"), &CustomMesh::get_is_warp_mesh);

        ClassDB::bind_method(D_METHOD("get_warp"), &CustomMesh::get_warp);
        ClassDB::bind_method(D_METHOD("set_warp", "warps"), &CustomMesh::set_warp);

        ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "warps", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT),
                    "set_warp", "get_warp");


        ClassDB::bind_method(D_METHOD("get_glue"), &CustomMesh::get_glue);
        ClassDB::bind_method(D_METHOD("set_glue", "glues"), &CustomMesh::set_glue);

        ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "glues", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT),
                    "set_glue", "get_glue");


        #define BIND_VECTOR2_ARRAY(prop) \
            ClassDB::bind_method(D_METHOD("set_"#prop,"value"), &CustomMesh::set_##prop); \
            ClassDB::bind_method(D_METHOD("get_"#prop), &CustomMesh::get_##prop); \
            ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY,#prop), "set_"#prop, "get_"#prop)
        #define BIND_INT32_ARRAY(prop) \
            ClassDB::bind_method(D_METHOD("set_"#prop,"value"), &CustomMesh::set_##prop); \
            ClassDB::bind_method(D_METHOD("get_"#prop), &CustomMesh::get_##prop); \
            ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY,#prop), "set_"#prop, "get_"#prop)



        BIND_VECTOR2_ARRAY(original_vertices);
        BIND_VECTOR2_ARRAY(base_vertices);
        BIND_VECTOR2_ARRAY(deformed_vertices);
        BIND_VECTOR2_ARRAY(internal_vertices);
        BIND_VECTOR2_ARRAY(interpolated_vertices);
        BIND_INT32_ARRAY(triangles);
    }


    Array CustomMesh::get_warp(){
        return warps;
    }

    void CustomMesh::set_warp(const Array &arr){
        warps = arr;
    }

    Array CustomMesh::get_glue(){
        return glues;
    }

    void CustomMesh::set_glue(const Array &arr){
        glues = arr;
    }

Ref<GlueGroup> CustomMesh::add_glue_group() {
    Ref<GlueGroup> g;
    g.instantiate();
    glue_groups.push_back(g);
    return g;
}

void CustomMesh::remove_glue_group(int index) {
    if (index >= 0 && index < (int)glue_groups.size())
        glue_groups.erase(glue_groups.begin() + index);
}

Array CustomMesh::get_glue_groups() {
    Array arr;
    for (auto &g : glue_groups) arr.append(g);
    return arr;
}

void CustomMesh::set_glue_groups(const Array &arr) {
    glue_groups.clear();
    for (int i = 0; i < arr.size(); ++i) {
        Ref<GlueGroup> g = arr[i];
        if (g.is_valid()) glue_groups.push_back(g);
    }
}


Ref<DeformLayer> CustomMesh::get_layer(int index) const {
    if (index < 0 || index >= (int)deform_layers.size()) return Ref<DeformLayer>();
    return deform_layers[index];
}

void CustomMesh::set_layer(int index, const Ref<DeformLayer> &layer) {
    if (!layer.is_valid()) return;
    while (index >= (int)deform_layers.size()) {
        add_deform_layer();
    }

    deform_layers[index] = layer;
    sync_deformation_arrays();
    _vertices_dirty.store(true);
    queue_redraw();
}

Array CustomMesh::get_layers() const {
    Array arr;
    for (auto &layer_ref : deform_layers) {
        arr.append(layer_ref);
    }
    return arr;
}

void CustomMesh::set_layers(const Array &arr) {
    deform_layers.clear();
    for (int i = 0; i < arr.size(); ++i) {
        Ref<DeformLayer> layer = arr[i];
        if (layer.is_valid()) deform_layers.push_back(layer);
    }
    sync_deformation_arrays();
    _vertices_dirty.store(true);
    queue_redraw();
}

void CustomMesh::set_interpolated_vertices(const PackedVector2Array &p) {
    std::lock_guard<std::recursive_mutex> lock(deform_mutex);
    interpolated_vertices = p;
    _vertices_dirty.store(true);
    queue_redraw();
}

PackedVector2Array CustomMesh::get_interpolated_vertices() const {
    std::lock_guard<std::recursive_mutex> lock(deform_mutex);
    return interpolated_vertices;
}

void CustomMesh::set_texture(const Ref<Texture2D> &p_texture) { texture = p_texture; _vertices_dirty.store(true); queue_redraw(); }
Ref<Texture2D> CustomMesh::get_texture() const { return texture; }

void CustomMesh::set_actor(Node *p_actor) { actor = p_actor; }
Node* CustomMesh::get_actor() const { return actor; }

void CustomMesh::set_editable(bool p) { editable = p; }
bool CustomMesh::get_editable() const { return editable; }

void CustomMesh::set_show_deformed_mesh(bool p) { show_deformed_mesh = p; _vertices_dirty.store(true); queue_redraw(); }
bool CustomMesh::get_show_deformed_mesh() const { return show_deformed_mesh; }

void CustomMesh::set_selected_vertex(int p) { selected_vertex = p; queue_redraw(); }
int CustomMesh::get_selected_vertex() const { return selected_vertex; }

void CustomMesh::set_deform_x(float p) { deform_x = p; queue_redraw(); }
float CustomMesh::get_deform_x() const { return deform_x; }

void CustomMesh::set_deform_y(float p) { deform_y = p; queue_redraw(); }
float CustomMesh::get_deform_y() const { return deform_y; }

void CustomMesh::set_original_vertices(const PackedVector2Array &p) { std::lock_guard<std::recursive_mutex> lock(deform_mutex); original_vertices = p; _vertices_dirty.store(true); queue_redraw(); }
PackedVector2Array CustomMesh::get_original_vertices() const { std::lock_guard<std::recursive_mutex> lock(deform_mutex); return original_vertices; }

void CustomMesh::set_base_vertices(const PackedVector2Array &p) { std::lock_guard<std::recursive_mutex> lock(deform_mutex); base_vertices = p; _vertices_dirty.store(true); queue_redraw(); }
PackedVector2Array CustomMesh::get_base_vertices() const { std::lock_guard<std::recursive_mutex> lock(deform_mutex); return base_vertices; }

void CustomMesh::set_deformed_vertices(const PackedVector2Array &p) { std::lock_guard<std::recursive_mutex> lock(deform_mutex); deformed_vertices = p; _vertices_dirty.store(true); queue_redraw(); }
PackedVector2Array CustomMesh::get_deformed_vertices() const { std::lock_guard<std::recursive_mutex> lock(deform_mutex); return deformed_vertices; }

void CustomMesh::set_triangles(const PackedInt32Array &p) { std::lock_guard<std::recursive_mutex> lock(deform_mutex); triangles = p; _cached_tri_uvs.resize(triangles.size()); _vertices_dirty.store(true); queue_redraw(); }
PackedInt32Array CustomMesh::get_triangles() const { std::lock_guard<std::recursive_mutex> lock(deform_mutex); return triangles; }

void CustomMesh::set_internal_vertices(const PackedVector2Array &p) { std::lock_guard<std::recursive_mutex> lock(deform_mutex); internal_vertices = p; _vertices_dirty.store(true); queue_redraw(); }
PackedVector2Array CustomMesh::get_internal_vertices() const { std::lock_guard<std::recursive_mutex> lock(deform_mutex); return internal_vertices; }

Ref<DeformLayer> CustomMesh::add_deform_layer() {
    std::lock_guard<std::recursive_mutex> lock(deform_mutex);
    Ref<DeformLayer> layer;
    layer.instantiate();
    layer->resize(original_vertices.size());
    deform_layers.push_back(layer);
    return layer;
}

void CustomMesh::remove_deform_layer(int index) {
    std::lock_guard<std::recursive_mutex> lock(deform_mutex);
    if (index >= 0 && index < deform_layers.size()) deform_layers.erase(deform_layers.begin() + index);
}

int CustomMesh::get_layer_count() const {
    return deform_layers.size();
}

void CustomMesh::add_internal_point(Vector2 p) {
    std::lock_guard<std::recursive_mutex> lock(deform_mutex);
    internal_vertices.append(p);
    original_vertices.append(p);
    deformed_vertices.append(p);
    interpolated_vertices.append(p);
    triangles = Geometry2D::get_singleton()->triangulate_delaunay(original_vertices);

    _vertices_dirty.store(true);
    queue_redraw();
}

bool CustomMesh::remove_nearest_internal_point(Vector2 p, double max_dist) {
    std::lock_guard<std::recursive_mutex> lock(deform_mutex);

    int best_i = -1;
    double best_d = max_dist;

    for (int i = 0; i < internal_vertices.size(); ++i) {
        double d = internal_vertices[i].distance_to(p);
        if (d < best_d) {
            best_d = d;
            best_i = i;
        }
    }

    if (best_i == -1) return false;

    internal_vertices.remove_at(best_i);
    original_vertices.remove_at(original_vertices.size() - internal_vertices.size() - 1 + best_i);
    deformed_vertices.remove_at(deformed_vertices.size() - internal_vertices.size() - 1 + best_i);
    interpolated_vertices.remove_at(interpolated_vertices.size() - internal_vertices.size() - 1 + best_i);

    triangles = Geometry2D::get_singleton()->triangulate_delaunay(original_vertices);

    _vertices_dirty.store(true);
    queue_redraw();
    return true;
}

void CustomMesh::deformations_3x3(double u, double v, bool redraw = false) { compute_interpolated_vertices(u,v,redraw); }

void CustomMesh::sync_deformation_arrays() {
    std::lock_guard<std::recursive_mutex> lock(deform_mutex);

    int n = original_vertices.size();
    auto ensure_size = [&](PackedVector2Array &arr) {
        while (arr.size() < n) {
            arr.append(original_vertices[arr.size()]);
        }
        if (arr.size() > n) arr.resize(n);
    };

    for (auto &layer_ref : deform_layers) {
        if (!layer_ref.is_valid()) continue;
        DeformLayer *layer = layer_ref.ptr();

        ensure_size(layer->top_left);
        ensure_size(layer->top_middle);
        ensure_size(layer->top_right);

        ensure_size(layer->middle_left);
        ensure_size(layer->center);
        ensure_size(layer->middle_right);

        ensure_size(layer->bottom_left);
        ensure_size(layer->bottom_middle);
        ensure_size(layer->bottom_right);
        layer->vertices.resize(n);
        for (int i = 0; i < n; ++i) {
            layer->vertices[i] = original_vertices[i];
        }
    }

    if (interpolated_vertices.size() != n) {
        interpolated_vertices.resize(n);
        for (int i = 0; i < n; ++i) {
            interpolated_vertices[i] = original_vertices[i];
        }
    } else {
        for (int i = 0; i < n; ++i) {
            if (interpolated_vertices[i] == Vector2()) {
                interpolated_vertices[i] = original_vertices[i];
            }
        }
    }

    _vertices_dirty.store(true, std::memory_order_relaxed);
}

void CustomMesh::toggle_mesh_view() { show_deformed_mesh = !show_deformed_mesh; _vertices_dirty.store(true); queue_redraw(); }

bool CustomMesh::is_triangle_valid(const Vector2 &a, const Vector2 &b, const Vector2 &c) const {
    double area = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
    return Math::abs(area)*0.5 > 0.001;
}

bool CustomMesh::is_inside_base(const Vector2 &p) const {
    std::lock_guard<std::recursive_mutex> lock(deform_mutex);
    return Geometry2D::get_singleton()->is_point_in_polygon(p, base_vertices);
}

Vector2 CustomMesh::apply_wobble_to_deformer( const Vector2 &wobble,double delta,const Vector2 &amp,double lerp_speed) {
    if (amp.x == 0.0 && amp.y == 0.0) return Vector2(0.5, 0.5);

    Vector2 safe_amp(amp);
    if (safe_amp.x == 0.0) safe_amp.x = 1e-6;
    if (safe_amp.y == 0.0) safe_amp.y = 1e-6;

    Vector2 target_pos((wobble.x / (2.0 * safe_amp.x)) + 0.5,
                       (wobble.y / (2.0 * safe_amp.y)) + 0.5);
    last_deform_pos.x = Math::lerp(float(last_deform_pos.x), float(target_pos.x), float(lerp_speed));
    last_deform_pos.y = Math::lerp(float(last_deform_pos.y), float(target_pos.y), float(lerp_speed));

    return last_deform_pos;
}


void CustomMesh::sample_target_mesh(const CustomMesh* target_mesh,const Vector2& position,float radius,int max_neighbors,Vector2& out_delta) {
    float radius_sq = radius * radius;

    struct Neighbor {
        int index;
        float dist_sq;
    };

    std::vector<Neighbor> neighbors;
    neighbors.reserve(16);

    for (int ti = 0; ti < target_mesh->original_vertices.size(); ++ti) {
        float d_sq = position.distance_squared_to(target_mesh->original_vertices[ti]);

        if (d_sq <= radius_sq) {
            neighbors.push_back({ti, d_sq});
        }
    }

    if (neighbors.empty()) {
        for (int ti = 0; ti < target_mesh->original_vertices.size(); ++ti) {
            float d_sq = position.distance_squared_to(target_mesh->original_vertices[ti]);
            neighbors.push_back({ti, d_sq});
        }

        std::partial_sort(
            neighbors.begin(),
            neighbors.begin() + std::min(max_neighbors, (int)neighbors.size()),
            neighbors.end(),
            [](const Neighbor& a, const Neighbor& b) {
                return a.dist_sq < b.dist_sq;
            }
        );

        neighbors.resize(std::min(max_neighbors, (int)neighbors.size()));
    }

    Vector2 total_delta;
    float total_weight = 0.0f;

    for (const Neighbor& n : neighbors) {
        float w = std::exp(-n.dist_sq / (radius_sq));

        if (w < 1e-5f)
            continue;

        for (auto &ext_layer_ref : target_mesh->deform_layers) {
            if (!ext_layer_ref.is_valid()) continue;
            const DeformLayer &ext_layer = *ext_layer_ref.ptr();

            Vector2 layer_delta;
            if (!sample_layer_point(
                    ext_layer,
                    n.index,
                    ext_layer.strength,
                    ext_layer.strength_v,
                    layer_delta))
                continue;

            if (ext_layer.motion == DeformLayer::MotionType::BOUNCY) {
                Vector2 bounce_delta;
                if (sample_layer_point(
                        ext_layer,
                        n.index,
                        ext_layer.bounce_lerp.x,
                        ext_layer.bounce_lerp.y,
                        bounce_delta)) {
                    total_delta += bounce_delta * ext_layer.target_strength * w;
                    total_weight += w;
                }
            } else {
                total_delta += layer_delta * ext_layer.target_strength * w;
                total_weight += w;
            }
        }
    }

    if (total_weight > 0.0f)
        out_delta = total_delta / total_weight;
    else
        out_delta = Vector2(0, 0);
}


void CustomMesh::update_physics(double delta, bool preview_physics) {
    std::lock_guard<std::recursive_mutex> lock(deform_mutex);

    if (original_vertices.is_empty() || deform_layers.empty())
        return;

    Vector2 global_target(deform_x, deform_y);

for (auto &layer_ref : deform_layers) {
    if (!layer_ref.is_valid()) continue;
    DeformLayer *layer = layer_ref.ptr();

    if (preview_physics || layer->motion == DeformLayer::MotionType::CUSTOM) {
        layer->strength   = global_target.x;
        layer->strength_v = global_target.y;
        layer->velocity   = 0.f;
        layer->velocity_v = 0.f;
        continue;
    }

    Vector2 target = global_target;
    apply_motion_modifier(*layer, target, delta);

    float accel_x = (target.x - layer->strength)
                    * layer->stiffness.x
                    / std::max(layer->mass.x, 1e-6f);

    float accel_y = (target.y - layer->strength_v)
                    * layer->stiffness.y
                    / std::max(layer->mass.y, 1e-6f);

    accel_y += layer->gravity;

    layer->velocity   += accel_x * float(delta);
    layer->velocity_v += accel_y * float(delta);

    layer->velocity   *= std::exp(-layer->damping.x * delta);
    layer->velocity_v *= std::exp(-layer->damping.y * delta);

    layer->velocity   += layer->external_velocity.x * float(delta);
    layer->velocity_v += layer->external_velocity.y * float(delta);

    layer->strength   += layer->velocity   * float(delta);
    layer->strength_v += layer->velocity_v * float(delta);
    apply_bounce_modifier(*layer , delta);

    }
    PackedVector2Array temp_vertices = original_vertices;

    for (int i = 0; i < deform_layers.size(); ++i) {
        const Ref<DeformLayer> &layer_ref = deform_layers[i];
        if (!layer_ref.is_valid()) continue;

        const DeformLayer &layer = *layer_ref.ptr();

        for (int j = 0; j < temp_vertices.size(); ++j) {
            Vector2 delta;
            if (!sample_layer_point(
                    layer,
                    j,
                    layer.strength,
                    layer.strength_v,
                    delta))
                continue;
            if (preview_physics|| layer.motion == DeformLayer::MotionType::CUSTOM){
            temp_vertices[j] += delta;
            }
            else if (layer.motion == DeformLayer::MotionType::BOUNCY){
                Vector2 bounce_delta;
                sample_layer_point(layer,j,layer.bounce_lerp.x,layer.bounce_lerp.y,bounce_delta);

            temp_vertices[j] += bounce_delta  * layer.target_strength;
            }
            else{
            temp_vertices[j] += delta * layer.target_strength;
            }
            
        }
    }

    interpolated_vertices = temp_vertices;

    // Glue
    for (int i = 0; i < glues.size(); ++i) {
        int glue_id = glues[i];
        GlueGroup *g = nullptr;
        for (auto &g_ref : glue_groups) {
            if (!g_ref.is_valid()) continue;
            if (g_ref->id != glue_id) continue;
            g = g_ref.ptr();
            break;
        }
        if (!g || g->indices.size() < 2) continue;

        Vector2 delta = interpolated_vertices[g->indices[0]] - g->last_position;
        g->last_position = interpolated_vertices[g->indices[0]];

        for (int j = 1; j < g->indices.size(); ++j) {
            int vi = g->indices[j];
            float w = (j < g->weights.size()) ? g->weights[j].x : 1.0f;
            interpolated_vertices[vi] += delta * w;
        }
    }

float radius = 75.0f;
int max_neighbors = 4;

PackedVector2Array source_vertices = temp_vertices;

for (int vi = 0; vi < temp_vertices.size(); ++vi) {

    Vector2 accumulated_delta;
    Vector2 current_pos = source_vertices[vi];

    std::vector<CustomMesh*> to_process;
    std::vector<CustomMesh*> processed;


    for (int i = 0; i < warps.size(); ++i) {
        int warp_id = warps[i];
        CustomMesh* target_mesh = nullptr;

        for (CustomMesh* mesh : mesh_registry) {
            if (!mesh) continue;
            if (mesh->mesh_id == warp_id) {
                target_mesh = mesh;
                break;
            }
        }
        if (target_mesh) {
            to_process.push_back(target_mesh);
        }
    }

    while (!to_process.empty()) {
        CustomMesh* current = to_process.back();
        to_process.pop_back();

        bool _processed = false;
        for (CustomMesh* p : processed) {
            if (p == current) {
                _processed = true;
                break;
            }
        }
        if (_processed) continue;
        processed.push_back(current);

        for (int wi = 0; wi < current->warps.size(); ++wi) {
            int next_warp_id = current->warps[wi];
            CustomMesh* next_target = nullptr;

            for (int mi = 0; mi < mesh_registry.size(); ++mi) {
                CustomMesh* mesh = mesh_registry[mi];
                if (!mesh) continue;
                if (mesh->mesh_id == next_warp_id) {
                    next_target = mesh;
                    break;
                }
            }

            if (next_target)
                to_process.push_back(next_target);
        }


        Vector2 total_delta;
        sample_target_mesh(current,current_pos + accumulated_delta, radius, max_neighbors, total_delta);

        accumulated_delta += total_delta;
    }

    temp_vertices[vi] += accumulated_delta;
}

    interpolated_vertices = temp_vertices;

    _vertices_dirty.store(true);
    queue_redraw();
}

double CustomMesh::move_toward_double(double current, double target, double max_delta) { return move_toward_fast(current,target,max_delta); }

void CustomMesh::compute_interpolated_vertices(double u_in, double v_in, bool redraw) {
    std::lock_guard<std::recursive_mutex> lock(deform_mutex);

    int n = original_vertices.size();
    if (n == 0 || deform_layers.empty())
        return;

    

    float u = Math::clamp((float)u_in, 0.f, 1.f);
    float v = Math::clamp((float)v_in, 0.f, 1.f);
    deform_x = u;
    deform_y = v;
    last_u = u;
    last_v = v;

    if (redraw){

        queue_redraw();
    }

}

void CustomMesh::_draw() {
    PackedVector2Array vertices_to_draw;
    PackedInt32Array triangles_copy;
    Vector2 tex_size;

    {
        std::lock_guard<std::recursive_mutex> lock(deform_mutex);
        if (original_vertices.is_empty() || triangles.is_empty() || texture.is_null()) return;

        // Use interpolated_vertices if deform layers exist, else fallback like old behavior
        vertices_to_draw = !deform_layers.empty() 
                               ? interpolated_vertices 
                               : (!interpolated_vertices.is_empty() ? interpolated_vertices 
                                                                    : (show_deformed_mesh ? deformed_vertices : original_vertices));

        triangles_copy = triangles;
        tex_size = texture->get_size();
    }

    const int vertex_count = vertices_to_draw.size();
    const int tri_count = triangles_copy.size();
    if (vertex_count == 0 || tri_count < 3) return;

    const Vector2 offset = -tex_size * 0.5f;
    const Vector2 inv_tex = (tex_size.x != 0.f && tex_size.y != 0.f) 
                            ? Vector2(1.f / tex_size.x, 1.f / tex_size.y) 
                            : Vector2();

    const Vector2 *vptr = vertices_to_draw.ptr();
    const Vector2 *uv_src = original_vertices.ptr(); // Use original vertices for UVs
    const int32_t *tri_ptr = triangles_copy.ptr();

    PackedVector2Array all_vertices;
    PackedVector2Array all_uvs;
    PackedColorArray all_colors;
    PackedInt32Array all_indices;

    int insert_index = 0;

    for (int t = 0; t + 2 < tri_count; t += 3) {
        int ai = tri_ptr[t], bi = tri_ptr[t + 1], ci = tri_ptr[t + 2];
        if (ai < 0 || bi < 0 || ci < 0) continue;
        if (ai >= vertex_count || bi >= vertex_count || ci >= vertex_count) continue;

        Vector2 va = vptr[ai] + offset;
        Vector2 vb = vptr[bi] + offset;
        Vector2 vc = vptr[ci] + offset;

        if (!fast_tri_valid(va, vb, vc)) continue;

        all_vertices.push_back(va);
        all_vertices.push_back(vb);
        all_vertices.push_back(vc);

        // --- Map UVs using original vertices, normalized ---
        all_uvs.push_back(uv_src[ai] * inv_tex);
        all_uvs.push_back(uv_src[bi] * inv_tex);
        all_uvs.push_back(uv_src[ci] * inv_tex);

        all_colors.push_back(Color(1,1,1,1));
        all_colors.push_back(Color(1,1,1,1));
        all_colors.push_back(Color(1,1,1,1));

        all_indices.push_back(insert_index++);
        all_indices.push_back(insert_index++);
        all_indices.push_back(insert_index++);
    }

    if (all_vertices.is_empty()) return;

    RenderingServer::get_singleton()->canvas_item_add_triangle_array(
        get_canvas_item(),
        all_indices,
        all_vertices,
        all_colors,
        all_uvs,
        PackedInt32Array(),
        PackedFloat32Array(),
        texture->get_rid()
    );
}

bool CustomMesh::sample_layer_point(const DeformLayer &layer, int i,float u_in, float v_in,Vector2 &out)
{
    float u = Math::clamp(u_in, 0.f, 1.f);
    float v = 1.f - Math::clamp(v_in, 0.f, 1.f);

    if (i >= layer.top_left.size())
        return false;

    const bool left = u <= 0.5f;
    const bool top  = v <= 0.5f;

    const float ut = left ? (u * 2.f) : ((u - 0.5f) * 2.f);
    const float vt = top  ? (v * 2.f) : ((v - 0.5f) * 2.f);

    const Vector2 *tl = layer.top_left.ptr();
    const Vector2 *tm = layer.top_middle.ptr();
    const Vector2 *tr = layer.top_right.ptr();
    const Vector2 *ml = layer.middle_left.ptr();
    const Vector2 *mc = layer.center.ptr();
    const Vector2 *mr = layer.middle_right.ptr();
    const Vector2 *bl = layer.bottom_left.ptr();
    const Vector2 *bm = layer.bottom_middle.ptr();
    const Vector2 *br = layer.bottom_right.ptr();

    const Vector2 *top_a = left ? tl : tm;
    const Vector2 *top_b = left ? tm : tr;
    const Vector2 *mid_a = left ? ml : mc;
    const Vector2 *mid_b = left ? mc : mr;
    const Vector2 *bot_a = left ? bl : bm;
    const Vector2 *bot_b = left ? bm : br;

    Vector2 top_row = fast_lerp(top_a[i], top_b[i], ut);
    Vector2 mid_row = fast_lerp(mid_a[i], mid_b[i], ut);
    Vector2 bot_row = fast_lerp(bot_a[i], bot_b[i], ut);

    Vector2 result = top
        ? fast_lerp(top_row, mid_row, vt)
        : fast_lerp(mid_row, bot_row, vt);

    out = result;
    return true;
}