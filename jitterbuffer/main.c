#include <stdio.h>
#include <stdlib.h>
#include <speex/speex_jitter.h>

// Define some constants for the test
#define FRAME_SIZE 1
#define SPEEX_JITTER_MAX_BUFFER_SIZE 200
unsigned int next_count = 0;
unsigned int current_get_count = 0;

void putBunchOfData(JitterBuffer *jitter, int n) {
    JitterBufferPacket jitter_packet2;
    int output;
    jitter_packet2.data = (char*)&output;
    jitter_packet2.len = 2;

    for (int i = 0; i < n; ++i) {
        int packet;
        packet = next_count;
        JitterBufferPacket jitter_packet;
        jitter_packet.data = (char*)&packet;
        jitter_packet.len = 2;
        jitter_packet.timestamp = next_count;
        jitter_packet.span = 1;
        jitter_packet.sequence = next_count;

        int left_count = 0;
        jitter_buffer_ctl(jitter, JITTER_BUFFER_GET_AVALIABLE_COUNT, &left_count);
        if (left_count == SPEEX_JITTER_MAX_BUFFER_SIZE) {
            jitter_buffer_get(jitter, &jitter_packet2, 1, NULL);
        }

        jitter_buffer_put(jitter, &jitter_packet);
        next_count++;
        //printf("Put packet %d into jitter buffer\n", packet);
    }
}

void getOnePacket(JitterBuffer *jitter) {
    JitterBufferPacket jitter_packet;
    int output;
    jitter_packet.data = (char*)&output;
    jitter_packet.len = 2;

    int timestamp1 = jitter_buffer_get_pointer_timestamp(jitter);
    //printf("before get timestamp is %d\n", timestamp);

    int ret = jitter_buffer_get(jitter, &jitter_packet, 1, NULL);
    if (ret == JITTER_BUFFER_OK) {
        //printf("Got packet from jitter buffer, output is %d \n", output[0]);
        current_get_count = output;
    } else {
        //printf("Failed to get packet from jitter buffer\n");
    }

    int timestamp2 = jitter_buffer_get_pointer_timestamp(jitter);
    //printf("after get before tick timestamp is %d\n", timestamp);
    jitter_buffer_tick(jitter);

    int timestamp3 = jitter_buffer_get_pointer_timestamp(jitter);
    //printf("after tick timestamp is %d\n", timestamp);

    if (timestamp2 != timestamp3) {
        printf("Timestamps are not equal, t1 %d t2 %d t3 %d, next_count %d, current_get_count %d\n",
             timestamp1, timestamp2, timestamp3, next_count, current_get_count);
    }
}

void getBunchOfData(JitterBuffer *jitter, int n) {
    for (int i = 0; i < n; ++i) {
        getOnePacket(jitter);
    }
}

void roundTrip(JitterBuffer *jitter, int n) {
    putBunchOfData(jitter, n);
    getBunchOfData(jitter, n);
}

int main() {
    // Initialize the jitter buffer
    JitterBuffer *jitter = jitter_buffer_init(20);   //this delay_step can make jump harder for larger value
    int num = 100;

    //set JITTER_BUFFER_SET_LATE_COST
    //int late_cost = 930000;
    //jitter_buffer_ctl(jitter, JITTER_BUFFER_SET_LATE_COST, &late_cost);

    printf("Put and get test with %d rounds\n", num);
    for (int i = 0; i < num; ++i) {
        putBunchOfData(jitter, 20);
        getBunchOfData(jitter, 18);
    }

    int timestamp3 = jitter_buffer_get_pointer_timestamp(jitter);
    printf("Put and get test end. t=%d, next_count %d, current_get_count %d\n", timestamp3, next_count, current_get_count);
    // Destroy the jitter buffer
    jitter_buffer_destroy(jitter);

    return 0;
}