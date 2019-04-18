/*
 * Copyright (c) 2016 Daniel Pirch
 *
 * Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file in the root of the source
 * tree. An additional intellectual property rights grant can be found
 * in the file PATENTS.  All contributing project authors may
 * be found in the AUTHORS file in the root of the source tree.
 */

#define _POSIX_C_SOURCE 200809L

#include <fvad.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sndfile.h>
#include <assert.h>

/*****************************************************************************/

// wget -P examples/ http://www.voiptroubleshooter.com/open_speech/american/OSR_us_000_0061_8k.wav
// make && rm -f examples/linkline_*.wav && examples/fvadwav -m 1 examples/OSR_us_000_0061_8k.wav

/*****************************************************************************/

typedef int16_t sample_t;
typedef bool boolean;

enum { samplerate = 8000, frame_ms = 10, framelen = samplerate / 1000 * frame_ms };

enum { RING_MAX = samplerate / 1000 * 10000 /* 10 second */ };

typedef struct ring_buffer_element_t ring_buffer_element_t;

struct ring_buffer_element_t {
    sample_t sample;
};

static struct {
    ring_buffer_element_t items[RING_MAX];
    size_t length, tail;
    bool initialized;
} ring_buffer;

/*****************************************************************************/

static void ring_buffer_push_back(const sample_t sample)
{
    if (ring_buffer.initialized)
    {
        if (++ring_buffer.length >= RING_MAX)
        {
            ring_buffer.length--;
        }

        if (++ring_buffer.tail >= RING_MAX)
        {
            ring_buffer.tail = 0;
        }
    }

    ring_buffer.initialized = true;
    ring_buffer.items[ring_buffer.tail].sample = sample;
}

/*****************************************************************************/

static void vad_collector(const sample_t sample, const boolean vadres)
{
  static boolean triggered;
  static size_t num_voiced;
  static int start = -1;

  enum { FILTER_VOICED=2000, FILTER_UNVOICED=100 };

  ring_buffer_push_back(sample);

  if (ring_buffer.tail == start)
  {
    // not enougth buffer !
    start = -1;
  }

  if (vadres)
  {
    if (num_voiced < FILTER_VOICED)
    {
      num_voiced++;
    }
  }
  else
  {
    if (num_voiced)
    {
      num_voiced--;
    }
  }

  if (triggered)
  {
    if (num_voiced < FILTER_UNVOICED)
    {
        triggered = false;
        fprintf(stderr, "tada !!!\n");
        if (start == -1)
        {
            fprintf(stderr, "skipped large sound\n");
            return;
        }
        // post

        SNDFILE *out_sf;

        SF_INFO in_info = (SF_INFO){
            .samplerate = samplerate,
            .channels = 1,
            .format = SF_FORMAT_WAV | SF_FORMAT_PCM_16
        };

        char out_filename[100];
        static int out_filename_i;
        sprintf(out_filename, "examples/linkline_%u.wav", out_filename_i++);

        out_sf = sf_open(out_filename, SFM_WRITE, &in_info);
        if (!out_sf) {
            fprintf(stderr, "Cannot open output file '%s': %s\n", out_filename, sf_strerror(NULL));
            return;
        }

        // TODO: look backward
        // for (int i = 0; i < FILTER_VOICED; i++)

        ring_buffer_element_t* curr = &ring_buffer.items[start];
        while (curr != &ring_buffer.items[ring_buffer.tail])
        {
            sample_t x[1];
            x[0] = curr->sample;
            //fprintf(stderr, "%d\n", x[0]);
            sf_write_short(out_sf, &x[0], 1);

            if (++start >= RING_MAX)
            {
                start = 0;
            }
            curr = &ring_buffer.items[start];
        }

        sf_close(out_sf);
    }
  }
  else
  {
    if (num_voiced >= FILTER_VOICED)
    {
        triggered = true;
        start = ring_buffer.tail;
    }
  }
}

/*****************************************************************************/

static bool process_sf(SNDFILE *infile, Fvad *vad,
    size_t framelen, SNDFILE *outfiles[2], FILE *listfile)
{
    bool success = false;
    double *buf0 = NULL;
    int16_t *buf1 = NULL;
    int vadres, prev = -1;
    long frames[2] = {0, 0};
    long segments[2] = {0, 0};

    if (framelen > SIZE_MAX / sizeof (double)
            || !(buf0 = malloc(framelen * sizeof *buf0))
            || !(buf1 = malloc(framelen * sizeof *buf1))) {
        fprintf(stderr, "failed to allocate buffers\n");
        goto end;
    }

    while (sf_read_double(infile, buf0, framelen) == framelen) {

        // Convert the read samples to int16
        for (size_t i = 0; i < framelen; i++)
            buf1[i] = buf0[i] * INT16_MAX;

        vadres = fvad_process(vad, buf1, framelen);
        if (vadres < 0) {
            fprintf(stderr, "VAD processing failed\n");
            goto end;
        }

        if (listfile) {
            fprintf(listfile, "%d\n", vadres);
        }

        vadres = !!vadres; // make sure it is 0 or 1

        if (outfiles[vadres]) {
            sf_write_double(outfiles[!!vadres], buf0, framelen);
        }

        frames[vadres]++;
        if (prev != vadres) segments[vadres]++;
        prev = vadres;

        for (size_t i = 0; i < framelen; i++)
            vad_collector(buf1[i], vadres);
    }

    printf("voice detected in %ld of %ld frames (%.2f%%)\n",
        frames[1], frames[0] + frames[1],
        frames[0] + frames[1] ?
            100.0 * ((double)frames[1] / (frames[0] + frames[1])) : 0.0);
    printf("%ld voice segments, average length %.2f frames\n",
        segments[1], segments[1] ? (double)frames[1] / segments[1] : 0.0);
    printf("%ld non-voice segments, average length %.2f frames\n",
        segments[0], segments[0] ? (double)frames[0] / segments[0] : 0.0);

    success = true;

end:
    if (buf0) free(buf0);
    if (buf1) free(buf1);
    return success;
}




