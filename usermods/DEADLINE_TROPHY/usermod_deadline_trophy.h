#pragma once

#define DEADLINE_VALUES_STRLEN 200

class DeadlineTrophyUsermod : public Usermod
{
public:

    static const int PIN_LOGOTHERM = 35;
    static const int PIN_INPUTVOLTAGE = 33;
    // static const int PIN_VCAP = 32; // unused, it seems

    // qm210: these are hacked into the AudioReactive UserMod because we are so loco
    static const int PIN_AR_SD = 12;
    static const int PIN_AR_WS = 13;
    static const int PIN_AR_CLK = 14;

    // for the analogRead(), average over some samples to smoothen fluctuations.
    static const int AVERAGE_SAMPLES = 10;

    /*
        logoTherm goes via voltage divider with R6, Thermistor TH1, R7

        R6: 15kOhm (Upper, Above the Division)
        TH1: ERT-J0EM103J (Thermistor, Below the Division) - R = 10 kOhm @ 25°, B-Value ~ 3900 K
        R7: 1kOhm (Lower, Below the Division, in Series with the Thermistor)
        Input Voltage V_CC = 5V in an ideal world

        Voltage Division:
        V_Therm = V_CC * (TH1 + R7) / (R6 + TH1 + R7) = V_CC * R6 / R_ges
        -> TH1 = 1 / (V_CC / V_Therm - 1) * R6 - R7

        12-bit ADC: (is somewhat non-linear, but who the shit cares.)
        V_Therm = 3.3V * AnalogReadResult / 4095 = AnalogReadResult * voltageAdcCoeff;

        -> TH1 = 1 / (V_CC / voltageAdcCoeff / AnalogReadResult - 1) * R6 - R7

        Steinhart-Hart (first-order, we are not THAT hart).
        1/T = 1/T0 + 1/B * ln(TH1 / R0)
        T0 = 298.15 K (nominal temperature where R(TH1) = R0)
        R0 = 1e4 Ohm (nominal resistance at T = T0)

        -> T = 1. / ( invT0 + invB * ln(TH1 / R0) ) in Kelvin

        it seems like we can use logf(), the float-valued natural logarithm.
    */
    static constexpr float logoR6_Ohm = 1.5e4;
    static constexpr float logoR7_Ohm = 1e3;
    static constexpr float logoThermistor_R0_Ohm = 1e4;
    static constexpr float logoTherm_OneOverT0 = 1. / (273.15 + 25);
    static constexpr float logoTherm_OneOverB = 1. / 3900.;
    static constexpr float voltageAdcCoeff = 3.3 / 4095.; // 12bit ADC

    /*
        input voltage V_CC goes via voltage divider with R23 = R24, i.e.
        V_pin33 = V_CC * (R23) / (R23 + R24)
        -> V_CC = 2. * V_pin33
                = 2. * (AnalogRead33Result) / 4095 * 3.3V
                = 2. * (AnalogRead33Result) * voltageAdcCoeff

    */

    const float maxCurrent =
        #ifdef USE_DEADLINE_4A_POWER_SUPPLY
            4.0;
        #else
            1.0;
        #endif

private:

    // analog readings
    uint16_t val_logoTherm[AVERAGE_SAMPLES];
    bool hasEnoughSamples = false;
    int sampleCursor = 0;
    float currentLogoTempKelvin = 0.;
    float maxLogoTempKelvin = 0.;
    float minLogoTempKelvin = 9999.;

    uint16_t val_inputVolt[AVERAGE_SAMPLES];
    float currentInputVoltage = 0.;
    float maxInputVoltage = 0.;
    float minInputVoltage = 9999.;

    int valueForInputCurrentSwitch = 0;
    float inputCurrentSwitch_waitSeconds = 1.;
    float inputCurrentSwitch_riseSeconds = 1.;

    long lastDebugOutAt;
    long now;
    float runningSec;
    float elapsedSec;
    float justBefore;

    // logo temp values
    float _logoRead;
    float _voltageRatio;
    float _R_TH1;
    float _logRatio;

    // current temp values
    float _vccRead;

    // limit maximum current / brightness to avoid damage
    float maxA = 0.;

