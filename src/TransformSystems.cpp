#include "TransformSystems.h"

res::TransformSystems::TransformSystems(flecs::world& world)
{
    world.module<TransformSystems>();

    auto onPostTickPhase = world.lookup(OnPostTickPhaseName.data());

    world.system<const cUpdatePositionFromMatrix, const cMatrix, cPosition>("Update Position From Matrix")
         .kind(onPostTickPhase)
         .each([](flecs::entity entity, const cUpdatePositionFromMatrix& updateComponent,
                  const cMatrix& matrixComponent, cPosition& positionComponent)
         {
             auto position = GetPositionFromMatrix(matrixComponent.matrix);
             positionComponent.x = position.x;
             positionComponent.y = position.y;
             positionComponent.z = position.z;

             entity.remove<cUpdatePositionFromMatrix>();

             std::cout << "Updating Position..." << "\n";
         });

    world.system<const cUpdateRotationFromMatrix, const cMatrix, cRotation>("Update Rotation From Matrix")
         .kind(onPostTickPhase)
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
