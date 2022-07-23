#include "inspector_game_battle_admin.h"
#include <engine.cpp>

namespace Piccolo
{
    void InspectorGameBattleAdmin::Init()
    {
        m_player       = new InspectorGameUnitData();
        m_player->team = InspectorGameTeam::Player;

        m_player->skills.clear();
        m_player->skills.push_back(*new InspectorGameSkillData({"Normal Attack!", 0, 0, 10.0f}));
        m_player->skills.push_back(*new InspectorGameSkillData({"Black bear attack heart!!", 2, 2, 20.0f}));
        m_player->skills.push_back(*new InspectorGameSkillData({"Lightning five whip!!!", 3, 3, 30.0f}));

        m_enemy       = new InspectorGameUnitData();
        m_enemy->team = InspectorGameTeam::Enemy;

        m_enemy->skills.clear();
        m_enemy->skills.push_back(*new InspectorGameSkillData({"Normal Attack!", 0, 0, 10.0f}));
        m_enemy->skills.push_back(*new InspectorGameSkillData({"Angry Crow Takes Flight!!", 2, 2, 15.0f}));
        m_enemy->skills.push_back(*new InspectorGameSkillData({"Tornado Decimates Trailer Park!!!", 3, 3, 20.0f}));

        m_units.clear();
        m_units.push_back(m_player);
        m_units.push_back(m_enemy);
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
                        break;
                    case InspectorGameBattleActionState::TurnEnd:
                        OnTurnEnd();
                        break;
                    case InspectorGameBattleActionState::WaitForInstruction:
                        CheckInstruction();
                        break;
                    default:
                        break;
                }
                break;
            case InspectorGameBattleState::Win:
                m_battle_info =
                    "Congratulations!!! You win!!!  ---I am looking for a job to feed myself. Please contact me...";
                break;
            case InspectorGameBattleState::Lose:
                m_battle_info = "!!!CAI!!!";
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
            UpdateCoolDown();
            m_action_state = InspectorGameBattleActionState::WaitForInstruction;
        }
        else
        {
            // show battle result...
        }
    }

    void InspectorGameBattleAdmin::UpdateCoolDown()
    {
        for (InspectorGameUnitData* unit : m_units)
        {
            for (InspectorGameSkillData& skill : unit->skills)
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

        // update current turn action skill cd
        // TODO: some skills may action multiple times in one turn, should use other method to update skill cd.
        for (InspectorGameSkillData* skill : m_current_action_skills)
        {
            skill->skill_cd = skill->skill_default_cd;
        }
        m_current_action_skills.clear();
    }

    void InspectorGameBattleAdmin::OnEnemyAction()
    {
        InspectorGameUnitData*  target = ChooseTarget(m_enemy);
        InspectorGameSkillData* skill  = ChooseSkill(m_enemy);
        float                   damage = OnUnitAttack(m_enemy, target, skill);

        m_battle_info =
            "Enemy use skill [" + skill->skill_name + "] attack you, deal [" + std::to_string(damage) + "] damge";
        m_action_state = InspectorGameBattleActionState::PlayEnemyAnimation;
    }

    float InspectorGameBattleAdmin::OnUnitAttack(InspectorGameUnitData*  attacker,
                                                 InspectorGameUnitData*  defender,
                                                 InspectorGameSkillData* skill)
    {
        float damage = skill->skill_damge;
        defender->hp -= damage;

        m_current_action_skills.push_back(skill);

        if (defender->hp <= 0)
        {
            defender->hp = 0;
            OnUnitDie(defender);
        }

        return damage;
    }

    void InspectorGameBattleAdmin::OnUnitDie(InspectorGameUnitData* dying_unit)
    {
        //@TODO: should check all unit in battle, ensure is aced.
        if (dying_unit->team == InspectorGameTeam::Player)
        {
            m_state = InspectorGameBattleState::Lose;
        }
        else
        {
            m_state = InspectorGameBattleState::Win;
        }
    }

    // Public APIs
    void InspectorGameBattleAdmin::SetPlayerInput(std::vector<bool> used_skill)
    {
        if (m_action_state == InspectorGameBattleActionState::WaitForInstruction)
        {
            for (int i = 0; i < used_skill.size(); ++i)
            {
                if (used_skill[i])
                {
                    InspectorGameSkillData* skill = &m_player->skills[i];
                    if (CheckSkillIsAvaliable(*skill))
                    {
                        float damage   = OnUnitAttack(m_player, m_enemy, skill);
                        m_action_state = InspectorGameBattleActionState::PlayPlayerAnimation;

                        m_battle_info = "You use skill [" + skill->skill_name + "] attack enemy, deal [" +
                                        std::to_string(damage) + "] damge";
                        break;
                    }
                    else
                    {
                        m_time_info = "This skill is cooling down...";
                    }
                }
            }
        }
    }

    // Tool functions
    // TODO: this function need to be expand to support target choose logic
    InspectorGameUnitData* InspectorGameBattleAdmin::ChooseTarget(InspectorGameUnitData* action_unit)
    {
        // is player
        if (action_unit->team == InspectorGameTeam::Player)
        {
            return m_enemy;
        }
        // is enemy
        else if (action_unit->team == InspectorGameTeam::Enemy)
        {
            return m_player;
        }
        else
        {
            LOG_ERROR("InspectorGameBattleAdmin::ChooseTarget ERROR! Plase check unit team data!");
            return m_player;
        }
    }

    InspectorGameSkillData* InspectorGameBattleAdmin::ChooseSkill(InspectorGameUnitData* action_unit)
    {
        if (action_unit->team == InspectorGameTeam::Player)
        {
            return ChooseSkillByAI(action_unit); // TODO: player has its own auto attack logic
        }
        // is enemy
        else if (action_unit->team == InspectorGameTeam::Enemy)
        {
            return ChooseSkillByAI(action_unit);
        }
        else
        {
            LOG_ERROR("InspectorGameBattleAdmin::ChooseTarget ERROR! Plase check unit team data!");
            return nullptr;
        }
    }

    InspectorGameSkillData* InspectorGameBattleAdmin::ChooseSkillByAI(InspectorGameUnitData* action_unit)
    {
        std::vector<float> skill_weights             = GetSkillWeight(action_unit);
        int                most_valuable_skill_index = 0; // default skill / normal attack
        int                largest_skill_wegith      = 0; // default skill / normal attack

        for (int i = 0; i < action_unit->skills.size(); i++)
        {
            InspectorGameSkillData skill = action_unit->skills[i];
            if (skill.skill_cd == 0)
            {
                float _current_skill_weight = skill_weights[i];
                if (largest_skill_wegith < _current_skill_weight) // TODO: each AI has its choose skill logic
                {
                    largest_skill_wegith      = _current_skill_weight;
                    most_valuable_skill_index = i;
                }
            }
        }

        return &action_unit->skills[most_valuable_skill_index];
    }

    std::vector<float> InspectorGameBattleAdmin::GetSkillWeight(InspectorGameUnitData* action_unit)
    {
        std::vector<float> skill_weights;
        for (InspectorGameSkillData& skill : action_unit->skills)
        {
            skill_weights.push_back(skill.skill_damge);
        }

        return skill_weights;
    }

} // namespace Piccolo