    float limit_inputVoltageThreshold = 4.6;
    bool limit_becauseVoltageDrop = false;
    bool limit_becauseTooHot = false;

    long inputVoltageThresholdReachedAt;
    bool inputVoltageWasReachedOnce = false;

    // Limit by Input Voltage, implemented with guessed values
    float maxA_attackFactorWhenCritical = 0.5;
    float maxA_releasePerSecond = 0.05;

    // Limit by Temperature, NOT YET IMPLEMENTED!
    float maxA_limitWhenTooHot = 0.7;
    float limit_cricitcalTemperature = 60;

public:

    void setup()
    {
        DEBUG_PRINTF("[DEADLINE] QM watches you! (non-creepily.) Say Hi at qm@z10.info\n");

        DEBUG_PRINTLN("[USERMOD_DEADLINE] TODO: Watch Temperature!");

        justBefore = millis();
        runningSec = 0;
        lastDebugOutAt = 0;

        hasEnoughSamples = false;
        inputVoltageWasReachedOnce = false;
        maxA = 0.;

        // 12-bit ADC is the default, but let's go sure (do we need? no idea.)
        analogSetWidth(12);

        // set InputCurrentSwitch to zero.
        dacWrite(DAC1, 0);
    }

    void loop()
    {
        now = millis();
        elapsedSec = 1e-3 * (now - justBefore);
        runningSec += elapsedSec;

        maxA = calcMaxCurrentLimiter();
        // this does the actual limitation:
        strip.ablMilliampsMax = static_cast<uint16_t>(maxA);

        readAnalogValues();
        if (!hasEnoughSamples)
            return;

        calcInputVoltage();
        calcLogoTherm();

        if (inputVoltageWasReachedOnce) {
            setInputCurrentSwitch(runningSec);
        }

        // these are for debugging
        if (currentLogoTempKelvin > maxLogoTempKelvin) {
            maxLogoTempKelvin = currentLogoTempKelvin;
        }
        if (currentLogoTempKelvin < minLogoTempKelvin) {
            minLogoTempKelvin = currentLogoTempKelvin;
        }
        if (currentInputVoltage > maxInputVoltage) {
            maxInputVoltage = currentInputVoltage;
        }
        if (currentInputVoltage < minInputVoltage) {
            minInputVoltage = currentInputVoltage;
        }

        /*
        if (now - lastDebugOutAt > 20000 || lastDebugOutAt == 0)
        {
            DEBUG_PRINTF("[DEADLINE_TROPHY] logoTemp=%f K (min %f, max %f) [DEBUG %f, %f, %f, %f]\n",
                currentLogoTempKelvin,
                minLogoTempKelvin,
                maxLogoTempKelvin,
                _logoRead,
                _voltageRatio,
                _R_TH1,
                _logRatio
            );
            DEBUG_PRINTF("[DEADLINE_TROPHY] inputV=%f (min %f, max %f) [DEBUG %f]\n",
                currentInputVoltage,
                maxInputVoltage,
                minInputVoltage,
                _vccRead
            );
            lastDebugOutAt = now;
        }
        */

       justBefore = now;
    }

    void setInputCurrentSwitch(float runningSec) {
        float linearRise = (runningSec - inputCurrentSwitch_waitSeconds) / inputCurrentSwitch_riseSeconds;
        valueForInputCurrentSwitch = constrain(
            static_cast<int>(255 * linearRise),
            0,
            255
        );
        dacWrite(DAC1, valueForInputCurrentSwitch);
    }

    float getAverage(uint16_t samples[])
    {
        float avg = 0;
        for (int s = 0; s < AVERAGE_SAMPLES; s++) {
            avg += samples[s];
        }
        return avg / AVERAGE_SAMPLES;
    }

    void readAnalogValues()
    {
        // read with a delay, as ppl on se internet do it - they must know =P
        val_logoTherm[sampleCursor] = analogRead(PIN_LOGOTHERM);
        delay(10);
        val_inputVolt[sampleCursor] = analogRead(PIN_INPUTVOLTAGE);
        delay(10);

        sampleCursor++;

        if (sampleCursor >= AVERAGE_SAMPLES)
        {
            hasEnoughSamples = true;
            sampleCursor = 0;
        }
        if (!hasEnoughSamples) {
            return;
        }
    }

