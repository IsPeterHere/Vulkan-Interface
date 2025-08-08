#include "BaseApp.h"

class INSTANCE
{
public:
    static INSTANCE* get_INSTANCE()
    {
        static bool first_call{ true };
        if (first_call)
            instance = new INSTANCE();
        first_call = false;
        return instance;
    }

    void main()
    {
        app.vertices =
        {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},

        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}
        };

        app.indices =
        {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
        };

        app.flush_mesh_update();
        //app.run();
        app.run_with_update_function(&update, 100);
    }

    static bool update()
    {
        instance->app.vertices[0].pos.z += 0.01f;
        return true;
    }

    BaseApp app{800,600};

private:
    static INSTANCE* instance;

    INSTANCE() {}
};

INSTANCE* INSTANCE::instance;

int main() 
{
    try 
    {
        INSTANCE* inst = INSTANCE::get_INSTANCE();
        inst->main();
        delete inst;
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}