#include "inspector_game_component.h"

namespace Piccolo
{
    void InspectorGameComponent() {}

    void InspectorGameComponent::tick(float delta_time)
    {
        if (m_is_initialized == false)
        {
            m_battle_admin = new InspectorGameBattleAdmin();
            m_battle_admin->Init();

            m_inspector_game_res.m_skill_1 = m_battle_admin->GetSkillName(1);
            m_inspector_game_res.m_skill_2 = m_battle_admin->GetSkillName(2);
            m_inspector_game_res.m_skill_3 = m_battle_admin->GetSkillName(3);


            m_is_initialized = true;
        }

        m_battle_admin->UpdateBattle();

        std::vector<bool> used_skill;
        used_skill.clear();
        used_skill.push_back(m_inspector_game_res.m_use_skill_1);
        used_skill.push_back(m_inspector_game_res.m_use_skill_2);
        used_skill.push_back(m_inspector_game_res.m_use_skill_3);
        m_battle_admin->SetPlayerInput(used_skill);
        //Clear input
        m_inspector_game_res.m_use_skill_1 = false;
        m_inspector_game_res.m_use_skill_2 = false;
        m_inspector_game_res.m_use_skill_3 = false;

        //Synchronize battle core data
        m_inspector_game_res.m_log_info  = m_battle_admin->GetBattleInfo();
        m_inspector_game_res.m_time_info = m_battle_admin->GetTimeInfo();
        m_inspector_game_res.m_player_hp = m_battle_admin->GetPlayerHP();
        m_inspector_game_res.m_enemy_hp = m_battle_admin->GetEnemyHP();
        m_inspector_game_res.m_skill_1_cool_down = m_battle_admin->GetSkillCoolDown(1);
        m_inspector_game_res.m_skill_2_cool_down = m_battle_admin->GetSkillCoolDown(2);
        m_inspector_game_res.m_skill_3_cool_down = m_battle_admin->GetSkillCoolDown(3);
    }

    InspectorGameComponent::~InspectorGameComponent() {}

} // namespace Piccolo