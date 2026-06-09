#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>

namespace TextRenderer {

bool init();
void shutdown();

glm::ivec2 measureText(const std::string& utf8Text, int fontSize, int wrapWidth = 0);

void drawText(const glm::mat4& viewProj,
              float x, float y,
              const std::string& utf8Text,
              int fontSize,
              const glm::vec3& color,
              float alpha = 1.0f,
              int wrapWidth = 0);

void drawTextTopLeft(const glm::mat4& viewProj,
                     float x, float topY,
                     const std::string& utf8Text,
                     int fontSize,
                     const glm::vec3& color,
                     float alpha = 1.0f,
                     int wrapWidth = 0);

void drawTextCentered(const glm::mat4& viewProj,
                      float centerX, float centerY,
                      const std::string& utf8Text,
                      int fontSize,
                      const glm::vec3& color,
                      float alpha = 1.0f,
                      int wrapWidth = 0);

}  // namespace TextRenderer
