#include "ppg_dsp.h"

#include <math.h>
#include <string.h>


#define PPG_BUF_LEN          500
#define SAMPLE_RATE_HZ       100.0f

#define MIN_HR_BPM           40.0f
#define MAX_HR_BPM           180.0f


#define MIN_PEAK_DISTANCE_SAMPLES 60
#define PEAK_THRESHOLD       0.25f


static uint32_t redBuffer[PPG_BUF_LEN];
static uint32_t irBuffer[PPG_BUF_LEN];

static uint16_t bufferIndex = 0;

static PPG_Result_t result = {0};

static float hrFiltered = 0;



static float mean(
        float *data,
        uint16_t len)
{
    float sum = 0;

    for(uint16_t i = 0; i < len; i++)
    {
        sum += data[i];
    }

    return sum / len;
}



static float rmsAC(
        float *data,
        uint16_t len,
        float dc)
{
    float sum = 0;


    for(uint16_t i = 0; i < len; i++)
    {
        float ac = data[i] - dc;

        sum += ac * ac;
    }


    return sqrtf(sum / len);
}



static void smooth(
        uint32_t *input,
        float *output,
        uint16_t len)
{

    output[0] = input[0];


    for(uint16_t i = 1; i < len-1; i++)
    {
        output[i] =
            (
                input[i-1] +
                input[i] +
                input[i+1]
            ) / 3.0f;
    }


    output[len-1] = input[len-1];
}




static float calculateHR(
        float *ir,
        uint16_t len,
        float dc,
        float ac)
{

    uint16_t peaks[20];

    uint16_t count = 0;


    /*
     * Adaptive threshold
     */
    float threshold =
            ac * PEAK_THRESHOLD;



    for(uint16_t i = 1; i < len-1; i++)
    {

        float prev =
                ir[i-1] - dc;


        float curr =
                ir[i] - dc;


        float next =
                ir[i+1] - dc;



        if(
            curr > prev &&
            curr > next &&
            curr > threshold
        )
        {

            if(
                count == 0 ||
                (i - peaks[count-1])
                >= MIN_PEAK_DISTANCE_SAMPLES
            )
            {

                if(count < 20)
                {
                    peaks[count++] = i;
                }

            }

        }

    }



    if(count < 2)
    {
        return 0;
    }



    float gap = 0;


    for(uint16_t i = 1; i < count; i++)
    {
        gap += peaks[i] - peaks[i-1];
    }


    gap /= (count-1);



    float seconds =
            gap / SAMPLE_RATE_HZ;



    float bpm =
            60.0f / seconds;



    if(
        bpm < MIN_HR_BPM ||
        bpm > MAX_HR_BPM
    )
    {
        return 0;
    }



    return bpm;
}




static void processPPG()
{

    float red[PPG_BUF_LEN];

    float ir[PPG_BUF_LEN];



    smooth(
        redBuffer,
        red,
        PPG_BUF_LEN
    );


    smooth(
        irBuffer,
        ir,
        PPG_BUF_LEN
    );



    float redDC =
            mean(red,PPG_BUF_LEN);


    float irDC =
            mean(ir,PPG_BUF_LEN);



    float redAC =
            rmsAC(
                red,
                PPG_BUF_LEN,
                redDC
            );


    float irAC =
            rmsAC(
                ir,
                PPG_BUF_LEN,
                irDC
            );



    float hr =
            calculateHR(
                ir,
                PPG_BUF_LEN,
                irDC,
                irAC
            );



    /*
     * Heart rate smoothing
     */
    if(hr > 0)
    {

        if(hrFiltered == 0)
        {
            hrFiltered = hr;
        }
        else
        {
            hrFiltered =
                0.2f * hr +
                0.8f * hrFiltered;
        }


        result.heart_rate =
                (uint16_t)hrFiltered;


        result.valid = 1;

    }
    else
    {
        result.valid = 0;
    }




    /*
     * SpO2 estimation
     */

    if(
        redDC > 0 &&
        irDC > 0 &&
        redAC > 0 &&
        irAC > 0
    )
    {

        float R =
            (redAC / redDC)
            /
            (irAC / irDC);



        float spo2 =
            104.0f -
            17.0f * R;



        if(spo2 > 100)
            spo2 = 100;


        if(spo2 < 0)
            spo2 = 0;



        result.spo2 =
                (uint8_t)spo2;

    }
    else
    {
        result.spo2 = 0;
    }



    result.signal_quality = 80;

}

void PPG_SetFingerPresent(bool present)
{
    if (!present)
    {
        result.heart_rate = 0;
        result.spo2 = 0;
        result.valid = 0;
        hrFiltered = 0;
    }
}




void PPG_Init(void)
{

    memset(
        redBuffer,
        0,
        sizeof(redBuffer)
    );


    memset(
        irBuffer,
        0,
        sizeof(irBuffer)
    );


    bufferIndex = 0;


    hrFiltered = 0;


    memset(
        &result,
        0,
        sizeof(result)
    );

}





void PPG_PushSample(
        uint32_t red,
        uint32_t ir)
{

    redBuffer[bufferIndex] = red;

    irBuffer[bufferIndex] = ir;



    bufferIndex++;



    if(bufferIndex >= PPG_BUF_LEN)
    {

        processPPG();


        bufferIndex = 0;

    }

}




PPG_Result_t PPG_GetResult(void)
{
    return result;
}
