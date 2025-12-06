#ifndef BULLET_MANAGER_H
#define BULLET_MANAGER_H

#include <godot_cpp/classes/multi_mesh_instance3d.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/collision_object3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <vector>
#include "damage_dealer.hpp"

using namespace godot;

struct BulletData {
    Transform3D transform;
    double timer;
};

class BulletManager : public MultiMeshInstance3D {
    GDCLASS(BulletManager, MultiMeshInstance3D)

private:
    Ref<DamageDealer> damage_dealer;
    uint32_t collision_mask = 1;
    double lifespan = 5.0;
    
    // --- Optimized Data Arrays ---
    std::vector<BulletData> active_bullets;
    std::vector<int> inactive_indices;
    std::vector<int> bullets_to_remove;

protected:
    static void _bind_methods();
    void _remove_queued_bullets();

public:
    BulletManager();
    ~BulletManager();

    void _ready() override;
    void _physics_process(double delta) override;
    void _exit_tree() override;

    // Public API
    void create_bullet(const Transform3D &p_transform);
    
    // Getters and Setters
    void set_damage_dealer(const Ref<DamageDealer> &p_dealer) { damage_dealer = p_dealer; }
    Ref<DamageDealer> get_damage_dealer() const { return damage_dealer; }
    
    void set_collision_mask(uint32_t p_mask) { collision_mask = p_mask; }
    uint32_t get_collision_mask() const { return collision_mask; }
    
    void set_lifespan(double p_lifespan) { lifespan = p_lifespan; }
    double get_lifespan() const { return lifespan; }

    Array get_active_bullet_transforms() const;
};

#endif
