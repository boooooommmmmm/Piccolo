#include "runtime/function/controller/character_controller.h"

#include "runtime/core/base/macro.h"

#include "runtime/function/framework/component/motor/motor_component.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/physics/physics_scene.h"

namespace Pilot
{
    CharacterController::CharacterController(const Capsule& capsule) : m_capsule(capsule)
    {
        m_rigidbody_shape                                    = RigidBodyShape();
        m_rigidbody_shape.m_geometry                         = PILOT_REFLECTION_NEW(Capsule);
        *static_cast<Capsule*>(m_rigidbody_shape.m_geometry) = m_capsule;

        m_rigidbody_shape.m_type = RigidBodyShapeType::capsule;

        Quaternion orientation;
        orientation.fromAngleAxis(Radian(Degree(90.f)), Vector3::UNIT_X);

        m_rigidbody_shape.m_local_transform =
            Transform(
                Vector3(0, 0, capsule.m_half_height + capsule.m_radius),
                orientation,
                Vector3::UNIT_SCALE);
    }

    Vector3 CharacterController::move(const Vector3& current_position, const Vector3& displacement)
    {
        std::shared_ptr<PhysicsScene> physics_scene =
            g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        ASSERT(physics_scene);

        std::vector<PhysicsHitInfo> hits;

        Transform world_transform = Transform(
            current_position + 0.1f * Vector3::UNIT_Z,
            Quaternion::IDENTITY,
            Vector3::UNIT_SCALE);

        Vector3 vertical_displacement   = displacement.z * Vector3::UNIT_Z;
        Vector3 horizontal_displacement = Vector3(displacement.x, displacement.y, 0.f);

        Vector3 vertical_direction   = vertical_displacement.normalisedCopy();
        Vector3 horizontal_direction = horizontal_displacement.normalisedCopy();

        Vector3 final_position = current_position;

        m_is_touch_ground = physics_scene->sweep(
            m_rigidbody_shape,
            world_transform.getMatrix(),
            Vector3::NEGATIVE_UNIT_Z,
            0.105f,
            hits);

        hits.clear();
        
        world_transform.m_position -= 0.1f * Vector3::UNIT_Z;

        // vertical pass
        if (physics_scene->sweep(
            m_rigidbody_shape,
            world_transform.getMatrix(),
            vertical_direction,
            vertical_displacement.length(),
            hits))
        {
            final_position += hits[0].hit_distance * vertical_direction;
        }
        else
        {
            final_position += vertical_displacement;
        }

        hits.clear();
                
        // side pass
        if (physics_scene->sweep(
                m_rigidbody_shape, world_transform.getMatrix(), horizontal_direction, horizontal_displacement.length(), hits))
        {
            //Check if can climb the stairs
            float max_hit_height = -999;
            Vector3 max_hit_position;
            for (int i = 0; i < hits.size(); i++)
            {
                float hit_height = hits[i].hit_position.z;
                if (hit_height > max_hit_height)
                {
                    max_hit_height = hit_height;
                    max_hit_position = hits[i].hit_position;
                }
            }            

            float delta_height = max_hit_height - world_transform.m_position.z;

            //climb the satirs
            if (delta_height > 0.000001f && delta_height < 0.3f) // stair is 0.25 height
            {
                m_is_touch_ground = true;

                float distance_from_hit_point = final_position.distance(max_hit_position);
                if (distance_from_hit_point > horizontal_displacement.length())
                {
                    Vector3 climb_stairs_direction = max_hit_position - world_transform.m_position;
                    final_position = world_transform.m_position + horizontal_direction * horizontal_displacement.length();
                    final_position.z = world_transform.m_position.z +
                                       climb_stairs_direction.length() /
                                        distance_from_hit_point *
                                        horizontal_displacement.length();
                }
                else
                {
                    final_position += horizontal_direction * horizontal_displacement.length();
                    final_position.z = max_hit_height;
                }
            }
            //if is not climbing stairs and hit collider
            else
            {
                Vector3 total_normal;
                for (int i = 0; i < hits.size(); i++)
                {
                    total_normal += hits[i].hit_normal.normalisedCopy();
                }
                total_normal.z = 0.0f;
                total_normal.normalise();
                horizontal_direction -= total_normal;

                final_position += horizontal_direction * horizontal_displacement.length();
            }
        }
        else
        {
            final_position += horizontal_displacement;
        }

        return final_position;
    }

} // namespace Pilot
