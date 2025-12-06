#include "bullet_manager.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <algorithm>

using namespace godot;

BulletManager::BulletManager() {
    active_bullets.reserve(1024); // optional default pool
}

BulletManager::~BulletManager() {
}

void BulletManager::_ready() {
    set_physics_process(true);

    ray_query.instantiate();
    ray_query->set_collide_with_areas(true);
    ray_query->set_collide_with_bodies(true);
}

void BulletManager::_exit_tree() {
    active_bullets.clear();
    bullets_to_remove.clear();
}

// ----------------------------------------
// Bullet Creation
// ----------------------------------------

void BulletManager::create_bullet(const Transform3D &p_transform) {
    BulletData data;
    data.transform = p_transform;
    data.timer = lifespan;

    active_bullets.push_back(data);
}

// ----------------------------------------
// Physics Loop
// ----------------------------------------

void BulletManager::_physics_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) return;

    Ref<MultiMesh> mm = get_multimesh();
    if (mm.is_null()) return;

    PhysicsDirectSpaceState3D *space_state = get_world_3d()->get_direct_space_state();

    bullets_to_remove.clear();

    double speed = damage_dealer.is_valid() ? damage_dealer->get_speed() : 50.0;

    int count = static_cast<int>(active_bullets.size());

    // Movement pass (mark removals)
    for (int i = 0; i < count; i++) {
        BulletData &b = active_bullets[i];

        // Lifespan
        b.timer -= delta;
        if (b.timer <= 0.0) {
            bullets_to_remove.push_back(i);
            continue;
        }

        Vector3 old_pos = b.transform.origin;
        Vector3 new_pos = old_pos + (-b.transform.basis.get_column(2) * speed * delta);

        bool collided = false;

        if (old_pos.distance_to(new_pos) > 0.0001f) {
            ray_query->set_collision_mask(collision_mask);
            ray_query->set_from(old_pos);
            ray_query->set_to(new_pos);

            Dictionary result = space_state->intersect_ray(ray_query);

            if (!result.is_empty()) {
                collided = true;

                Variant vcol = result["collider"];
                Object *obj = nullptr;

                if (vcol.get_type() == Variant::OBJECT)
                    obj = vcol;

                Node *collider = Object::cast_to<Node>(obj);

                Vector3 hit_pos = result["position"];

                if (collider && collider->has_method("hit") && damage_dealer.is_valid()) {
                    damage_dealer->set_global_position(hit_pos);
                    collider->call("hit", damage_dealer);
                }

                bullets_to_remove.push_back(i);
            }
        }

        if (!collided) {
            // Update position
            b.transform.origin = new_pos;
        }
    }

    // ----------------------------------------
    // Remove bullets BEFORE writing MultiMesh
    // ----------------------------------------
    if (!bullets_to_remove.empty()) {
        _remove_queued_bullets();
    }

    int final_count = static_cast<int>(active_bullets.size());
    mm->set_instance_count(final_count);

    // ----------------------------------------
    // Visual Buffer
    // ----------------------------------------
    PackedFloat32Array buffer;
    buffer.resize(final_count * 12);
    float *ptr = buffer.ptrw();

    for (int i = 0; i < final_count; i++) {
        const BulletData &b = active_bullets[i];
        size_t offset = i * 12;

        Vector3 r0 = b.transform.basis.get_column(0);
        Vector3 r1 = b.transform.basis.get_column(1);
        Vector3 r2 = b.transform.basis.get_column(2);

        ptr[offset + 0] = r0.x;
        ptr[offset + 1] = r0.y;
        ptr[offset + 2] = r0.z;
        ptr[offset + 3] = b.transform.origin.x;

        ptr[offset + 4] = r1.x;
        ptr[offset + 5] = r1.y;
        ptr[offset + 6] = r1.z;
        ptr[offset + 7] = b.transform.origin.y;

        ptr[offset + 8] = r2.x;
        ptr[offset + 9] = r2.y;
        ptr[offset + 10] = r2.z;
        ptr[offset + 11] = b.transform.origin.z;
    }

    mm->set_buffer(buffer);
}

// ----------------------------------------
// Removal - Swap & Pop
// ----------------------------------------

void BulletManager::_remove_queued_bullets() {
    std::sort(bullets_to_remove.begin(), bullets_to_remove.end(), std::greater<int>());

    for (int idx : bullets_to_remove) {
        int last = active_bullets.size() - 1;

        if (idx != last)
            active_bullets[idx] = std::move(active_bullets[last]);

        active_bullets.pop_back();
    }
}

// ----------------------------------------
// Bindings
// ----------------------------------------

void BulletManager::_bind_methods() {
    ClassDB::bind_method(D_METHOD("create_bullet", "transform"), &BulletManager::create_bullet);

    ClassDB::bind_method(D_METHOD("set_damage_dealer", "damage_dealer"), &BulletManager::set_damage_dealer);
    ClassDB::bind_method(D_METHOD("get_damage_dealer"), &BulletManager::get_damage_dealer);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "damage_dealer", PROPERTY_HINT_RESOURCE_TYPE, "DamageDealer"),
                 "set_damage_dealer", "get_damage_dealer");

    ClassDB::bind_method(D_METHOD("set_collision_mask", "collision_mask"), &BulletManager::set_collision_mask);
    ClassDB::bind_method(D_METHOD("get_collision_mask"), &BulletManager::get_collision_mask);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS),
                 "set_collision_mask", "get_collision_mask");

    ClassDB::bind_method(D_METHOD("set_lifespan", "lifespan"), &BulletManager::set_lifespan);
    ClassDB::bind_method(D_METHOD("get_lifespan"), &BulletManager::get_lifespan);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "lifespan"), "set_lifespan", "get_lifespan");

    ClassDB::bind_method(D_METHOD("get_active_bullet_transforms"),
                         &BulletManager::get_active_bullet_transforms);
}

Array BulletManager::get_active_bullet_transforms() const {
    Array arr;
    for (const BulletData &b : active_bullets) {
        arr.push_back(b.transform);
    }
    return arr;
}
