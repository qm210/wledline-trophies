#pragma once

#include "WiFiUdp.h"

#define DEADLINE_VALUES_STRLEN 250

class DeadlineUsermod : public Usermod
{
public:

    static const int PIN_LOGOTHERM = 35;
    static const int PIN_INPUTVOLTAGE = 33;
    // static const int PIN_VCAP = 32; // unused, it seems

    // for the analogRead(), average over some samples to smoothen fluctuations.
    static const int AVERAGE_SAMPLES = 10;

    static constexpr float KELVIN_OFFSET = 273.15;
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
    static constexpr float logoTherm_OneOverT0 = 1. / (25. + KELVIN_OFFSET);
    static constexpr float logoTherm_OneOverB = 1. / 3900.;
    static constexpr float voltageAdcCoeff = 3.3 / 4095.; // 12bit ADC

    /*
        input voltage V_CC goes via voltage divider with R23 = R24, i.e.
        V_pin33 = V_CC * (R23) / (R23 + R24)
        -> V_CC = 2. * V_pin33
                = 2. * (AnalogRead33Result) / 4095 * 3.3V
                = 2. * (AnalogRead33Result) * voltageAdcCoeff

    */

    const float maxCurrent = DEADLINE_MAX_AMPERE * 1000;

    bool doSendUdp = true;
    bool keepSendingUdp = true;
    float sendUdpEverySec = 0.210; // try to find minimum that works
    // <-- 0: disable automatic sending, only via trigger from wherever you program it to
    bool doDebugLogUdp = false;
    bool doOneVerboseDebugLogUdp = false;

private:

    IPAddress udpSenderIp;
    uint16_t udpSenderPort;
    // <-- let's use the same port as local port as well as remote, 'cause why not.
    bool udpSenderConnected = false;
    WiFiUDP udpSender;
    static const unsigned int MAX_UDP_SIZE = 1024;
    byte udpPacket[MAX_UDP_SIZE];

    void sendTrophyUdp();

    float sendUdpInSec = 0;

    bool debugLogUdp = false;
    unsigned long lastLoggedUdpAt = 0;
    uint32_t debugColor = 0;
    bool printDebugColor = true;

    // for monitoring the (temperature values etc)
    char controlLoopValues[DEADLINE_VALUES_STRLEN];


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
    // this is multiplied on the brightness.
    // it didn't work with strip.ablMilliampsMax due to estimateCurrentAndLimitBri()
    float attenuateFactor = 1.;

    bool limit_becauseVoltageDrop = false;
    bool limit_becauseTooHot = false;

    long inputVoltageThresholdReachedAt;
    bool inputVoltageWasReachedOnce = false;
    float secondsSinceVoltageThresholdReached = 0.;

    float limit_inputVoltageThreshold = 4.6;
    // Limit by Input Voltage, implemented with guessed values
    float attenuateByV_attackFactorWhenCritical = 0.4;
    float attenuateByV_releasePerSecond = 0.005;

    float limit_criticalTempDegC = 60;
    // Limit by Temperature, also guessed values
    float attenuateByT_attackFactorWhenCritical = 0.2;
    float attenuateByT_releasePerSecond = 0.002;

public:

    void setup()
    {
        // QM-TODO: Must make Configurable, of course.
        udpSenderPort = 3413;
        udpSenderIp = IPAddress(192, 168, 178, 20); // <- my IP right now.
        // First tried Broadcast, but is way too slow:
        // udpSenderIp = ~uint32_t(Network.subnetMask()) | uint32_t(Network.gatewayIP());

        DEBUG_PRINTF("[DEADLINE_TROPHY] QM watches you! (non-creepily.) Say Hi at qm@z10.info\n");

        justBefore = millis();
        runningSec = 0;

        hasEnoughSamples = false;
        inputVoltageWasReachedOnce = false;

        attenuateFactor = 0.;
        BusManager::setMilliampsMax(static_cast<uint16_t>(maxCurrent));

        DEBUG_PRINTF("[DEADLINE_TROPHY] max %f mA -> %d\n", maxCurrent, BusManager::ablMilliampsMax());

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

        sendTrophyUdp();

        // qm210: below are the temperature control loops, once considered super-important
        // but then never tested and somehow also not required anymore (needs low enough constant Ampere limit)

        attenuateFactor = calcMaxCurrentLimiter(elapsedSec);

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

        justBefore = now;
    }

