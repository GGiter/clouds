#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

struct FrustrumPlane
{
    // unit vector
    glm::vec3 normal = { 0.f, 1.f, 0.f };

    // distance from origin to the nearest point in the plane
    float     distance = 0.f;             
};

struct Frustum
{
    FrustrumPlane topFace;
    FrustrumPlane bottomFace;

    FrustrumPlane rightFace;
    FrustrumPlane leftFace;

    FrustrumPlane farFace;
    FrustrumPlane nearFace;
};

class PerspectiveCamera
{
public:
	PerspectiveCamera(float FOV, float Height, float Width);

	const glm::vec3& GetPosition() const { return m_Position; }
	const glm::vec3 GetViewMatrixPosition() const;
	void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix();; }

	float GetRotationX() const { return m_RotationX; }
	void SetRotationX(float rotation) { m_RotationX = rotation; RecalculateViewMatrix(); }

	float GetRotationY() const { return m_RotationY; }
    void SetRotationY(float rotation) { m_RotationY = rotation; RecalculateViewMatrix(); }

    float GetRotationZ() const { return m_RotationZ; }
    void SetRotationZ(float rotation) { m_RotationZ = rotation; RecalculateViewMatrix(); }

	const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
	const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
	const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

	void SetViewMatrix(const glm::mat4& viewMatrix) { m_ViewMatrix = viewMatrix; m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix; }
	bool IsBoxInFrustum(const glm::vec3 &localCenter, const glm::vec3 &extents,
                      const glm::mat4 &modelMatrix) const;
private:
    void RecalculateViewMatrix();
	void ExtractFrustumPlanes(Frustum &frustum) const;


	glm::mat4 m_ProjectionMatrix;
	glm::mat4 m_ViewMatrix;
	glm::mat4 m_ViewProjectionMatrix;

	glm::vec3 m_Position;
    float m_RotationX, m_RotationY, m_RotationZ;
};