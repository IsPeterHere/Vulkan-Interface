#include "BaseApp.h"

class INSTANCE
{
public:
    static INSTANCE* get_INSTANCE()
    {
        if (instance == NULL)
            instance = new INSTANCE();
        return instance;
    }

    static void destroy_INSTANCE()
    {
        delete instance;
        instance = NULL;
    }

    void main()
    {
        vary = MYR::PushConstant{ 0,16,&pushConstantValues,VK_SHADER_STAGE_FRAGMENT_BIT };
        app.createPushConstant(vary);

        app.initComponents();

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
        app.run_with_update_function(&update, 1);
    }

    static bool update(double delta_T)
    {
        instance->pushConstantValues[0] = instance->pushConstantValues[0] + 0.01;
        instance->app.vertices[0].pos.z += 0.01f;
        return true;
    }

    BaseApp app{800,600};
    MYR::PushConstant vary;
    glm::vec4 pushConstantValues{ 0.10, 0.10, 0.9,1 };
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