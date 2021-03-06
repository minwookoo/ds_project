#include "analysis.c"

#define FILE_NAME "input.txt"

/*
 * Function Name	: init_analysis
 * Parameter		: wordManager
 * Return value		: void
 * Role				: 
 *		1. get words' wordCnt.
 *			- count the number of the words
 *		2. create wordIdxTable.
 *		3. create axisIdxTable.
 *			- select proper axis words
 * Using function	:
 *		1. 
 * Description		:
 *		none
 */
void init_analysis(WordManager* wordManager)
{
	FILE * fp = fopen(FILE_NAME, "r");
	int i,j;

	//wordManager->word[0] <- junk
	wordManager->wordNum = 0;

	// get words' wordCnt and create wordIdxTable.
	while(1) {
		Word* curWord = wordManager->word[wordManager->wordNum];
		read_word_from_file(fp, curWord->word);

		if(!check_separator(curWord->word)) {
			register_word(wordManager, curWord->word);
		} else if(get_separator_type(curWord->word) == SEP_EOF) {
			break;
		}
	}

	fclose(fp);

	// create axisIdxTable.
	WordManager sampleManager;

	// select samples 
	if(wordManager->wordNum <= SAMPLE_NUM) {
		sampleManager->word = wordManager->word;
		sampleManager->wordNum = wordManager->wordNum;
		sampleManager->wordIdxTable = wordIdxTable;
		sampleManager->axisIdxTable = NULL;
	} else {
		sampleManager->word = (Word*) calloc(SAMPLE_NUM+1, sizeof(Word));
		sampleManager->wordNum = 0;
		sampleManager->wordIdxTable = (WordIdx*) calloc(MAX_HASH_SIZE, sizeof(WordIdx));
		sampleManager->axisIdxTable = NULL;
		//sampleManager->axisIdxTable = (AxisIdx*) calloc(SAMPLE_NUM+1, sizeof(AxisIdx));

		for(i=1; sampleManager->wordNum<=SAMPLE_NUM; ++i) {
			int wordIdx;
			if(wordIdx = wordManager->wordIdxTable[i]) {
				HashIdx hashIdx = check_word_existence(sampleManager, sampleManager->word[wordIdx].wordStr);
				sampleManager->wordIdxTable[hashIdx] = i;
				sampleManager->word[i] = wordManager->word[wordIdx];
			}
		}

		// insertion sort ascending order
		for(i=1; i<=sampleManager->wordNum; ++i) {
			Word word = sampleManager->word[i];
			for(j=i-1; j>=1 && test.word[j].wordCnt > word.wordCnt; --j)
				test.word[j+1] = test.word[j];
			test.word[j+1] = word;
		}
	}

	WordManager simulManager;

	if(sampleManager->wordNum <= SIMULATION_NUM) {
		simulManager->word = sampleManager->word;
		simulManager->wordNum = simulManager->wordNum;
		simulManager->wordIdx = wordIdxTable;
		simulManager->axisIdxTable = (AxisIdx*) calloc(simulManager->wordNum+1, sizeof(AxisIdx));
		for(i=1; i<=simulManager->wordNum; ++i) {
			simulManager->axisIdxTable[i] = i;
		}
	} else {
		simulManager->word = (Word*) calloc(SIMULATION_NUM+1, sizeof(Word));
		simulManager->wordNum = 0;
		simulManager->wordIdxTable = (WordIdx*) calloc(MAX_HASH_SIZE, sizeof(WordIdx));
		simulManager->axisIdxTable = (AxisIdx*) calloc(SIMULATION_NUM+1, sizeof(AxisIdx));
		for(i=1,j=SAMPLE_NUM*FILTER_RATE; i<=SIMULATION_NUM; ++i,--j) {
			simulManager->word[i] = sampleManager->word[j];
			HashIdx hashIdx = check_word_existence(simulManager, simulManager->word[i].wordStr);
			simulManager->wordIdxTable[hashIdx] = i;
			simulManager->axisIdxTable[i] = i;
		}
	}
	
	calculate_vector(simulManager);
	
	double rate[SIMULATION_NUM+1];
	WordIdx simulWordIdx[SIMULATION_NUM+1];
	for(i=1; i<=simulManager->wordNum; ++i) {
		rate[i] = 0;
		simulWordIdx[i] = i;
		for(j=1; j<=simulManager->wordNum; ++j) {
			rate[i] += simulManager->word[i].wordVec[j];
		}
		rate[i] = simulManager->word[i].wordVec[i] / rate[i];
	}

	// sort rate desc order. and get high 200rank i.
	// high 200 rank is axis.
	for(i=1; i<=simulManager->wordNum; ++i) {
		double r = rate[i];
		WordIdx wi = simulWordIdx[i];
		for(j=i-1; j>=1 && rate[j] < r; --j) {
			rate[j+1] = rate[j];
			simulWordIdx[j+1] = simulWordIdx[j];
		}
		rate[j+1] = r;
		simulWordIdx[j+1] = wi;
	}

	int axisNum = (simulManager->wordNum < AXIS_NUM ? simulManager->wordNum : AXIS_NUM);
	for(i=1; i<=axisNum; ++i) {
		WordIdx wordIdx = get_wordIdx(wordManager, simulManager->word[wordIdx[i]].wordStr);
		wordManager->axisIdxTable[i] = wordIdx;
	}

	free();

/*
	//select 1000
	Word *newWord = (Word*) calloc(SIMULATION_NUM+1, sizeof(Word));
	memset(sampleManager->wordIdxTable, 0, sizeof(WordIdx) * MAX_HASH_SIZE);
	for(i=1,j=SAMPLE_NUM*FILTER_RATE; i<=SIMULATION_NUM; ++i,--j) {
		newWord[i] = sampleManager->word[j];
		HashIdx hashIdx = check_word_existence(sampleManager, wordManager->word[wordIdx].wordStr);
		test.wordIdxTable[hashIdx] = i;
		test.axisIdxTable[i] = i;
	}
	test.wordNum = 1000+1;
*/



/*

	if(wordManager->wordNum <= 3000) {
		WordManager test = {wordManager->word, wordManager->wordNum, wordManager->wordIdxTable, 0};
		test.axisIdxTable = (AxisIdx*) calloc(test.wordNum, sizeof(AxisIdx));
		for(i=1; i<test.wordNum; ++i) {
			test.axisIdxTable[i] = i;
		}
	} else {
		WordManager test;
		test.word = (Word*) calloc(3000+1, sizeof(Word));
		test.wordNum = 1;//test.wordNum = 3000+1;
		test.wordIdxTable = (WordIdx*) calloc(MAX_HASH_SIZE, sizeof(WordIdx));
		test.axisIdxTable = (AxisIdx*) calloc(3000+1, sizeof(AxisIdx));

		//HashIdx hash[3000+1];
		for(i=0; test.wordNum < 3000+1; ++i) {
			int wordIdx;
			if(wordIdx = wordManager->wordIdxTable[i]) {
				//test.axisIdxTable[test.wordNum] = test.wordNum;
				HashIdx hashIdx = check_word_existence(&test, wordManager->word[wordIdx].wordStr);
				//hash[test.wordNum] = hashIdx;
				test.wordIdxTable[hashIdx] = test.wordNum;
				test.word[test.wordNum] = wordManager->word[wordIdx];
				++test.wordNum;
			}
		}

		// sort
		for(i=1; i<3000+1; ++i) {
			Word word = test.word[i];
			for(j=i-1; j>=1 && test.word[j].wordCnt > word.wordCnt; --j)
				test.word[j+1] = test.word[j];
			test.word[j+1] = word;
		}
*/

	}

