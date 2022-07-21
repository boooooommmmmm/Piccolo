#pragma once
#include "runtime/core/math/transform.h"
#include "runtime/core/meta/reflection/reflection.h"

namespace Piccolo
{
    REFLECTION_TYPE(InspectorGameComponentRes)
    CLASS(InspectorGameComponentRes, Fields)
    {
        REFLECTION_BODY(InspectorGameComponentRes);

    public:
        float m_player_hp {100.f};
    };
} // namespace Piccolo