    uint8_t getAttenuated(uint8_t brightness) {
        return static_cast<uint8_t>(static_cast<float>(brightness) * attenuateFactor);
    }

    void setInputCurrentSwitch(float runningSec) {
        if (!inputVoltageWasReachedOnce) {
            return;
        }
        secondsSinceVoltageThresholdReached = runningSec - inputVoltageThresholdReachedAt;
        if (secondsSinceVoltageThresholdReached > inputCurrentSwitch_waitSeconds + inputCurrentSwitch_riseSeconds) {
            return;
        }
        attenuateFactor = constrain(
            (secondsSinceVoltageThresholdReached - inputCurrentSwitch_waitSeconds) / inputCurrentSwitch_riseSeconds,
            0,
            1.
        );
        valueForInputCurrentSwitch = static_cast<int>(255 * attenuateFactor);
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
    }

    void calcInputVoltage()
    {
        _vccRead = getAverage(val_inputVolt);
        currentInputVoltage = 2. * voltageAdcCoeff * _vccRead;
    }

    void calcLogoTherm()
    {
        _logoRead = getAverage(val_logoTherm);
        _R_TH1 = -logoR7_Ohm;
        if (_logoRead == 0.0f) {
            // unclear whether 0K is top premium choice, but it makes visible that something went wrong :P
            currentLogoTempKelvin = 0.;
            return;
        }
        _voltageRatio = currentInputVoltage / (voltageAdcCoeff * _logoRead);
        _R_TH1 += 1. / (_voltageRatio - 1) * logoR6_Ohm;
        _logRatio = logf(_R_TH1 / logoThermistor_R0_Ohm);
        currentLogoTempKelvin = 1. / ( logoTherm_OneOverT0 + logoTherm_OneOverB * _logRatio );
    }

    float calcMaxCurrentLimiter(float dt) {

        if (!hasEnoughSamples) {
            return 0;
        }
        if (!inputVoltageWasReachedOnce) {
            if (currentInputVoltage > limit_inputVoltageThreshold) {
                DEBUG_PRINTF("[DEADLINE_TROPHY] reached for the first time at %.2f\n", runningSec);
                inputVoltageWasReachedOnce = true;
                inputVoltageThresholdReachedAt = runningSec;
                limit_becauseVoltageDrop = false;
            } else {
                return 0;
            }
        }

        if (currentInputVoltage < limit_inputVoltageThreshold) {
            if (!limit_becauseVoltageDrop) {
                limit_becauseVoltageDrop = true;
                attenuateFactor *= attenuateByV_attackFactorWhenCritical;
            } else {
                attenuateFactor += attenuateByV_releasePerSecond * dt;
            }
        }

        if (currentLogoTempKelvin >= limit_criticalTempDegC + KELVIN_OFFSET) {
            if (!limit_becauseTooHot) {
                limit_becauseTooHot = true;
                attenuateFactor *= attenuateByT_attackFactorWhenCritical;
            } else {
                attenuateFactor += attenuateByT_releasePerSecond * dt;
            }
        }

        if (attenuateFactor > 1.) {
            limit_becauseVoltageDrop = false;
            limit_becauseTooHot = false;
            attenuateFactor = 1.;
        }

        return attenuateFactor;
    }

    void addToConfig(JsonObject& doc)
    {
        JsonObject top = doc.createNestedObject(F("DeadlineTrophy"));
        // fun thing, fillUMPins() will look for entries in these "pin" arrays. deal with it.
        auto pins = top.createNestedArray("pin");
        pins.add(PIN_LOGOTHERM);
        pins.add(PIN_INPUTVOLTAGE);

        auto limV = top.createNestedObject("minVoltage");
        limV[F("threshold")] = limit_inputVoltageThreshold;
        limV[F("attenuate")] = attenuateByV_attackFactorWhenCritical;
        limV[F("release")] = attenuateByV_releasePerSecond;
        auto limT = top.createNestedObject("maxTemp");
        limT[F("critical")] = limit_criticalTempDegC;
        limT[F("attenuate")] = attenuateByT_attackFactorWhenCritical,
        limT[F("release")] = attenuateByT_releasePerSecond;
    }

