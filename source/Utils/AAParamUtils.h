#pragma once
#include <juce_core/juce_core.h>

namespace ParamNames
{
    struct ParamName
    {
        juce::String id;
        juce::String name;
    };

    inline ParamName getDriveNames() { return {"Drive", "Drive"}; }
    inline ParamName getToneNames() { return {"Tone", "Tone"}; }
    inline ParamName getLevelNames() { return {"Level", "Level"}; }
    inline ParamName getODModeNames() { return {"ODMode", "OD Mode"}; }
    inline ParamName getBypassNames() { return {"Bypass", "Bypass"}; }
}
