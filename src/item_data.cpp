// Try to keep your includes organized, so that you can easily see which ones come from your project and which ones are from Godot or other libraries
#include "item_data.hpp" // For headers that are our own, we use ""
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/variant.hpp"
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/object.hpp> // For headers coming from an external library we use <> angle brackets
#include <godot_cpp/core/print_string.hpp>

// Include things you might be using
// This line might be unnecessary if we are already including this header in the item_data.hpp file
#include <godot_cpp/classes/node.hpp>

using namespace godot;

void Test::say_hello(){
	print_line("oiiiii");
}

void Test::_bind_methods(){
	ClassDB::bind_method(D_METHOD("say_hello"), &Test::say_hello);
	ClassDB::bind_method(D_METHOD("get_my_data"), &Test::get_my_data);
	ClassDB::bind_method(D_METHOD("set_my_data", "new_data"), &Test::set_my_data);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "my_data"), "set_my_data", "get_my_data");
}

String Test::get_my_data() const{
	return my_data;
}

void Test::set_my_data(const String &new_data){
	my_data = new_data;
}
