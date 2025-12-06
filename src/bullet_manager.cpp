#include "bullet_manager.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <algorithm>

using namespace godot;

BulletManager::BulletManager() {
}

BulletManager::~BulletManager() {
}

void BulletManager::_ready() {
}

void BulletManager::_exit_tree() {
}

// --- Bullet Creation ---

void BulletManager::create_bullet(const Transform3D &p_transform) {
    int index;

    if (inactive_indices.empty()) {
        index = active_bullets.size();
        active_bullets.emplace_back(); // Grow vector
    } else {
        index = inactive_indices.back();
        inactive_indices.pop_back();
    }

    BulletData &data = active_bullets[index];
    data.transform = p_transform;
    data.timer = lifespan;
}

// --- Main Loop (Raycasting Implementation) ---

void BulletManager::_physics_process(double delta) {
    // Don't run logic in editor, only in game
    if (Engine::get_singleton()->is_editor_hint()) return;

    Ref<MultiMesh> mm = get_multimesh();
    if (mm.is_null()) return;

    size_t count = active_bullets.size();
    mm->set_instance_count(count);

    // 1. Prepare Physics Access
    PhysicsDirectSpaceState3D *space_state = get_world_3d()->get_direct_space_state();
    
    // Create the query object once to reuse it (Performance optimization)
    Ref<PhysicsRayQueryParameters3D> ray_query = memnew(PhysicsRayQueryParameters3D);
    ray_query->set_collision_mask(collision_mask);
    ray_query->set_collide_with_bodies(true);
    ray_query->set_collide_with_areas(true); // Set false if you don't want to hit Areas

    // 2. Prepare Visual Buffer
    PackedFloat32Array buffer = mm->get_buffer();
    if (buffer.size() != count * 12) {
        buffer.resize(count * 12);
    }
    float *ptr = buffer.ptrw();

    double dealer_speed = damage_dealer.is_valid() ? damage_dealer->get_speed() : 50.0;

    // --- LOOP ---
    for (size_t i = 0; i < count; i++) {
        BulletData &data = active_bullets[i];
        
        // A. Lifespan check
        data.timer -= delta;
        if (data.timer <= 0.0) {
            bullets_to_remove.push_back(i);
            continue;
        }

        // B. Calculate Movement
        Vector3 old_pos = data.transform.origin;
        Vector3 move_vec = -data.transform.basis.get_column(2) * dealer_speed * delta;
        Vector3 new_pos = old_pos + move_vec;

        bool collided = false;

        // C. Raycast Check (Only if bullet moved)
        if (old_pos != new_pos) {
            ray_query->set_from(old_pos);
            ray_query->set_to(new_pos);

            Dictionary result = space_state->intersect_ray(ray_query);

            if (!result.is_empty()) {
                collided = true;
                
                // Handle Collision
                Node *collider = Object::cast_to<Node>(result["collider"]);
                Vector3 hit_position = result["position"];

                if (collider && collider->has_method("hit")) {
                    if (damage_dealer.is_valid()) {
                        // Set the damage dealer to the exact impact point
                        damage_dealer->set_global_position(hit_position);
                        collider->call("hit", damage_dealer);
                    }
                }

                // Mark for removal
                bullets_to_remove.push_back(i);
            }
        }

        if (collided) {
             // If we hit something, don't update visual transform, just wait for cleanup
             continue; 
        }

        // D. Update Position
        data.transform.origin = new_pos;

        // E. Visual Update (Write to MultiMesh buffer)
        size_t offset = i * 12;

        ptr[offset + 0] = data.transform.basis.rows[0][0];
        ptr[offset + 1] = data.transform.basis.rows[0][1];
        ptr[offset + 2] = data.transform.basis.rows[0][2];
        ptr[offset + 3] = data.transform.origin.x;

        ptr[offset + 4] = data.transform.basis.rows[1][0];
        ptr[offset + 5] = data.transform.basis.rows[1][1];
        ptr[offset + 6] = data.transform.basis.rows[1][2];
        ptr[offset + 7] = data.transform.origin.y;

        ptr[offset + 8] = data.transform.basis.rows[2][0];
        ptr[offset + 9] = data.transform.basis.rows[2][1];
        ptr[offset + 10] = data.transform.basis.rows[2][2];
        ptr[offset + 11] = data.transform.origin.z;
    }

    mm->set_buffer(buffer);

    // 3. Cleanup
    if (!bullets_to_remove.empty()) {
        _remove_queued_bullets();
    }
}

// --- Removal (Swap-and-Pop) ---

void BulletManager::_remove_queued_bullets() {
    // Sort descending to ensure we remove from end to start to preserve indices
    std::sort(bullets_to_remove.begin(), bullets_to_remove.end(), std::greater<int>());

    for (int idx : bullets_to_remove) {
        if (idx >= active_bullets.size()) continue;

        size_t last_index = active_bullets.size() - 1;

        if (idx != last_index) {
            // Swap Data: Move the last bullet's data to the gap
            active_bullets[idx] = active_bullets[last_index];
            // No spatial grid to update anymore!
        }
        
        active_bullets.pop_back();
        // Since we are using an index pool, we don't strictly need inactive_indices 
        // if we just shrink the vector, but if you want to reuse allocated memory 
        // you might change the strategy. Here, we simply shrink.
    }
    
    // Clear the inactive indices logic if simply using vector size,
    // OR if you want to keep memory allocated, don't pop_back and instead mark as inactive.
    // For simplicity and MultiMesh compatibility, shrinking the vector is usually safest/easiest.
    
    bullets_to_remove.clear();
}

// --- Bind Methods ---

void BulletManager::_bind_methods() {
    ClassDB::bind_method(D_METHOD("create_bullet", "transform"), &BulletManager::create_bullet);
    
    ClassDB::bind_method(D_METHOD("set_damage_dealer", "damage_dealer"), &BulletManager::set_damage_dealer);
    ClassDB::bind_method(D_METHOD("get_damage_dealer"), &BulletManager::get_damage_dealer);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "damage_dealer", PROPERTY_HINT_RESOURCE_TYPE, "DamageDealer"), "set_damage_dealer", "get_damage_dealer");

    ClassDB::bind_method(D_METHOD("set_collision_mask", "collision_mask"), &BulletManager::set_collision_mask);
    ClassDB::bind_method(D_METHOD("get_collision_mask"), &BulletManager::get_collision_mask);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_mask", "get_collision_mask");

    ClassDB::bind_method(D_METHOD("set_lifespan", "lifespan"), &BulletManager::set_lifespan);
    ClassDB::bind_method(D_METHOD("get_lifespan"), &BulletManager::get_lifespan);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "lifespan"), "set_lifespan", "get_lifespan");

    ClassDB::bind_method(D_METHOD("get_active_bullet_transforms"), &BulletManager::get_active_bullet_transforms);
}

Array BulletManager::get_active_bullet_transforms() const {
    Array transforms;
    for (const BulletData& data : active_bullets) {
        transforms.append(data.transform);
    }
    return transforms;
}