    void appendConfigData(Print& settingsScript)
    {
        settingsScript.print(F("addInfo('DeadlineUsermod:pin[]',0,'Logo Temperature','LogoTherm');"));
        settingsScript.print(F("addInfo('DeadlineUsermod:pin[]',1,'Common Voltage','VCC');"));

        settingsScript.print(F("addInfo('DeadlineUsermod:minVoltage:threshold',1,'required minimum V<sub>CC</sub>');"));
        settingsScript.print(F("addInfo('DeadlineUsermod:minVoltage:attenuate',1,'dim by factor when below threshold');"));
        settingsScript.print(F("addInfo('DeadlineUsermod:minVoltage:release',1,'slowly go back to 1 (perSec) if safe');"));

        settingsScript.print(F("addInfo('DeadlineUsermod:maxTemp:critical',1,'critical Temperature in °C');"));
        settingsScript.print(F("addInfo('DeadlineUsermod:maxTemp:attenuate',1,'dim by factor when above critical');"));
        settingsScript.print(F("addInfo('DeadlineUsermod:maxTemp:release',1,'slowly go back to 1 (perSec) if safe');"));

        // qm210: was hard in xml.cpp at first, but I guess this belongs rather here.
        settingsScript.print(F("/* USERMOD DEADLINE */ "));
        settingsScript.print(F("d.DEADLINE_TROPHY_MOD = true;"));
        settingsScript.print(F("d.DEADLINE_VALUES = "));
        settingsScript.print(buildControlLoopValues());
        settingsScript.print(F(";"));
        settingsScript.print(F("/* END USERMOD DEADLINE */ "));
    }

    bool readFromConfig(JsonObject& root)
    {
        JsonObject top = root[FPSTR("DeadlineTrophy")];
        if (top.isNull()) {
            DEBUG_PRINTLN(F("DeadlineUsermod: No config found. (Using defaults.)"));
            return false;
        }

        JsonObject limV = top["minVoltage"];
        if (!limV.isNull()) {
            limit_inputVoltageThreshold = limV[F("threshold")] | limit_inputVoltageThreshold;
            attenuateByV_attackFactorWhenCritical = limV[F("attenuate")] | attenuateByV_attackFactorWhenCritical;
            attenuateByV_releasePerSecond = limV[F("release")] | attenuateByV_releasePerSecond;
        }
        JsonObject limT = top["maxTemp"];
        if (!limT.isNull()) {
            limit_criticalTempDegC =limT[F("critical")] | limit_criticalTempDegC;
            attenuateByT_attackFactorWhenCritical = limT[F("attenuate")] | attenuateByT_attackFactorWhenCritical;
            attenuateByT_releasePerSecond = limT[F("release")] | attenuateByT_releasePerSecond;
        }

        return true;
    }

    uint16_t getId()
    {
        return USERMOD_ID_DEADLINE_TROPHY;
    }

    const char* buildControlLoopValues()
    {
        // is called every time the sensor values are requested by the WebSocket (i.e. often!)

        if (!hasEnoughSamples) {
            sprintf(controlLoopValues, "{\"error\": \"not enough samples taken yet.\"}");
            return controlLoopValues;
        }
        // needs about 200 characters or something, adjust DEADLINE_VALUES_STRLEN if larger
        sprintf(
            controlLoopValues,
            "{\"dl\": {\"T\": %.3f, \"minT\": %.3f, \"maxT\": %.3f, \"VCC\": %.3f, \"maxVCC\": %.3f, \"minVCC\": %.3f, \"adcT\": %.1f, \"adcV\": %.1f, \"att\": %.2f, \"aboveT\":%d, \"belowV\":%d, \"sec\": %.2f}}",
            currentLogoTempKelvin - KELVIN_OFFSET,
            minLogoTempKelvin - KELVIN_OFFSET,
            maxLogoTempKelvin - KELVIN_OFFSET,
            currentInputVoltage,
            maxInputVoltage,
            minInputVoltage,
            _logoRead,
            _vccRead,
            attenuateFactor,
            limit_becauseTooHot,
            limit_becauseVoltageDrop,
            secondsSinceVoltageThresholdReached
        );

        return controlLoopValues;
    }

};

#define GET_DEADLINE_USERMOD() ((DeadlineUsermod*)UsermodManager::lookup(USERMOD_ID_DEADLINE_TROPHY))
