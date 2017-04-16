#include "uniforms.h"

const int MAX_SLABS = 10;

PassUniforms PassUniformData;
ModelUniforms ModelUniformData;

PassUniforms defaultPassUniforms{glm::mat4(1.0f), glm::vec4(0.4f), glm::vec4(0.7f), glm::vec4(0.7f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 10.0f, 0.0f, 1.0f), 0, 0.0f};
ModelUniforms defaultModelUniforms{ glm::mat4(1.0f), glm::mat4(1.0f), { glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f) }, glm::vec4(0.1f, 0.1f, 0.1f, 1.0f), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), glm::vec4(0.3f, 0.3f, 0.3f, 1.0f), 20.0f, 1, 0, 0, 0.0f, 0.0f, 0.0f };