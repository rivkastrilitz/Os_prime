/**
 *      task 3 in Operating System course
 *       =================================
 * 
 *  Code description:
 *  ----------------
 *   this code get as 2 numbers as input 
 *     argv[1] := seed number (for uniformity of output)
 *     argv[2] := number of random numbers to generate and check for primality
 * 
 *    in this code we create number of threads according the number of processors configured by the operating system.
 *    each of the threads iterate over a diffrent chunk of the randoms array and check the numbers for primality. 
 *    the threads count the number of primary numbers and summarize them.
 * 
 * 
 * 
 *    # the primality test is preformed by the Miller-Rabin algorithm.
 * 
 *    # we decided to get around the need for synchronization by given each thread 
 *      a diffrent cell on the array to work on.
 * 
 * 
 * AUTHOR: kfir ettinger And rivka 'shlumit' strilitz
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <stdint.h>

int numbersNum, rndSeed, threadsSize;
long sumArray[8]={0}, primeCtrArray[8]={0};
long* randomsArray;

/* 
 * calculates (a * b) % c taking into account that a * b might overflow 
 */
long long mulmod(long long a, long long b, long long mod)
{
    long long x = 0,y = a % mod;
    while (b > 0)
    {
        if (b % 2 == 1)
        {    
            x = (x + y) % mod;
        }
        y = (y * 2) % mod;
        b /= 2;
    }
    return x % mod;
}

/* 
 * modular exponentiation
 */
long long modulo(long long base, long long exponent, long long mod)
{
    long long x = 1, y = base;
    while (exponent > 0){
        if (exponent % 2 == 1)
            x = (x * y) % mod;
        y = (y * y) % mod;
        exponent = exponent / 2;
    }
    return x % mod;
}
 
/*
 * Miller-Rabin Primality test
 */
void Miller_Rabin(long long num, int threadId)
{
    //check numbers for primality
    int i, iteration = 5;
    long long s;
    if (num < 2)
        return;
    
    if (num != 2 && num % 2==0)
        return;
    
    s = num - 1;
    while (s % 2 == 0)
        s /= 2;

    for (i = 0; i < iteration; i++){
        long long a = rand() % (num - 1) + 1, temp = s;
        long long mod = modulo(a, temp, num);
        while (temp != num - 1 && mod != 1 && mod != num - 1){
            mod = mulmod(mod, mod, num);
            temp *= 2;
        }
        if (mod != num - 1 && temp % 2 == 0)
            return;
        
    }

    // update arrays at 'threadId' index
    primeCtrArray[threadId]++;
    sumArray[threadId] += num;
    return;
}

/**
 *  work division between threads
 */
void* generateAndCheck(void* thread_Id)
{
    int threadId = (intptr_t)thread_Id;
    int from, to, chunkSize;

    //for each thread => set chunk of randoms array he will iterate over (by threadId)
    chunkSize = (numbersNum+threadsSize -1)/threadsSize;
    from = threadId*chunkSize;
    
    if(from + chunkSize > numbersNum)
        to = numbersNum;
    else
        to = from + chunkSize;
    
    //run Miller–Rabin primality test for each random number
    for(int i = from; i < to; i++){
        Miller_Rabin(*(randomsArray + i), threadId);
    }
    return NULL;
}


int main(int argc, char *argv[])
{
    //check input legality and proccess it
    if(argc != 3) {
        printf("Too few arguments ");
        printf("USAGE: ./solution <prime pivot> <num of random numbers>");
        exit(0);
    }
    
    rndSeed = atoi(argv[1]);
    numbersNum = atoi(argv[2]);


    // threadsSize <- the number of processors configured by the operating system.
     threadsSize = get_nprocs_conf();

    // initialize randomsArray with 'numbersNum' numbers
    srand(rndSeed);
    randomsArray = (long*) malloc(sizeof(long)*numbersNum);
    for(int i = 0; i<numbersNum; i++){
        randomsArray[i] = rand();
    }
    
    // array of threads
    pthread_t threads[threadsSize];

    // create and run threads
    for(int i=0; i<threadsSize; i++){
        if(pthread_create(threads + i, NULL, &generateAndCheck, (void *)(intptr_t)i) != 0){
            perror("Failed to create thread.");
        }
    }

    //Joining the thread in a different loop to make parallelism.
    for(int i=0; i<threadsSize; i++){
        if(pthread_join(threads[i], NULL) != 0){
            perror("Failed to join thread.");
        }
    }

    // sum the data gatherd by threads
    long sum = 0, primeCtr = 0;
    for(int i=0;i<threadsSize;i++){
        sum+=sumArray[i];
        primeCtr+=primeCtrArray[i];
    }

    // print output in colors just so it will be a bit more joyful
    printf("\033[0;31m%ld\033[0;32m,\033[0;33m%ld\033[0;34m",sum, primeCtr);

    return 0;
}