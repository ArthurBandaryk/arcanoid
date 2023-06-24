#include "game.hxx"

#ifdef __ANDROID__
// IMPORTANT NOTE: we should define visibility default and
// replace `main` with `SDL_main`, otherwise there will be
// no dynamic symbol and `nativeRun()` function will not be
// able to find `SDL_main` function. Check this:
// https://github.com/urho3d/Urho3D/issues/2267
// And also this:
// https://github.com/libsdl-org/SDL/issues/2989
extern "C" __attribute__((visibility("default"))) int SDL_main(int /*argc*/, char* /*argv*/[])
#else
int main(int /*argc*/, char* /*argv*/[])
#endif
{
    arcanoid::game arcanoid {};
    arcanoid.main_loop();
    return EXIT_SUCCESS;
}
