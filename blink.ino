#define NUM_BITS 4

// LEDs from D2 → D9  (LSB → MSB)
int ledPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 8; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  Serial.println("\n=== Booth's Algorithm 4x4 Multiplier ===");
  Serial.println("Enter A (4-bit binary, multiplicand): ");
}

void loop() {
  static String A_str = "";
  static String B_str = "";
  static bool gotA = false;

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (!isValid(input)) {
      Serial.println("❌ Invalid input! Enter exactly 4 bits (0/1).");
      return;
    }

    if (!gotA) {
      // ---------- FIRST INPUT ----------
      A_str = input;
      gotA = true;
      showBits(A_str, 6); // Show A on D9–D6
      Serial.println("✅ Stored A = " + A_str);
      Serial.println("Enter B (4-bit binary, multiplier): ");
    } 
    else {
      // ---------- SECOND INPUT ----------
      B_str = input;
      showBits(B_str, 2); // Show B on D5–D2
      Serial.println("✅ Stored B = " + B_str);
      Serial.println("🕐 Displaying input mapping on LEDs...");

      // ✨ Delay added here so faculty can see B’s mapping on LEDs
      delay(4000);  // 4 seconds pause — change to any value you want (e.g. 5000 for 5s)

      // ---------- MULTIPLICATION STAGE ----------
      int A = binToSigned(A_str);
      int B = binToSigned(B_str);
      int product = boothMultiply(A, B);

      String prodBin = decToBinSigned(product, 8);
      Serial.println("\n🧮 Product:");
      Serial.print("Decimal: ");
      Serial.println(product);
      Serial.print("Binary:  ");
      Serial.println(prodBin);

      showAllBits(prodBin); // Display result on all 8 LEDs

      Serial.println("\nEnter next A (4-bit): ");
      gotA = false;
    }
  }
}

// ------------------- Utilities -------------------

bool isValid(String s) {
  if (s.length() != NUM_BITS) return false;
  for (int i = 0; i < s.length(); i++)
    if (s[i] != '0' && s[i] != '1') return false;
  return true;
}

// Convert binary string (4-bit two’s complement) → signed int
int binToSigned(String bin) {
  int val = 0;
  for (int i = 0; i < bin.length(); i++)
    val = (val << 1) | (bin[i] - '0');
  if (bin[0] == '1')  // negative number in two’s complement
    val -= (1 << bin.length());
  return val;
}

// Convert signed integer → two’s complement binary string
String decToBinSigned(int val, int bits) {
  if (val < 0) val = (1 << bits) + val;
  String s = "";
  for (int i = bits - 1; i >= 0; i--)
    s += ((val >> i) & 1) ? '1' : '0';
  return s;
}

// Show 4 bits starting from a given pin (start = 2 for B, 6 for A)
void showBits(String bits, int startPin) {
  for (int i = 0; i < 4; i++) {
    int pin = startPin + i;
    int bitVal = bits.charAt(3 - i) == '1' ? HIGH : LOW;
    digitalWrite(pin, bitVal);
  }
}

// Show 8 bits (MSB = D9, LSB = D2)
void showAllBits(String bits) {
  for (int i = 0; i < 8; i++) {
    int bitVal = bits.charAt(7 - i) == '1' ? HIGH : LOW;
    digitalWrite(ledPins[i], bitVal);
  }
}

// ------------------- Booth's Algorithm -------------------

int boothMultiply(int M, int Q) {
  int A = 0;
  int Qm1 = 0;
  int n = NUM_BITS;

  Serial.println("\nStep\tA\tQ\tQ-1\tOperation");
  for (int i = 0; i < n; i++) {
    int Q0 = Q & 1;
    String op = "NOP";
    if (Q0 == 1 && Qm1 == 0) {
      A = A - M;
      op = "A = A - M";
    } else if (Q0 == 0 && Qm1 == 1) {
      A = A + M;
      op = "A = A + M";
    }

    Serial.print(i + 1);
    Serial.print("\t");
    Serial.print(decToBinSigned(A, 4));
    Serial.print("\t");
    Serial.print(decToBinSigned(Q, 4));
    Serial.print("\t");
    Serial.print(Qm1);
    Serial.print("\t");
    Serial.println(op);

    // Arithmetic right shift
    Qm1 = Q & 1;
    int signA = (A < 0);
    Q = ((A & 1) << (n - 1)) | ((Q >> 1) & ((1 << (n - 1)) - 1));
    A = (A >> 1) | (signA << (n - 1));
  }

  return (A << n) | (Q & ((1 << n) - 1));
}
