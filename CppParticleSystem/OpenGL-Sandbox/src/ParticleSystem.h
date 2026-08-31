#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

// Generic properties for all particles
struct ParticleProperties
{
	glm::vec2 vInitialPosition{0.f};
	
	glm::vec4 vColorBegin, vColorEnd;

	float fSizeBegin, fSizeEnd;
	glm::vec2 vSizeVariation; // x = min size, y = max size para el fSizeBegin
	float fLifeTime = 1.0f;
	
	glm::vec2 vInitialVelocity;
	glm::vec4 vVelocityVariation;
	bool bSimulateGravity;
	float fGravityScalar;
	float fDrag; // Se calcula con la velocidad en negativo por una constante que es esta variable

	bool bSimulateCollision;
	float fRestitution; // Cuanto rebota
	glm::vec2 vCollisionNormal;
	glm::vec2 vSurfacePoint;

	std::vector<glm::vec2> tExternalForces;
};

class ParticleSystem
{
public:
	ParticleSystem();

	void OnUpdate(GLCore::Timestep _fTimeStep);
	void OnRender(GLCore::Utils::OrthographicCamera& _oCamera);

	void Emit(const ParticleProperties& _oParticleProps); // Create a particle
private:
	struct Particle // Modifiable for the Update
	{
		glm::vec2 vPosition;
		glm::vec2 vVelocity;
		glm::vec2 vVelocityVariation;
		float fSpin = 0.0f;

		float fLifeRemaining = 0.0f;
		ParticleProperties oParticleProps;

		float vSizeVariation;

		bool bActive = false;
	};
	std::vector<Particle> m_tParticlePool;
	uint32_t m_uPoolIndex = 999;

	GLuint m_iQuadVA = 0;
	std::unique_ptr<GLCore::Utils::Shader> m_pParticleShader;
	GLint m_ParticleShaderViewProj, m_ParticleShaderTransform, m_ParticleShaderColor;
};