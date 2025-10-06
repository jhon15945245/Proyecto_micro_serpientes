#include <Arduino.h>  
#include <LiquidCrystal.h>


LiquidCrystal lcd(8, 9, 4, 5, 6, 7);


#define BTN_UP     24
#define BTN_DOWN   25
#define BTN_LEFT   22
#define BTN_RIGHT  23
#define BTN_START  26


int buzzerPin = 3;
int buzzerPin2 = 10; 


bool gameStarted = false;
bool gameOver = false;
bool gamePaused = false;
int score = 0;
int highScore = 0;
int speedLevel = 1;


int totalSpecialEaten = 0;
int totalTrapEaten = 0;


enum GameMode { EASY, NORMAL, HARD };
GameMode currentMode = NORMAL;


const int GRID_WIDTH = 16;
const int GRID_HEIGHT = 2;
const int MAX_SNAKE_LENGTH = 64;

struct SnakeSegment {
  int x;
  int y;
};

SnakeSegment snake[MAX_SNAKE_LENGTH];
int snakeLength = 3;
int direction = 0;
int newDirection = 0;


SnakeSegment food;
bool foodEaten = true;


SnakeSegment specialFood;
bool specialFoodActive = false;
int specialFoodTimer = 0;   
const int SPECIAL_FOOD_DURATION = 10; 


SnakeSegment trapFood;
bool trapFoodActive = false;
int trapFoodTimer = 0;
const int TRAP_FOOD_DURATION = 12; 


unsigned long lastMoveTime = 0;
int moveDelay = 400;


byte dot[8] = {
  B00000,
  B00000,
  B00000,
  B01110,
  B01110,
  B00000,
  B00000,
  B00000
};

byte foodDot[8] = {
  B00000,
  B00000,
  B00100,
  B01110,
  B01110,
  B00100,
  B00000,
  B00000
};


byte headUp[8] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00000,
  B00000
};
byte headDown[8] = {
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};
byte headLeft[8] = {
  B00100,
  B01100,
  B11100,
  B11111,
  B11100,
  B01100,
  B00100,
  B00000
};
byte headRight[8] = {
  B00100,
  B00110,
  B00111,
  B11111,
  B00111,
  B00110,
  B00100,
  B00000
};


byte specialFoodDot[8] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};


byte trapFoodDot[8] = {
  B00000,
  B01010,
  B00100,
  B11111,
  B00100,
  B01010,
  B00000,
  B00000
};


void soundMove() {
  tone(buzzerPin, 2000, 30);
}
void soundEat() {
  tone(buzzerPin, 800, 100);
  delay(120);
  tone(buzzerPin, 1200, 100);
}
void soundEatSpecial() {
  tone(buzzerPin, 1500, 80);  
  delay(90);
  tone(buzzerPin, 1800, 80);  
}
void soundCrash() {
  tone(buzzerPin, 400, 200);
  delay(250);
  tone(buzzerPin, 200, 400);
  delay(400);
}
void soundStartMelody() {
  tone(buzzerPin, 1000, 150); delay(200);
  tone(buzzerPin, 1500, 150); delay(200);
  tone(buzzerPin, 2000, 300); delay(300);
}
void soundGameOverMelody() {
  tone(buzzerPin, 600, 200); delay(250);
  tone(buzzerPin, 400, 250); delay(250);
  tone(buzzerPin, 300, 400); delay(400);
}


int readButtons();
void setupGraphics();
void generateFood();
void generateSpecialFood();
void generateTrapFood();
void initSnake();
void drawGame();
void moveSnake();
bool checkCollisions();
void handleControls();
void showStartScreen();
void showGameOver();
void resetGame();


void animateSnakeDestruction() {
  int notas[] = {1200, 1400, 1600, 1800, 2000}; 

  for (int i = snakeLength - 1; i >= 0; i--) {
    lcd.setCursor(snake[i].x, snake[i].y);
    lcd.print(" "); 

    int nota = notas[i % 5];
    tone(buzzerPin, nota, 100);
    delay(120); 
  }
}


void showIntroAnimation() {
  String mensaje = "BIENVENIDO A SNAKE GAME  ";
  lcd.clear();
  for (int i = 0; i < mensaje.length() - 15; i++) {
    lcd.setCursor(0,0);
    lcd.print(mensaje.substring(i, i + 16));
    delay(200);
  }
}


