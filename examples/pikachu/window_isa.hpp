#ifndef WINDOW_HPP_
#define WINDOW_HPP_
#include <imgui.h>

#include <random>

#include "abcgOpenGL.hpp"
#include "model.hpp"
// #include "trackball.hpp"

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
  // static const int m_numPokemon{500};
  // int m_viewportWidth{};
  // int m_viewportHeight{};

  glm::ivec2 m_viewportSize{};

  Model m_model;
  Model m_pokemon;
  Model m_pista;
  int m_trianglesToDraw{};

  struct Pokemon {
    glm::vec3 m_position{};
    glm::vec3 m_rotationAxis{};
  };
  struct MiniPokemon {
    glm::vec3 m_position{};
    glm::vec3 m_rotationAxis{};
  };

  struct Floor {
    glm::vec3 m_position{};
    glm::vec3 m_rotationAxis{};
  };

  Floor pista0;
  Floor pista1;
  
  Pokemon pokemon_princ;


  std::array <MiniPokemon, 500> m_Pokemon;


  // TrackBall m_trackBall;
  float m_angle{};
  float angulo_princ{};

  // glm::vec3 m_eyePosition{};
  // glm::mat4 m_modelMatrix{1.0f};
  float angulo_pista{};
  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};

  std::default_random_engine m_randomEngine;
  //float m_FOV{30.0f};
  float m_FOV{150.0f};


  // std::array<glm::vec3, m_numPokemon> m_pokemonPositions;
  // std::array<glm::vec3, m_numPokemon> m_pokemonRotations;

  void randomizePokemon(MiniPokemon &pokemon);

  GLuint m_program{};
  const std::vector<const char *> m_pokemonNames{"PIKACHU", "EEVEE", "EKANS", "JIGGLYPUFF"};
  int m_currentPokemonIndex{0};

  // void loadModel(std::string_view path);

};

#endif