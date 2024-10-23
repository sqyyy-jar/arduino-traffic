#define DELAY_PD_ONLY 10 /* time in which only pedestrian lights are green */
#define DELAY_YELLOW_RED 10 /* time in which car lights are yellow red */
#define DELAY_BOTH 10 /* time in which both pedestrian and car lights are green */
#define DELAY_CAR_ONLY 10 /* time in which only car lights are green */
#define DELAY_YELLOW 10 /* time in which car lights are yellow */
#define DELAY_ALL_RED 10 /* time in which all lights are red */

typedef struct {
	byte red;
	byte yellow;
	byte green;
} car_light_t;

typedef struct {
	byte red;
	byte green;
} pd_light_t;

typedef struct {
	car_light_t *car_lights;
	int car_light_count;
	pd_light_t *pd_lights;
	int pd_light_count;
} stage_t;

typedef struct {
	stage_t *stages;
	int stage_count;
} traffic_t;

void car_light_red(car_light_t *light) {
	digitalWrite(light->red, HIGH);
	digitalWrite(light->yellow, LOW);
	digitalWrite(light->green, LOW);
}

void car_light_yellow_red(car_light_t *light) {
	digitalWrite(light->red, HIGH);
	digitalWrite(light->yellow, HIGH);
	digitalWrite(light->green, LOW);
}

void car_light_yellow(car_light_t *light) {
	digitalWrite(light->red, LOW);
	digitalWrite(light->yellow, HIGH);
	digitalWrite(light->green, LOW);
}

void car_light_green(car_light_t *light) {
	digitalWrite(light->red, LOW);
	digitalWrite(light->yellow, LOW);
	digitalWrite(light->green, HIGH);
}

void pd_light_red(pd_light_t *light) {
	digitalWrite(light->red, HIGH);
	digitalWrite(light->green, LOW);
}

void pd_light_green(pd_light_t *light) {
	digitalWrite(light->red, LOW);
	digitalWrite(light->green, HIGH);
}

void stage_setup(stage_t *stage) {
	for (int index = 0; index < stage->car_light_count; index++) {
		car_light_red(&stage->car_lights[index]);
	}
	for (int index = 0; index < stage->pd_light_count; index++) {
		pd_light_red(&stage->pd_lights[index]);
	}
}

void stage_tick(stage_t *stage) {
	if (stage->pd_light_count > 0) {
		for (int index = 0; index < stage->pd_light_count; index++) {
			pd_light_green(&stage->pd_lights[index]);
		}
		delay(DELAY_PD_ONLY);
	}
	for (int index = 0; index < stage->car_light_count; index++) {
		car_light_yellow_red(&stage->car_lights[index]);
	}
	delay(DELAY_YELLOW_RED);
	for (int index = 0; index < stage->car_light_count; index++) {
		car_light_green(&stage->car_lights[index]);
	}
	delay(DELAY_BOTH);
	for (int index = 0; index < stage->pd_light_count; index++) {
		pd_light_red(&stage->pd_lights[index]);
	}
	delay(DELAY_CAR_ONLY);
	for (int index = 0; index < stage->car_light_count; index++) {
		car_light_yellow(&stage->car_lights[index]);
	}
	delay(DELAY_YELLOW);
	for (int index = 0; index < stage->car_light_count; index++) {
		car_light_red(&stage->car_lights[index]);
	}
}

void traffic_run(traffic_t *traffic) {
	if (traffic->stage_count <= 0) {
		return;
	}
	int stage_index = 0;
	while (true) {
		stage_t *stage = &traffic->stages[stage_index];
		stage_tick(stage);
		if (++stage_index >= traffic->stage_count) {
			stage_index = 0;
		}
	}
}

traffic_t traffic = {
	.stages = (stage_t []){
		{
			.car_lights = {},
			.car_light_count = 0,
			.pd_lights = {},
			.pd_light_count = 0
		},
	},
	.stage_count = 1,
};

void setup() {
	traffic_run(&traffic);
}

void loop() {
}