void showStats() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Especiales:");
  lcd.print(totalSpecialEaten);
  lcd.setCursor(0,1);
  lcd.print("Venenosas:");
  lcd.print(totalTrapEaten);
  delay(3000);
}

-
int highScoreEASY = 0;
int highScoreNORMAL = 0;
int highScoreHARD = 0;

int gamesPlayedEASY = 0;
int gamesPlayedNORMAL = 0;
int gamesPlayedHARD = 0;

int maxSpeedEASY = 1;
int maxSpeedNORMAL = 1;
int maxSpeedHARD = 1;


void updateGameStats() {
  
  if (currentMode == EASY && score > highScoreEASY) highScoreEASY = score;
  if (currentMode == NORMAL && score > highScoreNORMAL) highScoreNORMAL = score;
  if (currentMode == HARD && score > highScoreHARD) highScoreHARD = score;

 
  if (currentMode == EASY) gamesPlayedEASY++;
  if (currentMode == NORMAL) gamesPlayedNORMAL++;
  if (currentMode == HARD) gamesPlayedHARD++;

  
  if (currentMode == EASY && speedLevel > maxSpeedEASY) maxSpeedEASY = speedLevel;
  if (currentMode == NORMAL && speedLevel > maxSpeedNORMAL) maxSpeedNORMAL = speedLevel;
  if (currentMode == HARD && speedLevel > maxSpeedHARD) maxSpeedHARD = speedLevel;
}

void showExtendedStats() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("HS:"); 
  lcd.print(highScoreEASY);
  lcd.print("/");
  lcd.print(highScoreNORMAL);
  lcd.print("/");
  lcd.print(highScoreHARD);

  lcd.setCursor(0,1);
  lcd.print("PJ:"); 
  lcd.print(gamesPlayedEASY);
  lcd.print("/");
  lcd.print(gamesPlayedNORMAL);
  lcd.print("/");
  lcd.print(gamesPlayedHARD);

  delay(3000); 

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("MaxVel:");
  lcd.print(maxSpeedEASY);
  lcd.print("/");
  lcd.print(maxSpeedNORMAL);
  lcd.print("/");
  lcd.print(maxSpeedHARD);

  delay(3000);
}


int readButtons() {
  if (digitalRead(BTN_UP) == LOW)    return 3;
  if (digitalRead(BTN_DOWN) == LOW)  return 2;
  if (digitalRead(BTN_LEFT) == LOW)  return 1;
  if (digitalRead(BTN_RIGHT) == LOW) return 0;
  if (digitalRead(BTN_START) == LOW) return 4;
  return -1;
}

void setupGraphics() {
  
  lcd.createChar(0, dot);
  lcd.createChar(1, foodDot);
  lcd.createChar(2, headUp);
  lcd.createChar(3, headDown);
  lcd.createChar(4, headLeft);
  lcd.createChar(5, headRight);
  lcd.createChar(6, specialFoodDot);
  lcd.createChar(7, trapFoodDot);
}

void generateFood() {
  bool validPosition = false;
  int attempts = 0;
  while (!validPosition && attempts < 100) {
    food.x = random(0, 16);
    food.y = random(0, 2);
    validPosition = true;
    for (int i = 0; i < snakeLength; i++) {
      if (snake[i].x == food.x && snake[i].y == food.y) {
        validPosition = false;
        break;
      }
    }
    attempts++;
  }
  if (validPosition) {
    foodEaten = false;
    lcd.setCursor(food.x, food.y);
    lcd.write(byte(1));
  }
}

void generateSpecialFood() {
  bool validPosition = false;
  int attempts = 0;
  while (!validPosition && attempts < 100) {
    specialFood.x = random(0, 16);
    specialFood.y = random(0, 2);
    validPosition = true;
    for (int i = 0; i < snakeLength; i++) {
      if (snake[i].x == specialFood.x && snake[i].y == specialFood.y) {
        validPosition = false;
        break;
      }
    }
    attempts++;
  }
  if (validPosition) {
    specialFoodActive = true;
    specialFoodTimer = SPECIAL_FOOD_DURATION;
    lcd.setCursor(specialFood.x, specialFood.y);
    lcd.write(byte(6)); 
  }
}

