#include "ppg_algo.h"
#include <string.h>
#include <math.h>

#define PPG_BUF_LEN          70     // 7s window at 10Hz — faster response than 10s
#define SAMPLE_RATE_HZ       10.0f  // must match main.c's SAMPLE_PERIOD_MS (100ms)
#define MIN_HR_BPM           40.0f
#define MAX_HR_BPM           180.0f
#define PEAK_MIN_FRACTION    0.15f  // peak must exceed this fraction of AC RMS to count
#define MAX_PEAKS            30     // generous upper bound for peaks per window

#define HR_EMA_ALPHA_NORMAL  0.3f   // normal smoothing for small fluctuations
#define HR_EMA_ALPHA_FAST    0.8f   // fast-track weight for large, real jumps
#define HR_CHANGE_THRESHOLD  15.0f  // BPM delta that triggers fast-track response

#define SPO2_EMA_ALPHA       0.3f

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

// Simple 3-sample moving average — smooths high-frequency noise/motion
// artifact before AC/DC calculations.
static void PPG_Smooth(uint32_t *in, float *out, uint16_t len)
{
    out[0] = (float)in[0];
    out[len-1] = (float)in[len-1];
    for (uint16_t i = 1; i < len - 1; i++) {
        out[i] = ((float)in[i-1] + (float)in[i] + (float)in[i+1]) / 3.0f;
    }
}

static float PPG_MeanFloat(float *buf, uint16_t len)
{
    float sum = 0.0f;
    for (uint16_t i = 0; i < len; i++) sum += buf[i];
    return sum / (float)len;
}

static float PPG_AcRmsFloat(float *buf, uint16_t len, float dc)
{
    float sum_sq = 0.0f;
    for (uint16_t i = 0; i < len; i++) {
        float ac = buf[i] - dc;
        sum_sq += ac * ac;
    }
    return sqrtf(sum_sq / (float)len);
}

// Amplitude-thresholded peak detection + inter-peak-interval timing.
static float PPG_EstimateHR(float *ir_smooth, uint16_t len, float dc, float ac_rms)
{
    float threshold = ac_rms * PEAK_MIN_FRACTION;

    int32_t peak_indices[MAX_PEAKS];
    uint16_t peak_count = 0;

    float prev = ir_smooth[0] - dc;
    for (uint16_t i = 1; i < len - 1; i++) {
        float curr = ir_smooth[i]   - dc;
        float next = ir_smooth[i+1] - dc;

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
    static float red_smooth[PPG_BUF_LEN];
    static float ir_smooth[PPG_BUF_LEN];
    PPG_Smooth(red, red_smooth, len);
    PPG_Smooth(ir, ir_smooth, len);

    float red_dc = PPG_MeanFloat(red_smooth, len);
    float ir_dc  = PPG_MeanFloat(ir_smooth, len);

    float red_ac_rms = PPG_AcRmsFloat(red_smooth, len, red_dc);
    float ir_ac_rms  = PPG_AcRmsFloat(ir_smooth, len, ir_dc);

    *hr_out = PPG_EstimateHR(ir_smooth, len, ir_dc, ir_ac_rms);

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
            // HR: adaptive smoothing — fast-track large real jumps,
            // damp small fluctuations as before
            if (hr_ema == 0.0f) {
                hr_ema = hr;
            } else {
                float delta = fabsf(hr - hr_ema);
                float alpha = (delta > HR_CHANGE_THRESHOLD) ? HR_EMA_ALPHA_FAST : HR_EMA_ALPHA_NORMAL;
                hr_ema = alpha * hr + (1.0f - alpha) * hr_ema;
            }

            if (spo2_ema == 0.0f) spo2_ema = spo2;
            else                  spo2_ema = SPO2_EMA_ALPHA * spo2 + (1.0f - SPO2_EMA_ALPHA) * spo2_ema;

            last_result.heart_rate     = (uint16_t)hr_ema;
            last_result.spo2           = (uint8_t)spo2_ema;
            last_result.signal_quality = 80;
            last_result.valid          = 1;
        }
        else
        {
            last_result.heart_rate     = 0;
            last_result.spo2           = 0;
            last_result.signal_quality = 0;
            last_result.valid          = 0;

            hr_ema   = 0.0f;
            spo2_ema = 0.0f;
        }

        buf_idx = 0;
    }
}

PPG_Result_t PPG_GetLastResult(void)
{
    return last_result;
}
