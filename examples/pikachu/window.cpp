#include "window.hpp"

#include <glm/gtc/random.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <fmt/core.h>
#include <imgui.h>
#include <cppitertools/itertools.hpp>
#include <glm/gtc/matrix_inverse.hpp>

void Window::onEvent(SDL_Event const &event) {
  glm::ivec2 mousePosition;
  SDL_GetMouseState(&mousePosition.x, &mousePosition.y);

  if (event.type == SDL_MOUSEMOTION) {
    m_trackBall.mouseMove(mousePosition);
  }
  if (event.type == SDL_MOUSEBUTTONDOWN &&
      event.button.button == SDL_BUTTON_LEFT) {
    m_trackBall.mousePress(mousePosition);
  }
  if (event.type == SDL_MOUSEBUTTONUP &&
      event.button.button == SDL_BUTTON_LEFT) {
    m_trackBall.mouseRelease(mousePosition);
  }
  if (event.type == SDL_MOUSEWHEEL) {
    m_zoom += (event.wheel.y > 0 ? -1.0f : 1.0f) / 5.0f;
    m_zoom = glm::clamp(m_zoom, -1.5f, 1.0f);
  }
}

void Window::onCreate() {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  abcg::glClearColor(0, 0, 0, 1);
  abcg::glEnable(GL_DEPTH_TEST);

  m_program =
      abcg::createOpenGLProgram({{.source = assetsPath + "depth.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "depth.frag",
                                  .stage = abcg::ShaderStage::Fragment}});


  loadModel(assetsPath + "pokemon_PIKACHU.obj");
  m_model.setupVAO(m_program); // manter ou não?

  m_trackBall.setAxis(glm::normalize(glm::vec3(-1, 0.1, 0.1)));
  m_trackBall.setVelocity(0.001f);

  for (const auto index : iter::range(m_numPokemon)) {
    auto &position{m_pokemonPositions.at(index)};
    auto &rotation{m_pokemonRotations.at(index)};

    randomizePokemon(position, rotation);
  }
}

void Window::loadModel(std::string_view path) {
  m_model.loadFromFile(path);
  m_model.setupVAO(m_program);
  m_trianglesToDraw = m_model.getNumTriangles();

}

void Window::randomizePokemon(glm::vec3 &position, glm::vec3 &rotation) {
  std::uniform_real_distribution<float> distPosXY(-20.0f, 20.0f);
  std::uniform_real_distribution<float> distPosZ(-100.0f, 0.0f);

  position = glm::vec3(distPosXY(m_randomEngine), distPosXY(m_randomEngine),
                       distPosZ(m_randomEngine));

  std::uniform_real_distribution<float> distRotAxis(-1.0f, 1.0f);

  rotation = glm::normalize(glm::vec3(distRotAxis(m_randomEngine),
                                      distRotAxis(m_randomEngine),
                                      distRotAxis(m_randomEngine)));
}

void Window::onUpdate() {

  m_modelMatrix = m_trackBall.getRotation();
  m_eyePosition = glm::vec3(0.0f, 0.0f, 2.0f + m_zoom);
  m_viewMatrix = glm::lookAt(m_eyePosition, glm::vec3(0.0f, 0.0f, 0.0f),
                             glm::vec3(0.0f, 1.0f, 0.0f));

  // Increase angle by 90 degrees per second
  float deltaTime{static_cast<float>(getDeltaTime())};
  m_angle = m_angle + glm::radians(90.0f) * deltaTime;

  // Update pokemons
  for (const auto index : iter::range(m_numPokemon)) {
    auto &position{m_pokemonPositions.at(index)};
    auto &rotation{m_pokemonRotations.at(index)};

    // Increase z by 10 units per second
    position.z += deltaTime * 10.0f;

    // If this star is behind the camera, select a new random position &
    // orientation and move it back to -100
    if (position.z > 0.1f) {
      randomizePokemon(position, rotation);
      position.z = -100.0f; // Back to -100
    }
  }
}

