#include <SPI.h>
#include <Ethernet.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
unsigned long ultimaCaptura = 0;

// Configuración de red (IP dinámica)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server(80);

// Pines para las ruedas
const int derecha_1 = 4;
const int derecha_2 = 3;
const int derecha_pwm = 2;

const int izquierda_1 = 6;
const int izquierda_2 = 5;
const int izquierda_pwm = 7;

// Configuración de la base de datos MySQL
/*IPAddress serverDb(91,208,207,108);
char usuario[] = "uedwkrwyweha5rcy";
char pass[] = "PshcRgfZkFIulygInL5Q";*/

//BD PAOLA
IPAddress serverDb(129,80,15,102);
char usuario[] = "sistemas";
char pass[] = "123456";
EthernetClient cliente;
MySQL_Connection conn((Client *)&cliente);

float temperatura;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

  // Inicializar el Ethernet Shield con IP dinámica (DHCP)
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Error obteniendo IP a través de DHCP");
    while (true);
  }

  Serial.print("Servidor iniciado. IP: ");
  Serial.println(Ethernet.localIP());

  // Iniciar el servidor
  server.begin();

  // Configurar pines
  pinMode(derecha_1, OUTPUT);
  pinMode(derecha_2, OUTPUT);
  pinMode(derecha_pwm, OUTPUT);

  pinMode(izquierda_1, OUTPUT);
  pinMode(izquierda_2, OUTPUT);
  pinMode(izquierda_pwm, OUTPUT);

  // Conectar a la base de datos MySQL
  Serial.print("Conectando a la base de datos...");
  if (conn.connect(serverDb, 3306, usuario, pass)) {
    Serial.println("Conexión exitosa.");
    delay(1000);
  } else {
    Serial.println("Fallo al conectar a la base de datos.");
    while (true);
  }

  detener();
}

