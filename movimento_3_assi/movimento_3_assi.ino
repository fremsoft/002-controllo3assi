/*
 * Programma per la movimentazione di tre motori passo passo
 * NEMA 17 (17HS08-1004S) con controller A4988 in modo interpolato.
 * 
 * Un Joystick collegato sugli ingressi analogici A0, A1 e un 
 * potenziometro collegato all'ingresso analogico A2 controllano 
 * i movimenti dei tre motori in fase di apprendimento.
 * 
 * Un pulsante collegato all'ingresso A3 (con pull-up interna)
 * permette di registrare le posizioni (con pressione veloce) 
 * e di avviare la movimentazione (con pressione prolungata)
 *  
 */

#define     MS1           2
#define     MS2           3
#define     MS3           4

#define     STEP_MOT1     5
#define     DIR_MOT1      6

#define     STEP_MOT2     8
#define     DIR_MOT2      9

#define     STEP_MOT3    10
#define     DIR_MOT3     11

#define     JOYSTICK_X   A0
#define     JOYSTICK_Y   A1
#define     JOYSTICK_Z   A2
#define     BUTTON       A3


/*  0 = GIRA BLU   CW
    1 = GIRA BLU   CCW
    2 = GIRA VERDE CW
    3 = GIRA VERDE CCW
    4 = GIRA ROSSO CW
    5 = GIRA ROSSO CCW
*/
int stato_sequenza;
int contatore;

int apprendimento;


long int pos_mot1_presente, pos_mot2_presente, pos_mot3_presente;
long int pos_mot1_futura,   pos_mot2_futura,   pos_mot3_futura;

#define NUMERO_MASSIMO_PASSI 100
long int percorso[NUMERO_MASSIMO_PASSI][3];
int numero_passi, passo_in_esecuzione, passo_precedente;


void setup() {

  Serial.begin ( 115200 );

  pinMode( MS1,       OUTPUT );
  pinMode( MS2,       OUTPUT );
  pinMode( MS3,       OUTPUT );
  pinMode( STEP_MOT1, OUTPUT );
  pinMode( DIR_MOT1,  OUTPUT );
  pinMode( STEP_MOT2, OUTPUT );
  pinMode( DIR_MOT2,  OUTPUT );
  pinMode( STEP_MOT3, OUTPUT );
  pinMode( DIR_MOT3,  OUTPUT );

  pinMode( BUTTON,    INPUT_PULLUP);

  /* MS1  MS2  MS3 Microstep Resolution

     Low  Low  Low   Full step
     High Low  Low   Half step
     Low  High Low   Quarter step
     High High Low   Eighth step
     High High High  Sixteenth step
   **/
  digitalWrite( MS1, HIGH );
  digitalWrite( MS2, HIGH );
  digitalWrite( MS3, HIGH );

  digitalWrite( STEP_MOT1, LOW );
  digitalWrite( DIR_MOT1,  LOW );

  digitalWrite( STEP_MOT2, LOW );
  digitalWrite( DIR_MOT2,  LOW );

  digitalWrite( STEP_MOT3, LOW );
  digitalWrite( DIR_MOT3,  LOW );

  stato_sequenza = 0;
  contatore = 0;

  pos_mot1_presente = 0;
  pos_mot2_presente = 0;
  pos_mot3_presente = 0;
  pos_mot1_futura = 0;
  pos_mot2_futura = 0;
  pos_mot3_futura = 0;

  numero_passi = 0;

  apprendimento = 1;
}

