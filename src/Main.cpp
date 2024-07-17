#include <iostream>
#include "./Game/Game.h"
#include <sol/sol.hpp>
#include <iostream>

int nativeCppCubeFunction(int n) {
    return (n * n * n);
}
void TestLua() {
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    // double binding
    lua["cube"] = nativeCppCubeFunction;


    lua.script_file("./assets/scripts/myscript.lua");

    int v = lua["some_variable"];
    std::cout << "value is " << v << std::endl;

    bool isFullScreen = lua["config"]["fullscreen"];

    sol::table config = lua["config"];
    int width = config["resolution"]["width"];
    int height = config["resolution"]["height"];

    sol::function functionFactorial = lua["factorial"];
    int functionResult = functionFactorial(5);
    std::cout << "the result is " << functionResult << std::endl;

}

int main(int argc, char* argv[]) {

    Game game;

    game.Initialize();

    game.Run();

    game.Destroy();

    // TestLua();

    return 0;
}
