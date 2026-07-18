#include "ppg_algo.h"
#include <string.h>
#include <math.h>

#define PPG_BUF_LEN       100     // 10s window at 10Hz effective sample rate
#define SAMPLE_RATE_HZ    10.0f   // must match main.c's SAMPLE_PERIOD_MS (100ms)
#define MIN_HR_BPM        40.0f
#define MAX_HR_BPM        180.0f
#define PEAK_MIN_FRACTION 0.15f   // peak must exceed this fraction of AC RMS to count
#define MAX_PEAKS         30      // generous upper bound for peaks per window
#define HR_EMA_ALPHA      0.3f    // smoothing factor: lower = smoother/slower to react
#define SPO2_EMA_ALPHA    0.3f

static uint32_t red_buf[PPG_BUF_LEN];
static uint32_t ir_buf[PPG_BUF_LEN];
static uint16_t buf_idx = 0;

static PPG_Result_t last_result = {0};

static float hr_ema   = 0.0f;
static float spo2_ema = 0.0f;

static float PPG_MeanU32(uint32_t *buf, uint16_t len)
{
    uint64_t sum = 0;
    for (uint16_t i = 0; i < len; i++) sum += buf[i];
    return (float)sum / (float)len;
}

static float PPG_AcRms(uint32_t *buf, uint16_t len, float dc)
{
    float sum_sq = 0.0f;
    for (uint16_t i = 0; i < len; i++) {
        float ac = (float)buf[i] - dc;
        sum_sq += ac * ac;
    }
    return sqrtf(sum_sq / (float)len);
}

// Amplitude-thresholded peak detection + inter-peak-interval timing.
static float PPG_EstimateHR(uint32_t *ir, uint16_t len, float dc, float ac_rms)
{
    float threshold = ac_rms * PEAK_MIN_FRACTION;

    int32_t peak_indices[MAX_PEAKS];
    uint16_t peak_count = 0;

    float prev = (float)ir[0] - dc;
    for (uint16_t i = 1; i < len - 1; i++) {
        float curr = (float)ir[i]   - dc;
        float next = (float)ir[i+1] - dc;

        if (curr > prev && curr > next && curr > threshold) {
            if (peak_count < MAX_PEAKS) {
                peak_indices[peak_count] = i;
                peak_count++;
            }
        }
        prev = curr;
    }

    if (peak_count < 2) return 0.0f;

    float total_gap = 0.0f;
    for (uint16_t i = 1; i < peak_count; i++) {
        total_gap += (float)(peak_indices[i] - peak_indices[i-1]);
    }
    float avg_gap_samples = total_gap / (float)(peak_count - 1);
    float avg_gap_seconds = avg_gap_samples / SAMPLE_RATE_HZ;

    float hr_raw = 60.0f / avg_gap_seconds;

    if (hr_raw < MIN_HR_BPM || hr_raw > MAX_HR_BPM) return 0.0f;

    return hr_raw;
}

static void PPG_Compute(uint32_t *red, uint32_t *ir, uint16_t len,
                         float *hr_out, float *spo2_out)
{
    float red_dc = PPG_MeanU32(red, len);
    float ir_dc  = PPG_MeanU32(ir, len);

    float red_ac_rms = PPG_AcRms(red, len, red_dc);
    float ir_ac_rms  = PPG_AcRms(ir, len, ir_dc);

    *hr_out = PPG_EstimateHR(ir, len, ir_dc, ir_ac_rms);

    if (red_dc <= 0.0f || ir_dc <= 0.0f || ir_ac_rms <= 0.0f) {
        *spo2_out = 0.0f;
        return;
    }

    float R = (red_ac_rms / red_dc) / (ir_ac_rms / ir_dc);
    float spo2 = 104.0f - 17.0f * R;

    if (spo2 > 100.0f) spo2 = 100.0f;
    if (spo2 < 0.0f)   spo2 = 0.0f;

    *spo2_out = spo2;
}

void PPG_Init(void)
{
    buf_idx = 0;
    hr_ema = 0.0f;
    spo2_ema = 0.0f;
    memset(&last_result, 0, sizeof(last_result));
}

void PPG_PushSample(uint32_t red_val, uint32_t ir_val)
{
    red_buf[buf_idx] = red_val;
    ir_buf[buf_idx]  = ir_val;
    buf_idx++;

    if (buf_idx >= PPG_BUF_LEN) {
        float hr, spo2;
        PPG_Compute(red_buf, ir_buf, PPG_BUF_LEN, &hr, &spo2);

        if (hr > 0.0f && spo2 > 0.0f)
        {
            // Smooth across windows so one noisy window doesn't jump wildly
            if (hr_ema == 0.0f)   hr_ema   = hr;
            else                  hr_ema   = HR_EMA_ALPHA   * hr   + (1.0f - HR_EMA_ALPHA)   * hr_ema;

            if (spo2_ema == 0.0f) spo2_ema = spo2;
            else                  spo2_ema = SPO2_EMA_ALPHA * spo2 + (1.0f - SPO2_EMA_ALPHA) * spo2_ema;

            last_result.heart_rate     = (uint16_t)hr_ema;
            last_result.spo2           = (uint8_t)spo2_ema;
            last_result.signal_quality = 80;
            last_result.valid          = 1;
        }
        else
        {
            // This window was rejected (no peaks / out of range) — don't
            // update the smoothed value, just mark quality as low.
            last_result.signal_quality = 0;
            last_result.valid          = (hr_ema > 0.0f) ? 1 : 0;  // keep last good value visible if we have one
        }

        buf_idx = 0;  // reset for next fresh, non-overlapping window
    }
}

PPG_Result_t PPG_GetLastResult(void)
{
    return last_result;
}
