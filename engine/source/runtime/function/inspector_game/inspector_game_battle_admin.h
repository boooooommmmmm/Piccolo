#pragma once

#include <cstdint>
#include <runtime/function/inspector_game/inspector_game_battle_admin.h>
#include <string>
#include <vector>

namespace Piccolo
{
    enum InspectorGameBattleState
    {
        InspectorGameBattleState_None, // not start
        Win,
        Lose // zero-sum game
    };

    enum InspectorGameBattleActionState : int
    {
        Settle,
        WaitForInstruction,
        PlayPlayerAnimation,
        WaitForEnemyDecision,
        PlayEnemyAnimation,
        TurnEnd,
        End
    };

    enum InspectorGameTeam : int
    {
        InspectorGameTeam_None, // not been settled,
        Player,
        Enemy
    };

    struct InspectorGameSkillData
    {
        std::string skill_name {"skill name"};
        int         skill_cd {0};
        int         skill_default_cd {0};
        float       skill_damge {0.0f};
    };

    struct InspectorGameUnitData
    {
        float                               hp {100.0f};
        InspectorGameTeam                   team {InspectorGameTeam::InspectorGameTeam_None};
        std::vector<InspectorGameSkillData> skills;
    };

    class InspectorGameBattleAdmin
    {
    public:
        void        Init();
        void        UpdateBattle();
        std::string GetBattleInfo() { return m_battle_info; }
        std::string GetTimeInfo() { return m_time_info; }
        float       GetPlayerHP() { return m_player->hp; }
        float       GetEnemyHP() { return m_enemy->hp; }
        int         GetSkillCoolDown(int skill_index) { return m_player->skills[skill_index - 1].skill_cd; }
        std::string GetSkillName(int skill_index) { return m_player->skills[skill_index - 1].skill_name; }

        void SetPlayerHP(float hp) { m_player->hp = hp; }
        void SetEnemyHP(float hp) { m_enemy->hp = hp; }
        void SetPlayerInput(std::vector<bool> used_skill);

    private:
        InspectorGameBattleState             m_state        = InspectorGameBattleState::InspectorGameBattleState_None;
        InspectorGameBattleActionState       m_action_state = InspectorGameBattleActionState::Settle;
        InspectorGameUnitData*               m_player;
        InspectorGameUnitData*               m_enemy;
        std::vector<InspectorGameUnitData*>  m_units;
        std::vector<InspectorGameSkillData*> m_current_action_skills;

        std::string m_battle_info {""};
        std::string m_time_info {"Wait for instructing µÈ´ýÖ¸Áî¡£¡£¡£"};

        const int m_animation_frames {600}; // default animation is 3 seconds
        int       m_animation_remain_frames {600};

        void SettleBattle();
        void CheckInstruction();
        void UpdateAnimation();
        void OnTurnEnd();
        void UpdateCoolDown();
        void OnEnemyAction();
        float
        OnUnitAttack(InspectorGameUnitData* attacker, InspectorGameUnitData* defender, InspectorGameSkillData* skill);
        void OnUnitDie(InspectorGameUnitData* dying_unit);

        // tool functions
        InspectorGameUnitData*  ChooseTarget(InspectorGameUnitData* current_unit);
        InspectorGameSkillData* ChooseSkill(InspectorGameUnitData* current_unit);
        InspectorGameSkillData* ChooseSkillByAI(InspectorGameUnitData* action_unit);
        std::vector<float>      GetSkillWeight(InspectorGameUnitData* action_unit);
        bool                    CheckSkillIsAvaliable(InspectorGameSkillData skill) { return skill.skill_cd == 0; }
    };
} // namespace Piccolo