void generateTrapFood() {
  bool validPosition = false;
  int attempts = 0;
  while (!validPosition && attempts < 100) {
    trapFood.x = random(0, 16);
    trapFood.y = random(0, 2);
    validPosition = true;
    for (int i = 0; i < snakeLength; i++) {
      if ((snake[i].x == trapFood.x && snake[i].y == trapFood.y) || 
          (food.x == trapFood.x && food.y == trapFood.y) || 
          (specialFood.x == trapFood.x && specialFood.y == trapFood.y)) {
        validPosition = false;
        break;
      }
    }
    attempts++;
  }
  if (validPosition) {
    trapFoodActive = true;
    trapFoodTimer = TRAP_FOOD_DURATION;
    lcd.setCursor(trapFood.x, trapFood.y);
    lcd.write(byte(7)); 
  }
}

void initSnake() {
  snakeLength = 3;
  direction = 0;
  newDirection = 0;
  snake[0].x = 8; snake[0].y = 0;
  snake[1].x = 7; snake[1].y = 0;
  snake[2].x = 6; snake[2].y = 0;
}

void drawGame() {
  lcd.clear();
  if (!foodEaten) {
    lcd.setCursor(food.x, food.y);
    lcd.write(byte(1));
  }
  if (specialFoodActive) {
    lcd.setCursor(specialFood.x, specialFood.y);
    lcd.write(byte(6));
  }
  if (trapFoodActive) {
    lcd.setCursor(trapFood.x, trapFood.y);
    lcd.write(byte(7));
  }

  lcd.setCursor(snake[0].x, snake[0].y);
  if (direction == 0) lcd.write(byte(5));   
  else if (direction == 1) lcd.write(byte(4)); 
  else if (direction == 2) lcd.write(byte(3)); 
  else if (direction == 3) lcd.write(byte(2)); 

  
  for (int i = 1; i < snakeLength; i++) {
    lcd.setCursor(snake[i].x, snake[i].y);
    lcd.write(byte(0));
  }
}

void moveSnake() {
  direction = newDirection;
  SnakeSegment newTail;
  newTail.x = snake[snakeLength - 1].x;
  newTail.y = snake[snakeLength - 1].y;
  for (int i = snakeLength - 1; i > 0; i--) {
    snake[i].x = snake[i - 1].x;
    snake[i].y = snake[i - 1].y;
  }
  switch (direction) {
    case 0: snake[0].x++; break;
    case 1: snake[0].x--; break;
    case 2: snake[0].y++; break;
    case 3: snake[0].y--; break;
  }

 
  if (currentMode == EASY) {
    if (snake[0].x < 0) snake[0].x = 15;
    if (snake[0].x > 15) snake[0].x = 0;
    if (snake[0].y < 0) snake[0].y = 1;
    if (snake[0].y > 1) snake[0].y = 0;
  }
  else if (currentMode == NORMAL) {
    if (snake[0].x < 0) snake[0].x = 15;
    if (snake[0].x > 15) snake[0].x = 0;
    if (snake[0].y < 0) snake[0].y = 0;
    if (snake[0].y > 1) snake[0].y = 1;
  }
  else if (currentMode == HARD) {
    if (snake[0].x < 0 || snake[0].x > 15 || snake[0].y < 0 || snake[0].y > 1) {
      gameOver = true;
      showGameOver();
      gameStarted = false;
      return;
    }
  }

  
  if (!foodEaten && snake[0].x == food.x && snake[0].y == food.y) {
    if (snakeLength < MAX_SNAKE_LENGTH) {
      snake[snakeLength].x = newTail.x;
      snake[snakeLength].y = newTail.y;
      snakeLength++;
      soundEat();
    }
    score++;
    foodEaten = true;
    if (snakeLength % 5 == 0 && moveDelay > 150) {
      moveDelay -= 30;
      speedLevel++;
    }
    if (score > highScore) {
      highScore = score;
    }
    generateFood();
  }

  
  if (specialFoodActive && snake[0].x == specialFood.x && snake[0].y == specialFood.y) {
    score += 5;
    totalSpecialEaten++;
    soundEatSpecial();
    specialFoodActive = false;
  }

  
  if (trapFoodActive && snake[0].x == trapFood.x && snake[0].y == trapFood.y) {
    totalTrapEaten++;
    if (currentMode == NORMAL || currentMode == HARD) {
      score -= 3;
      if (score < 0) score = 0;
      tone(buzzerPin, 200, 300);
      delay(150);
    }
    trapFoodActive = false;
  }

  
  if (specialFoodActive) {
    specialFoodTimer--;
    if (specialFoodTimer <= 0) {
      specialFoodActive = false;
    }
  } else {
    if (random(0, 30) == 0) {
      generateSpecialFood();
    }
  }

  if (trapFoodActive) {
    trapFoodTimer--;
    if (trapFoodTimer <= 0) {
      trapFoodActive = false;
    }
  } else {
    if (random(0, 40) == 0 && (currentMode == NORMAL || currentMode == HARD)) {
      generateTrapFood();
    }
  }

  soundMove();
}

