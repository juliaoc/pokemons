#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <random>

#include "abcgOpenGL.hpp"
#include "model.hpp"

class Window : public abcg::OpenGLWindow {
protected:
  void onEvent(SDL_Event const &event) override;
  void onCreate() override;
  void onUpdate() override;
  void onPaint() override;
  void onPaintUI() override;
  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;

private:
  std::default_random_engine m_randomEngine;

  glm::ivec2 m_viewportSize{};
  int m_trianglesToDraw{};

  Model m_model;
  Model m_carro;
  Model m_pista;

  struct Star {
    glm::vec3 m_position{};
    glm::vec3 m_rotationAxis{};
  };

  struct Carro {
    glm::vec3 m_position{};
    glm::vec3 m_rotationAxis{};
  };

  struct Floor {
    glm::vec3 m_position{};
    glm::vec3 m_rotationAxis{};
  };

  Floor pista0;
  Floor pista1;

  Carro carro0;
  Carro carro1;

  std::array<Star, 500> m_stars;

  float m_angle{};
  float angulo_carro0{};
  float angulo_carro1{};
  float angulo_pista{};


  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};
  float m_FOV{150.0f};

  GLuint m_program{};
  const std::vector<const char *> m_pokemonNames{"PIKACHU", "EEVEE", "EKANS", "JIGGLYPUFF"};
  int m_currentPokemonIndex{0};
  void randomizeStar(Star &star);
};

#endif