#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <runtime/function/inspector_game/inspector_game_battle_admin.h>

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
        float         skill_damge {0.0f};
    };

    struct InspectorGameUnitData
    {
        int                    hp {100};
        InspectorGameTeam      team {InspectorGameTeam::InspectorGameTeam_None};
        std::vector<InspectorGameSkillData> skills;
    };

    class InspectorGameBattleAdmin
    {
    public:
        void Init();
        void UpdateBattle();
        std::string GetBattleInfo();
        std::string GetTimeInfo();
        void        SetPlayerInput(std::vector<bool> used_skill);

    private:
        InspectorGameBattleState       m_state        = InspectorGameBattleState::InspectorGameBattleState_None;
        InspectorGameBattleActionState m_action_state = InspectorGameBattleActionState::Settle;
        InspectorGameUnitData *m_player;
        InspectorGameUnitData *m_enemy;
        std::vector<InspectorGameUnitData> m_units;

        std::string m_battle_info {""};
        std::string m_time_info {"Wait for instructing µÈ´ýÖ¸Áî¡£¡£¡£"};

        const int m_animation_frames {180}; //default animation is 3 seconds
        int m_animation_remain_frames {60};

        void                    SettleBattle();
        void                    CheckInstruction();
        void                    UpdateAnimation();
        void                    OnTurnEnd();
        void                    UpdateColdDown();
        void                    OnEnemyAction();
        float                   OnUnitAttack(InspectorGameUnitData attacker, InspectorGameUnitData defender, InspectorGameSkillData skill);
        void                    OnUnitDie(InspectorGameUnitData dying_unit);

        //tool functions
        InspectorGameUnitData ChooseTarget(InspectorGameUnitData current_unit);
        bool                  CheckSkillIsAvaliable(InspectorGameSkillData skill);

    };
} // namespace Piccolo