bool checkCollisions() {
  for (int i = 1; i < snakeLength; i++) {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
      return true;
    }
  }
  return false;
}

void handleControls() {
  int button = readButtons();
  if (button != -1) {
    switch (button) {
      case 0: if (direction != 1) newDirection = 0; break;
      case 1: if (direction != 0) newDirection = 1; break;
      case 2: if (direction != 3) newDirection = 2; break;
      case 3: if (direction != 2) newDirection = 3; break;
      case 4: 
        gamePaused = !gamePaused;
        if (gamePaused) {
          lcd.clear();
          lcd.setCursor(5, 0);
          lcd.print("PAUSA");
          lcd.setCursor(3, 1);
          lcd.print("Press START");
        }
        
        delay(300);
        break;
    }
  }
}


void showStartScreen() {
  static int menuIndex = 1; 
  static unsigned long lastCheck = 0;

  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("SNAKE GAME");

  lcd.setCursor(0, 1);
  if (menuIndex == 0) lcd.print(">FACIL     ");
  else if (menuIndex == 1) lcd.print(">NORMAL    ");
  else if (menuIndex == 2) lcd.print(">DIFICIL   ");

  if (millis() - lastCheck > 200) {
    int button = readButtons();
    if (button == 0) { menuIndex = (menuIndex + 1) % 3; lastCheck = millis(); }
    else if (button == 1) { menuIndex = (menuIndex + 2) % 3; lastCheck = millis(); }
    else if (button == 4) {
      if (menuIndex == 0) currentMode = EASY;
      if (menuIndex == 1) currentMode = NORMAL;
      if (menuIndex == 2) currentMode = HARD;
      soundStartMelody();
      gameStarted = true;
      resetGame();
      delay(500);
    }
  }
}

void showGameOver() {
  soundCrash();
  animateSnakeDestruction(); 
  soundGameOverMelody();

  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("GAME OVER");
  lcd.setCursor(2, 1);
  lcd.print("Puntaje:");
  lcd.print(score);

  delay(2000);

 
  showStats();

  /
  updateGameStats();      
  showExtendedStats();   
}

void resetGame() {
  gameOver = false;
  gamePaused = false;
  score = 0;
  speedLevel = 1;
  totalSpecialEaten = 0;
  totalTrapEaten = 0;

  if (currentMode == EASY) moveDelay = 500;
  else if (currentMode == NORMAL) moveDelay = 400;
  else if (currentMode == HARD) moveDelay = 300;

  initSnake();
  generateFood();
  specialFoodActive = false;
  specialFoodTimer = 0;
  trapFoodActive = false;
  trapFoodTimer = 0;
  lastMoveTime = millis();
}


void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  lcd.begin(16, 2);
  delay(100);
  lcd.clear();
  setupGraphics();
  randomSeed(analogRead(0));

 
  showIntroAnimation();
}


void loop() {
 
  if (!gameStarted) {
    showStartScreen();
    return;
  }
  if (gameOver) {
    gameStarted = false;
    return;
  }

  handleControls(); 

  if (!gamePaused) { 
    if (millis() - lastMoveTime > moveDelay) {
      moveSnake();
      if (gameOver) return;
      if (checkCollisions()) {
        gameOver = true;
        showGameOver();
        gameStarted = false;
        return;
      }
      drawGame();
      lastMoveTime = millis();
    }
  }
}