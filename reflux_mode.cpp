//  Ректификация

#include "reflux_mode.h"
int	countHaedEnd;

void EEPROM_float_write_refl(int addr, float val) {
	byte *x = (byte *)&val;
	for (byte i = 0; i < 4; i++) EEPROM.write(i + addr, x[i]);
}
float EEPROM_float_read_refl(int addr) {
	byte x[4];
	for (byte i = 0; i < 4; i++) x[i] = EEPROM.read(i + addr);
	float *y = (float *)&x;
	return y[0];
}

void loadEepromReflux() {
	int i;
	// Считаем что раньше сохраняли
	uint16_t index = 1400;
	if (EEPROM.read(index) == 2) {
		index++;
		for (i = 0; i < 8; i++) {
			temperatureSensor[i].member = EEPROM.read(index);  index++;
			temperatureSensor[i].priority = EEPROM.read(index);  index++;
			temperatureSensor[i].allertValue = EEPROM_float_read_refl(index); index += 4;
			temperatureSensor[i].delta = EEPROM.read(index);  index++;
			temperatureSensor[i].cutoff = EEPROM.read(index);  index++;
		}
		for (i = 0; i < 8; i++) {
			pwmOut[i].member = EEPROM.read(index);  index++;
		}
		for (i = 0; i < 4; i++) {
			adcIn[i].member = EEPROM.read(index);  index++;
		}
	}
	else {
		for (i = 0; i < 8; i++) {
			temperatureSensor[i].member = 0;
			temperatureSensor[i].priority = 0;
			temperatureSensor[i].allertValue = 0;
			temperatureSensor[i].delta = 0;
			temperatureSensor[i].cutoff = 0;
		}
		for (i = 0; i < 8; i++) {
			pwmOut[i].member = 0;
		}
		for (i = 0; i < 4; i++) {
			adcIn[i].member = 0;
		}
	}
}

// Обмен с Web
void initReflux()
{
	HTTP.on("/refluxSensorsGetTpl", handleRefluxSensorTpl);			// Отправка топологии датчиков при входе на закладку
	HTTP.on("/refluxSensorsSetLoad", handleRefluxSensorSetLoad);    // Отправка - Добавить датчики для процесса
	HTTP.on("/refluxSensorsSetSave", handleRefluxSensorSetSave);	// Прием выбранных датчиков
}

