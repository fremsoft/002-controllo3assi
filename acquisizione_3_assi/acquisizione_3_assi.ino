/*
 * Programma per la movimentazione di tre motori passo passo
 * NEMA 17 (17HS08-1004S) con controller A4988 con macchina a
 * stati in modo indipendente (un motore alla volta).
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


long int pos_mot1, pos_mot2, pos_mot3;

#define NUMERO_MASSIMO_PASSI 100
long int percorso[NUMERO_MASSIMO_PASSI][3];
int numero_passi, passo_in_esecuzione;



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

  pos_mot1 = 0;
  pos_mot2 = 0;
  pos_mot3 = 0;

  numero_passi = 0;

  apprendimento = 1;
}

void loop() {
  int x, y, z;

  if ( apprendimento != 0 ) {
    /* modalita' di apprendimento */
    x = analogRead( JOYSTICK_X );
    y = analogRead( JOYSTICK_Y );
    z = analogRead( JOYSTICK_Z );

    if ( analogRead( BUTTON ) < 500 ) {
      /* ho premuto il tasto */
      /* memorizzo la posizione */

      Serial.print(pos_mot1);
      Serial.print(", ");
      Serial.print(pos_mot2);
      Serial.print(", ");
      Serial.println(pos_mot3);

      percorso[numero_passi][0] = pos_mot1;
      percorso[numero_passi][1] = pos_mot2;
      percorso[numero_passi][2] = pos_mot3;
      if (numero_passi < (NUMERO_MASSIMO_PASSI - 1)) {
        numero_passi ++;
      }

      delay( 1000 );

      if ( analogRead( BUTTON ) < 500 ) {
        apprendimento = 0;
        passo_in_esecuzione = 0;
      }
    }
  }
  else {
    /* modalita' esecuzione */
    /* sto andando al punto percorso[passo_in_esecuzione] */

    /* simulatore x */
    if ( pos_mot1 < percorso[passo_in_esecuzione][0] ) {
      x = 1000;
      contatore = 10;
    }
    else if ( pos_mot1 > percorso[passo_in_esecuzione][0] ) {
      x = 0;
      contatore = 10;
    }
    else { /*  pos_mot1 == percorso[passo_in_esecuzione][0] */
      x = 500;
    }

    /* simulatore y */
    if ( pos_mot2 < percorso[passo_in_esecuzione][1] ) {
      y = 1000;
      contatore = 10;
    }
    else if ( pos_mot2 > percorso[passo_in_esecuzione][1] ) {
      y = 0;
      contatore = 10;
    }
    else { /*  pos_mot2 == percorso[passo_in_esecuzione][1] */
      y = 500;
    }

    /* simulatore z */
    if ( pos_mot3 < percorso[passo_in_esecuzione][2] ) {
      z = 1000;
      contatore = 10;
    }
    else if ( pos_mot3 > percorso[passo_in_esecuzione][2] ) {
      z = 0;
      contatore = 10;
    }
    else { /*  pos_mot3 == percorso[passo_in_esecuzione][2] */
      z = 500;
    }

    if ( contatore > 0 ) {
      contatore --;
    }
    else {
      /* ho raggiunto il punto, passo al prossimo */
      passo_in_esecuzione ++;
      if ( passo_in_esecuzione == numero_passi ) {
        passo_in_esecuzione = 0;
      }
      contatore = 10;
    }
  }

  if ( x > 700) {
    pos_mot1 ++;
    stato_sequenza = 0;
  }
  else if ( x < 300) {
    pos_mot1 --;
    stato_sequenza = 1;
  }
  else if ( y > 700) {
    pos_mot2 ++;
    stato_sequenza = 2;
  }
  else if ( y < 300) {
    pos_mot2 --;
    stato_sequenza = 3;
  }
  else if ( z > 700) {
    pos_mot3 ++;
    stato_sequenza = 4;
  }
  else if ( z < 300) {
    pos_mot3 --;
    stato_sequenza = 5;
  }
  else {
    stato_sequenza = -1; /* do nothing */
  }

  switch ( stato_sequenza ) {
    case 0 : // GIRA BLU   CW
      digitalWrite( DIR_MOT1,  HIGH );
      digitalWrite( STEP_MOT1, HIGH );
      delay(1);
      digitalWrite( STEP_MOT1, LOW );
      delay(1);
      break;

    case 1 : // GIRA BLU   CCW
      digitalWrite( DIR_MOT1,  LOW );
      digitalWrite( STEP_MOT1, HIGH );
      delay(1);
      digitalWrite( STEP_MOT1, LOW );
      delay(1);
      break;

    case 2 : // GIRA VERDE CW
      digitalWrite( DIR_MOT2,  HIGH );
      digitalWrite( STEP_MOT2, HIGH );
      delay(1);
      digitalWrite( STEP_MOT2, LOW );
      delay(1);
      break;

    case 3 : // GIRA VERDE CCW
      digitalWrite( DIR_MOT2,  LOW );
      digitalWrite( STEP_MOT2, HIGH );
      delay(1);
      digitalWrite( STEP_MOT2, LOW );
      delay(1);
      break;

    case 4 : // GIRA ROSSO CW
      digitalWrite( DIR_MOT3,  HIGH );
      digitalWrite( STEP_MOT3, HIGH );
      delay(1);
      digitalWrite( STEP_MOT3, LOW );
      delay(1);
      break;

    case 5 : // GIRA ROSSO CCW
      digitalWrite( DIR_MOT3,  LOW );
      digitalWrite( STEP_MOT3, HIGH );
      delay(1);
      digitalWrite( STEP_MOT3, LOW );
      delay(1);
      break;

    default :
      stato_sequenza = 0;
      break;

  }

}
