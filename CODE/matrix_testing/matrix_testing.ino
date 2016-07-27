#define rows 6
#define columns 3

char matrix[3][3] = {
  {'a','b','c'},
  {'d','e','f'},
  {'g','h','i'}
  };

char matrix_snake[rows][columns] = {
  {'a','b','c'},
  {'f','e','d'},
  {'g','h','i'},
  {'l','k','j'},
  {'m','n','o'},
  {'r','q','p'}
  };


void setup() {
  Serial.begin(9600);
}

//void loop() {
//  for (int i=0; i<3; i++)  {
//    for (int j=0; j<3; j++)  {
//      Serial.println(matrix[i][j]);
//      delay(200);
//    }
//  }
//  delay(500);
//  Serial.println("----------------------");
//}

void loop() {
  for (int i=0; i<rows; i++)  
  {
    if ( (i%2) == 0)  {
      for (int j=0; j<=(columns-1); j++)  
      {
        Serial.println(matrix_snake[i][j]);
        delay(200);
      }
    }
    else  {
      for (int j=(columns-1); j>=0; j--)  
      {
        Serial.println(matrix_snake[i][j]);
        delay(200);
      }
    }
  }
  delay(500);
  Serial.println("----------------------");
}
