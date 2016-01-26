#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "util/murmurhash.h"
#include "util/bloomfilter.h"
#include "util/numa_util.h"

static bool cb_contains_always(void* user, const uint64_t *string, size_t length)
{
	return true;
}

BloomFilter * bloom_filter_new(size_t filter_size, size_t num_hashes, int numa_node)
{
	BloomFilter	*bf;
	const size_t	bits_length = (filter_size + (CHAR_BIT * sizeof *bf->bits) - 1) / (CHAR_BIT * sizeof *bf->bits);
	const size_t	bits_size = bits_length * sizeof *bf->bits;

	if((bf = (BloomFilter*)Numa_alloc_onnode((sizeof *bf + bits_size), numa_node)) != NULL)
	{
		bf->m = filter_size;
		bf->k = num_hashes;
		bf->size = 0;
		bf->bits = (uint64_t *) (bf + 1);
		bf->bits_length = bits_length;
		bf->bits_size = bits_size;
		bf->contains =  cb_contains_always;
		bf->user = NULL;
		memset(bf->bits, 0, bits_size);

		/*printf("bits_length = %ld\n", bits_length);*/
		/*printf("bits_size = %ld\n", bits_size);*/
	}
	return bf;
}

void bloom_filter_flush(BloomFilter* bf){
	bf->size = 0;
	memset(bf->bits, 0, bf->bits_size);
}

BloomFilter * bloom_filter_new_with_probability(float prob, size_t num_elements, int numa_node)
{
	const float	m = -(num_elements * logf(prob)) / pow(log(2.f), 2.f);
	const float	k = logf(2.f) * m / num_elements;

	return bloom_filter_new((size_t) (m + .5f), (unsigned int) (k + 0.5f), numa_node);
}

void bloom_filter_destroy(BloomFilter *bf)
{
	Numa_free(bf, (sizeof *bf + bf->bits_size));
}

size_t bloom_filter_num_bits(const BloomFilter *bf)
{
	return bf->m;
}

size_t bloom_filter_num_hashes(const BloomFilter *bf)
{
	return bf->k;
}

size_t bloom_filter_size(const BloomFilter *bf)
{
	return bf->size;
}

void bloom_filter_insert(BloomFilter *bf, const uint64_t* string)
{
	/*const size_t	len = string_length > 0 ? string_length : sizeof(*string);*/
	const size_t	len =  sizeof(uint64_t);
	size_t		i;

	/* Repeatedly hash the string, and set bits in the Bloom filter's bit array. */
	for(i = 0; i < bf->k; i++)
	{
		const uint32_t	hash = murmurhash2(string, len, i);
		const size_t	pos = hash % bf->m;
		const size_t	slot = pos / (CHAR_BIT * sizeof *bf->bits);
		const size_t	bit = pos % (CHAR_BIT * sizeof *bf->bits);
#if defined BLOOM_FILTER_STANDALONE
		printf("hash(%s,%zu)=%u -> pos=%zu -> slot=%zu, bit=%zu\n", string, i, hash, pos, slot, bit);
#endif
		bf->bits[slot] |= 1UL << bit;
	}
	bf->size++;
}

bool bloom_filter_contains(const BloomFilter *bf, const uint64_t *string)
{
	/*const size_t	len = string_length > 0 ? string_length : sizeof(*string);*/
	const size_t	len =  sizeof(uint64_t);
	size_t		i;

	/* Check the Bloom filter, by hashing and checking bits. */
	for(i = 0; i < bf->k; i++)
	{
		const uint32_t	hash = murmurhash2(string, len, i);
		const size_t	pos = hash % bf->m;
		const size_t	slot = pos / (CHAR_BIT * sizeof *bf->bits);
		const size_t	bit = pos % (CHAR_BIT * sizeof *bf->bits);

		/* If a bit is not set, the element is not contained, for sure. */
		if((bf->bits[slot] & (1UL << bit)) == 0)
			return false;
	}
	/* Bit-checking says yes, call user's contains() function to make sure. */
	return bf->contains(bf->user, string, len);
}

/* -------------------------------------------------------------------------------------------------------------- */

#ifdef STAND_ALONE
int main(void)
{
	BloomFilter	*bf;

	bf = bloom_filter_new_with_probability(0.01, 100);
	/*uint32_t hash_a = murmurhash2(&a, size);*/
	/*printf("hash_a = %d\n", hash_a);*/
	/*uint32_t hash_b = murmurhash2(&b);*/
	/*printf("hash_b = %d\n", hash_b);*/
	/*uint32_t hash_c = murmurhash2(&c);*/
	/*printf("hash_c = %d\n", hash_c);*/
	/*uint32_t hash_d = murmurhash2(&d);*/
	/*printf("hash_d = %d\n", hash_d);*/
	for(int i = 0; i < 20; i++){
		bloom_filter_insert(bf, new uint64_t(i));
	}
	uint64_t a = 48;	
	printf("contains(a)=%d\n", bloom_filter_contains(bf, &a));
	/*printf("contains(b)=%d\n", bloom_filter_contains(bf, new int(12)));*/
	/*printf("contains(c)=%d\n", bloom_filter_contains(bf, new int(35)));*/
	/*printf("contains(d)=%d\n", bloom_filter_contains(bf, new int(90)));*/
	/*printf("bits_length = %d\n", bf->bits_length);*/
	bloom_filter_destroy(bf);
}
#endif