static bool parse_int(int *dest, const char *s, int min, int max)
{
    char *endp;
    long val;

    errno = 0;
    val = strtol(s, &endp, 10);
    if (!errno && !*endp && val >= min && val <= max) {
        *dest = val;
        return true;
    } else {
        return false;
    }
}


int main(int argc, char *argv[])
{
    int retval;
    const char *in_fname, *out_fname[2] = {NULL, NULL}, *list_fname = NULL;
    SNDFILE *in_sf = NULL, *out_sf[2] = {NULL, NULL};
    SF_INFO in_info = {0}, out_info[2];
    FILE *list_file = NULL;
    int mode, frame_ms = 10;
    Fvad *vad = NULL;

    /*
     * create fvad instance
     */
    vad = fvad_new();
    if (!vad) {
        fprintf(stderr, "out of memory\n");
        goto fail;
    }

    /*
     * parse arguments
     */
    for (int ch; (ch = getopt(argc, argv, "m:f:o:n:l:h")) != -1;) {
        switch (ch) {
        case 'm':
            if (!parse_int(&mode, optarg, 0, 3) || fvad_set_mode(vad, mode) < 0) {
                fprintf(stderr, "invalid mode '%s'\n", optarg);
                goto argfail;
            }
            break;
        case 'f':
            if (!parse_int(&frame_ms, optarg, 10, 30) || frame_ms % 10 != 0) {
                fprintf(stderr, "invalid frame length '%s'\n", optarg);
                goto argfail;
            }
            break;
        case 'o':
            out_fname[1] = optarg;
            break;
        case 'n':
            out_fname[0] = optarg;
            break;
        case 'l':
            list_fname = optarg;
            break;
        case 'h':
            printf(
                "Usage: %s [OPTION]... FILE\n"
                "Reads FILE in wav format and performs voice activity detection (VAD).\n"
                "Options:\n"
                "  -m MODE      set VAD operating mode (aggressiveness) (0-3, default 0)\n"
                "  -f DURATION  set frame length in ms (10, 20, 30; default 10)\n"
                "  -o FILE      write detected voice frames to FILE in wav format\n"
                "  -n FILE      write detected non-voice frames to FILE in wav format\n"
                "  -l FILE      write list of per-frame detection results to FILE\n"
                "  -h           display this help and exit\n",
                argv[0]);
            goto success;

        default: goto argfail;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "input file expected\n");
        goto argfail;
    }

    in_fname = argv[optind++];

    if (optind < argc) {
        fprintf(stderr, "unexpected argument '%s'; only one input file expected\n", argv[optind]);
        goto argfail;
    }

    /*
     * open and check input file
     */
    in_sf = sf_open(in_fname, SFM_READ, &in_info);
    if (!in_sf) {
        fprintf(stderr, "Cannot open input file '%s': %s\n", in_fname, sf_strerror(NULL));
        goto fail;
    }

    if (in_info.channels != 1) {
        fprintf(stderr, "only single-channel wav files supported; input file has %d channels\n", in_info.channels);
        goto fail;
    }

    if (fvad_set_sample_rate(vad, in_info.samplerate) < 0) {
        fprintf(stderr, "invalid sample rate: %d Hz\n", in_info.samplerate);
        goto fail;
    }

    /*
     * open required output files
     */
    for (int i = 0; i < 2; i++) {
        if (out_fname[i]) {
            out_info[i] = (SF_INFO){
                .samplerate = in_info.samplerate,
                .channels = 1,
                .format = SF_FORMAT_WAV | SF_FORMAT_PCM_16
            };
            out_sf[i] = sf_open(out_fname[i], SFM_WRITE, &out_info[i]);
            if (!out_sf[i]) {
                fprintf(stderr, "Cannot open output file '%s': %s\n", out_fname[i], sf_strerror(NULL));
                goto fail;
            }
        }
    }

    if (list_fname) {
        list_file = fopen(list_fname, "w");
        if (!list_file) {
            fprintf(stderr, "Cannot open output file '%s': %s\n", list_fname, strerror(errno));
            goto fail;
        }
    }

    /*
     * run main loop
     */
    if (!process_sf(in_sf, vad,
            (size_t)in_info.samplerate / 1000 * frame_ms, out_sf, list_file))
        goto fail;

    /*
     * cleanup
     */
success:
    retval = EXIT_SUCCESS;
    goto end;

argfail:
    fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
fail:
    retval = EXIT_FAILURE;
    goto end;

end:
    if (in_sf) sf_close(in_sf);
    for (int i = 0; i < 2; i++)
        if (out_sf[i]) sf_close(out_sf[i]);
    if (list_file) fclose(list_file);
    if (vad) fvad_free(vad);

    return retval;
}
