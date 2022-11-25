#define BRAKE  0
#define CW     1
#define CCW    2
#define RCW    3
#define LCW    4
#define RCCW   5
#define LCCW   6
#define CS_THRESHOLD 15   // Definition of safety current (Check: "1.3 Monster Shield Example").

//MOTOR 1
#define MOTOR_A1_PIN 7
#define MOTOR_B1_PIN 8

//MOTOR 2
#define MOTOR_A2_PIN 4
#define MOTOR_B2_PIN 9

#define PWM_MOTOR_1 5
#define PWM_MOTOR_2 6

#define CURRENT_SEN_1 A2
#define CURRENT_SEN_2 A3

#define EN_PIN_1 A0
#define EN_PIN_2 A1

#define MOTOR_1 0
#define MOTOR_2 1

short usSpeed = 110;  //default motor speed
unsigned short usMotor_Status = BRAKE;
 
void setup()                         
{
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);

  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);

  pinMode(PWM_MOTOR_1, OUTPUT);
  pinMode(PWM_MOTOR_2, OUTPUT);

  pinMode(CURRENT_SEN_1, OUTPUT);
  pinMode(CURRENT_SEN_2, OUTPUT);  

  pinMode(EN_PIN_1, OUTPUT);
  pinMode(EN_PIN_2, OUTPUT);

  Serial.begin(2400);              // Initiates the serial to do the monitoring 
  Serial.println("Begin motor control");

  showMenu();

}

void loop() 
{
  char user_input;   

  
  
  while(Serial.available())
  {
    user_input = Serial.read(); //Read user input and trigger appropriate function
    digitalWrite(EN_PIN_1, HIGH);
    digitalWrite(EN_PIN_2, HIGH); 
     
    if (user_input =='X')
    {
       Stop();
    }
    else if(user_input =='W')
    {
      Forward();
    }
    else if(user_input =='S')
    {
      Reverse();
    }
    else if(user_input =='E')
    {
      IncreaseSpeed();
    }
    else if(user_input =='Q')
    {
      DecreaseSpeed();
    }
    else if(user_input =='A')
    {
      LeftFoward();
    }
    else if(user_input =='D')
    {
      RightFoward();
    }
    else if(user_input =='Z')
    {
      LeftReverse();
    }
    else if(user_input =='C')
    {
      RightReverse();
    }
    else if(user_input =='1')
    {
      showMenu();
    }
    else if(user_input =='2')
    {
      defineSpeed(60);
    }
    else if(user_input =='3')
    {
      defineSpeed(90);
    }
    else if(user_input =='4')
    {
      defineSpeed(120);
    }
    else if(user_input =='5')
    {
      defineSpeed(150);
    }
    else if(user_input =='6')
    {
      defineSpeed(180);
    }
    else
    {
      Serial.println("Invalid option entered.");
    }
      
  }
}

void showMenu(){
  Serial.println(); //Print function list for user selection
  Serial.println("Enter number for control option:");
  Serial.println("W. FORWARD");
  Serial.println("S. REVERSE");
  Serial.println("X. STOP");
  Serial.println("A. LEFT FOWARD");
  Serial.println("D. RIGHT FOWARD");
  Serial.println("Z. LEFT REVERSE");
  Serial.println("C. RIGHT REVERSE");
  Serial.println("E. INCREASE SPEED");
  Serial.println("Q. DECREASE SPEED");
  Serial.println("R. READ CURRENT");
  Serial.println("1. SHOW MENU");
  Serial.println("2-6. SPEED 60-180");
  Serial.println();
}

void Stop()
{
  Serial.println("Stop");
  usMotor_Status = BRAKE;
  motorGo(MOTOR_1, usMotor_Status, 0);
  motorGo(MOTOR_2, usMotor_Status, 0);
}

void Forward()
{
  Serial.println("Forward");
  usMotor_Status = CW;
  motorGo(MOTOR_1, usMotor_Status, usSpeed);
  motorGo(MOTOR_2, usMotor_Status, usSpeed);
}

void Reverse()
{
  Serial.println("Reverse");
  usMotor_Status = CCW;
  motorGo(MOTOR_1, usMotor_Status, usSpeed);
  motorGo(MOTOR_2, usMotor_Status, usSpeed);
}

