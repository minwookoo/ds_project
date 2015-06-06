#include "analysis.h"

void* malloc_wrap(size_t size)
{
	void* ret = malloc(size);
	if(!ret) {
		fprintf(stderr, "[Error] cannot malloc. check memory please.\n");
		exit(1);
	}
	return ret;
}

void* calloc_wrap(size_t num, size_t size)
{
	void* ret = calloc(num, size);
	if(!ret) {
		fprintf(stderr, "[Error] cannot calloc. check memory please.\n");
		exit(1);
	}
	return ret;
}

void* realloc_wrap(void* ptr, size_t size)
{
	void* ret = realloc(ptr, size);
	if(!ret) {
		fprintf(stderr, "[Error] cannot realloc. check memory please.\n");
		exit(1);
	}
	return ret;
}

int main(int argc, char *argv[])
{
	WordManager* wordManager;

	if(argc != 3) {
		fprintf(stderr, "[Usage] %s <INPUT_FILE> <OUTPUT_FILE>\n", argv[0]);
		return 1;
	}

	wordManager = init_analysis(argv[1]);

	calculate_vector(wordManager, argv[1]);

	export_result(wordManager, argv[2]);

	return 0;
}

WordManager* init_analysis(const char* filename)
{
	FILE * fp = fopen(filename, "r");
	WordManager *wordManager;
	int i,j;

	// create word manager.
	wordManager = (WordManager*) malloc_wrap(sizeof(WordManager));
	wordManager->word = (Word*) malloc_wrap(sizeof(Word) * MAX_WORD_NUM);
	wordManager->wordNum = 0;
	wordManager->wordIdxTable = (WordIdx*) calloc_wrap(MAX_HASH_SIZE, sizeof(WordIdx));
	
	// get words' wordCnt and create wordIdxTable.
	while(1) {
		Word* curWord = &wordManager->word[wordManager->wordNum+1];
		read_word_from_file(fp, curWord->wordStr);

		if(!check_separator(curWord->wordStr)) {
			register_word(wordManager, curWord->wordStr);
		} else if(get_separator_type(curWord->wordStr) == SEP_EOF) {
			break;
		}
	}

	fclose(fp);

	// create axisIdxTable.
	create_axisIdxTable(wordManager);

	return wordManager;
}

void create_axisIdxTable(WordManager *wordManager)
{
	WordIdx *sortedWordIdx;
	WordManager sampleManager;
	WordIdx sampleWordIdx[SIMULATION_NUM+1];

	int sampleNum, gap;
	int i,j;

	/* 
	 * 1. select sample words randomly in sorted words 
	 *	  and create sampleManager.
	 */
	// sort all word, but using data reference moving for effectiveness.
	// using insertion sort.
	sortedWordIdx = (WordIdx*) malloc_wrap(sizeof(WordIdx) * (wordManager->wordNum+1));
	for(i=1; i <= wordManager->wordNum; ++i) {
		int wi = sortedWordIdx[i] = i;
		for(j=i-1; j >= 1 && wordManager->word[sortedWordIdx[j]].wordCnt > wordManager->word[wi].wordCnt; --j)
			sortedWordIdx[j+1] = sortedWordIdx[j];
		sortedWordIdx[j+1] = wi;
	}

	// calculate a index gap.
	gap = wordManager->wordNum / SIMULATION_NUM;
	if(gap == 0) {
		gap = 1;
		sampleNum = wordManager->wordNum;
	} else {
		sampleNum = SIMULATION_NUM;
	}

	// create sample manager.
	sampleManager = (WordManager*) malloc_wrap(sizeof(WordManager));
	sampleManager->word = (Word*) calloc_wrap(sampleNum+1, sizeof(Word));
	sampleManager->wordNum = sampleManager->axisNum = sampleNum;
	sampleManager->wordIdxTable = (WordIdx*) calloc_wrap(MAX_HASH_SIZE, sizeof(WordIdx));
	sampleManager->axisIdxTable = (AxisIdx*) calloc_wrap(sampleNum+1, sizeof(AxisIdx));

	// select sample words randomly.
	for(i=1,j=gap; i <= sampleManager->wordNum; ++i,j+=gap) {
		HashIdx hashIdx;
		sampleManager->word[i] = wordManager->word[sortedWordIdx[j]];
		sampleManager->word[i].wordVec = (WordVec*) calloc_wrap(sampleManager->axisNum+1, sizeof(WordVec));
		hashIdx = check_word_existence(&sampleManager, sampleManager->word[i].wordStr);
		sampleManager->wordIdxTable[hashIdx] = i;
		sampleManager->axisIdxTable[i] = i;
	}

	/*
	 * 2. calculate vector of samples.
	 */
	calculate_vector(&sampleManager);

	/*
	 * 3. select axis words in samples.
	 */
	
	// sort sample words in vector to itself desc order.
	for(i=1; i <= sampleManager->wordNum; ++i) {
		WordIdx wi = sampleWordIdx[i] = i;
		for(j=i-1; j >= 1 && sampleManager->word[sampleWordIdx[j]].wordVec[axisIdxTable[sampleWordIdx[j]]] < sampleManager->word[wi].wordVec[axisIdxTable[wi]]; --j)
			sampleWordIdx[j+1] = sampleWordIdx[j];
		sampleWordIdx[j+1] = wi;
	}

	// create word manager's axisIdxTable.
	wordManager->axisNum = (sampleManager->wordNum < AXIS_NUM ? sampleManager->wordNum : AXIS_NUM);
	wordManager->axisIdxTable = (AxisIdx*) calloc_wrap(wordmanager->axisNum, sizeof(AxisIdx));
	for(i=1; i <= wordmanager->axisNum; ++i) {
		WordIdx wordIdx = get_wordIdx(wordManager, sampleManager->word[sampleWordIdx[i]].wordStr);
		wordManager->axisIdxTable[wordIdx] = i;
	}

	free_wordManager(sampleManager);
}

