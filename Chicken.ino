#include <Arduino.h>

// LCD --------------------------------------------------------------------
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Servo --------------------------------------------------------------------
#include <Servo.h>
Servo myServo;
int pos = 90;
int plus = 1;

// Keypad --------------------------------------------------------------------
#include <Keypad.h>
const byte ROWS = 5;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'m', 'n', '#', '*'},
    {'1', '2', '3', 'u'},
    {'4', '5', '6', 'd'},
    {'7', '8', '9', 't'},
    {'l', '0', 'r', 'e'}};

byte colPins[COLS] = {52, 50, 48, 46};
byte rowPins[ROWS] = {36, 38, 40, 42, 44};
Keypad pad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//Sensors ---------------------------------------------------------------------
const int sensorPin = A0;
float sensorValue;
float voltageOut;
float currentTemperature;
String currentHumidity;
const int buzzer = 22;
const int tempLED = 7;
const int humiditySensor = 6;
const int humidLED = 9;

// Menu ----------------------------------------------------------------------
#include <MenuBackend.h>

float tempratureLimit = 38;
String humidity = "LOW";
String turner = "ACTIVE";
int turnerCounter = 0;

//Users ---------------------------------------------------------------------
String passwords[20] = {"123"};
bool hasLoggedOn = false;
int currentUser = -1;

// Dictionary ----------------------------------------------------------------------
const char *turnStr = "State:";
const char *settingStr = "Setting";
const char *setterStr = "Setter";
const char *hatcherStr = "Hatcher";
const char *accountStr = "Account";
const char *createAccountStr = "Create Account";
const char *changePasswordStr = "Change Password";
const char *exitStr = "Exit";
const char *displayStr = "Display";
const char *changeTempStr = "Change Temperature Limit";

// ----------------------------------------------------------------------

String readStr()
{
  String str = "";
  char readKey = ' ';
  while (readKey != 'e')
  {
    readKey = pad.getKey();
    if (readKey)
    {
      if (readKey != 'e')
      {
        str += readKey;
        lcd.setCursor(0, 1);
        lcd.print(str);
      }
    }
  }
  return str;
}

/*
  This is an important function
  Here all use events are handled
  This is where you define a behaviour for a menu item
*/
void menuUseEvent(MenuUseEvent used)
{
  if (used.item.getName() == setterStr)
  {

    humidity = "HIGH";
    turner = "ACTIVE";
  }
  else if (used.item.getName() == hatcherStr)
  {
    temprature = readStr().toInt();
    humidity = "LOW";
    turner = "PASSIVE";
  }
  else if (used.item.getName() == createAccountStr)
  {
    lcd.clear();
    lcd.print("Enter username");
    int user = readStr().toInt();
    Serial.println(user);
    lcd.clear();
    lcd.print("Enter Password");
    String password = readStr();
    passwords[user] = password;
  }
  else if (used.item.getName() == changePasswordStr)
  {
    passwords[0] = readStr();
  }
  else if (used.item.getName() == changeTempStr)
  {
    temprature = readStr().toFloat();
  }
  else if (used.item.getName() == exitStr)
  {
    hasLoggedOn = false;
  }
}

/*
  This is an important function
  Here we get a notification whenever the user changes the menu
  That is, when the menu is navigated
*/
void menuChangeEvent(MenuChangeEvent changed)
{
  lcd.begin(16, 2);
  lcd.print(changed.to.getName());
}

//this controls the menu backend and the event generation
MenuBackend menu = MenuBackend(menuUseEvent, menuChangeEvent);
MenuItem changeTempI = MenuItem(changeTempStr);
MenuItem setting = MenuItem(settingStr);
MenuItem setter = MenuItem(settingStr);
MenuItem hatcher = MenuItem(hatcherStr);

MenuItem account = MenuItem(accountStr);
MenuItem createAccount = MenuItem(createAccountStr);
MenuItem changePassword = MenuItem(changePasswordStr);
MenuItem exits = MenuItem(exitStr);