void loop() {
  int x, y, z;
  int delta_x, delta_y, delta_z;
  

  if ( apprendimento != 0 ) {
    /* modalita' di apprendimento */
    x = analogRead( JOYSTICK_X );
    
    if ( x > 700) {
      pos_mot1_futura ++;
    }
    else if ( x < 300) {
      pos_mot1_futura --;
    }

    y = analogRead( JOYSTICK_Y );
    if ( y > 700) {
      pos_mot2_futura ++;
    }
    else if ( y < 300) {
      pos_mot2_futura --;
    }
    
    z = analogRead( JOYSTICK_Z );
    if ( z > 700) {
      pos_mot3_futura ++;
    }
    else if ( z < 300) {
      pos_mot3_futura --;
    }
  
    if ( analogRead( BUTTON ) < 500 ) {
      /* ho premuto il tasto */
      /* memorizzo la posizione */

      Serial.print(pos_mot1_presente);
      Serial.print(", ");
      Serial.print(pos_mot2_presente);
      Serial.print(", ");
      Serial.println(pos_mot3_presente);

      percorso[numero_passi][0] = pos_mot1_presente;
      percorso[numero_passi][1] = pos_mot2_presente;
      percorso[numero_passi][2] = pos_mot3_presente;
      if (numero_passi < (NUMERO_MASSIMO_PASSI - 1)) {
        numero_passi ++;
      }

      delay( 1000 );

      if ( analogRead( BUTTON ) < 500 ) {
        apprendimento = 0;
        passo_in_esecuzione = 0;
        passo_precedente = numero_passi - 1;
      }
    }
  }
  else {
    /* modalita' esecuzione */

    Serial.print(pos_mot1_presente);
    Serial.print(", ");
    Serial.print(pos_mot2_presente);
    Serial.print(", ");
    Serial.println(pos_mot3_presente);


    
    /* sto andando al punto percorso[passo_in_esecuzione] */

    if (
         ( pos_mot1_presente == percorso[passo_in_esecuzione][0] )
         &&
         ( pos_mot2_presente == percorso[passo_in_esecuzione][1] )
         &&
         ( pos_mot3_presente == percorso[passo_in_esecuzione][2] )
       )
    {
      passo_precedente = passo_in_esecuzione;
      passo_in_esecuzione ++;
      if ( passo_in_esecuzione >= numero_passi ) {
        passo_in_esecuzione = 0;
      } 
    }
    else
    {
      // determino il delta maggiore
      delta_x = percorso[passo_in_esecuzione][0] - percorso[passo_precedente][0];
      delta_y = percorso[passo_in_esecuzione][1] - percorso[passo_precedente][1];
      delta_z = percorso[passo_in_esecuzione][2] - percorso[passo_precedente][2];

      if ((abs(delta_x) >= abs(delta_y)) && ( abs(delta_x) >= abs(delta_z)))
      {
        // comanda l'asse x (mot1)
        if (percorso[passo_in_esecuzione][0] > pos_mot1_presente) { pos_mot1_futura ++; }
        if (percorso[passo_in_esecuzione][0] < pos_mot1_presente) { pos_mot1_futura --; }

        // interpolazione motore 2
        pos_mot2_futura = map(pos_mot1_futura, 
                              percorso[passo_precedente][0],
                              percorso[passo_in_esecuzione][0],
                              percorso[passo_precedente][1],
                              percorso[passo_in_esecuzione][1]);

        // interpolazione motore 3
        pos_mot3_futura = map(pos_mot1_futura, 
                              percorso[passo_precedente][0],
                              percorso[passo_in_esecuzione][0],
                              percorso[passo_precedente][2],
                              percorso[passo_in_esecuzione][2]);
      }
      else
      if ((abs(delta_y) >= abs(delta_x)) && ( abs(delta_y) >= abs(delta_z)))
      {
        // comanda l'asse y (mot2)
        if (percorso[passo_in_esecuzione][1] > pos_mot2_presente) { pos_mot2_futura ++; }
        if (percorso[passo_in_esecuzione][1] < pos_mot2_presente) { pos_mot2_futura --; }

        // interpolazione motore 1
        pos_mot1_futura = map(pos_mot2_futura, 
                              percorso[passo_precedente][1],
                              percorso[passo_in_esecuzione][1],
                              percorso[passo_precedente][0],
                              percorso[passo_in_esecuzione][0]);

        // interpolazione motore 3
        pos_mot3_futura = map(pos_mot2_futura, 
                              percorso[passo_precedente][1],
                              percorso[passo_in_esecuzione][1],
                              percorso[passo_precedente][2],
                              percorso[passo_in_esecuzione][2]);
      }
      else
      /* if ((abs(delta_z) >= abs(delta_x)) && ( abs(delta_z) >= abs(delta_y))) */
      {
        // comanda l'asse z (mot3)
        if (percorso[passo_in_esecuzione][2] > pos_mot3_presente) { pos_mot3_futura ++; }
        if (percorso[passo_in_esecuzione][2] < pos_mot3_presente) { pos_mot3_futura --; }

        // interpolazione motore 1
        pos_mot1_futura = map(pos_mot3_futura, 
                              percorso[passo_precedente][2],
                              percorso[passo_in_esecuzione][2],
                              percorso[passo_precedente][0],
                              percorso[passo_in_esecuzione][0]);
                              
        // interpolazione motore 2
        pos_mot2_futura = map(pos_mot3_futura, 
                              percorso[passo_precedente][2],
                              percorso[passo_in_esecuzione][2],
                              percorso[passo_precedente][1],
                              percorso[passo_in_esecuzione][1]);
      }
    }
  }

  if (pos_mot1_futura > pos_mot1_presente) {
    // GIRA BLU   CW
    digitalWrite( DIR_MOT1,  HIGH );
    delayMicroseconds(10);
    digitalWrite( STEP_MOT1, HIGH );
    pos_mot1_presente ++;
  }

  if (pos_mot1_futura < pos_mot1_presente) {
    // GIRA BLU   CCW
    digitalWrite( DIR_MOT1,  LOW );
    delayMicroseconds(10);
    digitalWrite( STEP_MOT1, HIGH );
    pos_mot1_presente --;
  }

  if (pos_mot2_futura > pos_mot2_presente) {
    // GIRA VERDE CW
    digitalWrite( DIR_MOT2,  HIGH );
    delayMicroseconds(10);
    digitalWrite( STEP_MOT2, HIGH );
    pos_mot2_presente ++;
  }

  if (pos_mot2_futura < pos_mot2_presente) {
    // GIRA VERDE CCW
    digitalWrite( DIR_MOT2,  LOW );
    delayMicroseconds(10);
    digitalWrite( STEP_MOT2, HIGH );
    pos_mot2_presente --;
  }

  if (pos_mot3_futura > pos_mot3_presente) {
    // GIRA ROSSO CW
    digitalWrite( DIR_MOT3,  HIGH );
    delayMicroseconds(10);
    digitalWrite( STEP_MOT3, HIGH );
    pos_mot3_presente ++;
  }

  if (pos_mot3_futura < pos_mot3_presente) {
    // GIRA ROSSO CCW
    digitalWrite( DIR_MOT3,  LOW );
    delayMicroseconds(10);
    digitalWrite( STEP_MOT3, HIGH );
    pos_mot3_presente --;
  }

  delay(1);
  digitalWrite( STEP_MOT1, LOW );
  digitalWrite( STEP_MOT2, LOW );
  digitalWrite( STEP_MOT3, LOW );
  delay(1);

}