void LeftReverse()
{
  Serial.println("Left Reverse");
  usMotor_Status = LCCW;
  motorGo(MOTOR_1, usMotor_Status, usSpeed);
  motorGo(MOTOR_2, usMotor_Status, usSpeed);
}

void RightReverse()
{
  Serial.println("Right Reverse");
  usMotor_Status = RCCW;
  motorGo(MOTOR_1, usMotor_Status, usSpeed);
  motorGo(MOTOR_2, usMotor_Status, usSpeed);
}

void LeftFoward()
{
  Serial.println("Left Foward");
  usMotor_Status = LCW;
  motorGo(MOTOR_1, usMotor_Status, usSpeed);
  motorGo(MOTOR_2, usMotor_Status, usSpeed);
}

void RightFoward()
{
  Serial.println("Right Foward");
  usMotor_Status = RCW;
  motorGo(MOTOR_1, usMotor_Status, usSpeed);
  motorGo(MOTOR_2, usMotor_Status, usSpeed);
}


void defineSpeed(uint8_t speed)
{
  usSpeed = speed;
  if(usSpeed > 255)
  {
    usSpeed = 255;  
  }
  
  Serial.print("Speed: ");
  Serial.println(usSpeed);

  motorGo(MOTOR_1, usMotor_Status, usSpeed);
  motorGo(MOTOR_2, usMotor_Status, usSpeed);  
}

void IncreaseSpeed()
{
  usSpeed = usSpeed + 10;
  if(usSpeed > 255)
  {
    usSpeed = 255;  
  }
  
  Serial.print("Speed: ");
  Serial.println(usSpeed);

  motorGo(MOTOR_1, usMotor_Status, usSpeed);
  motorGo(MOTOR_2, usMotor_Status, usSpeed);  
}

void DecreaseSpeed()
{
  usSpeed = usSpeed - 10;
  if(usSpeed < 0)
  {
    usSpeed = 0;  
  }
  
  Serial.print("Speed: ");
  Serial.println(usSpeed);

  motorGo(MOTOR_1, usMotor_Status, usSpeed);
  motorGo(MOTOR_2, usMotor_Status, usSpeed);  
}

void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm)         //Function that controls the variables: motor(0 ou 1), direction (cw ou ccw) e pwm (entra 0 e 255);
{
  if(motor == MOTOR_1)
  {
    if(direct == CW || direct == LCW)
    {
      digitalWrite(MOTOR_A1_PIN, LOW); 
      digitalWrite(MOTOR_B1_PIN, HIGH);
    }
    else if(direct == CCW || direct == LCCW)
    {
      digitalWrite(MOTOR_A1_PIN, HIGH);
      digitalWrite(MOTOR_B1_PIN, LOW);      
    }
    else
    {
      digitalWrite(MOTOR_A1_PIN, LOW);
      digitalWrite(MOTOR_B1_PIN, LOW);            
    }
    
    analogWrite(PWM_MOTOR_1, pwm); 
  }
  else if(motor == MOTOR_2)
  {
    if(direct == CW || direct == RCW)
    {
      digitalWrite(MOTOR_A2_PIN, LOW);
      digitalWrite(MOTOR_B2_PIN, HIGH);
    }
    else if(direct == CCW || direct == RCCW)
    {
      digitalWrite(MOTOR_A2_PIN, HIGH);
      digitalWrite(MOTOR_B2_PIN, LOW);      
    }
    else
    {
      digitalWrite(MOTOR_A2_PIN, LOW);
      digitalWrite(MOTOR_B2_PIN, LOW);            
    }
    
    analogWrite(PWM_MOTOR_2, pwm);
  }

  Serial.println("| A1 | B1 | A2 | B2 |");
  Serial.print("| D");
  Serial.print(MOTOR_A1_PIN);
  Serial.print(" | D");
  Serial.print(MOTOR_B1_PIN);
  Serial.print(" | D");
  Serial.print(MOTOR_A2_PIN);
  Serial.print(" | D");
  Serial.print(MOTOR_B2_PIN);
  Serial.println(" |");
  Serial.print("|  ");
  Serial.print(digitalRead(MOTOR_A1_PIN));
  Serial.print(" |  ");
  Serial.print(digitalRead(MOTOR_B1_PIN));
  Serial.print(" |  ");
  Serial.print(digitalRead(MOTOR_A2_PIN));
  Serial.print(" |  ");
  Serial.print(digitalRead(MOTOR_B2_PIN));
  Serial.println(" |");
  
}


