#ifndef DAMAGE_DEALER_H
#define DAMAGE_DEALER_H

#include <godot_cpp/classes/resource.hpp>

using namespace godot;

class DamageDealer : public Resource {
    GDCLASS(DamageDealer, Resource)

private:
    bool can_parry = true;
    double damage = 15.0;
    double speed = 5.0;
    double knockback_force = 0.0;
    Vector3 global_position;

protected:
    static void _bind_methods();

public:
    // Getters and Setters
    void set_can_parry(bool p_value) { can_parry = p_value; }
    bool get_can_parry() const { return can_parry; }

    void set_damage(double p_value) { damage = p_value; }
    double get_damage() const { return damage; }

    void set_speed(double p_value) { speed = p_value; }
    double get_speed() const { return speed; }

    void set_knockback_force(double p_value) { knockback_force = p_value; }
    double get_knockback_force() const { return knockback_force; }

    void set_global_position(const Vector3 &p_value) { global_position = p_value; }
    Vector3 get_global_position() const { return global_position; }
};

#endif
