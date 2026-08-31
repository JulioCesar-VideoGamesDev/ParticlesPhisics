#include "ParticleSystem.h"

#include "Random.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

ParticleSystem::ParticleSystem()
{
	m_tParticlePool.resize(1000);
}

void ParticleSystem::OnUpdate(GLCore::Timestep _fTimeStep)
{
	for (Particle& oParticle : m_tParticlePool)
	{
		if (!oParticle.bActive)
		{
			continue;
		}

		//@TODO: Physics simulation
		// Update de tamaño, color, lifetime, etc.
		
		// Check Collisions

		auto ApplyBounce = [&oParticle]()
			{
				glm::vec2 collisionNormal = glm::normalize(oParticle.oParticleProps.vCollisionNormal);

				oParticle.vVelocity -= (1.0f + oParticle.oParticleProps.fRestitution) *
					glm::dot(oParticle.vVelocity, collisionNormal) *
					collisionNormal;
			};

		// Check collisions in Y axis.
		if (oParticle.oParticleProps.vCollisionNormal.y > 0 && oParticle.vPosition.y < oParticle.oParticleProps.vSurfacePoint.y)
		{
			ApplyBounce();
		}
		else if (oParticle.oParticleProps.vCollisionNormal.y < 0 && oParticle.vPosition.y > oParticle.oParticleProps.vSurfacePoint.y)
		{
			ApplyBounce();
		}
		// Check collisions in X axis.
		if (oParticle.oParticleProps.vCollisionNormal.x > 0 && oParticle.vPosition.x < oParticle.oParticleProps.vSurfacePoint.x)
		{
			ApplyBounce();
		}
		else if (oParticle.oParticleProps.vCollisionNormal.x < 0 && oParticle.vPosition.x > oParticle.oParticleProps.vSurfacePoint.x)
		{
			ApplyBounce();
		}

		// Gravity
		oParticle.vVelocity.y += oParticle.oParticleProps.bSimulateGravity ? oParticle.oParticleProps.fGravityScalar * static_cast<float>(_fTimeStep) : 0;
		// External Forces

		glm::vec2 vResultingForce(0.0f);

		for (const glm::vec2& force : oParticle.oParticleProps.tExternalForces)
		{
			vResultingForce += force;
		}

		oParticle.vVelocity += vResultingForce * static_cast<float>(_fTimeStep);

		// Drag
		oParticle.vVelocity -= oParticle.vVelocity * oParticle.oParticleProps.fDrag * static_cast<float>(_fTimeStep);
		
		oParticle.vPosition += oParticle.vVelocity * static_cast<float>(_fTimeStep);

		oParticle.fLifeRemaining -= static_cast<float>(_fTimeStep);
		oParticle.bActive = oParticle.fLifeRemaining > 0.f;

		oParticle.fSpin += 0.01f * _fTimeStep;
	}
}

void ParticleSystem::OnRender(GLCore::Utils::OrthographicCamera& _oCamera)
{
	if (!m_iQuadVA)
	{
		float vertices[] = {
			 -0.5f, -0.5f, 0.0f,
			  0.5f, -0.5f, 0.0f,
			  0.5f,  0.5f, 0.0f,
			 -0.5f,  0.5f, 0.0f
		};

		glCreateVertexArrays(1, &m_iQuadVA);
		glBindVertexArray(m_iQuadVA);

		GLuint quadVB, quadIB;
		glCreateBuffers(1, &quadVB);
		glBindBuffer(GL_ARRAY_BUFFER, quadVB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glEnableVertexArrayAttrib(quadVB, 0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

		uint32_t indices[] = {
			0, 1, 2, 2, 3, 0
		};

		glCreateBuffers(1, &quadIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		m_pParticleShader = std::unique_ptr<GLCore::Utils::Shader>(GLCore::Utils::Shader::FromGLSLTextFiles("assets/shader.glsl.vert", "assets/shader.glsl.frag"));
		m_ParticleShaderViewProj = glGetUniformLocation(m_pParticleShader->GetRendererID(), "u_ViewProj");
		m_ParticleShaderTransform = glGetUniformLocation(m_pParticleShader->GetRendererID(), "u_Transform");
		m_ParticleShaderColor = glGetUniformLocation(m_pParticleShader->GetRendererID(), "u_Color");
	}

	glUseProgram(m_pParticleShader->GetRendererID());
	glUniformMatrix4fv(m_ParticleShaderViewProj, 1, GL_FALSE, glm::value_ptr(_oCamera.GetViewProjectionMatrix()));

	for (auto& particle : m_tParticlePool)
	{
		if (!particle.bActive)
			continue;

		// Fade away particles
		float life = particle.fLifeRemaining / particle.oParticleProps.fLifeTime;
		glm::vec4 color = glm::lerp(particle.oParticleProps.vColorEnd, particle.oParticleProps.vColorBegin, life);

		float size = glm::lerp(particle.oParticleProps.fSizeEnd, particle.oParticleProps.fSizeBegin + particle.vSizeVariation, life);
		
		// Render
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { particle.vPosition.x, particle.vPosition.y, 0.0f })
			* glm::rotate(glm::mat4(1.0f), particle.fSpin, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size, size, 1.0f });
		glUniformMatrix4fv(m_ParticleShaderTransform, 1, GL_FALSE, glm::value_ptr(transform));
		glUniform4fv(m_ParticleShaderColor, 1, glm::value_ptr(color));
		glBindVertexArray(m_iQuadVA);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}
}

void ParticleSystem::Emit(const ParticleProperties& _oParticleProps)
{
	Particle& oParticle = m_tParticlePool[m_uPoolIndex];

	// @TODO: Initialize particle
	oParticle.bActive = true;
	oParticle.fLifeRemaining = _oParticleProps.fLifeTime;
	oParticle.oParticleProps = _oParticleProps;

	// Random size
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_real_distribution<float> randomSize(
		_oParticleProps.vSizeVariation.x,
		_oParticleProps.vSizeVariation.y
	);
	oParticle.vSizeVariation = randomSize(gen);
	
	// Position
	oParticle.vPosition = _oParticleProps.vInitialPosition;

	// Random Velocity
	std::uniform_real_distribution<float> randomVelocityX(
		_oParticleProps.vVelocityVariation.x,
		_oParticleProps.vVelocityVariation.y
	);
	std::uniform_real_distribution<float> randomVelocityY(
		_oParticleProps.vVelocityVariation.z,
		_oParticleProps.vVelocityVariation.a
	);
	oParticle.vVelocityVariation = glm::vec2(randomVelocityX(gen), randomVelocityY(gen));
	oParticle.vVelocity = _oParticleProps.vInitialVelocity + oParticle.vVelocityVariation;

	// Spin
	oParticle.fSpin = Random::Float() * 2.0f * glm::pi<float>();

	m_uPoolIndex = --m_uPoolIndex % m_tParticlePool.size();
}