void Window::onPaint() {
  auto aspect{static_cast<float>(m_viewportWidth) /
              static_cast<float>(m_viewportHeight)};
  
  m_projMatrix = glm::perspective(glm::radians(m_FOV), aspect, 0.01f, 100.0f);
  glDisable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  m_model.setupVAO(m_program);

  onUpdate();

  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  abcg::glUseProgram(m_program);

  // Get location of uniform variables
  GLint viewMatrixLoc{abcg::glGetUniformLocation(m_program, "viewMatrix")};
  GLint projMatrixLoc{abcg::glGetUniformLocation(m_program, "projMatrix")};
  GLint modelMatrixLoc{
  abcg::glGetUniformLocation(m_program, "modelMatrix")};
  GLint colorLoc{abcg::glGetUniformLocation(m_program, "color")};

  // Set uniform variables that have the same value for every model
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);

    // Set uniform variables for the current model
  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);

  if (m_currentPokemonIndex == 0) {
    abcg::glUniform4f(colorLoc, 1.0f, 0.843137255f, 0.0f, 1.0f); // Yellow
  } 
  if (m_currentPokemonIndex == 1) {
    abcg::glUniform4f(colorLoc, 0.62745098f, 0.321568627f, 0.176470588f, 1.0f); // Brown
  } 
  if (m_currentPokemonIndex == 2) {
    abcg::glUniform4f(colorLoc, 0.541176471f, 0.168627451f, 0.88627451f, 1.0f); // Purple
  } 
  if (m_currentPokemonIndex == 3) {
    abcg::glUniform4f(colorLoc, 1.0f, 0.752941176f, 0.796078431f, 1.0f); // PINK
  } 

  m_model.render(m_trianglesToDraw);

  // Render each Pokemon
  for (const auto index : iter::range(m_numPokemon)) {

    auto &position{m_pokemonPositions.at(index)};
    auto &rotation{m_pokemonRotations.at(index)};

    // Compute model matrix of the current pokemon
    glm::mat4 modelMatrix{1.0f};
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));
    modelMatrix = glm::rotate(modelMatrix, m_angle, rotation);

    // Set uniform variable
    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);

    m_model.render();
  }

}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();

  {
    auto size{ImVec2(400, 85)};
    auto position{ImVec2((m_viewportWidth - size.x) / 2.0f,
                         (m_viewportHeight - size.y) / 2.0f - 150.0f)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                           ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoInputs};
    ImGui::Begin("Inicio", nullptr, flags);
    ImGui::End();

  }

  {
    auto const assetsPath{abcg::Application::getAssetsPath()};
    auto widgetSize{ImVec2(180, 40)};
    ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - widgetSize.x - 5, m_viewportHeight - widgetSize.y - 5));
    ImGui::SetNextWindowSize(widgetSize);
    auto flags{ImGuiWindowFlags_NoDecoration};
    ImGui::Begin("Widget window", nullptr, flags);

    static std::size_t currentIndex{};

    ImGui::PushItemWidth(120);
    if (ImGui::BeginCombo("Pokemon", m_pokemonNames.at(currentIndex))) {
      for (auto index : iter::range(m_pokemonNames.size())) {
        const bool isSelected{currentIndex == index};
        if (ImGui::Selectable(m_pokemonNames.at(index), isSelected))
          currentIndex = index;
        if (isSelected) ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    if (static_cast<int>(currentIndex) != m_currentPokemonIndex) {
      m_currentPokemonIndex = currentIndex;

      m_model.loadFromFile(fmt::format("{}pokemon_{}.obj", assetsPath, m_pokemonNames.at(m_currentPokemonIndex)));
      m_model.setupVAO(m_program);
      m_trianglesToDraw = m_model.getNumTriangles();
    }
    ImGui::End();
    
}
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportWidth = size.x;
  m_viewportHeight = size.y;

  m_trackBall.resizeViewport(size.x, size.y);
}

void Window::onDestroy() {
  abcg::glDeleteProgram(m_program);
}