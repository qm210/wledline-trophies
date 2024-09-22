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
    uint16_t logoThermValue = 0;
    uint16_t otherThermValue = 0;
    uint16_t vCapValue = 0;

    float limitFunc_logoThermSlope = 0.0;
    float limitFunc_logoThermOffset = 0.0;
    float limitFunc_otherThermSlope = 0.0;
    float limitFunc_otherThermOffset = 0.0;

    float vCapThreshold = 4.6;
    float vInputThreshold = 4.6;

public:

    void setup()
    {
        // TODO @btr fun might go here
        DEBUG_PRINTLN("[DEADLINE] DeadlineTrophyUsermod::setup() says hello!");
    }

    void loop()
    {
        // TODO @btr moar fun might go here

        logoThermValue = analogRead(PIN_LOGOTHERM);
        otherThermValue = analogRead(PIN_OTHERTHERM);
        vCapValue = analogRead(PIN_VCAP);

        // cf. button.cpp: handleAnalog()
        // - would yield() be useful? (or is this thing fast anyhow)
        // - button.cpp handleAnalog() does some smoothing, maybe that's nice for us, too

        // just some testing whether we see ANY change
        // --> i.e. update the LED Settings page by F5 and watch it change :)
        strip.ablMilliampsMax = random(0, 800);

        // qm210: let's try to update the UI per web socket
        // updateInterfaces(CALL_MODE_WS_SEND);
    }

    void addToConfig(JsonObject& doc)
    {
        JsonObject top = doc.createNestedObject(F("DeadlineTrophy"));
        // fun thing, fillUMPins() will look for entries in these "pin" arrays. deal with it.
        auto pins = top.createNestedArray("pin");
        pins.add(PIN_LOGOTHERM);
        pins.add(PIN_OTHERTHERM);
        pins.add(PIN_VCAP);
    }

    void appendConfigData()
    {
      oappend(SET_F("addInfo('DeadlineTrophy:pin[]',0,'limit current','LogoTherm');"));
      oappend(SET_F("addInfo('DeadlineTrophy:pin[]',1,'?','OtherTherm');"));
      oappend(SET_F("addInfo('DeadlineTrophy:pin[]',2,'for inrush current','VCap');"));
    }

    // bool readFromConfig(JsonObject& doc)
    // {
    //     // TODO @qm210 / @btr: find out, what is the return value used for??
    //     return true;
    // }

    uint16_t getId()
    {
        return USERMOD_ID_DEADLINE_TROPHY;
    }

};
