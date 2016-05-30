#include <iostream>

#define MAX_DIST 2
#define NUM_MIN_MATCHES 4
#define NBITS 72
#define KNN_RATIO 0.6
#define VERBOSE false
#define TRAIN 0
#define GENERATE_KP_DES false
#define NUM_SCENES 51
#define NUM_OBJECTS 8
#define FIRST_OBJECT 2
#define LAST_OBJECT 8
#define NUM_EXEC 1
#define SEARCH_ALL_KP false

extern const std::string DIR_IMGS_TEST;
extern const std::string DIR_OBJECTS;
extern const std::string ROOT_DIR;
extern const std::string DIR_DES_OBJECTS;
extern const std::string DIR_TXT_RESULTS;
extern const std::string DIR_KP_OBJECTS;
extern const std::string DIR_FILTERS;
extern const std::string DIR_IMG_MATCHES;

extern std::string objs[NUM_OBJECTS];
extern float percKpSal[NUM_OBJECTS];
extern const bool GT[NUM_OBJECTS][NUM_SCENES];

extern const int expectedNumKp;
extern const float desiredFPProb;