// Отправка топологии датчиков при входе на закладку
void handleRefluxSensorTpl() {
	int i, k;

	loadEepromReflux();

	String dataForWeb = "{";
	// датчики температуры
#if defined Debug_en
	Serial.println(""); Serial.println("Топология отправка:");
#endif
	for (i = 1; i <= DS_Cnt; i++) {
		k = 0;
		while (1) {
			if (temperatureSensor[k].num == i) {
				dataForWeb += "\"t" + String(i) + "\":{\"value\":" + String(temperatureSensor[k].data);
				dataForWeb += ",\"name\":\"" + String(temperatureSensor[k].name) + "\",\"color\":" + String(temperatureSensor[k].color);
				dataForWeb += ",\"member\":" + String(temperatureSensor[k].member) + ",\"delta\":" + String(temperatureSensor[k].delta);
				dataForWeb += ",\"cutoff\":" + String(temperatureSensor[k].cutoff) + ",\"priority\":" + String(temperatureSensor[k].priority);
				dataForWeb += ",\"allertValue\":" + String(temperatureSensor[k].allertValueIn) + "},";
				break;
			}
			if (k < 7) k++;
			else break;
		}
#if defined Debug_en
		Serial.print(i); Serial.print(" "); Serial.println(k);
		Serial.print("Номер: ");  Serial.println(temperatureSensor[k].num);
		Serial.print("Участвует: ");  Serial.println(temperatureSensor[k].member);
		Serial.print("Уставка: ");  Serial.println(temperatureSensor[k].allertValueIn);//Serial.println(temperatureSensor[k].allertValue);
#endif
	}
	// выходы ШИМ
	for (i = 0; i < 8; i++) {
		dataForWeb += "\"out" + String(i + 1) + "\":{\"value\":" + String(pwmOut[i].data) + ",\"name\":\"" + String(pwmOut[i].name) + "\",\"member\":" + String(pwmOut[i].member) + "},";
	}
	// входы АЦП
	dataForWeb += "\"in1\":{\"value\":" + String(adcIn[0].data) + ",\"name\":\"" + String(adcIn[0].name) + "\",\"member\":" + String(adcIn[0].member) + "},";
	dataForWeb += "\"in2\":{\"value\":" + String(adcIn[1].data) + ",\"name\":\"" + String(adcIn[1].name) + "\",\"member\":" + String(adcIn[0].member) + "},";
	dataForWeb += "\"in3\":{\"value\":" + String(adcIn[2].data) + ",\"name\":\"" + String(adcIn[2].name) + "\",\"member\":" + String(adcIn[0].member) + "},";
	dataForWeb += "\"in4\":{\"value\":" + String(adcIn[3].data) + ",\"name\":\"" + String(adcIn[3].name) + "\",\"member\":" + String(adcIn[0].member) + "}}";
	HTTP.send(200, "text/json", dataForWeb);
}
// Отправка - Добавить датчики для процесса
void handleRefluxSensorSetLoad() {
	int i, k;
	loadEepromReflux();
	String dataForWeb = "{";
	// датчики температуры
#if defined Debug_en
	Serial.println(""); Serial.println("Расстановка отправка:");
#endif
	for (i = 1; i <= DS_Cnt; i++) {
		k = 0;
		while (1) {
			if (temperatureSensor[k].num == i) {
				dataForWeb += "\"t" + String(i) + "\":{\"value\":" + String(temperatureSensor[k].data);
				dataForWeb += ",\"name\":\"" + String(temperatureSensor[k].name) + "\",\"color\":" + String(temperatureSensor[k].color);
				dataForWeb += ",\"member\":" + String(temperatureSensor[k].member) + ",\"priority\":" + String(temperatureSensor[k].priority);
				dataForWeb += ",\"allertValue\":" + String(temperatureSensor[k].allertValue) + "},";
				break;
			}
			if (k < 7) k++;
			else break;
		}
#if defined Debug_en
		Serial.print(i); Serial.print(" "); Serial.println(k);
		Serial.print("Номер: ");  Serial.println(temperatureSensor[k].num);
		Serial.print("Участвует: ");  Serial.println(temperatureSensor[k].member);
#endif
	}
	// выходы ШИМ
	for (i = 0; i < 8; i++) {
		dataForWeb += "\"out" + String(i + 1) + "\":{\"value\":" + String(pwmOut[i].data) + ",\"name\":\"" + String(pwmOut[i].name) + "\",\"member\":" + String(pwmOut[i].member) + "},";
	}
	// входы АЦП
	dataForWeb += "\"in1\":{\"value\":" + String(adcIn[0].data) + ",\"name\":\"" + String(adcIn[0].name) + "\",\"member\":" + String(adcIn[0].member) + "},";
	dataForWeb += "\"in2\":{\"value\":" + String(adcIn[1].data) + ",\"name\":\"" + String(adcIn[1].name) + "\",\"member\":" + String(adcIn[0].member) + "},";
	dataForWeb += "\"in3\":{\"value\":" + String(adcIn[2].data) + ",\"name\":\"" + String(adcIn[2].name) + "\",\"member\":" + String(adcIn[0].member) + "},";
	dataForWeb += "\"in4\":{\"value\":" + String(adcIn[3].data) + ",\"name\":\"" + String(adcIn[3].name) + "\",\"member\":" + String(adcIn[0].member) + "}}";
	HTTP.send(200, "text/json", dataForWeb);
}
// Прием выбранных датчиков
void handleRefluxSensorSetSave() {
	String arg;
	uint16_t index = 1400;
	int i, k;
	// парсим ответ от браузера в переменные
#if defined Debug_en
	Serial.println(""); Serial.println("Расстановка прием:");
#endif
	for (i = 1; i <= DS_Cnt; i++) {
		k = 0;
		while (1) {
			if (temperatureSensor[k].num == i) {
				arg = "t" + String(i);
				temperatureSensor[k].member = HTTP.arg(arg + "[member]").toInt();
				temperatureSensor[k].priority = HTTP.arg(arg + "[priority]").toInt();
				temperatureSensor[k].allertValue = HTTP.arg(arg + "[allertValue]").toFloat();
				temperatureSensor[k].delta = HTTP.arg(arg + "[delta]").toInt();
				temperatureSensor[k].cutoff = HTTP.arg(arg + "[cutoff]").toInt();
				break;
			}
			if (k < 7) k++;
			else break;
		}
#if defined Debug_en		
		Serial.print(i); Serial.print(" "); Serial.println(k);
		Serial.print("Номер: ");  Serial.println(temperatureSensor[k].num);
		Serial.print("Участвует: ");  Serial.println(temperatureSensor[k].member);
#endif		
	}
	for (i = 0; i < 8; i++) {
		arg = "out" + String(i + 1);
		pwmOut[i].member = HTTP.arg(arg + "[member]").toInt();
	}
	for (i = 0; i < 4; i++) {
		arg = "in" + String(i + 1);
		adcIn[i].member = HTTP.arg(arg + "[member]").toInt();
	}
	HTTP.send(200, "text/json", "{\"result\":\"ok\"}");

	// сохраним в EEPROM
	EEPROM.write(index, 0x02); index++; // 2-й процесс = ректификация
	// Датчики температуры
	for (i = 0; i < 8; i++) {
		EEPROM.write(index, temperatureSensor[i].member);  index++;
		EEPROM.write(index, temperatureSensor[i].priority);  index++;
		EEPROM_float_write_refl(index, temperatureSensor[i].allertValue); index += 4;
		EEPROM.write(index, temperatureSensor[i].delta); index++;
		EEPROM.write(index, temperatureSensor[i].cutoff); index++;
	}
	for (i = 0; i < 8; i++) {
		EEPROM.write(index, pwmOut[i].member);  index++;
	}
	for (i = 0; i < 4; i++) {
		EEPROM.write(index, adcIn[i].member);  index++;
	}

	EEPROM.commit();
	delay(100);
}

