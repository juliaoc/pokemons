#include "window.hpp"

#include <glm/gtc/random.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

void Window::onEvent(SDL_Event const &event) {
  // Keyboard events
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) {
      // Mover o carro para a esquerda
      carro0.m_position.x -= 0.01f;
    }

    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) {
      // Mover o carro para a direita
      carro0.m_position.x += 0.01f;
    }
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

  m_model.loadObj(assetsPath + "box.obj");
  m_model.setupVAO(m_program);

  m_carro.loadObj(assetsPath + "modelo_carro.obj");
  m_carro.setupVAO(m_program);

  // Inicializa posição do carro na tela
  carro0.m_position = glm::vec3(0.0f, -0.02f, -0.001f);
  // carro1.m_position = glm::vec3(-0.05f, -0.02f, -0.005f);
  carro1.m_position = glm::vec3(glm::linearRand(-0.05f, 0.05f), -0.02f, -0.005f);


  // Camera at (0,0,0) and looking towards the negative z
  glm::vec3 const eye{0.0f, 0.0f, 0.0f};
  glm::vec3 const at{0.0f, 0.0f, -1.0f};
  glm::vec3 const up{0.0f, 1.0f, 0.0f};
  m_viewMatrix = glm::lookAt(eye, at, up);

  // Setup stars
  for (auto &star : m_stars) {
    randomizeStar(star);
  }
}

void Window::randomizeStar(Star &star) {
  // Random position: x and y in [-20, 20), z in [-100, 0)
  std::uniform_real_distribution<float> distPosY(-20.0f, 40.0f);  
  std::uniform_real_distribution<float> distPosZ(-100.0f, 0.0f);

  // Randomly select one of the two sides
  std::uniform_int_distribution<int> distPosSide(0, 1);
  int sorteia_lado = distPosSide(m_randomEngine);

  std::uniform_real_distribution<float> distPosX;
  
  if (sorteia_lado == 0) {
    distPosX = std::uniform_real_distribution<float>(25.0f, 40.0f);
  } else {
    distPosX = std::uniform_real_distribution<float>(-40.0f, -25.0f);
  } 

  star.m_position =
        glm::vec3(distPosX(m_randomEngine), distPosY(m_randomEngine),
                  distPosZ(m_randomEngine));

  // Random rotation axis
  star.m_rotationAxis = glm::sphericalRand(1.0f);
}

void Window::onUpdate() {
  // Increase angle by 90 degrees per second
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};
  m_angle = glm::wrapAngle(m_angle + glm::radians(90.0f) * deltaTime);

  // Update stars
  for (auto &star : m_stars) {
    // Increase z by 10 units per second
    star.m_position.z += deltaTime * 10.0f;

    // If this star is behind the camera, select a new random position &
    // orientation and move it back to -100
    if (star.m_position.z > 0.1f) {
      randomizeStar(star);
      star.m_position.z = -100.0f; // Back to -100
    }

    carro1.m_position.z += deltaTime * 0.001f * 0.5f;
    if (carro1.m_position.z > 0.01f) {
      randomizeStar(star);
      carro1.m_position.z = -4.0f; // Back to -100
    }
  }
}

void Window::onPaint() {
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  abcg::glUseProgram(m_program);

  // Get location of uniform variables
  auto const viewMatrixLoc{abcg::glGetUniformLocation(m_program, "viewMatrix")};
  auto const projMatrixLoc{abcg::glGetUniformLocation(m_program, "projMatrix")};
  auto const modelMatrixLoc{
      abcg::glGetUniformLocation(m_program, "modelMatrix")};
  auto const colorLoc{abcg::glGetUniformLocation(m_program, "color")};

  // Set uniform variables that have the same value for every model
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);
  abcg::glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // White

  // Render each star
  for (auto &star : m_stars) {
    // Compute model matrix of the current star
    glm::mat4 modelMatrix{1.0f};
    modelMatrix = glm::translate(modelMatrix, star.m_position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));
    modelMatrix = glm::rotate(modelMatrix, m_angle, star.m_rotationAxis);

    // Set uniform variable
    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);

    m_model.render();
  }

  // Desenha pista:
  pista1.m_position = glm::vec3(0.0f,-10.0f,-30.0f);
  pista1.m_rotationAxis = glm::vec3(0.2f,0.8f,0.3f);

  // Compute model matrix of the current star
  glm::mat4 matrizPista{1.0f};
  matrizPista = glm::translate(matrizPista, pista1.m_position);
  matrizPista = glm::scale(matrizPista, glm::vec3(60.0f,2.0f,240.0f));
  pista1.m_rotationAxis = glm::vec3(1.0f,0.0f,0.0f);
  angulo_pista = glm::radians(20.0f);
  matrizPista = glm::rotate(matrizPista, angulo_pista, pista1.m_rotationAxis);

  // Set uniform variable
  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &matrizPista[0][0]);

  m_model.render();