MenuItem displays = MenuItem(displayStr);
MenuItem turnerState = MenuItem(turnStr);

//this function builds the menu and connects the correct items together
void menuSetup()
{
  menu.getRoot().add(setting);

  //SETTING
  setting.addRight(setter);

  setter.addAfter(hatcher);

  //ACCOUNT
  setting.addAfter(account);
  account.addRight(createAccount);
  createAccount.addAfter(changePassword);
  changePassword.addAfter(exits);
  //
  createAccount.addBefore(exits);
  changePassword.addBefore(createAccount);
  exits.addBefore(changePassword);
  //
  createAccount.addLeft(account);
  changePassword.addLeft(account);

  //DISPLAY
  account.addAfter(displays);
  displays.addRight(turnerState);
  turnerState.addAfter(turnerState);
  //
  turnerState.addBefore(turnerState);
  //
  turnerState.addLeft(displays);
  displays.addAfter(changeTempI);
}

void turnServo()
{
  if (turner == "ACTIVE")
  {
    Serial.println(turner);
    if (pos % 180 == 0)
      plus *= -1;
    pos += plus;
    myServo.write(pos);
    delay(50);
  }
}

void updateEverything()
{
  turnServo();
  updateState();
}

void login()
{
  while (true)
  {
    lcd.clear();
    lcd.print("Enter Username:");
    int username = readStr().toInt();
    lcd.clear();
    lcd.print("Enter Password:");
    String password = readStr();
    Serial.println(username);
    Serial.println(password);
    if (password == passwords[username])
    {
      lcd.clear();
      currentUser = username;
      hasLoggedOn = true;
      break;
    }
    else
    {
      lcd.clear();
      lcd.print("WRONG!!");
      delay(1000);
    }
  }
}

void setup()
{
  menuSetup();
  MenuItemRef current = menu.getCurrent();
  lcd.print(current.getName());
  Serial.begin(9600);
  //temperature sensor setup
  pinMode(sensorPin, INPUT);
  pinMode(tempLED, OUTPUT);

  //temprature buzzer
  pinMode(buzzer, OUTPUT);

  //humidity
  pinMode(humiditySensor, INPUT);
  pinMode(30, OUTPUT);

  //servo
  myServo.attach(10);
}

void updateState()
{
  sensorValue = analogRead(sensorPin);
  voltageOut = (sensorValue * 5000) / 1024;
  currentTemperature = voltageOut / 15.6 + 9;
  if (menu.getCurrent().getName() == turnStr)
  {
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(currentTemperature);
    lcd.print("H:");
    lcd.print(humidity);
    lcd.print("R:");
    lcd.print(pos);
  }
}

void loop()
{
  if (!hasLoggedOn)
    login();

  char readKey = pad.getKey();
  if (readKey)
  {
    if (readKey == 'u')
      menu.moveUp();
    else if (readKey == 'd')
      menu.moveDown();
    else if (readKey == 'r')
      menu.moveRight();
    else if (readKey == 'l')
      menu.moveLeft();
    else if (readKey == 'e')
    {
      menu.use();
      menu.moveLeft();
    }

    if (currentTemperature < tempratureLimit)
    {
      Serial.print("Heater");
      digitalWrite(tempLED, HIGH);
    }
    else
    {
      digitalWrite(tempLED, LOW);
      Serial.print("Cooler");
    }

    //temprature buzzer
    if (abs(currentTemperature - tempratureLimit) > 5)
      digitalWrite(buzzer, HIGH);
    else
      digitalWrite(buzzer, LOW);

    //humidity
    int h = digitalRead(humiditySensor);
    if (h == 1)
    {
      currentHumidity = "HIGH";
      digitalWrite(humidLED, HIGH);
    }
    else
    {
      currentHumidity = "LOW";
      digitalWrite(humidLED, LOW);
    }
    updateEverything();
  }
  updateEverything();
}
