#include <Arduino.h>

class Button
{
  public:
    Button(int pin);

    bool exlusivePressed();
  private:
    int _pin;
    int _lastState;
};

// Constructor
Button::Button(int pin)
{
  _pin = pin;
  _lastState = HIGH;
  pinMode(_pin, INPUT_PULLUP);
}

bool Button::exlusivePressed() {
  int state = digitalRead(_pin);
  if (state != _lastState) {
    _lastState = state;
    return state == LOW;
  }
  return false;
}
