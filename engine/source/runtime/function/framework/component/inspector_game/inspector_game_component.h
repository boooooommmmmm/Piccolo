#pragma once

#include "runtime/function/framework/component/component.h"
#include "runtime/resource/res_type/components/inspector_game.h"
#include "runtime/function/render/render_object.h"

#include <vector>
#include <runtime/function/inspector_game/inspector_game_battle_admin.h>

namespace Piccolo
{
    REFLECTION_TYPE(InspectorGameComponent)
    CLASS(InspectorGameComponent : public Component, WhiteListFields)
    {
        REFLECTION_BODY(InspectorGameComponent)
    public:
        InspectorGameComponent() = default;

        ~InspectorGameComponent() override;

        void tick(float delta_time) override;

    private:
        META(Enable)
        InspectorGameComponentRes m_inspector_game_res;

        InspectorGameBattleAdmin *m_battle_admin;
        bool m_is_initialized {false};
    };

} // namespace Piccolo