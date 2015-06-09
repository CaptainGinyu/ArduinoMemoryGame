#include <LiquidCrystal.h>

//standard setup of pins for the lcd
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int switchPins[4] = {6, 7, 8, 9};
const int ledPins[4] = {14, 15, 16, 17};
const int piezoPin = 10;
const int noteValues[4] = {262, 294, 330, 349};

int switchStates[4];

int score;
int currentSequenceLength;
int startingButtonPressed;
int hasGameStarted;
int isGameOver;
int correctButtonPressed;

//a node for the list that's going to hold the
//values that correspond the buttons that
//the player needs to press correctly
struct listNode
{
  int value;
  listNode* next;  
};
typedef struct listNode ListNode;

//having a node that stays at the beginning of the list
//and a node that traverses through the list
ListNode* startingNode;
ListNode* nodeTraverser;

/*
Starts up the 16 by 2 lcd
Sets new random seed
Starts up the serial monitor for debugging
Sets up each of the switch pins and led pins
Sets hasGameStarted and startingButtonPressed to false
*/
void setup()
{
  lcd.begin(16, 2);
  randomSeed(analogRead(0));
  
  Serial.begin(9600);
  
  for (int i = 0; i < 4; i++)
  {
    pinMode(switchPins[i], INPUT);
    pinMode(ledPins[i], OUTPUT);
  }
  
  hasGameStarted = 0;
  startingButtonPressed = 0;
  
  lcd.print("Press any button");
  lcd.setCursor(0, 1);
  lcd.print("to start playing");
}

/*
Begins the game 
Generates the first sequence (of 1 value)
and shows that sequence
*/
void startGame()
{
  Serial.println("Starting game");
  hasGameStarted = 1;
  
  correctButtonPressed = 0;
  currentSequenceLength = 1;
  score = 0;
  isGameOver = 0;
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
  
  startingNode = (ListNode*) malloc(sizeof(ListNode));
  startingNode->next = NULL;
  startingNode->value = NULL;
  
  nodeTraverser = startingNode;
  updateSequence(currentSequenceLength);
  showCurrentSequence();
}

/*
Detects button presses
If correct button is pressed AND released, moves on to correctButtonHandler
Otherwise, if a wrong button if pressed, lcd displays "Wrong!" and game ends
In both cases, the LED corresponding to the button pressed lights up
*/
void switchesHandler()
{
  for (int i = 0; i < 4; i++)
  {
    switchStates[i] = digitalRead(switchPins[i]);
    
    if (correctButtonPressed)
    {
      if (switchStates[nodeTraverser->value] == LOW)
      {
        correctButtonPressed = 0;
        digitalWrite(ledPins[nodeTraverser->value], LOW);
        noTone(piezoPin);
        correctButtonHandler();
      }
    }
    else
    {
      if (switchStates[i] == HIGH)
      {
        digitalWrite(ledPins[i], HIGH);
        tone(piezoPin, noteValues[i]);
        if (nodeTraverser->value == i)
        {
          correctButtonPressed = 1;    
        }
        else
        {
          lcd.setCursor(0, 0);
          Serial.print("Pressed ");
          Serial.print(i);
          Serial.print(", but correct was ");
          Serial.print(nodeTraverser->value);
          lcd.print("Wrong!!!        ");
          isGameOver = 1;       
          delay(1000);  
        }
      }
      else
      {     
        digitalWrite(ledPins[i], LOW);
        noTone(piezoPin);
      }  
    }
  } 
}

/*
Increases the game score by 1 point for the correct button
that is pressed AND released
Then if the entire sequence is complete, a new sequence gets
generated and shown
Otherwise, we just move to the next number in the sequence
*/
void correctButtonHandler()
{  
  score += 1;
  lcd.setCursor(7, 1);
  lcd.print(score);
  
  if (!nodeTraverser->next)
  {
    lcd.setCursor(0, 0);
    lcd.print("Cool! Good job! ");
    delay(1000);
    currentSequenceLength++;
    updateSequence(currentSequenceLength);
    showCurrentSequence();
    nodeTraverser = startingNode;
  }
  else
  {      
    nodeTraverser = nodeTraverser->next;  
  }
}

/*
Turns on and off the LEDs and musical tones
corresponding to the numbers
in our current sequence with delays of 1 second in between
*/
void showCurrentSequence()
{
  Serial.println("Showing current sequence");
  turnOffAllLeds();
  noTone(piezoPin);
  lcd.setCursor(0, 0);
  lcd.print("Watch!          ");
  delay(1000);
  
  ListNode* traverser = startingNode;
  
  while (traverser)
  {
    Serial.print(traverser->value);
    Serial.print(" ");
    digitalWrite(ledPins[traverser->value], HIGH);
    tone(piezoPin, noteValues[traverser->value]);
    delay(1000);
    digitalWrite(ledPins[traverser->value], LOW);
    noTone(piezoPin);
    delay(1000);
    
    traverser = traverser->next;
  }  
  
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("Your move!      ");
  Serial.println(" ");
}

/*
Generates a random sequence of a given length
consisting of numbers 0 to 3
that the player will have to press in the game
*/
void updateSequence(int desiredLength)
{
  Serial.println("updating sequence");
  ListNode* traverser = startingNode;
  traverser->value = random(4);
  Serial.print(traverser->value);
  
  for (int i = 1; i < desiredLength; i++)
  {
    if (!traverser->next)
    {
      traverser->next = (ListNode*) malloc(sizeof(ListNode));
      traverser->next->next = NULL;
    }
    
    traverser->next->value = random(4);
    Serial.print(" ");
    Serial.print(traverser->next->value);
    traverser = traverser->next;
  }
  traverser = NULL;
  Serial.println(" ");
}

//turns off all 4 LEDs
void turnOffAllLeds()
{
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(ledPins[i], LOW);
  }
}

void loop()
{
  /*
  if the player has started the game
  we check go to switchesHandler to handle button presses
  and if we the player loses the game as a result of a wrong
  button press, we display the game over message and turn off
  all LEDs
  */
  if (hasGameStarted)
  {
    if (!isGameOver)
    {
      switchesHandler();
    }
    else
    {
      lcd.setCursor(0, 0);
      lcd.print("Game over!!!");
      turnOffAllLeds();
      noTone(piezoPin);
    }
  }
  /*
  Otherwise, we check to wait for the player to press AND release
  any button to start the game 
  */
  else
  {
    turnOffAllLeds();
    noTone(piezoPin);
    for (int i = 0; i < 4; i++)
    {
      switchStates[i] = digitalRead(switchPins[i]);
    }
    if (startingButtonPressed)
    {
      if (switchStates[0] == LOW && switchStates[1] == LOW && switchStates[2] == LOW && switchStates[3] == LOW)
      {
        lcd.clear();
        delay(1000);
        startGame();
      }
    }
    else
    {
      if (switchStates[0] == HIGH || switchStates[1] == HIGH || switchStates[2] == HIGH || switchStates[3] == HIGH)
      {
        startingButtonPressed = 1;
      }
    }
  }
}
