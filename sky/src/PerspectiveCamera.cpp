#include "PerspectiveCamera.h"
#include <glm/gtc/matrix_transform.hpp>

#pragma optimize("", off)
PerspectiveCamera::PerspectiveCamera(float FOV, float Height, float Width)
	: m_ProjectionMatrix(glm::perspective(FOV ,  Width / Height , 0.1f , 1000.0f )), m_RotationX(0.0f), m_RotationY(0.0f), m_RotationZ(0.0f), m_Position(0.f, 0.f, 0.f)
{
	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

const glm::vec3 PerspectiveCamera::GetViewMatrixPosition() const
{
    // Extract the camera position by reversing the transformations
    glm::mat4 inverseViewMatrix = glm::inverse(m_ViewMatrix);
    
    glm::vec3 cameraPosition = glm::vec3(inverseViewMatrix[3]);

    // If you are seeing Z inversion and it's not intended, negate the Z value
    cameraPosition.z = -cameraPosition.z;  // Optional, depending on coordinate system
    
    return cameraPosition;  // Return by value, not reference
}

void PerspectiveCamera::RecalculateViewMatrix()
{
    glm::mat4 transform =  glm::translate(glm::mat4(1.0f), m_Position);
    transform =  glm::rotate(transform, m_RotationX, glm::vec3(1, 0, 0));
    transform =  glm::rotate(transform, m_RotationY, glm::vec3(0, 1, 0));
    transform =  glm::rotate(transform, m_RotationZ, glm::vec3(0, 0, 1));
    m_ViewMatrix = transform;
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void PerspectiveCamera::ExtractFrustumPlanes(Frustum& frustum) const {
    const glm::mat4& vp = m_ViewProjectionMatrix;

    // Left plane
    frustum.leftFace.normal = glm::vec3(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0]);
    frustum.leftFace.distance = vp[3][3] + vp[3][0];

    // Right plane
    frustum.rightFace.normal = glm::vec3(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0]);
    frustum.rightFace.distance = vp[3][3] - vp[3][0];

    // Bottom plane
    frustum.bottomFace.normal = glm::vec3(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1]);
    frustum.bottomFace.distance = vp[3][3] + vp[3][1];

    // Top plane
    frustum.topFace.normal = glm::vec3(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1]);
    frustum.topFace.distance = vp[3][3] - vp[3][1];

    // Near plane
    frustum.nearFace.normal = glm::vec3(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2]);
    frustum.nearFace.distance = vp[3][3] + vp[3][2];

    // Far plane
    frustum.farFace.normal = glm::vec3(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2]);
    frustum.farFace.distance = vp[3][3] - vp[3][2];

    // Normalize planes
    auto normalizePlane = [](FrustrumPlane& plane) {
        float length = glm::length(plane.normal);
        plane.normal /= length;
        plane.distance /= length;
    };

    normalizePlane(frustum.leftFace);
    normalizePlane(frustum.rightFace);
    normalizePlane(frustum.bottomFace);
    normalizePlane(frustum.topFace);
    normalizePlane(frustum.nearFace);
    normalizePlane(frustum.farFace);
}

struct AABB {
    glm::vec3 center;  // Center of the box
    glm::vec3 extents; // Half-size in each axis

    // Check if the AABB is on or in front of a frustum plane
    bool isOnOrForwardPlane(const FrustrumPlane& plane) const {
        // Compute the radius of the AABB projected onto the plane normal
        float projectedRadius = extents.x * std::abs(plane.normal.x) +
                                extents.y * std::abs(plane.normal.y) +
                                extents.z * std::abs(plane.normal.z);

        // Compute the signed distance from the center to the plane
        float distanceToPlane = glm::dot(plane.normal, center) + plane.distance;

        // If the distance + radius < 0, the box is completely outside the plane
        return distanceToPlane + projectedRadius >= 0;
    }
};

bool PerspectiveCamera::IsBoxInFrustum(const glm::vec3& localCenter, const glm::vec3& extents, const glm::mat4& modelMatrix) const {
    Frustum frustum;
    ExtractFrustumPlanes(frustum);

    // Transform the local center into world space
    glm::vec3 globalCenter = glm::vec3(modelMatrix * glm::vec4(localCenter, 1.0f));

    // Compute the scaled axes based on the model matrix
    glm::vec3 right = glm::vec3(modelMatrix[0]) * extents.x; // X-axis scaled
    glm::vec3 up = glm::vec3(modelMatrix[1]) * extents.y;    // Y-axis scaled
    glm::vec3 forward = glm::vec3(modelMatrix[2]) * extents.z; // Z-axis scaled

    // Compute the new half-extents by projecting each axis
    float newExtentsX = std::abs(glm::dot(glm::vec3(1, 0, 0), right)) +
                        std::abs(glm::dot(glm::vec3(1, 0, 0), up)) +
                        std::abs(glm::dot(glm::vec3(1, 0, 0), forward));

    float newExtentsY = std::abs(glm::dot(glm::vec3(0, 1, 0), right)) +
                        std::abs(glm::dot(glm::vec3(0, 1, 0), up)) +
                        std::abs(glm::dot(glm::vec3(0, 1, 0), forward));

    float newExtentsZ = std::abs(glm::dot(glm::vec3(0, 0, 1), right)) +
                        std::abs(glm::dot(glm::vec3(0, 0, 1), up)) +
                        std::abs(glm::dot(glm::vec3(0, 0, 1), forward));

    AABB globalAABB(globalCenter, glm::vec3(newExtentsX, newExtentsY, newExtentsZ));

    // Test the transformed AABB against the frustum planes
    return (globalAABB.isOnOrForwardPlane(frustum.leftFace) &&
            globalAABB.isOnOrForwardPlane(frustum.rightFace) &&
            globalAABB.isOnOrForwardPlane(frustum.bottomFace) &&
            globalAABB.isOnOrForwardPlane(frustum.topFace) &&
            globalAABB.isOnOrForwardPlane(frustum.nearFace) &&
            globalAABB.isOnOrForwardPlane(frustum.farFace));
}
