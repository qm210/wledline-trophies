#pragma once

class DeadlineTrophyUsermod : public Usermod
{
public:

    static const int PIN_LOGOTHERM = 35;
    static const int PIN_OTHERTHERM = 33;
    static const int PIN_VCAP = 32;

    // qm210: these are hacked into the AudioReactive UserMod because we are so loco
    static const int PIN_AR_SD = 12;
    static const int PIN_AR_WS = 13;
    static const int PIN_AR_CLK = 14;

private:

    // analog readings
    uint16_t current_logoTherm = 0;
    uint16_t current_otherTherm = 0;
    uint16_t current_vCap = 0;

    uint16_t max_logoTherm = 0;
    uint16_t max_otherTherm = 0;
    uint16_t max_vCap = 0;

    float limitFunc_logoThermSlope = 0.0;
    float limitFunc_logoThermOffset = 0.0;
    float limitFunc_logoThermCritical = 0.0;
    float limitFunc_otherThermSlope = 0.0;
    float limitFunc_otherThermOffset = 0.0;
    float limitFunc_otherThermCritical = 0.0;

    float vCapThreshold = 4.6;
    float vInputThreshold = 4.6;

    int _counter = 0;

    long lastDebugOutAt = 0;

public:

    void setup()
    {
        // TODO @btr fun might go here
        DEBUG_PRINTLN("[DEADLINE] DeadlineTrophyUsermod::setup() says hello!");

        lastDebugOutAt = millis();
    }

    void loop()
    {
        // TODO @btr moar fun might go here

        current_logoTherm = analogRead(PIN_LOGOTHERM);
        current_otherTherm = analogRead(PIN_OTHERTHERM);
        current_vCap = analogRead(PIN_VCAP);
        // - button.cpp handleAnalog() does some smoothing, maybe that's nice?

        if (current_logoTherm > max_logoTherm) {
            max_logoTherm = current_logoTherm;
        }
        if (current_otherTherm > max_otherTherm) {
            max_otherTherm = current_otherTherm;
        }
        if (current_vCap > max_vCap) {
            max_vCap = current_vCap;
        }

        auto now = millis();
        if (now - lastDebugOutAt > 10000)
        {
            DEBUG_PRINTF("[DEADLINE_TROPHY] logoTherm=%d (max %d)\n", current_logoTherm, max_logoTherm);
            DEBUG_PRINTF("[DEADLINE_TROPHY] otherTherm=%d (max %d)\n", current_otherTherm, max_otherTherm);
            DEBUG_PRINTF("[DEADLINE_TROPHY] vCap=%d (max %d)\n", current_vCap, max_vCap);
            lastDebugOutAt = now;
        }


        // cf. button.cpp: handleAnalog()
        // - would yield() be useful? (or is this thing fast anyhow)
        yield();

        // just some testing whether we see ANY change
        // --> i.e. update the LED Settings page by F5 and watch it change :)
        // auto testSine = 400. + 50.0 * sin_t(0.01 * static_cast<float>(_counter));
        // strip.ablMilliampsMax = static_cast<uint16_t>(testSine);

        // qm210: let's try to update the UI per web socket
        // updateInterfaces(CALL_MODE_WS_SEND);

        _counter++;
    }

    void addToConfig(JsonObject& doc)
    {
        JsonObject top = doc.createNestedObject(F("DeadlineTrophy"));
        // fun thing, fillUMPins() will look for entries in these "pin" arrays. deal with it.
        auto pins = top.createNestedArray("pin");
        pins.add(PIN_LOGOTHERM);
        pins.add(PIN_OTHERTHERM);
        pins.add(PIN_VCAP);

        auto lim = top.createNestedObject("lim");
        auto lt_lim = lim.createNestedObject("lt");
        lt_lim["m"] = limitFunc_logoThermSlope;
        lt_lim["b"] = limitFunc_logoThermOffset;
        lt_lim["crit"] = limitFunc_logoThermCritical;
        auto ot_lim = lim.createNestedObject("ot");
        ot_lim["m"] = limitFunc_otherThermSlope;
        ot_lim["b"] = limitFunc_otherThermOffset;
        ot_lim["crit"] = limitFunc_otherThermCritical;
    }

    void appendConfigData()
    {
      oappend(SET_F("addInfo('DeadlineTrophy:pin[]',0,'limit current','LogoTherm');"));
      oappend(SET_F("addInfo('DeadlineTrophy:pin[]',1,'?','OtherTherm');"));
      oappend(SET_F("addInfo('DeadlineTrophy:pin[]',2,'for inrush current','VCap');"));

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

    uint16_t logoTherm() { return current_logoTherm; }
    uint16_t otherTherm() { return current_otherTherm; }
    uint16_t vCap() { return current_vCap; }
};
