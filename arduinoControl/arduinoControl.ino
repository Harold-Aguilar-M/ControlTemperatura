#include <OneWire.h>		   // Libreria para la comunicación usada por el sensor ds18b20
#include <DallasTemperature.h> // Libreria para usar el sensor ds18b20

OneWire ourWire(2);					 //Se establece el pin 2  como bus OneWire
DallasTemperature sensors(&ourWire); //Se declara una variable u objeto para nuestro sensor

#include <LiquidCrystal_I2C.h> // Libreria para LCD I2C
#include <Wire.h>			   // Libreria para usar SDA y SCL

LiquidCrystal_I2C lcd(0x27, 20, 4); // Configuración de la pantalla

#include <Keypad.h> // Libreria para usar teclado matricial

#include <string.h>

const byte rows = 4;					// Número de filas
const byte columns = 4;					// Número de columnas
byte pinsRows[rows] = {12, 11, 10, 9}; // Pins donde se encuentran conectadas las filas en orden F1, F2,...
byte pinsCol[columns] = {8, 7, 6, 5};	// Pins donde se encuentran conectadas las columnas en orden C1, C2,...
char keys[rows][columns] = {			// Asignamos los carateres en la matriz de filas y columnas
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', '.'}};

// Creamos el objeto de la libreria, makeKeymap realiza el mapeo de los caracteres
Keypad keyboard = Keypad(makeKeymap(keys), pinsRows, pinsCol, rows, columns);

char key; // Creamos una variable char que almacenara la tecla que se presione

String str = ""; // Var para recibir los datos por el port Serie
const char separator = ',';
const int dataLength = 5;		  // Datos a recibir
float data[dataLength] = {0, 18}; // Array para los datos

const int fan = 3, // Pin del vantilador
	heater = 4;	   // Pin del calentador

// data[0]	// Temperatura Actual
// data[1]	// Temperatura Ideal
// data[2] 	// Estado del calentador 0 apagado , 1 encendido
// data[3]	// Estado del ventilador 0 apagado , 1 encendido
// data[4]	// Estado de operación 0 automático , 1 manual

volatile unsigned int cuenta = 0;

void setup()
{
	SREG = (SREG & 0b01111111);				 //Desabilitar interrupciones
	TIMSK2 = TIMSK2 | 0b00000001;			 //Habilita la interrupcion por desbordamiento
	TCCR2B = 0b00000111;					 //Configura preescala para que FT2 sea de 7812.5Hz
	SREG = (SREG & 0b01111111) | 0b10000000; //Habilitar interrupciones //Desabilitar interrupciones
	Serial.begin(9600);
	lcd.init();		 // Inicializamos el LCD
	lcd.backlight(); // Encender luz de fondo
	sensors.begin(); //Se inicia el sensor
	pinMode(fan, OUTPUT);
	pinMode(heater, OUTPUT);
}

String floatToString(float floatVal)
{
	char charVal[5];
	String stringVal = "";
	dtostrf(floatVal, 3, 2, charVal);
	for (int i = 0; i < sizeof(charVal); i++)
	{
		stringVal += charVal[i];
	}
	return stringVal;
}

void receiveData()
{
	str = Serial.readStringUntil('\n');
	for (int i = 0; i < dataLength; i++)
	{
		int index = str.indexOf(separator);
		data[i] = str.substring(0, index).toFloat();
		str = str.substring(index + 1);
	}
}

void tempControl()
{
	if (!int(data[4]))
	{
		if (data[0] < data[1])
		{
			digitalWrite(heater, LOW);
			digitalWrite(fan, LOW);
			data[2] = 1;
			data[3] = 0;
		}
		else if (data[0] > (data[1] + 0.99))
		{
			digitalWrite(heater, HIGH);
			digitalWrite(fan, HIGH);
			data[2] = 0;
			data[3] = 1;
		}
		else if (data[0] >= data[1] || data[0] <= (data[1] + 0.99))
		{
			digitalWrite(fan, LOW);
			digitalWrite(heater, HIGH);
			data[2] = 0;
			data[3] = 0;
		}
	}
	else if (int(data[4]))
	{
		digitalWrite(heater, !int(data[2]));
		digitalWrite(fan, int(data[3]));
	}
}

void showData()
{
	lcd.setCursor(0, 0);
	lcd.print("TempAct = ");
	lcd.print(data[0]);
	lcd.write(B11011111);
	lcd.print("C");
	lcd.setCursor(0, 1);
	lcd.print("TempIdeal = ");
	lcd.print(data[1]);
	lcd.write(B11011111);
	lcd.print("C");
	lcd.setCursor(0, 3);
	lcd.print("A cambiar TempIdeal");
	lcd.setCursor(9, 2);
	if (int(data[2]))
		lcd.write(B11101101);
	if (int(data[3]))
		lcd.write(B00101010);
}

void sendData()
{
	String strS = "";
	strS.concat(data[0]);
	strS.concat(",");
	strS.concat(data[1]);
	strS.concat(",");
	strS.concat(int(data[2]));
	strS.concat(",");
	strS.concat(int(data[3]));
	strS.concat(",");
	strS.concat(int(data[4]));
	Serial.println(strS);
	strS = "";
}

void loop()
{

	sensors.requestTemperatures();		  // Se envía el comando para leer la temperatura
	data[0] = sensors.getTempCByIndex(0); // Se obtiene la temperatura en ºC

	tempControl(); // Control de temperatura
	showData();	   // Mostrar datos en pantalla LCD

	key = keyboard.getKey(); // Obtenemos el caracter correspondiente a la tecla presionada

	// Cambiar la temperatura ideal
	if (key != NO_KEY)
	{
		if (key == 'A')
		{
			int i = 0;
			String temp = "";
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print("Coloque la nueva Tem");
			lcd.setCursor(2,1);
			lcd.print("B - Salir");
			lcd.setCursor(5, 3);
			lcd.print("-> ");
			while (i < 2)
			{
				key = keyboard.waitForKey();
				if (key >= '0' && key <= '9')
				{
					lcd.print(key);
					temp += key;
					i++;
				}
				if (key == 'B')
				{
					i = 4;
				}
			}
			if (temp.length() > 0)
			{
				data[1] = temp.toInt();
				sendData(); // Se envian los datos en un string de forma csv
			}
			lcd.clear();
		}
	}

	// Se reciben datos si hay en el buffer
	if (Serial.available())
	{
		lcd.clear();
		receiveData(); 	// Se reciben los datos en un string y se separan para su uso
		sendData();
	}

}

ISR(TIMER2_OVF_vect)
{
	cuenta++;
	if (cuenta > 100) // 20 aprox 1 s
	{
		sendData(); // Se envian los datos en un string de forma csv
		cuenta = 0;
	}
}