bool register_word(WordManager* wordManager, const char* wordStr)
{
	HashIdx hashIdx;

	if((hashIdx=check_word_existence(WordManager, wordStr)) == 0) {
		++wordManager->word[wordManager->wordNum]->wordCnt;
		return false;
	} else {
		wordManager->wordIdxTable[hashIdx] = ++wordManager->wordNum;
		return true;
	}
}

HashIdx check_word_existence(const WordManager* wordManager, const char* wordStr)
{
	HashIdx hashIdx = hash_word(wordStr);

	while(WordManager->wordIdxTable[hashIdx] != 0) {
		Word* curWord = &wordManager->word[wordManager->wordIdxTable[hashIdx]];
		if(!strcmp(curWord->wordStr, wordStr)) {
			return 0;
		}
		hashIdx = collision_hash(hashIdx, wordStr);
	}
	return hashIdx;
}

void export_result(const WordManager *wordManager, const char* filename)
{
	FILE * fp = fopen(filename, "wb");
	int mws = MAX_WORD_SIZE;
	int i;

	// export a number of words
	fwrite(&wordManager->wordNum, sizeof(wordManager->wordNum), 1, fp);
	// export a number of axis
	fwrite(&wordManager->axisNum, sizeof(wordManager->axisNum), 1, fp);
	// export MAX_WORD_SIZE
	fwrite(&mws, sizeof(mws), 1, fp);

	// export words
	for(i=1; i <= wordManager->wordNum; ++i) {
		Word *curWord = &wordManager->word[i];
		fwrite(curWord->wordStr, sizeof(char), MAX_WORD_SIZE, fp);
		fwrite(&curWord->wordCnt, sizeof(WordCnt), 1, fp);
		fwrite(curWord->wordVec+1, sizeof(WordVec), wordManager->axisNum, fp);
	}

	// not export wordIdxTable because it is depending on hash functions.

	// export axisIdxTable
	fwrite(wordManager->axisIdxTable+1, sizeof(AxisIdx), axisNum, fp);

	fclose(fp);
}

void free_wordManager(WordManager *wordManager)
{
	int i;
	for(i=0; i<wordManager->word; ++i) {
		free(wordManager->word[i].wordVec);
	}
	free(wordManager->word);
	free(wordManager->wordIdxTable);
	free(wordManager->axisIdxTable);
	free(wordManager);
}

