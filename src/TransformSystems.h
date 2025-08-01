#pragma once

#include <iostream>

#include "Phases.h"
#include "TransformComponents.h"
#include "MathUtils.h"

#include "flecs.h"


namespace res
{
    struct TransformSystems
    {
        explicit TransformSystems(flecs::world& world)
        {
            // ReSharper disable once CppExpressionWithoutSideEffects
            world.module<TransformSystems>();

            world.system<const cUpdatePositionFromMatrix, const cMatrix, cPosition>()
                 .kind(OnPostTick)
                 .each([](flecs::entity entity, const cUpdatePositionFromMatrix& updateComponent,
                          const cMatrix& matrixComponent, cPosition& positionComponent)
                 {
                     auto position = GetPosition(matrixComponent.matrix);
                     positionComponent.x = position.x;
                     positionComponent.y = position.y;
                     positionComponent.z = position.z;

                     entity.remove<cUpdatePositionFromMatrix>();

                     std::cout << "Updating Position..." << "\n";
                 });

            world.system<const cUpdateRotationFromMatrix, const cMatrix, cRotation>()
                 .kind(OnPostTick)
                 .each([](flecs::entity entity, const cUpdateRotationFromMatrix& updateComponent,
                          const cMatrix& matrixComponent, cRotation& rotationComponent)
                 {
                     auto quat = QuaternionFromMatrix(matrixComponent.matrix);

                     rotationComponent.x = quat.x;
                     rotationComponent.y = quat.y;
                     rotationComponent.z = quat.z;
                     rotationComponent.w = quat.w;

                     entity.remove<cUpdateRotationFromMatrix>();

                     std::cout << "Updating Rotation..." << "\n";
                 });
        }
    };
}