void loop() {
  EthernetClient client = server.available();

  if (client) {
    String currentLine = "";
    Serial.println("Cliente conectado.");
    currentTime = millis();
    previousTime = currentTime;
    // Leer la solicitud del cliente
    String request = "";
    boolean currentLineIsBlank = true; 
    char c;
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        c = client.read();
        request += c;
        Serial.print(c);

        if (c == '\n') {

          if(request.length() > 0)
          {
            Serial.println("Solicitud recibida:");
            Serial.println(request);

            // Verificar y analizar la solicitud del cliente
            if (request.indexOf("GET /adelante") != -1) {
              adelante();
            } else if (request.indexOf("GET /atras") != -1) {
              atras();
            } else if (request.indexOf("GET /derecha") != -1) {
              derecha();
            } else if (request.indexOf("GET /izquierda") != -1) {
              izquierda();
            } else if (request.indexOf("GET /detener") != -1) {
              detener();
            }
            
            int valorArduino = analogRead(A0);
            float voltaje = (valorArduino / 1024.0) * 5.0;
            temperatura = (voltaje)*10;

              //Serial.println("hhaha");
              // Responder al cliente con la página HTML
              {client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/html");
              client.println();
              client.println("<html>");
              client.println("<head>");
              client.println("<meta charset='UTF-8'>");
              client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
              client.println("<link href='https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css' rel='stylesheet'>");
              client.println("<title>Control Del Carro</title>");
              client.println("<style>");
              client.println("html, body { margin: 0; padding: 0; height: 100%; overflow: hidden; }");
              client.println("body::before { content: ''; position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-image: url('https://i.pinimg.com/originals/c3/2a/cb/c32acb8b243d78c196a5ed7c89d52ea5.jpg'); background-size: cover; background-position: center; background-repeat: no-repeat; background-attachment: fixed; opacity: 0.5; z-index: -1; }");
              client.println(".control-grid { display: flex; flex-direction: column; align-items: center; gap: 10px; }");
              client.println(".row-buttons { display: flex; gap: 100px; }");
              client.println("button { width: 100px; }");
              client.println("</style>");
              client.println("</head>");
              client.println("<body class='bg-light'>");
              // Mostrar la temperatura
              client.print("<h2>Temperatura Actual: ");
              client.print(temperatura);  // Muestra la temperatura capturada
              client.println(" °C</h2>");
              client.println("<div class='container text-center my-5'>");
              client.println("<h1 class='mb-4'>Control Del Carro</h1>");
              client.println("<div class='control-grid'>");
              client.println("<button class='btn btn-dark btn-lg' onclick=\"location.href='/adelante'\"><i class='fas fa-caret-up fa-2x'></i></button>");
              client.println("<div class='row-buttons'>");
              client.println("<button class='btn btn-dark btn-lg' onclick=\"location.href='/izquierda'\"><i class='fas fa-caret-left fa-2x'></i></button>");
              client.println("<button class='btn btn-dark btn-lg' onclick=\"location.href='/derecha'\"><i class='fas fa-caret-right fa-2x'></i></button>");
              client.println("</div>");
              client.println("<button class='btn btn-dark btn-lg' onclick=\"location.href='/atras'\"><i class='fas fa-caret-down fa-2x'></i></button>");
              client.println("<button class='btn btn-danger btn-lg mt-3' onclick=\"location.href='/detener'\">Detener</button>");
              client.println("</div>");
              client.println("</div>");
              client.println("<script src='https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js'></script>");
              client.println("<script src='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/js/all.min.js'></script>");
              // client.println("<script>document.querySelectorAll('button').forEach(btn => { btn.onclick = function() { btn.disabled = true; setTimeout(() => btn.disabled = false, 1000); }; });</script>");
              client.println("</body>");
              client.println("</html>");
              client.println();}

            // Verificar si han pasado 30 segundos
            unsigned long tiempoActual = millis();
            if (tiempoActual - ultimaCaptura >= 30000) {
              capturaTemperatura();
              ultimaCaptura = tiempoActual;
            }

              break;
          }else { // if you got a newline, then clear currentLine
          currentLine = "";
          } 
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    delay(1);
    client.stop(); 
    Serial.println("Cliente desconectado.");
}
}

// Función: Adelante
void adelante() {
  moverCarro(255, LOW, HIGH, LOW, HIGH, "Adelante");
}

// Función: Atrás
void atras() {
  moverCarro(255, HIGH, LOW, HIGH, LOW, "Atrás");
}

// Función: Derecha
void derecha() {
  moverCarro(150, LOW, HIGH, HIGH, LOW, "Derecha");
}

// Función: Izquierda
void izquierda() {
  moverCarro(150, HIGH, LOW, LOW, HIGH, "Izquierda");
}

// Función: Detener
void detener() {
  moverCarro(0, LOW, LOW, LOW, LOW, "Detenido");
}

// Función para mover el carro
void moverCarro(int pwm, int d1, int d2, int i1, int i2, String direccion) {
  analogWrite(derecha_pwm, pwm);
  analogWrite(izquierda_pwm, pwm);

  digitalWrite(derecha_1, d1);
  digitalWrite(derecha_2, d2);
  digitalWrite(izquierda_1, i1);
  digitalWrite(izquierda_2, i2);

  Serial.println("Moviendo " + direccion);
}

// Funcion de temperatura
void capturaTemperatura() {
  int valorArduino = analogRead(A0);
  float voltaje = (valorArduino / 1024.0) * 5.0;
  temperatura = (voltaje) * 100;
  
  char query[256]; 
  //sprintf(query, "USE byfcaqpyah9l6i1xrpj2; INSERT INTO Temperatura(temperatura, ubicacion) VALUES(%d, '19M');", (int)temperatura);
  sprintf(query, "USE carro; INSERT INTO Temperatura(temperatura, ubicacion) VALUES(%d, '19M');", (int)temperatura);
  
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  cur_mem->execute(query);
  delete cur_mem;
}