    void calcInputVoltage()
    {
        _vccRead = getAverage(val_inputVolt);
        currentInputVoltage = 2. * voltageAdcCoeff * _vccRead;
    }

    void calcLogoTherm()
    {
        _logoRead = getAverage(val_logoTherm);
        _voltageRatio = currentInputVoltage / (voltageAdcCoeff * _logoRead);
        _R_TH1 = 1. / (_voltageRatio - 1) * logoR6_Ohm - logoR7_Ohm;
        _logRatio = logf(_R_TH1 / logoThermistor_R0_Ohm);
        currentLogoTempKelvin = 1. / ( logoTherm_OneOverT0 + logoTherm_OneOverB * _logRatio );
    }

    float calcMaxCurrentLimiter() {
        if (!hasEnoughSamples) {
            return 0;
        }
        if (!inputVoltageWasReachedOnce) {
            if (currentInputVoltage > limit_inputVoltageThreshold) {
                inputVoltageWasReachedOnce = true;
                inputVoltageThresholdReachedAt = now;
                limit_becauseVoltageDrop = false;
            } else {
                return 0;
            }
        }

        if (currentInputVoltage < limit_inputVoltageThreshold) {
            if (!limit_becauseVoltageDrop) {
                limit_becauseVoltageDrop = true;
                maxA *= maxA_attackFactorWhenCritical;
            } else {
                maxA += maxA_releasePerSecond;
            }
        }

        // TODO @qm210 implement temperature control

        return MIN(maxA, maxCurrent);
    }

    void addToConfig(JsonObject& doc)
    {
        JsonObject top = doc.createNestedObject(F("DeadlineTrophy"));
        // fun thing, fillUMPins() will look for entries in these "pin" arrays. deal with it.
        auto pins = top.createNestedArray("pin");
        pins.add(PIN_LOGOTHERM);
        pins.add(PIN_INPUTVOLTAGE);

        // TODO @qm210 make the maxA-limiter parameters configurable.

        auto lim = top.createNestedObject("lim");
        // auto lt_lim = lim.createNestedObject("temp");
        // lt_lim["Tstart"] = limitFunc_logoThermOffset;
        // lt_lim["mApK"] = limitFunc_logoThermSlope;
        // lt_lim["Tcrit"] = limitFunc_logoThermCritical;
        lim["Vlim"] = limit_inputVoltageThreshold;
        // auto a_in = lim.createNestedObject("A_in");
        // a_in["wait"] = 2.;
        // a_in["slop"] = 5.;
    }

    void appendConfigData()
    {
      oappend(SET_F("addInfo('DeadlineTrophy:pin[]',0,'Logo Temperature','LogoTherm');"));
      oappend(SET_F("addInfo('DeadlineTrophy:pin[]',1,'Common Voltage','VCC');"));
      // TODO:  APPEND CONFIG
    }

    bool readFromConfig(JsonObject& root)
    {
        // TODO @qm210: READ CONFIG

        // JsonObject top = root[FPSTR(_name)];
        return true;
    }

    uint16_t getId()
    {
        return USERMOD_ID_DEADLINE_TROPHY;
    }

    void printValueJson(char line[])
    {
        if (!hasEnoughSamples) {
            sprintf(line, "{\"error\": \"not enough samples taken yet.\"}");
            return;
        }
        // needs about 200 characters or something, I haven't counted.
        // --> define the DEADLINE_VALUES_STRLEN so this string fits
        sprintf(
            line,
            "{\"dl\": {\"T\": %.3f, \"minT\": %.3f, \"maxT\": %.3f, \"VCC\": %.3f, \"maxVCC\": %.3f, \"minVCC\": %.3f, \"adcT\": %.1f, \"adcV\": %.1f}}",
            currentLogoTempKelvin - 273.15,
            minLogoTempKelvin - 273.15,
            maxLogoTempKelvin - 273.15,
            currentInputVoltage,
            maxInputVoltage,
            minInputVoltage,
            _logoRead,
            _vccRead
        );
    }

};

#define GET_DEADLINE_USERMOD() ((DeadlineTrophyUsermod*)usermods.lookup(USERMOD_ID_DEADLINE_TROPHY))