#include "game.hxx"

int main(int /*argc*/, char* /*argv*/[])
{
    arcanoid::game arcanoid {};
    arcanoid.main_loop();
    return EXIT_SUCCESS;
}
