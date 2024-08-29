#include <SoftwareSerial.h>

#define OK "OK"
#define AT "AT"

String gsm_response = "";
String phone_number = "+91XXXXXXXXXX";

SoftwareSerial gsm(2, 3);

unsigned long current_time = 0;

void setup()
{
    Serial.begin(9600);

    Serial.println(F("Please wait for 20 seconds.."));
    Serial.println(F("GSM Initializing.."));

    /* Initializing GSM...
       Don't worry about baudrate, below function will handle everything
       Just wait for 20 seconds atleast to Initialize GSM */
    initializeGSM();

    // ==========================================================================
    // Below code just checks whether GSM is now in working condition or not
    Serial.println(F("Waiting for GSM to response"));
    while (!getResponse(AT, OK, 5000))
    {
        // if GSM is not working.
        delay(200);
    }
    Serial.println(F("GSM is ready now!"));
    // ==========================================================================

    /* Below function will test the GSM Module with all the required AT commands
       and there responses in order to check our GSM Module is fully functional
       or not. */
    if (connectGSM())
    {
        Serial.println(F("GSM Connected!"));
    }

    // gsm.println("ATD" + phone_number);
    sendSMS(phone_number, "Hello from EC200U GSM!");
    readSMS();
}

void loop()
{
}

bool getResponse(String cmd, String response, int timeout)
{
    Serial.println(cmd);
    current_time = millis();
    gsm_response = "";
    int index = -1;
    int network_status = 0;

    gsm.println(cmd);
    while (true)
    {
        if (gsm.available())
        {
            gsm_response = gsm.readString();
            gsm_response.trim();
            Serial.println(gsm_response);
            index = gsm_response.indexOf(response);
            if (cmd == "AT+CREG?")
            {
                network_status = gsm_response.substring(gsm_response.indexOf(',') + 1, gsm_response.indexOf(',') + 2).toInt();
                if (network_status == 1 || network_status == 5)
                {
                    Serial.println(F("GSM Network registered!"));
                    return true;
                }
                else
                {
                    Serial.println(F("ERROR: Failed to register network."));
                    Serial.println(F("       Press RESET."));
                    return false;
                }
            }
            else
            {
                if (index >= 0)
                {
                    return true;
                }
            }
        }
        if (millis() - current_time >= timeout)
        {
            Serial.println(F("ERROR: GSM response timeout."));
            Serial.println(F("       Press RESET."));
            return false;
        }
    }
}

bool readSMS(void)
{
    while (!getResponse(F("AT+CNMI=2,2,0,0,0"), OK, 1000))
        ;
    gsm_response = "";
    while (true)
    {
        if (gsm.available())
        {
            gsm_response = gsm.readString();
            gsm_response.trim();
            Serial.println(gsm_response);
        }
    }
}

void initializeGSM(void)
{
    gsm.begin(115200);
    delay(10000);
    gsm.println(F("AT+IPR=9600"));
    delay(10000);
    gsm.end();
    gsm.begin(9600);
}

bool connectGSM(void)
{
    while (!getResponse(F("ATE0"), OK, 1000))
        ;
    while (!getResponse(F("AT+CLIP=1"), OK, 1000))
        ;
    while (!getResponse(F("AT+CVHU=0"), OK, 1000))
        ;
    while (!getResponse(F("AT+CTZU=1"), OK, 1000))
        ;
    while (!getResponse(F("AT+CMGF=1"), OK, 1000))
        ;
    while (!getResponse(F("AT+CSQ"), OK, 1000))
        ;
    while (!getResponse(F("AT+CREG?"), OK, 1000))
        ;
    while (!getResponse(F("AT+CGREG?"), OK, 1000))
        ;
    while (!getResponse(F("AT+CMGD=1,4"), OK, 1000))
        ;

    // Configure URC port to UART1
    if (getResponse(F("AT+QURCCFG=\"urcport\",\"uart1\""), OK, 1000)) 
    {
        Serial.println(F("URC port configured to UART1"));
    }
    else 
    {
        Serial.println(F("Failed to configure URC port"));
    }

    return true;
}

void sendSMS(String number, String sms)
{
    while (!getResponse(F("AT+CMGF=1"), OK, 1000))
        ;
    while (!getResponse("AT+CMGS=\"" + number + "\"\r", ">", 1000))
        ;
    gsm.print(sms);
    delay(1000);
    gsm.println((char)26);
}