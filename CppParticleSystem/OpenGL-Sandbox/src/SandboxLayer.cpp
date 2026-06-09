#include "SandboxLayer.h"

using namespace GLCore;
using namespace GLCore::Utils;

SandboxLayer::SandboxLayer()
	: m_CameraController(16.0f / 9.0f)
{
}

SandboxLayer::~SandboxLayer()
{
}

void SandboxLayer::OnAttach()
{
	EnableGLDebugging();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//@TODO: Init particle props here
	m_oParticleProps.vColorBegin = { 61 / 255.0f, 158 / 255.0f, 219 / 255.0f, 1.0f };
	m_oParticleProps.vColorEnd = { 197 / 255.0f, 219 / 255.0f, 233 / 255.0f, 1.0f };
	m_oParticleProps.fSizeBegin = 0.25f, m_oParticleProps.fSizeEnd = 0.0f;
	m_oParticleProps.vSizeVariation = { 0.f, 0.8f };
	m_oParticleProps.fLifeTime = { 1.f };
	m_oParticleProps.vInitialVelocity = { 0.f, 0.8f };
}

void SandboxLayer::OnDetach()
{
	// Shutdown
}

void SandboxLayer::OnEvent(Event& event)
{
	// Events
	m_CameraController.OnEvent(event);

	if (event.GetEventType() == EventType::WindowResize)
	{
		WindowResizeEvent& e = (WindowResizeEvent&)event;
		glViewport(0, 0, e.GetWidth(), e.GetHeight());
	}
}

void SandboxLayer::OnUpdate(Timestep _fTimeStep)
{
	m_CameraController.OnUpdate(_fTimeStep);

	// Render

	glClearColor(0,0,0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//--- Get emitter position based on camera zoom ---
	float x, y = 0.f;
	auto width = GLCore::Application::Get().GetWindow().GetWidth();
	auto height = GLCore::Application::Get().GetWindow().GetHeight();

	auto bounds = m_CameraController.GetBounds();
	x = 0.5 * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
	y = bounds.GetHeight() * 0.5f - 0.5f * bounds.GetHeight();
	//------------------------------------------------------

	//@TODO: Particle system calls
	m_oParticleProps.vInitialPosition = { x, y };
	m_ParticleSystem.Emit(m_oParticleProps);
	m_ParticleSystem.OnUpdate(_fTimeStep);
	m_ParticleSystem.OnRender(m_CameraController.GetCamera());
}

void SandboxLayer::OnImGuiRender()
{
	// @TODO: Full ImGui editor menu
	ImGui::Begin("Settings");

	ImGui::ColorEdit4("Birth Color", glm::value_ptr(m_oParticleProps.vColorBegin));
	ImGui::ColorEdit4("Death Color", glm::value_ptr(m_oParticleProps.vColorEnd));
	ImGui::Separator();
	ImGui::InputFloat("Initial Size", &m_oParticleProps.fSizeBegin);
	ImGui::InputFloat("Final Size", &m_oParticleProps.fSizeEnd);
	ImGui::InputFloat2("Size Variation", glm::value_ptr(m_oParticleProps.vSizeVariation));
	ImGui::InputFloat("Life Time", &m_oParticleProps.fLifeTime);
	ImGui::Separator();
	ImGui::InputFloat2("Initial Velocity", glm::value_ptr(m_oParticleProps.vInitialVelocity));
	ImGui::InputFloat4("Velocity Variation", glm::value_ptr(m_oParticleProps.vvelocity));

	ImGui::End();
}
