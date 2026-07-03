#include "ucBoardDriver.h"

//Konstanten

#define     MASKE_ONOFF_SCHALTER    (1<<0)
#define     MASKE_KLEINE_BTN        (1<<0)
#define     MASKE_GROSSE_BTN        (1<<1)
#define     MASKE_REINIGUNG_BTN     (1<<6)
#define     LED_EIN                 1023
#define     TOGGLE_ZEIT             500
#define     AUFHEIZEN_ZEIT          5000
#define     BRUHEN_KLEIN_ZEIT       4500
#define     BRUHEN_GROSS_ZEIT       8500


//Hauptprogramm
int main(void)
{
    typedef enum zustand_t {AUS, EIN} zustand_t;
    
    zustand_t zustand = AUS;
    
    typedef enum programm_t {AUFHEIZEN, BEREIT, WASSERWARNUNG, BRUHEN_KLEIN, BRUHEN_GROSS, REINIGUNG} programm_t;
    
    programm_t programm = AUFHEIZEN;
    
    
    //Variablen
    
    uint8_t inSchalter = 0;
    uint16_t inWasserstand = 0;
    uint8_t Bruhvorgaenge = 0;
    uint8_t inBtnAlt = 0;
    uint8_t inBtnNeu = 0;
    uint16_t ausfullung = 0;
    uint8_t inPosFlanken = 0;
    uint8_t inOnOffSchalter = 0;
    uint8_t inKleineTasseBtn = 0;
    uint8_t inGrosseTasseBtn = 0;
    uint8_t inReinigunBtn = 0;
    uint8_t inWasserschutz = 0;
    uint16_t inSystemzeit_ms = 0;
    uint16_t outRGB_Rot = 0;
    uint16_t outRGB_Grun = 0;
    uint16_t outRGB_Blau = 0;
    uint16_t outLEDs = 0;
    uint16_t HeizenStart_ms = 0;
    uint16_t Toggle_ms = 0;
    uint16_t BruhenStart_ms = 0;
    uint16_t deltaTime = 0;
    uint16_t interval = 0;
    
    
    //Initialisieren
    initBoard(0);
    
    //Unendlichschlaufe
    while(1)
    {
        //Eingabe------------------------------------------------------------------
        inSchalter = switchReadAll();
        inWasserstand = adcRead(ADC_08_POTI_1);
        inBtnAlt = inBtnNeu;
        inBtnNeu = buttonReadAllPL();
        inPosFlanken = inBtnNeu & ~inBtnAlt;
        inOnOffSchalter = (inSchalter & MASKE_KLEINE_BTN) > 0;
        inKleineTasseBtn = (inPosFlanken & MASKE_KLEINE_BTN) > 0;
        inGrosseTasseBtn = (inPosFlanken & MASKE_GROSSE_BTN) > 0;
        inReinigunBtn = (inPosFlanken & MASKE_REINIGUNG_BTN) > 0;
        inWasserschutz = (uint32_t)inWasserstand * 100 / 1023;
        inSystemzeit_ms = getSystemTimeMs();
        
        //Verarbeitung-------------------------------------------------------------
        
        switch (zustand){
            
            case AUS:
            
            outRGB_Rot = 0;
            outRGB_Grun = 0;
            outRGB_Blau = 0;
            outLEDs = 0;
            
            if(inOnOffSchalter) {
                HeizenStart_ms = inSystemzeit_ms;
                Toggle_ms = inSystemzeit_ms;
                programm = AUFHEIZEN;
                zustand = EIN;
                outRGB_Blau = LED_EIN;
            }
            
            break;
            
            case EIN:
            
            if(!inOnOffSchalter) zustand = AUS;
            
            switch (programm){
                
                case AUFHEIZEN:
                
                outRGB_Rot = 0;
                outRGB_Grun = 0;
                outLEDs = 0;
                
                if (TOGGLE_ZEIT <= inSystemzeit_ms - Toggle_ms) {
                    outRGB_Blau = !outRGB_Blau * LED_EIN;
                    Toggle_ms = inSystemzeit_ms;
                }
                
                if (AUFHEIZEN_ZEIT <= inSystemzeit_ms - HeizenStart_ms) programm = BEREIT;
                
                break;
                
                case BEREIT:
                
                if(inWasserschutz < 20) programm = WASSERWARNUNG;
                
                if (Bruhvorgaenge > 5) programm = REINIGUNG;
                
                outRGB_Rot = 0;
                outRGB_Grun = LED_EIN;
                outRGB_Blau = 0;
                outLEDs = 0;
                
                if(inKleineTasseBtn) {
                    BruhenStart_ms = inSystemzeit_ms;
                    Toggle_ms = inSystemzeit_ms;
                    outRGB_Grun = 0;
                    ausfullung = 1;
                    programm = BRUHEN_KLEIN;
                }
                
                if(inGrosseTasseBtn) {
                    BruhenStart_ms = inSystemzeit_ms;
                    Toggle_ms = inSystemzeit_ms;
                    outRGB_Grun = 0;
                    ausfullung = 1;
                    programm = BRUHEN_GROSS;
                }
                
                break;
                
                case WASSERWARNUNG:
                
                outRGB_Rot = LED_EIN;
                outRGB_Grun = 0;
                outRGB_Blau = 0;
                outLEDs = 0;
                
                if(inWasserschutz >= 20) programm = BEREIT;
                
                break;
                
                case BRUHEN_KLEIN:
                
                outRGB_Rot = 0;
                outRGB_Blau = 0;
                
                deltaTime = inSystemzeit_ms - BruhenStart_ms;

                interval = deltaTime / TOGGLE_ZEIT;

                if (deltaTime <= BRUHEN_KLEIN_ZEIT) {
                    if (interval % 2 == 0) {
                        outRGB_Grun += 2;
                        } else {
                        outRGB_Grun -= 2;
                    }
                }
                
                
                if (TOGGLE_ZEIT <= inSystemzeit_ms - Toggle_ms) {
                    
                    outLEDs = outLEDs + ausfullung;
                    ausfullung = ausfullung * 2;
                    Toggle_ms = inSystemzeit_ms;
                }
                
                if (BRUHEN_KLEIN_ZEIT <= inSystemzeit_ms - BruhenStart_ms) {
                    
                    Bruhvorgaenge = Bruhvorgaenge + 1;
                    
                    programm = BEREIT;
                }
                
                break;
                
                case BRUHEN_GROSS:
                
                outRGB_Rot = 0;
                outRGB_Blau = 0;
                
                deltaTime = inSystemzeit_ms - BruhenStart_ms;

                interval = deltaTime / TOGGLE_ZEIT;

                if (deltaTime <= BRUHEN_GROSS_ZEIT) {
                    if (interval % 2 == 0) {
                        outRGB_Grun += 2;
                        } else {
                        outRGB_Grun -= 2;
                    }
                }

                
                if (TOGGLE_ZEIT <= inSystemzeit_ms - Toggle_ms) {
                    
                    outLEDs = outLEDs + ausfullung;
                    ausfullung = ausfullung * 2;
                    Toggle_ms = inSystemzeit_ms;
                }
                
                if (BRUHEN_GROSS_ZEIT <= inSystemzeit_ms - BruhenStart_ms) {
                    
                    Bruhvorgaenge = Bruhvorgaenge + 1;
                    
                    programm = BEREIT;
                }
                
                
                break;
                
                case REINIGUNG:
                
                outRGB_Rot = LED_EIN;
                outRGB_Grun = LED_EIN;
                outRGB_Blau = 0;
                
                outLEDs = 0;
                
                if(inReinigunBtn) {
                    Bruhvorgaenge = 0;
                    programm = BEREIT;
                }
                
                break;
            }
            
            break;
            
        }
        //Ausgabe------------------------------------------------------------------
        
        rgbWrite(outRGB_Rot, outRGB_Grun, outRGB_Blau);
        ledWriteAll(outLEDs);
        lcdWriteText(1, 5, "%4u/100%", inWasserschutz);
    }
}