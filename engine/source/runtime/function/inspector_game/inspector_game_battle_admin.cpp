#include "inspector_game_battle_admin.h"
#include <engine.cpp>

namespace Piccolo
{
    void InspectorGameBattleAdmin::Init()
    {
        m_player       = new InspectorGameUnitData();
        m_player->team = InspectorGameTeam::Player;

        m_player->skills.clear();
        m_player->skills.push_back(*new InspectorGameSkillData({"skill1", 0, 10.0f}));
        m_player->skills.push_back(*new InspectorGameSkillData({"skill2", 2, 20.0f}));
        m_player->skills.push_back(*new InspectorGameSkillData({"skill3", 3, 30.0f}));

        m_enemy       = new InspectorGameUnitData();
        m_enemy->team = InspectorGameTeam::Player;

        m_enemy->skills.clear();
        m_enemy->skills.push_back(*new InspectorGameSkillData({"skill1", 0, 10.0f}));
        m_enemy->skills.push_back(*new InspectorGameSkillData({"skill2", 2, 20.0f}));
        m_enemy->skills.push_back(*new InspectorGameSkillData({"skill3", 3, 30.0f}));

        m_units.clear();
        m_units.push_back(*m_player);
        m_units.push_back(*m_enemy);
    }

    void InspectorGameBattleAdmin::UpdateBattle()
    {
        switch (m_state)
        {
            case InspectorGameBattleState::InspectorGameBattleState_None:
                switch (m_action_state)
                {
                    case InspectorGameBattleActionState::Settle:
                        SettleBattle();
                        break;
                    case InspectorGameBattleActionState::PlayPlayerAnimation:
                    case InspectorGameBattleActionState::PlayEnemyAnimation:
                        UpdateAnimation();
                        break;
                    case InspectorGameBattleActionState::WaitForEnemyDecision:
                        OnEnemyAction();
                    case InspectorGameBattleActionState::TurnEnd:
                        OnTurnEnd();
                    case InspectorGameBattleActionState::WaitForInstruction:
                        CheckInstruction();
                        break;
                    default:
                        break;
                }
                break;
            case InspectorGameBattleState::Win:
                m_battle_info = "congratulations!!!";
                break;
            case InspectorGameBattleState::Lose:
                m_battle_info = "CAI";
                break;
        }
    }

    void InspectorGameBattleAdmin::SettleBattle()
    {
        m_battle_info = "Settling battle...";
        // Player is always attack first.
        m_action_state = InspectorGameBattleActionState::WaitForInstruction;
        --m_animation_remain_frames;
    }

    void InspectorGameBattleAdmin::CheckInstruction()
    {
        m_battle_info = "Plase cast a skill...";
        // Do nothing, just wait player to choose a skill.
        // Will check player input every frame through [SetPlayerInput] function.
        // Player input is detected by refection system.
    }

    void InspectorGameBattleAdmin::UpdateAnimation()
    {
        --m_animation_remain_frames;

        if (m_animation_remain_frames == 0)
        {
            m_action_state = m_action_state == InspectorGameBattleActionState::PlayPlayerAnimation ?
                                 InspectorGameBattleActionState::WaitForEnemyDecision :
                                 InspectorGameBattleActionState::TurnEnd;

            m_animation_remain_frames = m_animation_frames;
        }
        else
        {
            float reamin_animation_perentage = (float)m_animation_remain_frames / (float)m_animation_frames;
            reamin_animation_perentage *= 20.0f;

            std::string temp_str = "";
            for (int i = 0; i < reamin_animation_perentage; ++i)
            {
                temp_str.append("=");
            }

            m_time_info = temp_str;
        }
    }

    void InspectorGameBattleAdmin::OnTurnEnd()
    {
        if (m_state == InspectorGameBattleState::InspectorGameBattleState_None)
        {
            UpdateColdDown();
            m_action_state = InspectorGameBattleActionState::WaitForInstruction;
        }
        else
        {
            // show battle result...
        }
    }

    void InspectorGameBattleAdmin::UpdateColdDown()
    {
        for (InspectorGameUnitData unit : m_units)
        {
            for (InspectorGameSkillData skill : unit.skills)
            {
                if (skill.skill_cd > 0)
                {
                    --skill.skill_cd;
                }
                else
                {
                    skill.skill_cd = 0;
                }
            }
        }
    }

    void InspectorGameBattleAdmin::OnEnemyAction()
    {
        InspectorGameUnitData target = ChooseTarget(*m_enemy);
        float                 damage = OnUnitAttack(*m_enemy, target, m_enemy->skills[0]);

        //m_battle_info = ("Enemy attacks you, deal [{%f}] damge", damage);
        
        m_battle_info = "Enemy attacks you, deal [" + std::to_string(damage) + "] damge";
        m_battle_info = "Enemy attacking...";
        m_action_state = InspectorGameBattleActionState::PlayEnemyAnimation;
    }

    float InspectorGameBattleAdmin::OnUnitAttack(InspectorGameUnitData  attacker,
                                                 InspectorGameUnitData  defender,
                                                 InspectorGameSkillData skill)
    {
        float damage = skill.skill_damge;
        defender.hp -= damage;

        if (defender.hp <= 0)
        {
            defender.hp = 0;
            OnUnitDie(defender);
        }

        return damage;
    }

    void InspectorGameBattleAdmin::OnUnitDie(InspectorGameUnitData dying_unit)
    {
        //@TODO: should check all unit in battle, ensure is aced.
        if (dying_unit.team == InspectorGameTeam::Player)
        {
            m_state = InspectorGameBattleState::Lose;
        }
        else
        {
            m_state = InspectorGameBattleState::Win;
        }
    }

    // Public APIs
    std::string InspectorGameBattleAdmin::GetBattleInfo() { return m_battle_info; }

    std::string InspectorGameBattleAdmin::GetTimeInfo() { return m_time_info; }

    void InspectorGameBattleAdmin::SetPlayerInput(std::vector<bool> used_skill)
    {
        if (m_action_state == InspectorGameBattleActionState::WaitForInstruction)
        {
            for (int i = 0; i < used_skill.size(); ++i)
            {
                if (used_skill[i])
                {
                    float damage   = OnUnitAttack(*m_player, *m_enemy, m_player->skills[i]);
                    m_action_state = InspectorGameBattleActionState::PlayPlayerAnimation;

                    m_battle_info =
                        ("You use skill [%s] attack enemy, deal [{%f}] damge", m_player->skills[i].skill_name, damage);

                    m_battle_info = "you Attacking...";
                    break;
                }
            }
        }
    }

    // Too functions
    InspectorGameUnitData InspectorGameBattleAdmin::ChooseTarget(InspectorGameUnitData unit)
    {
        // is player
        if (unit.team == InspectorGameTeam::Player)
        {
            return *m_enemy;
        }
        // is enemy
        else if (unit.team == InspectorGameTeam::Player)
        {
            return *m_player;
        }
        else
        {
            LOG_ERROR("InspectorGameBattleAdmin::ChooseTarget ERROR! Plase check unit team data!");
            return *m_player;
        }
    }

    bool InspectorGameBattleAdmin::CheckSkillIsAvaliable(InspectorGameSkillData skill) { return skill.skill_cd == 0; }
} // namespace Piccolo