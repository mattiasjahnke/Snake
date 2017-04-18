

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Button.cpp"

#define SPRITE_WIDTH  8
#define SPRITE_HEIGHT 6

Adafruit_SSD1306 display(4);

Button rightButton(9);
Button leftButton(11);
Button downButton(8);
Button upButton(10);

struct Point {
  int8_t x;
  int8_t y;
};

bool operator==(const Point& lhs, const Point& rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

void operator+=(Point& lhs, const Point& rhs) {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
}

Point food;
Point body[128];
Point dir;
uint8_t snakeLength = 2;
bool acceptInput = true;

unsigned long previousMillis = 0;
unsigned long gameInterval = 250;

static const unsigned char PROGMEM headSprite[] = {
B00110000,
B00110000,
B11001111,
B11001111,
B11111111,
B11111111,
};

static const unsigned char PROGMEM bodySprite[] = {
B00011111,
B00011111,
B01111111,
B11111110,
B11111000,
B11111000,
};

static const unsigned char PROGMEM foodSprite[] = {
B00011000,
B00011000,
B01100110,
B01100110,
B00011000,
B00011000,
};

#if (SSD1306_LCDHEIGHT != 64)
#error(“Height incorrect, please fix Adafruit_SSD1306.h!“);
#endif

void restartGame();
void spawnFood();
void handlePlayerInput();
void updateSnake();
void displayMenu();
void displayGameOver();
void drawBoundary();
void drawSnake();
void drawFood();
void drawSprite(int16_t x, int16_t y, const uint8_t *bitmap, uint16_t color);

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  randomSeed(analogRead(0));

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.display();

  displayMenu();
}

void loop() {

  handlePlayerInput();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= gameInterval) {
    previousMillis = currentMillis;
    acceptInput = true;

    updateSnake();

    if (body[0] == food) {
      snakeLength++;
      spawnFood();
    }

    // Look for deadly collision
    for (uint8_t i = 1; i < snakeLength; i++) {
      if (body[0] == body[i]) {
        displayGameOver();
        displayMenu();
      }
    }

    display.clearDisplay();

    drawBoundary();
    drawSnake();
    drawFood();

    display.display();
  }
}

void handlePlayerInput() {
  if (acceptInput) {
    acceptInput = false;
    if (leftButton.exlusivePressed() && dir.x == 0) {
      dir = {-1, 0};
    } else if (rightButton.exlusivePressed() && dir.x == 0) {
      dir = {1, 0};
    } else if (upButton.exlusivePressed() && dir.y == 0) {
      dir = {0, -1};
    } else if (downButton.exlusivePressed() && dir.y == 0) {
      dir = {0, 1};
    } else {
      acceptInput = true;
    }
  }
}

// ===== GAME LOGIC =====

void updateSnake() {
  Point head = body[0];
  head += dir;
  if (head.y < 0) { head.y = 7; }
  else if (head.y > 7) { head.y = 0; }
  else if (head.x < 0) { head.x = 15; }
  else if (head.x > 15) { head.x = 0; }
  for (uint8_t i = snakeLength - 1; i > 0; i--) {
    body[i] = body[i - 1];
  }
  body[0] = head;
}

void restartGame() {
  dir = {1, 0};
  body[0] = {5, 4};
  snakeLength = 2;
  spawnFood();
}

void spawnFood() {
  Point available[128];
  uint8_t nrAvailable = 0;
  for (uint8_t i = 0; i < 128; i++) {
    Point p = {i % 16, floor(i / 16)};
    bool isAvailable = true;
    for (uint8_t i = 0; i < snakeLength; i++) {
      if (body[i] == p) {
        isAvailable = false;
        break;
      }
    }
    if (isAvailable) {
      available[nrAvailable++] = p;
    }
  }

  if (nrAvailable > 0) {
    food = available[random(nrAvailable)];
  }
}

// ==== DRAWING FUNCTIONS =====

void drawBoundary() {
  display.drawRect(0, 0, SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT, WHITE);
}

void drawSnake() {
  for (uint8_t i = 0; i < snakeLength; i++) {
    drawSprite(body[i].x * 8,
               body[i].y * 8,
               i == 0 ? headSprite : bodySprite,
               WHITE);
  }
}

void drawFood() {
  drawSprite(food.x * 8, food.y * 8, foodSprite, WHITE);
}

void drawSprite(int16_t x, int16_t y, const uint8_t *bitmap, uint16_t color) {
  display.drawBitmap(x, y, bitmap, SPRITE_WIDTH, SPRITE_HEIGHT, color);
}

// ==== SCENES ====

void displayMenu() {
  display.clearDisplay();

  display.setCursor(20, 8);
  display.print("SNAKE");
  drawSprite(56, 8, headSprite, WHITE);
  display.setCursor(20, 18);
  display.print("By @engineerish");

  display.setCursor(32, 30);
  display.print("Easy");
  display.setCursor(32, 40);
  display.print("Medium");
  display.setCursor(32, 50);
  display.print("Hard");
  display.display();

  bool chosen = false;
  int8_t difficulty = 0;

  while (!chosen) {
    delay(20);

    drawSprite(20, 30 + difficulty * 10, headSprite, BLACK);

    if (upButton.exlusivePressed()) {
      if (--difficulty < 0) {
        difficulty = 2;
      }
    } else if (downButton.exlusivePressed()) {
      if (++difficulty >= 3) {
        difficulty = 0;
      }
    } else if (rightButton.exlusivePressed()) {
      chosen = true;
    }

    drawSprite(20, 30 + difficulty * 10, headSprite, WHITE);

    display.display();
  }

  display.clearDisplay();
  display.setCursor(52, 22);

  if (difficulty == 0) {
    gameInterval = 250;
    display.print("Easy");
  } else if (difficulty == 1) {
    gameInterval = 175;
    display.print("Medium");
  } else if (difficulty == 2) {
    gameInterval = 100;
    display.print("Hard");
  }

  display.setCursor(32, 36);
  display.print("Get Ready...");

  display.display();

  delay(3000);

  restartGame();
}

void displayGameOver() {
  display.clearDisplay();
  display.setCursor(32, 26);
  display.print("Game over!");
  display.setCursor(32, 36);
  display.print("Score: ");
  display.print(snakeLength);
  display.display();
  delay(5000);
}
