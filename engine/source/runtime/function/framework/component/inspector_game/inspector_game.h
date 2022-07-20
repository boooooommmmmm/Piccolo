#pragma once

#include "runtime/core/math/vector3.h"
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/function/framework/component/component.h"
#include "runtime/resource/res_type/data/basic_shape.h"

namespace Piccolo
{
    REFLECTION_TYPE(InspectorGame)
    CLASS(InspectorGame : public Component, WhiteListFields)
    {
        REFLECTION_BODY(InspectorGame)
    public:
        InspectorGame() = default;

        ~InspectorGame() override;

        void tick(float delta_time) override;

    private:

        META(Enable)
        float m_player_hp {100.f};
    };

} // namespace Piccolo