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
#define DATANUM 2500000
static char data[DATANUM][DATASIZE];

struct fromto { int from, to; };

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
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int md_len;

#ifdef LOCK_STATIC_EVP_MD_CTX
        static EVP_MD_CTX mdctx;
        static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
        static unsigned char first = 1;

        pthread_mutex_lock(&lock);
        if (first) {
                EVP_MD_CTX_init(&mdctx);
                first = 0;
        }
#else
        EVP_MD_CTX mdctx;
        EVP_MD_CTX_init(&mdctx);
#endif

        if (!EVP_DigestInit_ex(&mdctx, EVP_sha256(), NULL) ||
            !EVP_DigestUpdate(&mdctx, data[num], DATASIZE) ||
            !EVP_DigestFinal_ex(&mdctx, hash, &md_len)) {
                fprintf(stderr, "ERROR");
                return;
        }

#ifdef LOCK_STATIC_EVP_MD_CTX
        pthread_mutex_unlock(&lock);
#endif

        return;

        for(i = 0; i < md_len; i++)
                printf("%02x", hash[i]);
        printf("\n");
}

void *hash_slice(void *arg)
{
        int n, from, to;
        struct fromto *ft = (struct fromto *) arg;

        from = ft->from;
        to = ft->to;

        for(n = from; n < to; n++)
                hash_one(n);

        return NULL;
}

void hash_all(int num)
{
        int i;
        pthread_t *t;
        struct fromto *ft;
        clock_t start, end;

        start = clock();

        t = malloc(num * sizeof *t);
        for(i = 0; i < num; i++) {
                ft = malloc(sizeof(struct fromto));
                ft->from = i * (DATANUM/num);
                ft->to = ((i+1) * (DATANUM/num)) > DATANUM ?
                        DATANUM : (i+1) * (DATANUM/num);
                pthread_create(&t[i], NULL, hash_slice, ft);
        }

        for(i = 0; i < num; i++)
                pthread_join(t[i], NULL);

        end = clock();

        printf("%d threads: %ld hashes/s, total = %.3fs\n",
                num, DATANUM * CLOCKS_PER_SEC / (end-start),
                (end-start)/(double)CLOCKS_PER_SEC);
        free(t);
        sleep(1);
}

int main(int argc, char **argv)
{
        init_openssl();
        init_data();

        hash_all(1);
        hash_all(2);
        hash_all(4);
        hash_all(8);
        hash_all(12);
        hash_all(24);

        return 0;
}
