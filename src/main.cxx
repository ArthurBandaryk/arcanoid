#include "game.hxx"

int main(int, char**)
{
    arcanoid::game arcanoid {};
    arcanoid.main_loop();
    return EXIT_SUCCESS;
}
