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
        std::string m_log_info {"Log info here"};
        std::string m_time_info {"Time info here"};

        float m_enemy_hp {100.f};

        std::string m_divide_line {"====================="};

        float       m_player_hp {100.f};
        std::string m_skill_1 {"Attack ��ͨ����"};
        bool        m_use_skill_1 {false};
        std::string m_skill_2 {"Black bear attack heart �ڻ�����"};
        bool        m_use_skill_2 {false};
        std::string m_skill_3 {"Lightning five whip ����������"};
        bool        m_use_skill_3 {false};
    };
} // namespace Piccolo