#include "damage_dealer.hpp"

void DamageDealer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_can_parry", "can_parry"), &DamageDealer::set_can_parry);
    ClassDB::bind_method(D_METHOD("get_can_parry"), &DamageDealer::get_can_parry);
    ClassDB::bind_method(D_METHOD("set_damage", "damage"), &DamageDealer::set_damage);
    ClassDB::bind_method(D_METHOD("get_damage"), &DamageDealer::get_damage);
    ClassDB::bind_method(D_METHOD("set_speed", "speed"), &DamageDealer::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &DamageDealer::get_speed);
    ClassDB::bind_method(D_METHOD("set_knockback_force", "force"), &DamageDealer::set_knockback_force);
    ClassDB::bind_method(D_METHOD("get_knockback_force"), &DamageDealer::get_knockback_force);
    // Position is internal logic, but we bind it if you need to access it from GDScript
    ClassDB::bind_method(D_METHOD("set_global_position", "position"), &DamageDealer::set_global_position);
    ClassDB::bind_method(D_METHOD("get_global_position"), &DamageDealer::get_global_position);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "can_parry"), "set_can_parry", "get_can_parry");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "damage"), "set_damage", "get_damage");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "knockback_force"), "set_knockback_force", "get_knockback_force");
}