//	calculate_vector(test);



	
	//get_axisList(wordManager->word+1, wordManager->wordNum-1, wordManager->axisIdxTable+1);
	//WordManager tmp = {wordManager->word+1, wordManager->wordNum-1, wordManager->axisIdxTable+1};
	//get_axisIdxTable(&tmp);
}
/*
//void get_axisList(const Word *words, int wordNum, AxisIdx *axisCheck)
void get_axisIdxTable(WordManager* wordManager)
{
	int i;

	// recursive exit condition
	if(wordManager->wordNum <= AXIS_NUM) {
		for(i=0; i<wordManager->wordNum; ++i) {
			wordManager->axisIdxTable = 1;
		}
		*axisNum = wordNum;
		return;
	}

	static const int SPLIT_NUM = 10;
	int offset = wordNum / SPLIT_NUM;

	// divide problem
	for(i=0; i<SPLIT_NUM-1; ++i) {
		get_axisList(words+i*offset, offset, axisCheck+i*offset, axisNum);
	}
	get_axisList(words+i*offset, wordNum - (offset*(SPLIT_NUM-1)), axisCheck+i*offset, axisNum);

	// merge result
	
}
*/
/*
 * Function Name	: register_word
 * Parameter		: wordManager, a pointer of the word string
 * Return value		: true if success, false if fail
 * Role				: 
 *		1. get hashIdx of the word using hash_word.
 *			- hash(word) = hashIdx
 *		2. check word's existence using wordIdxTable.
 *		3. if word exists, increase word's wordCnt and return false.
 *		4. else, insert word to wordManager, increase wordNum, register new wordIdx to wordIdxTable and return true.
 * Using function	:
 *		1. 
 * Description		:
 *		none
 */
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

/*
 * Function Name	: check_word_existence
 * Parameter		: wordManager, a pointer of the word string
 * Return value		: 0 if exist, hashIdx which will inserted if not exist
 * Role				: 
 *		1. get hashIdx of the word using hash_word.
 *			- hash(word) = hashIdx
 *		2. check word's existence using wordIdxTable.
 * Using function	:
 *		1. hash_word
 * Description		:
 *		none
 */
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