// =============================================================================

  // Desenha carro0:
  pista0.m_position = glm::vec3(0.0f,-0.0f,-90.0f);
  pista0.m_rotationAxis = glm::vec3(0.2f,0.8f,0.3f);

  glm::mat4 matrizCarro0{1.0f};
  matrizCarro0 = glm::translate(matrizCarro0, carro0.m_position);
  matrizCarro0 = glm::scale(matrizCarro0, glm::vec3(0.02f,0.02f,0.02f));

  pista0.m_rotationAxis = glm::vec3(1.0f,0.0f,0.0f);
  angulo_carro0 = glm::radians(0.0f);
  matrizCarro0 = glm::rotate(matrizCarro0, angulo_carro0, pista0.m_rotationAxis);
  // Set uniform variable
  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &matrizCarro0[0][0]);

  m_carro.render();

// =============================================================================

  // Desenha carro1:
  pista1.m_position = glm::vec3(0.0f,-0.0f,-90.0f);
  pista1.m_rotationAxis = glm::vec3(0.2f,0.8f,0.3f);

  glm::mat4 matrizCarro1{1.0f};
  matrizCarro1 = glm::translate(matrizCarro1, carro1.m_position);
  matrizCarro1 = glm::scale(matrizCarro1, glm::vec3(0.02f,0.02f,0.02f));

  pista1.m_rotationAxis = glm::vec3(1.0f,0.0f,0.0f);
  angulo_carro1 = glm::radians(20.0f);
  matrizCarro1 = glm::rotate(matrizCarro1, angulo_carro1, pista1.m_rotationAxis);

  // Set uniform variable
  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &matrizCarro1[0][0]);

  m_carro.render();

  abcg::glUseProgram(0);
}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();

  {
    auto const widgetSize{ImVec2(218, 62)};
    ImGui::SetNextWindowPos(ImVec2(m_viewportSize.x - widgetSize.x - 5, 5));
    ImGui::SetNextWindowSize(widgetSize);
    ImGui::Begin("Widget window", nullptr, ImGuiWindowFlags_NoDecoration);

    {
      ImGui::PushItemWidth(120);
      static std::size_t currentIndex{};
      std::vector<std::string> const comboItems{"Perspective", "Orthographic"};

      if (ImGui::BeginCombo("Projection",
                            comboItems.at(currentIndex).c_str())) {
        for (auto const index : iter::range(comboItems.size())) {
          auto const isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected))
            currentIndex = index;
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      ImGui::PushItemWidth(170);
      auto const aspect{gsl::narrow<float>(m_viewportSize.x) /
                        gsl::narrow<float>(m_viewportSize.y)};
      if (currentIndex == 0) {
        m_projMatrix =
            glm::perspective(glm::radians(m_FOV), aspect, 0.01f, 100.0f);

        ImGui::SliderFloat("FOV", &m_FOV, 5.0f, 179.0f, "%.0f degrees");
      } else {
        m_projMatrix = glm::ortho(-20.0f * aspect, 20.0f * aspect, -20.0f,
                                  20.0f, 0.01f, 100.0f);
      }
      ImGui::PopItemWidth();
    }

    ImGui::End();
  }
}

void Window::onResize(glm::ivec2 const &size) { m_viewportSize = size; }

void Window::onDestroy() {
  m_model.destroy();
  m_carro.destroy();
  abcg::glDeleteProgram(m_program);
}