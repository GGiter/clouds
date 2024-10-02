#include "TracingSystem.h"

bool TracingSystem::DoesAABBIntersect(AABBCollision* A, AABBCollision* B)
{
    if(A->GetPosition().x < B->GetPosition().x + B->GetExtent().x && 
       A->GetPosition().x + A->GetExtent().x > B->GetPosition().x &&
       A->GetPosition().y < B->GetPosition().y + B->GetExtent().y && 
       A->GetPosition().y + A->GetExtent().y > B->GetPosition().y && 
       A->GetPosition().z < B->GetPosition().z + B->GetExtent().z && 
       A->GetPosition().z + A->GetExtent().z > B->GetPosition().z)
    {
       return true;
    }

    return false;
}

bool TracingSystem::DoesAABBOverlap(AABBCollision* A, AABBCollision* B)
{
    if(A->GetPosition().x <= B->GetPosition().x + B->GetExtent().x &&
       A->GetPosition().y <= B->GetPosition().y + B->GetExtent().y &&
       A->GetPosition().z <= B->GetPosition().z + B->GetExtent().z)
    {
       return true;
    }

    return false;
}

glm::vec3 TracingSystem::GetIntersectionPoint(AABBCollision* A, AABBCollision* B)
{
    return glm::vec3();
}
