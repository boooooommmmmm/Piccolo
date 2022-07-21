#pragma once

#include "runtime/function/framework/component/component.h"
#include "runtime/resource/res_type/components/inspector_game.h"
#include "runtime/function/render/render_object.h"

#include <vector>

namespace Piccolo
{
    REFLECTION_TYPE(InspectorGameComponent)
    CLASS(InspectorGameComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(InspectorGameComponent)
    public:
        InspectorGameComponent() = default;

        //~InspectorGameComponent() override;

        //void tick(float delta_time) override;

    private:
        META(Enable)
        InspectorGameComponentRes m_inspector_game_res;
    };

} // namespace Piccolo