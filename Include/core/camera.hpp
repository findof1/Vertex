#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "math.h"

enum CameraMovement
{
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  UP,
  DOWN
};

class Camera
{
public:
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;

  float Yaw;
  float Pitch;

  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;

  Camera(const Camera &other)
  {
    Position = other.Position;
    Front = other.Front;
    Up = other.Up;
    Right = other.Right;
    WorldUp = other.WorldUp;

    Yaw = other.Yaw;
    Pitch = other.Pitch;

    MovementSpeed = other.MovementSpeed;
    MouseSensitivity = other.MouseSensitivity;
    Zoom = other.Zoom;

    // Update vectors just to be safe
    updateCameraVectors();
  }

  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
         float yaw = -90.0f,
         float pitch = 0.0f)
      : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
        MovementSpeed(2.5f),
        MouseSensitivity(0.1f),
        Zoom(75.0f)
  {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
  }

  glm::mat4 getViewMatrix() const
  {
    return glm::lookAt(Position, Position + Front, Up);
  }

  glm::mat4 getProjectionMatrix(float aspectRatio) const
  {
    return glm::perspective(glm::radians(Zoom), aspectRatio, 0.01f, 1000.0f);
  }

  void processKeyboard(CameraMovement direction, float deltaTime)
  {

    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
    {
      Position += Front * velocity;
    }
    if (direction == BACKWARD)
    {
      Position -= Front * velocity;
    }
    if (direction == LEFT)
    {
      Position -= Right * velocity;
    }
    if (direction == RIGHT)
    {
      Position += Right * velocity;
    }
    if (direction == UP)
    {
      Position += WorldUp * velocity;
    }
    if (direction == DOWN)
    {
      Position -= WorldUp * velocity;
    }
  }

  void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
  {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch)
    {
      if (Pitch > 89.0f)
        Pitch = 89.0f;
      if (Pitch < -89.0f)
        Pitch = -89.0f;
    }

    updateCameraVectors();
  }

  void invertPitch()
  {
    Pitch = -Pitch;
    updateCameraVectors();
  }

  void processMouseScroll(float yoffset)
  {
    Zoom -= yoffset;
    if (Zoom < 1.0f)
      Zoom = 1.0f;
    if (Zoom > 90.0f)
      Zoom = 90.0f;
  }

private:
  void updateCameraVectors()
  {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
  }
};

#endif
