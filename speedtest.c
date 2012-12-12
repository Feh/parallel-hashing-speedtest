#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/md5.h>

static pthread_mutex_t *ssl_locks = NULL;
static long *ssl_lock_count;

#define DATASIZE 16
#define DATANUM 200000
static char data[DATANUM][DATASIZE];

unsigned long ssl_thread_id() {
    return (unsigned long)pthread_self();
}

void ssl_locking_callback(int mode, int type, const char *file, int line) {
    if (mode & CRYPTO_LOCK) {
            pthread_mutex_lock(&ssl_locks[type]);
            ssl_lock_count[type]++;
    } else {
            pthread_mutex_unlock(&ssl_locks[type]);
    }
}

void init_openssl(void)
{
        int i;

        ssl_locks = calloc(CRYPTO_num_locks(), sizeof(pthread_mutex_t));
        ssl_lock_count = OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));

        for (i = 0; i < CRYPTO_num_locks(); i++) {
                ssl_lock_count[i] = 0;
                pthread_mutex_init(&ssl_locks[i], NULL);
        }

        CRYPTO_set_id_callback(ssl_thread_id);
        CRYPTO_set_locking_callback(ssl_locking_callback);

        SSL_load_error_strings();
        SSL_library_init();
}

/* create a large set of random 16-character-strings */
void init_data(void)
{
        int n, i;
        unsigned int seedp = 0xdeadbeef; /* make the randomness predictable */
        char alpha[] = "abcdefghijklmnopqrstuvwxyz";

        for(n = 0; n < DATANUM; n++) {
                for(i = 0; i < DATASIZE; i++)
                        data[n][i] = alpha[rand_r(&seedp) % 26];
                // printf("data[%d]: %16s\n", n, data[n]);
                // printf("%16s\n", data[n]);
        }
}

void hash_one(int num)
{
        int i;
        EVP_MD_CTX mdctx;
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int md_len;

        /* protect this with mutex */
        EVP_MD_CTX_init(&mdctx);

        if (!EVP_DigestInit_ex(&mdctx, EVP_sha256(), NULL) ||
            !EVP_DigestUpdate(&mdctx, data[num], DATASIZE) ||
            !EVP_DigestFinal_ex(&mdctx, hash, &md_len)) {
                fprintf(stderr, "ERROR");
                return;
        }

        for(i = 0; i < md_len; i++)
                printf("%02x", hash[i]);
        printf("\n");
}

void hash_all(void)
{
        int i;

        for(i = 0; i < DATANUM; i++)
                hash_one(i);
}

int main(int argc, char **argv)
{
        init_openssl();
        init_data();
        hash_all();
        return 0;
}
