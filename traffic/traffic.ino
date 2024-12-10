#include <SPI.h>
#include <RF24.h>

#define CE_PIN 48
#define CSN_PIN 49
// MISO 50
// MOSI 51
// SCK	52

#define CLIENT
#define TRAFFIC
#define MIN_RECV_DELAY 200
#define PEDESTRIAN_DELAY 1000
#define ORANGE_DELAY 1000
#define GREEN_DELAY 1000
#define OVERLAP_DELAY 1000
#define YELLOW_DELAY 1000
#define RED_DELAY 1000

struct light2_t {
	byte green;
	byte red;
};

struct light3_t {
	byte green;
	byte yellow;
	byte red;
};

void light2_green(light2_t *light) {
	digitalWrite(light->green, HIGH);
	digitalWrite(light->red, LOW);
}

void light2_red(light2_t *light) {
	digitalWrite(light->green, LOW);
	digitalWrite(light->red, HIGH);
}

void light3_green(light3_t *light) {
	digitalWrite(light->red, LOW);
	digitalWrite(light->yellow, LOW);
	digitalWrite(light->green, HIGH);
}

void light3_yellow(light3_t *light) {
	digitalWrite(light->red, LOW);
	digitalWrite(light->yellow, HIGH);
	digitalWrite(light->green, LOW);
}

void light3_red(light3_t *light) {
	digitalWrite(light->red, HIGH);
	digitalWrite(light->yellow, LOW);
	digitalWrite(light->green, LOW);
}

void light3_orange(light3_t *light) {
	digitalWrite(light->red, HIGH);
	digitalWrite(light->yellow, HIGH);
	digitalWrite(light->green, LOW);
}

enum trafficstate_t {
	/* green pedestrian light */
	horizontal_1,
	/* orange car light */
	horizontal_2,
	/* green car light */
	horizontal_3,
	/* red pedestrian light */
	horizontal_4,
	/* yellow car light */
	horizontal_5,
	/* red car light */
	horizontal_6,
	/* green pedestrian light */
	vertical_1,
	/* orange car light */
	vertical_2,
	/* green car light */
	vertical_3,
	/* red pedestrian light */
	vertical_4,
	/* yellow car light */
	vertical_5,
	/* red car light */
	vertical_6,
	/* all red */
	emergency,
};

enum signal_t {
	emergency_start,
	emergency_end,
};

struct payload_t {
	signal_t signal;
};

// horizontal, vertical
const light2_t lights2[] = {
	{ 8, 9 },
	{ A2, A3 },
};
// rechts, links, unten, oben
const light3_t lights3[] = {
	{ 2, 3, 4 },
	{ 5, 6, 7 },
	{ 10, 11, 12 },
	{ 13, 20, 21 },
};

RF24 radio(CE_PIN, CSN_PIN);
char addresses[][6] = { "1Node", "2Node" };
payload_t payload;
trafficstate_t state;
trafficstate_t next_state;
long state_delay;

void init_pin(byte pin) {
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
}

void setup_lights(void) {
	for (int i = 0; i < 2; i++) {
		init_pin(lights2[i].green);
		init_pin(lights2[i].red);
	}
	for (int i = 0; i < 4; i++) {
		init_pin(lights3[i].green);
		init_pin(lights3[i].yellow);
		init_pin(lights3[i].red);
	}
}

void setup_radio(void) {
	if (!radio.begin()) {
		Serial.println(F("radio hardware is not responding"));
		while (1) {}
	}
	radio.setPALevel(RF24_PA_LOW);
	radio.setPayloadSize(sizeof(payload_t));
	radio.openWritingPipe(&addresses[1]);
	radio.openReadingPipe(1, &addresses[0]);
}

void setup_broadcaster(void) {
	setup_radio();
	radio.stopListening();
}

void setup_client(void) {
	setup_radio();
	radio.startListening();
}

void setup(void) {
	Serial.begin(9600);
	setup_lights();
#ifdef CLIENT
	setup_client();
#else
	setup_broadcaster();
#endif
}

void handle_payload(void) {
	switch (payload.signal) {
	case emergency_start:
		next_state = emergency;
		state_delay = 0;
		break;
	case emergency_end:
		next_state = horizontal_1;
		state_delay = RED_DELAY;
		break;
	}
}

void tick_traffic(void) {
	state = next_state;
	switch (state) {
	case horizontal_1:
		light2_green(&lights2[0]);
		next_state = horizontal_2;
		state_delay = PEDESTRIAN_DELAY;
		break;
	case horizontal_2:
		light3_orange(&lights3[0]);
		light3_orange(&lights3[1]);
		next_state = horizontal_3;
		state_delay = ORANGE_DELAY;
		break;
	case horizontal_3:
		light3_green(&lights3[0]);
		light3_green(&lights3[1]);
		next_state = horizontal_4;
		state_delay = OVERLAP_DELAY;
		break;
	case horizontal_4:
		light2_red(&lights2[1]);
		next_state = horizontal_5;
		state_delay = GREEN_DELAY;
		break;
	case horizontal_5:
		light3_yellow(&lights3[0]);
		light3_yellow(&lights3[1]);
		next_state = horizontal_6;
		state_delay = YELLOW_DELAY;
		break;
	case horizontal_6:
		light3_red(&lights3[0]);
		light3_red(&lights3[1]);
		next_state = vertical_1;
		state_delay = RED_DELAY;
		break;
	case vertical_1:
		light2_green(&lights2[1]);
		next_state = vertical_2;
		state_delay = PEDESTRIAN_DELAY;
		break;
	case vertical_2:
		light3_orange(&lights3[2]);
		light3_orange(&lights3[3]);
		next_state = vertical_3;
		state_delay = ORANGE_DELAY;
		break;
	case vertical_3:
		light3_green(&lights3[2]);
		light3_green(&lights3[3]);
		next_state = vertical_4;
		state_delay = OVERLAP_DELAY;
		break;
	case vertical_4:
		light2_red(&lights2[1]);
		next_state = vertical_5;
		state_delay = GREEN_DELAY;
		break;
	case vertical_5:
		light3_yellow(&lights3[2]);
		light3_yellow(&lights3[3]);
		next_state = vertical_6;
		state_delay = YELLOW_DELAY;
		break;
	case vertical_6:
		light3_red(&lights3[2]);
		light3_red(&lights3[3]);
		next_state = horizontal_1;
		state_delay = RED_DELAY;
		break;
	case emergency:
		light2_red(&lights2[0]);
		light2_red(&lights2[1]);
		light3_red(&lights3[0]);
		light3_red(&lights3[1]);
		light3_red(&lights3[2]);
		light3_red(&lights3[3]);
		next_state = emergency;
		state_delay = 200;
		break;
	}
}

void tick_broadcaster(void) {
	long start = millis();
	while (millis() - start < state_delay) {
		bool result = radio.write(&payload, sizeof(payload_t));
		if (!result) {
			Serial.println(F("Transmission failed or timed out"));
		}
	}
}

void tick_client(void) {
	long start = millis();
	long diff;
	while (diff = millis() - start, diff < state_delay) {
		uint8_t port;
		if (!radio.available(&port)) {
			continue;
		}
		radio.read(&payload, sizeof(payload_t));
		Serial.print("received payload: ");
		Serial.println(payload.signal);
		if (diff > MIN_RECV_DELAY) {
			delay(MIN_RECV_DELAY);
		} else {
			delay(diff);
			break;
		}
	}
}

void loop(void) {
#ifdef CLIENT
	tick_client();
#else
	tick_broadcaster();
#endif
#ifdef TRAFFIC
	tick_traffic();
#endif
}