void refluxLoop() {
	float temperatureTubeCurPressure;
	// уставку применим только после достижения 90 гр. в кубе
	if (settingBoilTube != 0) {
		temperatureStartPressure = settingColumn - (760 - pressureSensor.data)*0.037;
		temperatureTubeCurPressure = temperatureSensor[DS_Tube].data - (760 - pressureSensor.data)*0.037;
		temperatureSensor[DS_Tube].allertValue = temperatureStartPressure + settingBoilTube + (temperatureSensor[DS_Tube].data - temperatureTubeCurPressure);
	}
	else temperatureSensor[DS_Tube].allertValue = 0;

	switch (processMode.step) {
		// пришли при старте ректификации
		case 0: {
			loadEepromReflux();
	#if defined TFT_Display
			// подготовка данных для вывода на TFT
			csOn(TFT_CS);
			//tft.fillScreen(ILI9341_BLACK);
			tftStartForGraph();
			displayTimeInterval = millis() + 1000;
			DefCubOut = Display_out_temp;
			csOff(TFT_CS);
	#endif
			//csOn(PWM_CH1);			// открыть клапан отбора
			heaterStatus = 1;		// включили нагрев
			heaterPower = 100;		// установили мощность на ТЭН 100 %
			processMode.step = 1;	// перешли на следующий шаг алгоритма
			countHaedEnd = 0;
			break;
		}
		// ждем начала подъема температуры в царге и включаем воду на охлаждение и понижаем мощность на ТЭН
		case 1: {
			if (temperatureSensor[DS_Tube].data >= 45.0) {
				csOn(PWM_CH2);				// включаем клапан подачи воды
				heaterPower = 65;			// установили мощность на ТЭН 65 %
				settingAlarm = true;		// подаем звуковой сигнал
				timePauseOff = millis();	// обнулим счетчик времени для зв.сигнала
				processMode.step = 2;		// перешли на следующий шаг алгоритма
			}
			break;
		}
		// пищалка на 10 сек.
		case 2: {
			if (timePauseOff < millis() && millis() >= 10000 + timePauseOff) {
				settingAlarm = false;	// выключили звуковой сигнал
				processMode.step = 3;	// перешли на следующий шаг алгоритма
			}
			break;
		}
		// ждем окончание стабилизации 20 минут и увеличиваем мощность для отбора голов
		case 3: {
			if (timePauseOff < millis() && (millis() - timePauseOff) >= 1190000) {
				settingAlarm = true;	// подаем звуковой сигнал
				heaterPower = 65;		// установили мощность на ТЭН 65 %
				processMode.step = 4;	// перешли на следующий шаг алгоритма
			}
			break;
		}
		// пищалка на 10 сек.
		case 4: {
			if (timePauseOff < millis() && (millis() - timePauseOff) >= 10000) {
				settingAlarm = false;	// выключили звуковой сигнал
				if (adcIn[0].member == 1) {		// датчик уровня присутствует
					processMode.step = 5;		// перешли на следующий шаг алгоритма отбора голов
				}
				else {
					processMode.step = 6;		// пропустили следующий шаг алгоритма т.к. нет датчика уровня
				}
			}
			break;
		}
		// ждем срабатывание датчика уровня в приемной емкоси голов если он есть, включаем пищалку, поднимаем мощность ТЭН для отбора
		case 5: {
			if (adcIn[0].allert == true) {
				countHaedEnd++;
				if (countHaedEnd > 10) {		// антирдебезг :)
					settingAlarm = true;		// подаем звуковой сигнал
					timePauseOff = millis();	// обнулим счетчик времени для зв.сигнала
					processMode.step = 6;		// перешли на следующий шаг алгоритма
				}
			}
			else countHaedEnd = 0;

//   НОВОЕ  ....................................................................................................................
			// рулим клапаном на отборе голов
			if (headValveOff <= millis()) {
				csOn(PWM_CH1);		// открыть клапан отбора
				headValveOn = millis() + headValveOpen;		// обнулим счетчик времени для открытого состояния клапана
				headValve = true;
			}

			if (headValveOn > millis()) {
				while (headValveOn > millis()) {			// ждем пока клапан открыт
					yield();
				}
			}

			if (headValve == true) {
				csOff(PWM_CH1);									// закрыли клапан отбора
				headValveOff = millis() + headValveClose;		// обнулим счетчик времени для закрытого состояния клапана
				headValve = false;
			}
//			....................................................................................................................

			if (processMode.step == 6) csOn(PWM_CH1);		// открыть клапан отбора
			break;
		}
		// ждем окончание по достижению температуры в кубе и рулим клапаном отбора
		case 6: {
			if (temperatureSensor[DS_Cube].allertValue > 0 && temperatureSensor[DS_Cube].data >= temperatureSensor[DS_Cube].allertValue) {
				heaterStatus = 0;							// выключили ТЭН
				heaterPower = 0;							// установили мощность на ТЭН 0 %
				timePauseOff = millis();					// обнулим счетчик времени для зв.сигнала
				temperatureSensor[DS_Cube].allert = true;	// сигнализация для WEB
				settingAlarm = true;						// подаем звуковой сигнал
				csOff(PWM_CH1);								// закрыли клапан отбора
				processMode.step = 7;						// перешли на следующий шаг алгоритма
			}
			else if (temperatureSensor[DS_Tube].allertValue > 0 && temperatureSensor[DS_Tube].data >= temperatureSensor[DS_Tube].allertValue) {
				//temperatureSensor[DS_Tube].allert = true;	// сигнализация для WEB
				settingAlarm = true;						// подаем звуковой сигнал
				csOff(PWM_CH1);								// закрыли клапан отбора
				//temperatureTubeRect = temperatureSensor[DS_Tube].data - temperatureSensor[DS_Tube].allertValue;	// запомнили температуру с учетом гистерезиса уставки
			}
			else if (adcIn[0].allert == true) settingAlarm = true;	// подаем звуковой сигнал
			else settingAlarm = false;								// выключили звуковой сигнал

			if (temperatureSensor[DS_Tube].data <= temperatureSensor[DS_Tube].allertValue - settingBoilTube) {
				csOn(PWM_CH1);	// открыли клапан отбора
				if (adcIn[0].allert == false) settingAlarm = false;
			}

			break;
		}
		// после завершения процесса ждем 120 сек. и выключаем клапана и пищалку
		case 7: {
			if (millis() >= 10000 + timePauseOff) {
				settingAlarm = false;						// выключили звуковой сигнал
			}
			if (millis() >= 120000 + timePauseOff) {
				csOff(PWM_CH2);								// закрыли клапан подачи воды
				temperatureSensor[DS_Cube].allert = false;	// сигнализация для WEB
				processMode.allow = 0;						// вышли из режима ректификации
				processMode.step = 0;						// обнулили шаг алгоритма
			}
			break;
		}
	}
}
