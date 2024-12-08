#include "api.h"

namespace pbrt
{
    Options PbrtOptions;

    constexpr int MaxTransforms = 2;
    constexpr int StateTransformBits = 1 << 0;
    constexpr int EndTransformBits = 1 << 1;
    constexpr int AllTransformBits = (1 << MaxTransforms) - 1;

    struct TransformSet
    {
    };

    struct RenderOptions
    {
    };

    enum class APIState { Uninitialized, OptionsBlock, WorldBlock };
    static APIState currentApiState = APIState::Uninitialized;
    static TransformSet curTransform;
    static uint32_t activeTransformBits = AllTransformBits;
    static std::unique_ptr<RenderOptions> renderOptions;

    void pbrtInit(const Options& opt)
    {
        PbrtOptions = opt;
        renderOptions.reset(new RenderOptions);
    }

    void pbrtWorldBegin()
    {
        currentApiState = APIState::WorldBlock;
        activeTransformBits = AllTransformBits;
    }

    void pbrtWorldEnd()
    {
        currentApiState = APIState::OptionsBlock;
        renderOptions.reset(new RenderOptions);
    }

    void pbrtCleanup()
    {

    }
}