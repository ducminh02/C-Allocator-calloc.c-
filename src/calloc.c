#include "../include/calloc.h"
// Für memset
#include <string.h>

static void * MEM;
static size_t MEM_SIZE;

/* Die aktuelle Position für das Next Fit Verfahren */
static mem_block * last_allocation = NULL;

void my_calloc_init(void * mem, size_t mem_size){
	MEM = mem;
	MEM_SIZE = mem_size;

	/* Initial existiert genau ein mem_block direkt am Anfang des Speichers */
	mem_block * beginning = MEM;

	beginning->next = NULL;
	beginning->prev = NULL;

	/* Der verwaltete Speicher des initialen mem_blocks ist die des
	 * gesamten Speichers minus die Größe seiner eigenen Verwaltungsstruktur
	 * Da sowohl MEM_SIZE ein Vielfaches von 8 ist und sizeof(mem_block) auch
	 * mindestens ein vielfaches von 8 oder mehr ist, ist das LSB
	 * auch direkt 0 -> free.
	 */
	beginning->size = MEM_SIZE - sizeof(mem_block);

	/* Unser Zeiger für das Next Fit Verfahren */
	last_allocation = beginning;
}

/* +------------------------------------+ *
 * | Hier ist Platz für Hilfsfunktionen | *
 * +------------------------------------+ */
size_t roundUp(size_t numToRound, int multiple)
{
    if (multiple == 0)
        return numToRound;

    int remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;

    return numToRound + multiple - remainder;

}


/* -------------------------------------- */

void * my_calloc(size_t nmemb, size_t size, int c) {

    size_t sum = roundUp(nmemb * size, 8);
    mem_block *runner = last_allocation;

    if (nmemb * size == 0) {
        return last_allocation + 1;
    }

    while (runner != NULL) {

        // case runner is belegt, goto next
        if (runner->size % 2 == 1) {
            runner = runner->next;
            continue;
        }

        // case when sum = size or to small to make new block
        if (runner->size >= sum && runner->size < sum + 8 + sizeof(mem_block)) {
            memset(runner + 1, c, sum);
            runner->size++;
            last_allocation = runner;
            return last_allocation + 1;
        }
        
        // case when size big enough for new block
        if (runner->size >= sum + 8 + sizeof(mem_block)) {
            // create new
            mem_block *new = (void *) runner + sizeof(mem_block) + sum;

            // change size
            new->size = runner->size - sizeof(mem_block) - sum;
            runner->size = sum + 1;

            // set pointer
            new->next = runner->next;
            runner->next = new;
            new->prev = runner;
            
            if (new->next != NULL) {
                new->next->prev = new;
            }
            memset(runner + 1, c, sum);
            last_allocation = runner;
            return last_allocation + 1;
        }
        
        // size to small for sum
        runner = runner->next;
    }

    runner = MEM;
    while (runner != last_allocation) {
        // case runner is belegt, goto next
        if (runner->size % 2 == 1) {
            runner = runner->next;
            continue;
        }

        // case when sum = size
        if (runner->size == sum) {
            memset(runner + 1, c, sum);
            runner->size++;
            last_allocation = runner;
            return last_allocation + 1;
        }

        // case when size to small to make new block
        if (runner->size < sum + 8 + sizeof(mem_block)) {
            memset(runner + 1, c, sum);
            runner->size++;
            last_allocation = runner;
            return last_allocation + 1;
        }
        
        // case when size big enough for new block
        if (runner->size >= sum + 8 + sizeof(mem_block)) {
            // create new
            mem_block *new = (void *) runner + sizeof(mem_block) + sum;

            // change size
            new->size = runner->size - sizeof(mem_block) - sum;
            runner->size = sum + 1;

            // set pointer
            new->next = runner->next;
            runner->next = new;
            new->prev = runner;
            
            if (new->next != NULL) {
                new->next->prev = new;
            }
            memset(runner + 1, c, sum);
            last_allocation = runner;
            return last_allocation + 1;
        }
        
        // size to small for sum
        runner = runner->next;
    }
    return  NULL;
}

void my_free(void *ptr){
	// TODO

    if (!ptr) {
        return;
    }

    mem_block * lincoln = (mem_block*)ptr - 1;
    mem_block * b4 = lincoln->prev;
    mem_block * after = lincoln->next;

    // free lincoln
    lincoln->size = lincoln->size &~1 ;

    // case when block after is free
    if (after != NULL && after->size % 2 == 0 ) {
        lincoln->size = lincoln->size + after->size + sizeof(mem_block);
        lincoln->size = lincoln->size &~1 ;

        lincoln->next = after->next;
        if (after->next != NULL) {
            mem_block* tempo = after->next;
            tempo->prev = lincoln;
        }
        if (after == last_allocation) {
            last_allocation = lincoln;
        }
    }
    // case when block b4 is free
    if (b4 != NULL && b4->size % 2 == 0 ){
        b4->size = b4->size + lincoln->size + sizeof(mem_block);
        b4->size = b4->size &~1 ;
        b4->next = lincoln->next;
        if (lincoln->next!=NULL){
            lincoln->next->prev=b4;
        }
        if (lincoln == last_allocation) {
            last_allocation = b4;
        }
        return;
    }
}


